#include "tcp_connection.hh"

#include <iostream>

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {  // DONE
    _time_since_last_segment_received = 0;                     // Reset time since last segment received

    if (seg.header().rst) {  // Early termination if RST flag have received
        reset_connection();  // Reset connection
        return;
    }

    _receiver.segment_received(seg);  // Receive segment

    if (seg.header().ack) {                                          // Check for ACK flag
        _sender.ack_received(seg.header().ackno, seg.header().win);  // Receive ACK
    }

    if (_receiver.ackno().has_value() && (seg.length_in_sequence_space == 0) &&
        seg.header().segno == _receiver.ackno().value() - 1) {
        _sender.send_empty_segment();  // Send empty segment
    }

    return;
}

bool TCPConnection::active() const { return _is_active; }  // DONE

size_t TCPConnection::write(const string &data) {            // DONE
    size_t bytes_written = _sender.stream_in().write(data);  // Write data to stream
    send_segment();                                          // Send segment

    return bytes_written;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {  // DONE
    _sender.tick(ms_since_last_tick);                        // Update sender's tick

    _time_since_last_segment_received += ms_since_last_tick;

    if (_sender.consecutive_retransmissions() > _cfg.MAX_RETX_ATTEMPTS)  // Check for max retransmission attempts
        shutdown_connection(true);                                       // Shutdown connection

    if (have_sent_syn)
        send_segment();  // Send segment anytime if SYN is sent

    return;
}

void TCPConnection::end_input_stream() {  // DONE
    _sender.stream_in().end_input();      // End input stream
    send_segment();
}

void TCPConnection::connect() {  // DONE
    _is_active = true;

    // Send SYN segment if connection is not established
    send_segment();

    if (!_have_sent_syn)
        have_sent_syn = true;

    return;
}

void TCPConnection::send_segment() {  // DONE
    _sender.fill_window();            // Fill segments in window
    bool is_last_segment = _sender.segments_out().empty();
    const bool CLEAN = true;
    while (!is_last_segment) {
        // Pop out segment from sender's queue
        TCPSegment segment = _sender.segments_out().front();
        _sender.segments_out().pop();

        // Check if segment is last segment
        is_last_segment = _sender.segments_out().empty();

        // SYN flag and FIN flag is set in the sender's fill_window method.
        // Seqno is set in the sender's fill_window method.
        // 1. Have to set RST flag if connection is reset
        // 2. Have to set ack flag and ackno if receiver has ackno
        // 3. Have to set window size

        // 1. Set RST flag if connection is reset and the segment is last segment
        if (_is_rst && is_last_segment)
            segment.header().rst = true;

        // 2. Set ackno if receiver has ackno
        if (_receiver.ackno().has_value()) {
            segment.header().ack = true;
            segment.header().ackno = _receiver.ackno().value();
        }

        // 3. Set window size
        segment.header().win = _receiver.window_size();

        _segments_out.push(segment);  // Push segment to the queue
    }

    shutdown_connection(CLEAN);

    return;
}

void TCPConnection::reset_connection() {  // DONE
    _is_active = false;
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();

    return;
}

void TCPConnection::shutdown_connection(bool is_clean) {  // WIP
    if (is_clean) {                                       // clean shutdown
        // End prereq #1, 2, 3
        if (_receiver.unassembled_bytes == 0 && _receiver.stream_out().input_ended() &&
            _sender.stream_in.input_ended() && _sender.bytes_in_flight() == 0) {
            // End prereq #4 Option A
            if (time_since_last_segment_received() >= 10 * _cfg.rt_timeout && !_linger_after_streams_finish)
                _is_active = false;

            // End prereq #4 Option B
            if (_receiver.stream_out().input_ended() && !_sender.stream_in().eof()) {
                _linger_after_streams_finish = false;
            }
        }
    }

    else {  // unclean shutdown
        reset_connection();

        is_rst = true;
        send_segment();
    }
    _is_active = false;
}

TCPConnection::~TCPConnection() {  // DONE
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            shutdown_connection(false);
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
