#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`
using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity)
    , _capacity(capacity)
    , _unassembled_bytes(0)
    , _unassembled_start(0)
    , eof_index(uint64_t(-1))
    , aux_storage(capacity, 0)
    , occupied(capacity, 0) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    /*
     * The function that assembles the substrings and writes them into the output stream in order
     *
     * parameters:
     *     const string& data: A raw substring
     *     const size_t index: The index of the first byte in the substring
     *     const bool eof: The last byte of the substring is end of file
     *
     * return:
     *     void
     */
    const int UNOCCUPIED = 0;
    const int OCCUPIED = 1;

    // Make a substring of the input data
    Substring input = Substring(data, index, eof);
    size_t assemblable_length = 0;  // The length of the assemblable substring

    // save eof index if the substring contains eof
    if (eof)
        eof_index = input.end;

    // If the output stream is full, early termination
    if (_output.remaining_capacity() == 0)  // If the output stream is full, early termination
        return;

    for (size_t i = max(input.start, _unassembled_start);
         i < min(_unassembled_start + _output.remaining_capacity(), input.end);
         i++) {
        if (occupied[i - _unassembled_start] == OCCUPIED)  // If the byte has been pushed more than once, skip it
            continue;

        aux_storage[i - _unassembled_start] = input.data[i - input.start];  // Store the byte
        occupied[i - _unassembled_start] = OCCUPIED;                        // Mark the byte as occupied
        _unassembled_bytes++;                                               // Increase the number of unassembled bytes
    }

    for (size_t i = _unassembled_start; i < _unassembled_start + _output.remaining_capacity(); i++) {
        if (occupied[i - _unassembled_start] == UNOCCUPIED)
            break;
        assemblable_length++;
    }

    if (assemblable_length > 0) {
        _output.write(aux_storage.substr(0, assemblable_length));  // Write the assemblable substring into the output
        _unassembled_start += assemblable_length;                  // Update the start of the unassembled substring
        _unassembled_bytes -= assemblable_length;                  // Update the number of unassembled bytes
        aux_storage = aux_storage.substr(assemblable_length);
        aux_storage += string(assemblable_length, 0);        // Update the auxiliary storage
        occupied = occupied.substr(assemblable_length);      // Update the occupied status
        occupied += string(assemblable_length, UNOCCUPIED);  // Update the occupied status
    }

    if (eof_index != uint64_t(-1) && _unassembled_start == eof_index)  // If the end of the file is reached
        _output.end_input();
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _unassembled_bytes == 0; }
