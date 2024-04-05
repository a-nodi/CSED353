#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _retransmission_timer(RetransmissionTimer(retx_timeout)) {}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {
    TCPSegment segment;
    string payload = "";

    size_t payload_size;
    size_t sendable_bytes;
    size_t outstanding_bytes;

    if (!have_sent_syn) {  // No SYN packet sent yet
        // TCPSegment syn_segment;
        segment = create_segment(next_seqno(), payload, true, false);  // Create segment (SYN, no payload)
        send_segment(segment, _next_seqno);                            // Send segment
        have_sent_syn = true;                                          // Send SYN only once

        return;
    }

    sendable_bytes = static_cast<size_t>(max(_receiver_window_size, uint16_t(1)));  // Calculate sendable bytes
    outstanding_bytes = _next_seqno - _last_ackno;                                  // Outstanding bytes

    sendable_bytes -= outstanding_bytes;  // Subtract outstanding bytes (i.e sent but haven't acked. if ignore this
                                          // bytes, overflow may occur)

    // Receiver window not full, not fin, and stream not empty while stream is not eof
    while (sendable_bytes > 0 && !have_sent_fin && !(!_stream.eof() && _stream.buffer_empty())) {
        // Create Segment
        payload_size =
            min(_stream.buffer_size(), min(TCPConfig::MAX_PAYLOAD_SIZE, sendable_bytes));  // Clip payload size
        payload = _stream.read(payload_size);  // Fill out payload from stream

        sendable_bytes -= payload_size;  // Update sendable bytes

        // Add FIN flag if stream is empty
        if (_stream.eof() && segment.length_in_sequence_space() < sendable_bytes) {
            if (sendable_bytes > 0)
                have_sent_fin = true;  // After FIN, no more segment will be sent
            sendable_bytes--;          // FIN flag occupies space in window
        }
        segment = create_segment(
            next_seqno(), payload, false, have_sent_fin);  // Create segment (FIN if FIN flag set, payload)

        send_segment(segment, _next_seqno);  // Send segment
    }

    return;
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    uint64_t abs_ackno = unwrap(ackno, _isn, _next_seqno);
    _receiver_window_size = window_size;

    // Variables for tracking acked segments
    uint64_t segment_seqno;
    uint64_t segment_payload_length;

    // Flag to check if the current segment has been acked
    bool is_acked = true;

    // Filter out duplicate ackno && Invalid ackno
    if (abs_ackno <= _last_ackno || abs_ackno > _next_seqno)
        return;

    // Update last ackno
    _last_ackno = abs_ackno;

    // Flush out segments that have been acked
    while (!_segments_outstanding.empty() && is_acked) {
        segment_seqno = _segments_outstanding.top().first;
        segment_payload_length = _segments_outstanding.top().second.length_in_sequence_space();

        // If the current segment has been acked
        if (segment_seqno + segment_payload_length <= abs_ackno) {
            _segments_outstanding.pop();  // Pop out acked segment
            _bytes_in_flight -= segment_payload_length;
        } else  // Current segment has not been acked. Stop flushing
            is_acked = false;
    }

    // Stop timer if no outstanding segments, else restart timer
    if (_segments_outstanding.empty())
        _retransmission_timer.stop();
    else
        _retransmission_timer.start();

    return;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    // Update retransmission timer
    _retransmission_timer.tick(ms_since_last_tick);

    // Retransmit segment if timer has expired and there are outstanding segments
    if (_retransmission_timer.is_expired() && !_segments_outstanding.empty()) {
        if (_receiver_window_size >= 1)  // Perform retransmission only if receiver window size >= 1
            _retransmission_timer.exponential_backoff();

        // Restart timer
        _retransmission_timer.reset_time();
        _retransmission_timer.reset_is_expired();

        // Retransmit segment
        _segments_out.push(_segments_outstanding.top().second);
    }

    // Stop timer if no outstanding segments
    if (_segments_outstanding.empty())
        _retransmission_timer.stop();

    return;
}

unsigned int TCPSender::consecutive_retransmissions() const {
    return _retransmission_timer.consecutive_retransmissions();
}

void TCPSender::send_empty_segment() {
    const bool EMPTY = true;

    // Create empty segment
    TCPSegment empty_segment;
    empty_segment.header().seqno = next_seqno();      // Set wrapped seqno
    send_segment(empty_segment, _next_seqno, EMPTY);  // Send empty segment

    return;
}

TCPSegment TCPSender::create_segment(const WrappingInt32 seqno, string &payload, const bool is_syn, const bool is_fin) {
    TCPSegment segment;

    segment.header().seqno = seqno;                  // Set wrapped seqno
    segment.payload() = Buffer(std::move(payload));  // Set payload // std::move(payload)
    segment.header().syn = is_syn;                   // Set SYN flag
    segment.header().fin = is_fin;                   // Set FIN flag

    return segment;
}

void TCPSender::send_segment(TCPSegment &segment, uint64_t next_seqno, bool is_empty) {
    uint64_t segment_length;

    // Early termination if segment is empty
    if (is_empty) {
        _segments_out.push(segment);
        return;
    }

    _segments_out.push(segment);                        // Send segment to receiver
    _segments_outstanding.push({next_seqno, segment});  // Track outstanding segments

    // Track resources
    segment_length = static_cast<uint64_t>(segment.length_in_sequence_space());
    _next_seqno += segment_length;
    _bytes_in_flight += segment_length;

    // Start retransmission timer if not running
    if (!_retransmission_timer.is_running())
        _retransmission_timer.start();

    return;
}

RetransmissionTimer::RetransmissionTimer(const unsigned int retransmission_timeout)
    : _initial_retransmission_timeout(retransmission_timeout), _retransmission_timeout(retransmission_timeout) {}

void RetransmissionTimer::tick(const size_t ms_since_last_tick) {
    if (_is_running) {  // Update retransmission time only if timer is running
        _retransmission_time += ms_since_last_tick;

        if (_retransmission_time >= _retransmission_timeout) {  // If expired
            _is_expired = true;
            _consecutive_retransmissions++;  // Increment consecutive retransmissions
        }
    }

    return;
}

void RetransmissionTimer::start() {
    // Update timer state to start
    _is_running = true;
    _is_expired = false;
    _retransmission_time = 0;
    _consecutive_retransmissions = 0;
    _retransmission_timeout = _initial_retransmission_timeout;

    return;
}

void RetransmissionTimer::stop() {
    // Update timer state to stop
    _is_running = false;
    _is_expired = false;
    _retransmission_time = 0;

    return;
}
