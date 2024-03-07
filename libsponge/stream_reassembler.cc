#include "stream_reassembler.hh"
#include <iostream>  // TODO: remove this line
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity), _unassembled_bytes(0), is_eof(false), aux_storage(compare()) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    /*
    
    
    
    
    */    

    bool is_concatenable = true;
    _unassembled_bytes += data.length();
    aux_storage.push(Substring(data, index, eof)); // Contain the substring in to auxilary storage and sort using the index
    
    do {
        Substring current = aux_storage.top();

        if (int(_output.bytes_written()) < current.start) {
            is_concatenable = false;
            continue;
        }
        
        aux_storage.pop();
        
        if (int(_output.bytes_written()) <= current.end){            
            _output.write(current.data.substr(max(int(_output.bytes_written() - current.start), 0), current.length));
        } 

        _unassembled_bytes -= current.length;

        if (current.eof) {
            // is_eof = true;
            if (int(_capacity) >= current.end)_output.end_input();
            is_concatenable = false;
        }

        if (aux_storage.empty()) 
            is_concatenable = false;
        

    } while(is_concatenable); 
    
    


    return;
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _unassembled_bytes == 0; }
