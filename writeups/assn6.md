Assignment 6 Writeup
=============

My name: Yoonhyeok Lee

My POVIS ID: leeyoonhyuk0

My student ID (numeric): 20220923

This assignment took me about 10 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the Router:
    There are two main methods of router class. The description of below is the explanation of the methods.

    1. void Router::route_one_datagram(InternetDatagram &dgram)
    First, Create route_entry struct from given arguments.
    Second, push back route_entry to routing_table

    2. void Router::route_one_datagram(InternetDatagram &dgram)
    First, Decrease given datagram's ttl(time to live) by 1.
    Second, if given datagram's ttl is 0, drop the datagram and early terminate the method.
    Third, for all route_entry in routing table, check the matching condition

    the matching condition is like this
    - 1-1. the prefix and the subnet-masked destination ip address matchs
    - 1-2. current prefix's length is longer than the last matched longest prefix's length
    - 2. default routing: prefix and length of prefix is 0

    Forth, if the destination ip address and prefix matchs, perform longest prefix matching
    Fifth, if the matched prefix exists, send datagram with matched route entry.

Implementation Challenges:
    Handling default route was challenging.
    I spent 2 hours to debug the default routing.
    I added default routing condition to solve the issue. 
    Also there was a overflow on ttl, 
    which solved by giving condition to decrease the ttl.

Remaining Bugs:
    Potential bugs that randomly occur in first make remaining. Details are in the optional part. 

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: 
    Why there are one or two testcases fails randomly if i perform  
    "make clean", "make", "make check_lab4".  
    At first make, some testcase takes long time and gives a timeout.  
    But after the second make, it doesn't gives a timeout. it works fine. 
    I guess maybe there are some caching performed after the first make.
