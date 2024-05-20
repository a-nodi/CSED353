#include "router.hh"

#include <iostream>

using namespace std;

// Dummy implementation of an IP router

// Given an incoming Internet datagram, the router decides
// (1) which interface to send it out on, and
// (2) what next hop address to send it to.

// For Lab 6, please replace with a real implementation that passes the
// automated checks run by `make check_lab6`.

// You will need to add private members to the class declaration in `router.hh`

//! \param[in] route_prefix The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
//! \param[in] prefix_length For this route to be applicable, how many high-order (most-significant) bits of the route_prefix will need to match the corresponding bits of the datagram's destination address?
//! \param[in] next_hop The IP address of the next hop. Will be empty if the network is directly attached to the router (in which case, the next hop address should be the datagram's final destination).
//! \param[in] interface_num The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    // cerr << "DEBUG: adding route " << Address::from_ipv4_numeric(route_prefix).ip() << "/" << int(prefix_length)
    //     << " => " << (next_hop.has_value() ? next_hop->ip() : "(direct)") << " on interface " << interface_num <<
    //     "\n";

    routing_entry entry = {route_prefix, prefix_length, next_hop, interface_num};  // Create routing entry
    routing_table.push_back(entry);  // Contain the routing entry in the routing table
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    uint32_t route_prefix;
    uint8_t prefix_length;
    uint8_t max_prefix_length = 0;
    uint32_t dest_ip;
    uint32_t subnet_mask;
    size_t interface_num;

    bool route_exists = false;
    bool default_route = false;

    optional<Address> dest_address{};

    if (dgram.header().ttl > 0)  // Overflow prevention
        dgram.header().ttl--;    // Decrease the TTL

    if (dgram.header().ttl <= 0)  // Early termination, The Drop the datagram if the TTL is less than 1
        return;

    // Search all the routing table entries
    for (auto &entry : routing_table) {
        route_prefix = entry.route_prefix;
        prefix_length = entry.prefix_length;
        dest_ip = dgram.header().dst;
        subnet_mask = 0xffffffff << (32 - prefix_length);  // Mask for destination IP address

        // Check if the route exists or it is a default route
        route_exists = (route_prefix == (subnet_mask & dest_ip));   // Longest prefix match
        default_route = (route_prefix == 0 && prefix_length == 0);  // Condition of default routing

        // Update routing information if the route exists or it is a default route
        if (default_route || (route_exists && prefix_length >= max_prefix_length)) {
            interface_num = entry.interface_num;  // Matched interface number
            dest_address = entry.next_hop.has_value()
                               ? entry.next_hop.value()
                               : Address::from_ipv4_numeric(dest_ip);  // Matched next hop or destination
            max_prefix_length = prefix_length;                         // Update the maximum prefix length
        }
    }

    // Send the datagram to the next hop or the destination
    if (dest_address.has_value()) {
        interface(interface_num).send_datagram(dgram, dest_address.value());
    }

    return;
}

void Router::route() {
    // Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());
            queue.pop();
        }
    }
}
