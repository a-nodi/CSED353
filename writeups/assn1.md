Assignment 1 Writeup
=============

My name: Yoonhyeok Lee

My POVIS ID: leeyoonhyuk0

My student ID (numeric): 20220923

This assignment took me about [24] hours to do (including the time on studying, designing, and writing the code).

Program Structure and Design of the StreamReassembler:
[I used unordered map as an auxiliary storage to contain the substrings that are not assembled
 First, I check the indices of each byte of the input substring to check it can be stored in the auxiliary storage.
     Condition to be stored in the aux storage:
     - index should be bigger then the index of the first byte of unassembled substring
     - index should be smaller then the maximum index calculated by capacity
     - index is not already in the auxilary storage 
 Second, If the byte can be stored in the aux storage, store it with content of the byte and flag that is eof or not.
 Third, Write the each byte to bytestream if the end byte of bytestream is neighbor of the first byte of current unassembled substring.
 Forth, If the current unassembled substring is end of file, give bytestream a eof.
 Fifth, Delete the concatenated byte from the aux storage.
 Sixth, Update the parameters related with unassembled substring. 
 ]

Implementation Challenges:
[At first time, i used priority queue as a aux storage to sort the substrings.
 But there was a problem when the code performs a concatenation of overlapping substrings.
 So i had to change the priority queue to unordered map]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [processing overlapping substrings.]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [In real environment, this stream reassembler can operate perfactly and efficiently. there are some test that takes some times. However, it passes the timeout test. I think it will mostly work well.]
,