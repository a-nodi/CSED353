Assignment 7 Writeup
=============

My name: Yoonhyeok Lee

My POVIS ID: leeyoonhyuk0

My student ID (numeric): 20220923

My assignment partner's name: Mingyu Nam

My assignment partner's POVIS ID: mgnam

My assignment partner's ID (numeric): 20220887

This assignment took me about [1] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Solo portion:
- Did your implementation successfully start and end a conversation with another copy of itself?
    Yes, Running ./apps/lab7 server tomahawk.postech.ac.kr 63950 and ./apps/lab7 client tomahawk.postech.ac.kr 63951 made a successful connection.
    passing message was successful too.

- Did it successfully transfer a one-megabyte file, with contents identical upon receipt?
    Yes, Running ./apps/lab7 server tomahawk.postech.ac.kr 63950 < /tmp/big.txt and </dev/null ./apps/lab7 client tomahawk.postech.ac.kr 63951 > /tmp/big-received.txt
    made a successful connection.
    passing 1MB file was successful too.
    The sha256sum results are like this.
    f977ea12fa6f6fdb5194189249be4dad305d3633adf35b25652aa6a60264d3b7  /tmp/big.txt
    f977ea12fa6f6fdb5194189249be4dad305d3633adf35b25652aa6a60264d3b7  /tmp/big-received.txt

- Please describe what code changes, if any, were necessary to pass these steps.
    No code change needed for passing the steps.

Group portion:
- What is your team’s name? Who is your partner (and what is their POVIS ID)?
    The team name is 'Hamming'.

- Did your implementations successfully start and end a conversation with each other (with each implementation acting as “client” or as “server”)?
    Yes, Running ./apps/lab7 server tomahawk.postech.ac.kr 63950 and ./apps/lab7 client tomahawk.postech.ac.kr 63951 made a successful connection.
    passing message was successful too.

- Did you successfully transfer a one-megabyte file between your two implementations, with contents identical upon receipt?
    Yes, When I have ran ./apps/lab7 server tomahawk.postech.ac.kr 63950 < /tmp/big.txt and partner ran  </dev/null ./apps/lab7 client tomahawk.postech.ac.kr 63951 > /tmp/big-received.txt,
    it made a successful connection. passing 1MB file was successful too. 
    The sha256sum results are like this.
    b2c53158bd2e8a7c867d4e1b25d999c1d23cde314c5c490eb9297a8b116f8ac6  /tmp/big.txt
    b2c53158bd2e8a7c867d4e1b25d999c1d23cde314c5c490eb9297a8b116f8ac6  /tmp/big-received.txt

When My partner have ran ./apps/lab7 server tomahawk.postech.ac.kr 63950 < /tmp/big.txt and i ran  </dev/null ./apps/lab7 client tomahawk.postech.ac.kr 63951 > /tmp/big-received.txt,
    it made a successful connection. passing 1MB file was successful too. 
    The sha256sum results are like this.
    eead977b99ff2e2a4c1b4a6c64acad5db96c7e5791657de51b683990669c0008  /tmp/big.txt
    eead977b99ff2e2a4c1b4a6c64acad5db96c7e5791657de51b683990669c0008  /tmp/big-received.txt 

- Please describe what code changes, if any, were necessary to pass these steps, either by you or your partner.
    No code change need for passing the steps.

Creative portion (optional):
[]

Other remarks:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
