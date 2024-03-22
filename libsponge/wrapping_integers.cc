#include "wrapping_integers.hh"

#include <iostream>

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    const uint64_t moduler_base = 1ul << 32;
    uint64_t isn_raw_value = static_cast<uint64_t>(isn.raw_value());  // Type conversion to match the type of n
    uint32_t seqno = static_cast<uint32_t>((isn_raw_value + n) % moduler_base);  // Seqno to absolute_seqno operation

    return WrappingInt32{seqno};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    WrappingInt32 recent_ackno = wrap(checkpoint, isn);  // Relative ackno of the absolute recent ackno
    uint64_t offset =
        static_cast<uint64_t>(n - recent_ackno);  // Difference between the relative recent ackno and the n
    bool overflow_occured = static_cast<int64_t>(checkpoint + offset) <
                            0;  // Check if the overflow occured(overflow can occur: uint64_t + uint64_t -> uint64_t)
    uint64_t overflow_offset = overflow_occured ? (1ul << 32) : 0;  // Offset to resolve the overflow

    // Calculate the absolute_seqno
    return checkpoint + offset + overflow_offset;
}
