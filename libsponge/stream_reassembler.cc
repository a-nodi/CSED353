#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), 
                                                              _capacity(capacity), 
                                                              _unassembled_bytes(0), 
                                                              _unassembled_start(0),
                                                              aux_storage() {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string& data, const size_t index, const bool eof) {
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


    // Make a substring of the input data
    Substring input = Substring(data, index, eof);

    // If the input is empty, early termination
    if (input.start == input.end) {
        if (eof)  // If the input is empty and the end of the file is reached, end the input
            _output.end_input();

        return;
    }

    // Allocate the input substring to the aux_storage to be reassembled
    for (size_t i = input.start; i < input.end; i++) {
        // Case 1: Current byte is already reassembled
        if (i < _unassembled_start) 
            continue;
        
        // Case 2: Current byte is out of the capacity
        if (i >= _unassembled_start + _capacity) 
            break;

        // Case 3: Current byte can be reassembled
        if (aux_storage.find(i) == aux_storage.end() && i < _capacity + _output.bytes_read()) {
            aux_storage[i] = make_pair(data.substr(i - input.start, 1), (i == input.end - 1 && eof)); // Store the byte (one byte and eof) in the aux_storage

            _unassembled_bytes++;
        }
    }

    // Reassemble the substrings in the aux_storage
    while (aux_storage.find(_unassembled_start) != aux_storage.end()) {
        // Write the reassembled byte to the output stream if it can be reassembled
        _output.write(aux_storage[_unassembled_start].first);

        // Check if the reassembled byte is the last byte of the file
        if (aux_storage[_unassembled_start].second) 
            _output.end_input();
        
        // Remove the reassembled byte from the aux_storage
        aux_storage.erase(_unassembled_start);

        _unassembled_start++;
        _unassembled_bytes--;
    }

    return;
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _unassembled_bytes == 0; }
