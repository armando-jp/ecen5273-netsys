Author  : Armando Pinales
Date    : 02/13/2022
Course  : ECEN 5273
Semester: Spring 2022

University of Colorado Boulder

[Reliability]
This application is required to implement a reliable transfer protocol ontop of
UDP. This was accomplished through the use of:
    * send-and-wait algorithm
    * ACKs
    * CRC32
    * sequence numbers

All packets that are transmitted between the server and client application have
the following structure:

================================================================================
DATA TYPE    :    (uint32_t)        (uint32)     (char)  (uint32_t)
PACKET VISUAL: [ SEQUENCE NUMBER | PAYLOAD SIZE | PAYLOAD | CRC32 ]
SIZE (BYTES) :          4                4         <=500      4
================================================================================
As a result, the maximum possible size of a packet is approximately 512 bytes.
Smaller packets are possible if the payload is smaller. The smallest possible
packet in theory is 12 bytes.

The crc32 is generated using the bytes in the SEQUENCE NUMBER, PAYLOAD SIZE, and
PAYLOAD fields. The client receiving the packet can then compare the CRC32 field
in the packet to the CRC32 that it generates based on the received packet. If
the CRC32 values match, then it is known that the packet was received without
any transmission errors.

Upon receiving a packet with no errors, and assuming that the packet is the
expected packet and not a duplicate (duplicates are known by comparing the
SEQUENCE NUMBER field to the previous packets SEQUENCE NUMBER), and ACK is sent.
ACK packets are structed exactly as described above, the only difference is the
payload filed contains the string "ACK" (no null termination).



[Client Description]
The client application is composed of a while loop which waits for a user to
enter a command. Once a valid command is detected, the client will run the
approriate command. If an invalid command is detected, the user is prompted with
an "invalid command entered message" and is given another opprotunity to enter a
correct command. There are 5 possible commands:

[1] get
The 'get' command takes a single argument which represents the desired file to be
transfered from the server to the client. For this command, a state machine has
been implemented which cycles through the necessary states in order to implement
a relibale transfer protocol with the server. 

[2] put
The 'put' command takes a single argument with represents the file on the client
which is to be transfered to the server. For this command, a state machine has
been implemented in order to implement a reliable transfer protocol with the server.

[3] delete
The 'delete' command takes a single argument which represents the file on the
server which is to be deleted. An ACK from the server indicates that the the
command was sucessfully received. 

[4] ls
The 'ls' command sends a request that the server transmits file infomration
relating to files in its local directory. A state machine has been implemented
to ensure a relibale data transmission. 

[Server Description]
The server application is composed of a while loop which waits to receive a
command packet from the client. When a command packet is received, the requested
command is extracted from the payload as well as any arguments. The application
then enters the appropriate routine to handle the request from the client. After
completing the request, the client returns to the main loop and waits for a new
command from a client.

[1] get
The 'get' command takes a single argument which denotes the file to be sent to
the client. A state machine has been implemented to handle the server-side file
transfer procedure. The application will transmit the file and return to waiting
for a new command upon completion.

[2] put
The 'put' command takes a single argument which denotes the file that will be
written locally. A state machine has been implemented to handle the server-side
file receiving procedure. Once the server receives the final ACK which signals
the end of the file transfer, it will close the newly made file and return to
waiting for a new command.

[3] delete
The delete command takes a single argument which denotes the file to delete
locally. The file is deleted and an ACK is sent to the server which dentes
sucess. The server then returns to a waiting state for a new comand.

[4] ls
The ls command instructs the server to create a payload which contains the local
directory contents. Once the contents have been generated, they are transmitted
to the client. Upon the final exchange of ACKs, the server returns to a waiting
state for a new command.

[5] exit
The exit command indicates to the server that it should exit gracefully. Before
the server exits, an ACK is sent to the client to indicate that it the command
was received and that it will terminate. This ends the server application.


[How to Run]
    1. Navigate to the directory containing the makefile.
    2. Run 'make'
    3. Run either server_app or client_app.
    4. Repeat steps for second machine, but run the opposite application that
       was run on the first machine.