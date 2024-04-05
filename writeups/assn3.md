Assignment 3 Writeup
=============

My name: Yoonhyeok Lee

My POVIS ID: leeyoonhyuk0

My student ID (numeric): 20220923

This assignment took me about 16 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPSender:  
I Designed the RetransmissionTimer and the TCPSender Seperately.  
TCPSender has a RetrnasmissionTimer as a member instance and controls it to keep track of retransmission time.  
The working pipeline of TCPSender is like this.  
First, it sends SYN packet to estabilish the TCP connection
Second, it repeatly slices the stream and wraps it as payload, mark seqno and sends it to receiver.  
In Addition, if the segment is last payload of stream, it marks FIN flag to segment.  
Third, tick() updates RetransmissionTimers time.  
If retransmission expire or ack happens, it manupulates its timer.  
Forth, if ack receives, it updates the state of tracking outgoing segments. 


Implementation Challenges:  
There were so many test cases I didn't considered during implementations.  
For example, I didn't considered bytes of outstanding segments that haven't acked and received by receiver.  
In this case, I had to subtract bytes of outstanding segments from sendable bytes.  
Another example, I didn't considered duplicate acknos and invalid acknos.  
I wrote some early terminations when those acknos has been received.     
Debuging the code with testcase codes, I was able to fix flaws of my TCPSender.  

Remaining Bugs:
No Bugs Remaining

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
