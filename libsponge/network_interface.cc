#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();
    const uint32_t current_ip = _ip_address.ipv4_numeric();
    const size_t ARP_SEND_COOLDOWN = 5000;
    EthernetFrame frame;
    ARPMessage arp_message;

    if (arp_table.find(next_hop_ip) != arp_table.end() &&
        arp_table[next_hop_ip].ethernet_address == EthernetAddress() &&
        accumulated_time - arp_table[next_hop_ip].marked_time <= ARP_SEND_COOLDOWN)
        return;  // ARP request cooldown

    // Destination Ethernet address is already known && the entry is not expired
    if (arp_table.find(next_hop_ip) != arp_table.end() &&
        accumulated_time - arp_table[next_hop_ip].marked_time <= ARP_SEND_COOLDOWN) {
        // Construct Ethernet frame (from current interface to next hop's Ethernet address, with IPv4 type, and the
        // datagram as payload)
        frame = construct_frame(
            _ethernet_address, arp_table[next_hop_ip].ethernet_address, EthernetHeader::TYPE_IPv4, dgram.serialize());

        _frames_out.push(frame);
    } else if (arp_table.find(next_hop_ip) == arp_table.end() ||
               (arp_table.find(next_hop_ip) != arp_table.end() &&
                arp_table[next_hop_ip].ethernet_address == EthernetAddress() &&
                accumulated_time - arp_table[next_hop_ip].marked_time > ARP_SEND_COOLDOWN)) {
        // Send ARP request to resolve the Ethernet address
        arp_message = construct_arp_request(_ethernet_address, current_ip, next_hop_ip);
        frame = construct_frame(
            _ethernet_address, ETHERNET_BROADCAST, ARPMessage::OPCODE_REQUEST, BufferList(arp_message.serialize()));

        arp_table[next_hop_ip] =
            EthernetAndTime{EthernetAddress(), accumulated_time};  // Mark the entry(empty) as pending ARP request

        // Temporally store given datagram to send it after receiving the ARP reply
        if (temporal_data_storage.find(next_hop_ip) == temporal_data_storage.end())
            temporal_data_storage[next_hop_ip] = queue<InternetDatagram>();

        temporal_data_storage[next_hop_ip].push(dgram);
    }

    _frames_out.push(frame);

    return;
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    const uint32_t current_ip = _ip_address.ipv4_numeric();
    InternetDatagram dgram;
    ARPMessage arp_message;
    ARPMessage reply_message;
    ParseResult parse_result;
    EthernetFrame temp_frame;

    // Early termination, ignore any frames not destined for the network interface
    if (frame.header().dst != _ethernet_address && frame.header().dst != ETHERNET_BROADCAST)
        return nullopt;

    // Ealry termination, ignore any frame that are not IPv4 or ARP
    if (frame.header().type != EthernetHeader::TYPE_IPv4 && frame.header().type != EthernetHeader::TYPE_ARP)
        return nullopt;

    // Parse the IPv4 Datagram and return parsed data if no error while parsing
    if (frame.header().type == EthernetHeader::TYPE_IPv4) {
        parse_result = dgram.parse(frame.payload());
        if (parse_result == ParseResult::NoError)
            return dgram;

        return nullopt;
    }

    else if (frame.header().type == EthernetHeader::TYPE_ARP) {
        parse_result = arp_message.parse(frame.payload());
        if (parse_result != ParseResult::NoError)
            return nullopt;

        // Save IP-Ethernet mapping from ARP reply
        arp_table[arp_message.sender_ip_address] =
            EthernetAndTime{arp_message.sender_ethernet_address, accumulated_time};

        // Send the temporally stored datagrams of currently received ARP reply
        if (temporal_data_storage.find(arp_message.sender_ip_address) != temporal_data_storage.end()) {
            while (!temporal_data_storage[arp_message.sender_ip_address].empty()) {
                // Pop out temporally stored datagrame
                dgram = temporal_data_storage[arp_message.sender_ip_address].front();
                temporal_data_storage[arp_message.sender_ip_address].pop();

                // Construct frame
                temp_frame = construct_frame(_ethernet_address,
                                             arp_message.sender_ethernet_address,
                                             EthernetHeader::TYPE_IPv4,
                                             dgram.serialize());

                // Send the frame
                _frames_out.push(temp_frame);
            }
            // Delete queue from temporal storage
            temporal_data_storage.erase(arp_message.sender_ip_address);
        }

        // If the ARP message is a request, send a reply
        if (arp_message.opcode == ARPMessage::OPCODE_REQUEST) {
            reply_message = construct_arp_request(_ethernet_address, current_ip, arp_message.sender_ip_address);
            temp_frame = construct_frame(_ethernet_address,
                                         arp_message.sender_ethernet_address,
                                         EthernetHeader::TYPE_ARP,
                                         BufferList(reply_message.serialize()));

            _frames_out.push(temp_frame);
        }
    }

    return nullopt;
}
//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    const size_t MAPPING_TIMEOUT = 30000;

    // Accumulate time since last tick
    accumulated_time += ms_since_last_tick;

    // Expire entries in the ARP table that have been there for more than 30 seconds
    for (auto it = arp_table.begin(); it != arp_table.end();) {
        if (accumulated_time - it->second.marked_time > MAPPING_TIMEOUT)
            it = arp_table.erase(it);  // Erase the entry and move to the next iterator
        else
            it++;  // Move to the next iterator
    }

    return;
}

EthernetHeader NetworkInterface::construct_header(const EthernetAddress &source_address,
                                                  const EthernetAddress &destination_address,
                                                  const uint16_t type) {
    EthernetHeader header;

    header.src = source_address;
    header.dst = destination_address;
    header.type = type;

    return header;
}

EthernetFrame NetworkInterface::construct_frame(const EthernetAddress &source_address,
                                                const EthernetAddress &destination_address,
                                                const uint16_t type,
                                                const BufferList &payload) {
    EthernetFrame frame;

    frame.header() = construct_header(source_address, destination_address, type);
    frame.payload() = payload;

    return frame;
}

ARPMessage NetworkInterface::construct_arp_request(const EthernetAddress &source_ethernet_address,
                                                   const uint32_t source_ip_address,
                                                   const uint32_t target_ip_address) {
    ARPMessage arp_message;

    arp_message.opcode = ARPMessage::OPCODE_REQUEST;
    arp_message.sender_ethernet_address = source_ethernet_address;
    arp_message.sender_ip_address = source_ip_address;
    arp_message.target_ip_address = target_ip_address;

    return arp_message;
}