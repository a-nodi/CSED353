#include "tcp_receiver.hh"

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    uint64_t raw_absolute_seqno;
    size_t absolute_seqno;

    uint64_t stream_index;
    uint64_t syn_packet_offset;
    string payload;

    if (seg.header().syn) {
        has_syn = true;
        isn = WrappingInt32{seg.header().seqno.raw_value()};
    }

    if (!has_syn)
        return;

    if (seg.header().fin)
        has_fin = true;

    stream_index = static_cast<uint64_t>(_reassembler.stream_out().bytes_written());
    raw_absolute_seqno = unwrap(seg.header().seqno, isn, stream_index);
    syn_packet_offset = static_cast<uint64_t>(!seg.header().syn);

    absolute_seqno = static_cast<size_t>(raw_absolute_seqno - syn_packet_offset);
    payload = seg.payload().copy();

    _reassembler.push_substring(payload, absolute_seqno, seg.header().fin);

    return;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    uint64_t stream_index;
    uint64_t syn_packet_offset;
    uint64_t fin_packet_offset;
    optional<WrappingInt32> ackno;
    if (!has_syn)
        return nullopt;

    stream_index = static_cast<uint64_t>(_reassembler.stream_out().bytes_written());
    syn_packet_offset = static_cast<uint32_t>(has_syn);
    fin_packet_offset = static_cast<uint32_t>(has_fin && _reassembler.empty());

    ackno =
        static_cast<optional<WrappingInt32>>(WrappingInt32(isn + syn_packet_offset + stream_index + fin_packet_offset));

    return ackno;
}

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }
