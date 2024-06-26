#ifndef SPONGE_LIBSPONGE_TCP_SENDER_HH
#define SPONGE_LIBSPONGE_TCP_SENDER_HH

#include "byte_stream.hh"
#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <functional>
#include <queue>
#include <vector>

//! \brief The "sender" part of a TCP implementation.

//! A RetransmissionTimer is used to manage the retransmission timeout
class RetransmissionTimer {
  private:
    //! The initial retransmission timeout
    unsigned int _initial_retransmission_timeout;

    //! The current retransmission timeout
    unsigned int _retransmission_timeout;

    //! The current retransmission time
    unsigned int _retransmission_time{0};

    //! The number of consecutive retransmissions
    unsigned int _consecutive_retransmissions{0};

    //! The expired state of RetransmissionTimer
    bool _is_expired{false};

    //! The running state of RetransmissionTimer
    bool _is_running{false};

  public:
    //! Initialize a RetransmissionTimer
    RetransmissionTimer(const unsigned int retransmission_timeout);

    //! \brief Notifies the RetransmissionTimer of the passage of time
    void tick(const size_t ms_since_last_tick);

    //! \brief Perform exponential backoff for retransmission timeout
    void exponential_backoff() { _retransmission_timeout *= 2; };

    //! \brief Start the RetransmissionTimer
    void start();

    //! \brief Stop the RetransmissionTimer
    void stop();

    //! \brief Return the RetransmissionTimer is running
    bool is_running() const { return _is_running; }

    //! \brief Return the RetransmissionTimer has expired
    bool is_expired() const { return _is_expired; }

    //! \brief Return the consecutive retransmissions count
    unsigned int consecutive_retransmissions() const { return _consecutive_retransmissions; }

    //! \brief Reset the retransmission time
    void reset_time() { _retransmission_time = 0; }

    //! \brief Reset the expired state of RetransmissionTimer
    void reset_is_expired() { _is_expired = false; }

    //! \brief Reset the consecutive retransmissions count
    void reset_consecutive_retransmissions() { _consecutive_retransmissions = 0; }
};

//! Accepts a ByteStream, divides it up into segments and sends the
//! segments, keeps track of which segments are still in-flight,
//! maintains the Retransmission Timer, and retransmits in-flight
//! segments if the retransmission timer expires.
class TCPSender {
  private:
    //! our initial sequence number, the number for our SYN.
    WrappingInt32 _isn;

    //! outbound queue of segments that the TCPSender wants sent
    std::queue<TCPSegment> _segments_out{};

    //! retransmission timer for the connection
    unsigned int _initial_retransmission_timeout;

    //! outgoing stream of bytes that have not yet been sent
    ByteStream _stream;

    //! the (absolute) sequence number for the next byte to be sent
    uint64_t _next_seqno{0};

    //! the last ackno received
    uint64_t _last_ackno{0};

    //! the last window size received
    uint16_t _receiver_window_size{1};

    //! the number of bytes outstanding
    uint64_t _bytes_in_flight{0};

    //! SYN flag
    bool have_sent_syn{false};

    //! FIN flag
    bool have_sent_fin{false};

    //! Retransmission timer
    RetransmissionTimer _retransmission_timer;

    //! Comparator for priority queue
    struct cmp {
        bool operator()(const std::pair<uint64_t, TCPSegment> &a, const std::pair<uint64_t, TCPSegment> &b) {
            return a.first > b.first;
        }
    };

    //! Priority queue to track outstanding segments
    std::priority_queue<std::pair<uint64_t, TCPSegment>, std::vector<std::pair<uint64_t, TCPSegment>>, cmp>
        _segments_outstanding{};

    //! \brief Create a segment with the given sequence number, payload, and flags
    TCPSegment create_segment(const WrappingInt32 seqno,
                              std::string &payload,
                              const bool is_syn = false,
                              const bool is_fin = false);

    //! \brief Send a segment and update states
    void send_segment(TCPSegment &segment, uint64_t next_seqno, bool is_empty = false);

  public:
    //! Initialize a TCPSender
    TCPSender(const size_t capacity = TCPConfig::DEFAULT_CAPACITY,
              const uint16_t retx_timeout = TCPConfig::TIMEOUT_DFLT,
              const std::optional<WrappingInt32> fixed_isn = {});

    //! \name "Input" interface for the writer
    //!@{
    ByteStream &stream_in() { return _stream; }
    const ByteStream &stream_in() const { return _stream; }
    //!@}

    //! \name Methods that can cause the TCPSender to send a segment
    //!@{

    //! \brief A new acknowledgment was received
    void ack_received(const WrappingInt32 ackno, const uint16_t window_size);

    //! \brief Generate an empty-payload segment (useful for creating empty ACK segments)
    void send_empty_segment();

    //! \brief create and send segments to fill as much of the window as possible
    void fill_window();

    //! \brief Notifies the TCPSender of the passage of time
    void tick(const size_t ms_since_last_tick);
    //!@}

    //! \name Accessors
    //!@{

    //! \brief How many sequence numbers are occupied by segments sent but not yet acknowledged?
    //! \note count is in "sequence space," i.e. SYN and FIN each count for one byte
    //! (see TCPSegment::length_in_sequence_space())
    size_t bytes_in_flight() const;

    //! \brief Number of consecutive retransmissions that have occurred in a row
    unsigned int consecutive_retransmissions() const;

    //! \brief TCPSegments that the TCPSender has enqueued for transmission.
    //! \note These must be dequeued and sent by the TCPConnection,
    //! which will need to fill in the fields that are set by the TCPReceiver
    //! (ackno and window size) before sending.
    std::queue<TCPSegment> &segments_out() { return _segments_out; }
    //!@}

    //! \name What is the next sequence number? (used for testing)
    //!@{

    //! \brief absolute seqno for the next byte to be sent
    uint64_t next_seqno_absolute() const { return _next_seqno; }

    //! \brief relative seqno for the next byte to be sent
    WrappingInt32 next_seqno() const { return wrap(_next_seqno, _isn); }
    //!@}
};

#endif  // SPONGE_LIBSPONGE_TCP_SENDER_HH
