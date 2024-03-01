#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream(const size_t capacity) : buffer(""), buffer_capacity(capacity), _buffer_size(0), _bytes_written(0), _bytes_read(0), is_input_ended(false) { }

size_t ByteStream::write(const string &data) {
    /* The fucntion that write data into buffer
     *  
     * parameters:
     *     const string &data: A data string
     *
     * return:
     *     size_t write_length: length of data written in buffer
     */

    size_t write_length = min(data.length(), remaining_capacity()); 
    
    // Write buffer
    buffer = buffer + data.substr(0, write_length);	
    
    // Update buffer parameters
    _buffer_size += write_length;
    _bytes_written += write_length;

    return write_length;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const { return buffer.substr(0, len); }

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    /* The function that pops data from buffer using given length
     *
     * parameters:
     *     const size_t len: length of data to pop from buffer
     *
     * return:
     *     void
     */
	
    size_t pop_length = min(_buffer_size, len);
    
    // Pop data from buffer
    buffer = buffer.substr(pop_length, _buffer_size);
    
     // Update buffer parameters
    _buffer_size -= pop_length;
    _bytes_read += pop_length;

    return;
 }

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    /* The function that reads data from buffer
     * 
     * parameter:
     *     const size_t: length of data to read from buffer
     *
     * return:
     *     const output_data: A data that read from buffer
     */
	
    size_t read_length = min(_buffer_size, len);

    string output_data = peek_output(read_length); // Copy data from buffer
    
    pop_output(read_length); // Pop Copied data

    return output_data;
}

void ByteStream::end_input() { 
    is_input_ended = true;
    return;
}

bool ByteStream::input_ended() const { return is_input_ended; }

size_t ByteStream::buffer_size() const { return _buffer_size; }

bool ByteStream::buffer_empty() const { return _buffer_size == 0; }

bool ByteStream::eof() const { return is_input_ended && buffer_empty(); }

size_t ByteStream::bytes_written() const { return _bytes_written; }

size_t ByteStream::bytes_read() const { return _bytes_read; }

size_t ByteStream::remaining_capacity() const { return buffer_capacity - _buffer_size; }



