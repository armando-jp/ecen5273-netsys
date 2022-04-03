Author  : Armando Pinales
Date    : 03/16/2022
Course  : ECEN 5273
Semester: Spring 2022

University of Colorado Boulder



How to build:
1) cd into the assignment3/submission directory. 
2) make clean
3) make

How to run:
1) ./webserver 60002

How to test in a second terminal:
1) (echo -en "GET http://netsys.cs.colorado.edu HTTP/1.0\r\nHost: netsys.cs.colorado.edu\r\n\r\n") | nc 127.0.0.1 60002

How to test in browser (Mozilla Firefox on Windows)
1) Use VPN to access school network (if not already on campus)
2) Configure proxy settings for Firefox
3) Go to http://netsys.cs.colorado.edu in forefox.


[Commands to tar]
    1. tarm-cf [file_name.tar] [file_1] [directory_1] ... [file_n] [directory_n]

[Commmand to un-tar]
    1. tar -xf [file_name.tar] -C /desired/path