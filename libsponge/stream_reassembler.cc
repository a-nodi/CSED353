#include "stream_reassembler.hh"
#include <iostream>  // TODO: remove this line
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
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    Substring input = Substring(data, index, eof);

    if (input.start == input.end) {
        if (eof) 
            _output.end_input();
        
        return;
    }

    for (size_t i = input.start; i < input.end; i++) {
        if (i < _unassembled_start) 
            continue;
        
        if (i >= _unassembled_start + _capacity) 
            break;

        if (aux_storage.find(i) == aux_storage.end() && i < _capacity + _output.bytes_read()) {
            aux_storage[i] = make_pair(data.substr(i - input.start, 1), (i == input.end - 1 && eof));
            
            _unassembled_bytes++;
        }

    }

    while (aux_storage.find(_unassembled_start) != aux_storage.end()) {
        _output.write(aux_storage[_unassembled_start].first);
        if (aux_storage[_unassembled_start].second) 
            _output.end_input();
        
        aux_storage.erase(_unassembled_start);
        _unassembled_start++;
        _unassembled_bytes--;

    }

    return;
}



size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _unassembled_bytes == 0; }
