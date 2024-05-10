Assignment 5 Writeup
=============

My name: Yoonhyeok Lee

My POVIS ID: leeyoonhyuk0

My student ID (numeric): 20220923

This assignment took me about 18 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the NetworkInterface:
I designed Network interface with 3 main methods.

- void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop):  
First, it checks there is no Ip-ethernet mapping of given next-hop-ip and is ARP cooldown. If so, it early terminates itself.  
Second, it checks there is valid Ip-Ethernet mapping of given next-hop-ip. If so, it sends the given datagram to next hop ip.  
Third, it checks there is no Ip-Ethernet mapping of given next-hop-ip or not in ARP cooldown. If so, it send ARP request and store the datagram temporally.  

- optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame):  
First, it checks the given frame is destined for the network interface. If not, it early terminates itself.  
Second, it checks the given frame's protocal is IPv4 or ARP. If not, it early terminates itself.  
Third, it checks the given frame's protocal is IPv4 and parsed content has no error. If so, returns the given datagram.  
Forth, it checks the given frame's protocal is ARP and parsed content has no error. If so, it does the following execution.  
1. It saves the IP-Ethernet-time mapping to ARP table.  
2. It transmits the datagrams corresponing to current mapping that were stored temporally.  
3. Reply the ARP request if the frame was ARP request frame.  

- void NetworkInterface::tick(const size_t ms_since_last_tick):  
First, the function accumulated the time in accumulated_time variable.  
Second, check all of the ip-ethernet mappings is expired or not. If expired, remove the mapping from the ARP table.


I created helper methods to create Ethernet header, Ethernet frame, ARP message.

Implementation Challenges:
Understanding the ARP and Implementing it was quite chellenging.
I change the data structure several times to make the code clean and efficient.  
One of the case that was challenging is the managing time of ip-ethernet mapping and ARP response cooldown.   
At first time, I made ip-ethernet mapping and ip-timer mapping seperatly, but it got messy during the implementation.  
So i paired up ethernet and time with EthernetAndTime sturct to manage both of them at once. After pairing, the code got clean relatively.  

Remaining Bugs:
Potential bugs that randomly occur in first make remaining. Details are in the optional part. 

- Optional: I had unexpected difficulty with:  [describe]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: 
    Why there are one or two testcases fails randomly if i perform  
    "make clean", "make", "make check_lab4".  
    At first make, some testcase takes long time and gives a timeout.  
    But after the second make, it doesn't gives a timeout. it works fine. 
    I guess maybe there are some caching performed after the first make.
