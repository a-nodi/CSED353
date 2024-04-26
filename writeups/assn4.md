Assignment 4 Writeup
=============

My name: Yoonhyeok Lee

My POVIS ID: leeyoonhyuk0

My student ID (numeric): 20220923

This assignment took me about 60 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Your benchmark results (without reordering, with reordering): [0.68, 1.24]

Program Structure and Design of the TCPConnection:
I Designed TCPConnection using TCPReceiver and TCPSender. 
These are the details of the essential part of TCPConnection.  

#### If TCPConnection receives segment from remote,  
It receives the segment if the Connection is active and the segment is valid.  
After the validation check, it updates the sender to prepare the segments that should be sent in next sending.  
And creates an ACK segment to reply the remote peer.  
It sends the segment and attempts to perform clean shutdown.  

#### If Time passes(by calling "tick" function), 
First, it checks the retransmission count.  
If retransmission counts are too much, it attempts an unclean shutdown.  
If not, it sends retranmission segments and attempts a clean shutdown.  

#### If TCPConnection sends segment to remote,
It pops the all or the segments from the TCPSender's queue and pushes the segments into queue itself.  
while pushing the segment, It sets RST Flag, ACK Flag, Ackno, and window size of local peer.  

#### If TCPConnction Attempts to shutdown,
It checks whether the shutdown should be clean shutdown or unclean shutdown.  
If clean shutdown, it checks the prerequisites to determine clean shutdown should be performed.  
If the prerequisite are all true, it inactivates the linger and itself.  
If unclean shutdown, it inactivates the connection and send RST segment(if no segments left, it creates one) to the remote peer.  

I made some helper function that determines the True or False of prerequisites.  

Implementation Challenges:  
There were multiple edge cases in real-life TCP connections.  
I had to check all the cases that can happen, and then optimize the code.  
Optimizing sometimes gave and error, which was very brain bending.  

Remaining Bugs:
Potential bugs that randomly occur in first make remaining. Details are in the optional part. 

- Optional: I had unexpected difficulty with:  
    active close / passive close.  
    To debug the code, i had to open all of the test codes.  
    And see what happens in the test time.  
    

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about:  
    Why there are one or two testcases fails randomly if i perform  
    "make clean", "make", "make check_lab4".  
    At first make, some testcase takes long time and gives a timeout.  
    But after the second make, it doesn't gives a timeout. it works fine. 
    I guess maybe there are some caching performed after the first make.
