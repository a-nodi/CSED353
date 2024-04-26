#include "tcp_connection.hh"

#include <cstdint>
#include <iostream>
// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

using namespace std;

const bool CLEAN = true;
const bool UNCLEAN = false;

size_t TCPConnection::remaining_outbound_capacity() const {
    return is_active ? _sender.stream_in().remaining_capacity() : 0;
}

size_t TCPConnection::bytes_in_flight() const { return is_active ? _sender.bytes_in_flight() : 0; }

size_t TCPConnection::unassembled_bytes() const { return is_active ? _receiver.unassembled_bytes() : 0; }

size_t TCPConnection::time_since_last_segment_received() const {
    return is_active ? _time_since_last_segment_received : 0;
}

void TCPConnection::segment_received(const TCPSegment &seg) {
    /*
     * The function that receives a segment from the peer and processes it.
     *
     * parameters:
     *      const TCPSegment &seg: the segment received from the peer
     *
     * return:
     *      void
     */

    TCPHeader header = seg.header();
    bool is_ack = seg.header().ack;
    bool ack_for_syn, ack_for_fin, ack_for_normal, ack_for_keep_alive;

    _time_since_last_segment_received = 0;  // Reset time since last segment received

    if (!is_active || (is_ack && !have_sent_syn))  // Early termination if connection is not active or Ignore ack packet
                                                   // that came before sending syn packet
        return;

    if (header.rst) {        // Early termination if RST flag have received
        reset_connection();  // Reset connection
        return;
    }

    _receiver.segment_received(seg);  // Receive segment

    if (is_ack)                                          // Check for ACK flag
        _sender.ack_received(header.ackno, header.win);  // Prepare more segments

    ack_for_syn = header.syn && have_sent_syn;  // SYN(local) -> SYN + ACK(remote) -> "ACK"(local)
    ack_for_fin = header.fin;                   // FIN(local) -> FIN + ACK(remote) -> "ACK"(local)
    ack_for_normal =
        seg.payload().str().length() > 0 && _receiver.ackno().has_value();  // Segment(remote) -> "ACK"(local)
    ack_for_keep_alive = _receiver.ackno().has_value() && (seg.length_in_sequence_space() == 0) &&
                         header.seqno == _receiver.ackno().value() - 1;  // Invalid Segment(remote) -> "ACK"(local)

    if (ack_for_syn || ack_for_fin || ack_for_normal || ack_for_keep_alive)  // Check for ACK flag
        _sender.send_empty_segment();                                        // Create ACK segment

    send_segment();              // Send segment which created after the ack
    shutdown_connection(CLEAN);  // Check clean shutdown should be done

    return;
}

bool TCPConnection::active() const { return is_active; }

size_t TCPConnection::write(const string &data) {
    if (!is_active)  // Early termination if connection is not active
        return 0;

    return _sender.stream_in().write(data);
}

bool TCPConnection::is_fully_received() {
    return _receiver.unassembled_bytes() == 0 && _receiver.stream_out().input_ended();
}

bool TCPConnection::is_fully_sent() { return _sender.stream_in().input_ended() && have_sent_syn; }

bool TCPConnection::is_fully_acknowledged() { return _sender.bytes_in_flight() == 0; }

bool TCPConnection::no_recent_acknowledgement() { return time_since_last_segment_received() >= 10 * _cfg.rt_timeout; }

bool TCPConnection::fully_received_but_not_fully_sent() { return is_fully_received() && !is_fully_sent(); }

bool TCPConnection::too_many_retransmissions() {
    return _sender.consecutive_retransmissions() > _cfg.MAX_RETX_ATTEMPTS;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    /*
     * The function that is called periodically when time elapses.
     *
     * parameters:
     *      const size_t ms_since_last_tick: the number of milliseconds since the last call to this method
     *
     * return:
     *      void
     */

    if (!is_active || !have_sent_syn)  // Early termination if connection is not active or not established
        return;

    _time_since_last_segment_received += ms_since_last_tick;  // Update connection's tick
    _sender.tick(ms_since_last_tick);                         // Update sender's tick

    // Too many consecutive retransmissions without success
    if (too_many_retransmissions())
        shutdown_connection(UNCLEAN);  // Check unclean shutdown should be done

    send_segment();              // Send retransmission segment
    shutdown_connection(CLEAN);  // Check clean shutdown should be done

    return;
}

void TCPConnection::end_input_stream() {
    /*
     * The function ends the input stream.
     *
     * parameters:
     *      void
     *
     * return:
     *      void
     */

    if (!is_active)  // Early termination if connection is not active
        return;

    // End input stream
    _sender.stream_in().end_input();

    // Send FIN segment if input stream is ended
    send_segment();

    return;
}

void TCPConnection::connect() {
    /*
     * The function that establishs the connection.
     *
     * parameters:
     *      void
     *
     * return:
     *      void
     */

    is_active = true;

    // Send SYN segment if connection is not established
    send_segment();

    return;
}

void TCPConnection::send_segment() {
    /*
     * The function that sends segment to the peer.
     *
     * parameters:
     *      void
     *
     * return:
     *      void
     */

    TCPSegment seg;
    bool is_last_seg;

    _sender.fill_window();  // Fill segments in window
    is_last_seg = _sender.segments_out().empty();

    // First segment is SYN segment
    if (!have_sent_syn && !is_last_seg)
        have_sent_syn = true;

    // Send all segment in sender's queue
    while (!is_last_seg) {
        // Pop out segment from sender's queue
        seg = _sender.segments_out().front();
        _sender.segments_out().pop();

        // Check if segment is last segment
        is_last_seg = _sender.segments_out().empty();

        seg = build_header(seg, is_last_seg);  // Build header

        _segments_out.push(seg);  // Push segment to the queue
    }

    return;
}

TCPSegment &TCPConnection::build_header(TCPSegment &seg, bool is_last_seg) {
    /*
     * The function that builds(RST, ackno, win) header for the segment.
     *
     * parameters:
     *      TCPSegment &seg: the header of the segment
     *      bool is_last_seg: the flag that indicates if the segment is the last segment
     *
     * return:
     *      TCPSegment seg: the segment with header
     */

    optional<WrappingInt32> ackno = _receiver.ackno();

    /*
        SYN flag and FIN flag is set in the sender's fill_window method.
        Seqno is set in the sender's fill_window method.
        1. Have to set RST flag if connection has been reset
        2. Have to set ack flag and ackno if receiver has ackno
        3. Have to set window size
    */

    // 1. Set RST flag if the connection has to be reset
    if (is_rst && is_last_seg)
        seg.header().rst = true;

    // 2. Set ackno if receiver has ackno
    if (ackno.has_value()) {
        seg.header().ack = true;
        seg.header().ackno = ackno.value();
    }

    // 3. Set window size (upper bound to max value of uint16_t)
    seg.header().win = min(numeric_limits<uint16_t>::max(), static_cast<uint16_t>(_receiver.window_size()));

    return seg;
}

void TCPConnection::reset_connection() {
    /*
     * The function that sends segment to the peer.
     *
     * parameters:
     *      void
     *
     * return:
     *      void
     */

    // inactivate the connection
    is_active = false;

    // Reset the sender and receiver
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();

    return;
}

void TCPConnection::shutdown_connection(bool is_clean) {
    /*
     * The function that checks if the connection should be shutdown.
     *
     * parameters:
     *      bool is_clean: the flag that indicates if the connection should be shutdown cleanly
     *
     * return:
     *      void
     */

    bool prereq_1, prereq_2, prereq_3, prereq_4a, prereq_4b;

    // Shutdown is already done
    if (!is_active)
        return;

    if (is_clean) {                                       // Clean shutdown
        prereq_1 = is_fully_received();                   // 1. FULLY RECEIVED
        prereq_2 = is_fully_sent();                       // 2. FULLY SENT
        prereq_3 = is_fully_acknowledged();               // 3. FULLY ACKNOWLEDGED
        prereq_4a = no_recent_acknowledgement();          // 4.A NO RECENT ACKNOWLEDGEMENT
        prereq_4b = fully_received_but_not_fully_sent();  // 4.B FULLY RECEIVED BUT NOT FULLY SENT

        // End prereq #4 Option B
        if (prereq_4b)
            _linger_after_streams_finish = false;

        // End prereq #1, #2, #3, #4 Option A (Added linger option because of passive close test)
        if (prereq_1 && prereq_2 && prereq_3 && (prereq_4a || !_linger_after_streams_finish))
            is_active = false;
    }

    else {  // Unclean shutdown
        reset_connection();

        // Create RST segment if there is no segment
        if (_sender.segments_out().empty())
            _sender.send_empty_segment();

        is_rst = true;   // Set RST
        send_segment();  // Send RST segment
    }

    return;
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            shutdown_connection(UNCLEAN);
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
