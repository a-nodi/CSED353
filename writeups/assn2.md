Assignment 2 Writeup
=============

My name: [Yoonhyeok Lee]

My POVIS ID: [leeyoonhyuk0]

My student ID (numeric): [20220923]

This assignment took me about [16] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
[I designed TCPReceiver to receive segment and calculate the absolute sequence number of packet, and store it into stream reassembler if the syn packet has been received before handling the current packet. To calculate the absolute sequence number using relative stream index, I designed unwrapping function to convert the relative stream index to absolute sequence number, refurring to the assignment2 description pdf.  

Also I implemented the function that calculates the ackno. i used isn, stream index and syn/fin indicating variables to calculate the ackno.

]

Implementation Challenges:
[There were some problems with implicit type casts. My code gave an unexpected values during the test. It happens to be a implicit type casts or overflow problems(especially the test case  
         test_should_be(unwrap(WrappingInt32(UINT32_MAX), WrappingInt32(0), 0), static_cast<uint64_t>(UINT32_MAX));  
 in t_wrapping_ints_unwrap).  
 I used static_cast<>() function to explicity convert types of variables.  it worked pretty well i think. But sadly, the reability of the code got worse. However, I think there is nothing I can do because the operations of uint64 and uint32 are mixed.

 Also, It took me a while to read and understand the codes in tcp_helpers/and util/. It took me a while to write the code according to the given skeleton codes format because there were so many skeleton codes all of a sudden. There were some cases where the operator was well defined in the skeleton code, so it took me a while to edit the operations I wrote in my code to make it work efficiently.
]

Remaining Bugs:
[No Bug remaining.]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
