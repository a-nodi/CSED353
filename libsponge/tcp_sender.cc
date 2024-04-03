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
    // No SYN packet sent
    if (!have_sent_syn) {
        // Create & send syn packet
        TCPSegment syn_segment;
        syn_segment.header().syn = true;
        syn_segment.header().seqno = next_seqno();  // set wrapped seqno
        send_segment(syn_segment, _next_seqno);
        have_sent_syn = true;

        return;
    }

    size_t sendable_bytes = max(_receiver_window_size, size_t(1));
    size_t ackno_diff = _next_seqno - _last_ackno;

    sendable_bytes -= ackno_diff;

    while (sendable_bytes > 0 && !is_fin) {
        // Create Segment
        TCPSegment segment;
        size_t payload_size = min(TCPConfig::MAX_PAYLOAD_SIZE, sendable_bytes);
        string payload = _stream.read(payload_size);     // Fill out payload from stream
        segment.payload() = Buffer(std::move(payload));  // Move payload into segment

        // Add fin flag if stream is empty
        if (_stream.eof()) {  // TODO: 여기 조건 하나 빼놓음
            sendable_bytes--;
            segment.header().fin = true;
            is_fin = true;
        }

        send_segment(segment, _next_seqno);
        sendable_bytes -= payload_size;
    }
    return;
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    uint64_t abs_ackno = unwrap(ackno, _isn, _next_seqno);
    _receiver_window_size = window_size;

    uint64_t segment_seqno;
    uint64_t segment_payload_length;

    // Update last ackno
    _last_ackno = abs_ackno;

    // Flush out segments that have been acked
    while (!_segments_outstanding.empty()) {
        segment_seqno = _segments_outstanding.top().first;
        segment_payload_length = _segments_outstanding.top().second.length_in_sequence_space();

        // If the current segment has been acked
        if (segment_seqno + segment_payload_length <= abs_ackno) {
            _segments_outstanding.pop();
            _bytes_in_flight -= segment_payload_length;
        } else  // Current segment has not been acked. Stop flushing
            break;
    }

    return;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    _retransmission_timer.tick(ms_since_last_tick);

    if (_retransmission_timer.is_expired()) {
        _segments_out.push(_segments_outstanding.top().second);  // TODO: 이거 없어도 되지 않음?
        if (_receiver_window_size > 0)
            _retransmission_timer.exponential_backoff();
    }

    if (_segments_outstanding.empty())
        _retransmission_timer.stop();

    return;
}

unsigned int TCPSender::consecutive_retransmissions() const {
    return _retransmission_timer.consecutive_retransmissions();
}

void TCPSender::send_empty_segment() {
    TCPSegment empty_segment;
    empty_segment.header().seqno = next_seqno();
    send_segment(empty_segment, _next_seqno, true);

    return;
}

void TCPSender::send_segment(TCPSegment &segment, uint64_t next_seqno, bool is_empty) {
    // Early termination if segment is empty
    if (is_empty) {
        _segments_out.push(segment);
        return;
    }

    // Track outstanding segments
    _segments_out.push(segment);
    _segments_outstanding.push({next_seqno, segment});

    // Track resources
    _next_seqno += segment.length_in_sequence_space();
    _bytes_in_flight += segment.length_in_sequence_space();

    // Start retransmission timer
    if (!_retransmission_timer.is_running())
        _retransmission_timer.start();

    return;
}

RetransmissionTimer::RetransmissionTimer(const unsigned int retransmission_timeout)
    : _initial_retransmission_timeout(retransmission_timeout), _retransmission_timeout(retransmission_timeout) {}

void RetransmissionTimer::tick(const size_t ms_since_last_tick) {
    if (_is_running) {
        _retransmission_time += ms_since_last_tick;
        if (_retransmission_time >= _retransmission_timeout) {  // expired
            _is_expired = true;
            _consecutive_retransmissions++;
        }
    }

    return;
}

void RetransmissionTimer::exponential_backoff() {
    _is_expired = false;
    _retransmission_timeout *= 2;
    _retransmission_time = 0;

    return;
}

void RetransmissionTimer::start() {
    _is_running = true;
    _is_expired = false;
    _retransmission_time = 0;
    _retransmission_timeout = _initial_retransmission_timeout;

    return;
}

void RetransmissionTimer::stop() {
    _is_running = false;
    _is_expired = false;
    _retransmission_time = 0;

    return;
}
