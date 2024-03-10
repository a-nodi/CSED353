Assignment 1 Writeup
=============

My name: Yoonhyeok Lee

My POVIS ID: leeyoonhyuk0

My student ID (numeric): 20220923

This assignment took me about [32] hours to do (including the time on studying, designing, and writing the code).

Program Structure and Design of the StreamReassembler:
[I designed auxiliary storage using string as a byte array to contain the substrings that are not assembled.  
 First, the program checks the current substring is eof. If so, it marks the eof index  
 Second, the program checks the early termination conditions.  
 third, store the each byte of substring into aux storage.
 forth, calculate the length of assemblable substrings
 fitfh, update the aux storage and related parameters
 sixth, mark eof if the eof byte has entered the output stream
 ]

Implementation Challenges:
[At first time, i used priority queue as a aux storage to sort the substrings.  
 But there was a problem when the code performs a concatenation of overlapping substrings.  
 So i had to change the priority queue to unordered map
 
 Updated Challenges
 I found that in my VM, my code gives timeout for some heavy tests.  
 I didn't noticed that because i tested my code in my linux machine, not VM
 I had to changed my aux storage from unordered map to string.  
 ]  

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [processing overlapping substrings.]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: []
,