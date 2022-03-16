Author  : Armando Pinales
Date    : 02/13/2022
Course  : ECEN 5273
Semester: Spring 2022

University of Colorado Boulder



How to build:
1) make
2) ./webserver [port number]
3) type "localhost:[port number] in web browser
4) enjoy

How to test POST:
1. (echo -en "POST /index.html HTTP/1.1\r\nHost: localhost\r\nConnection: Keep-alive\r\n\r\nPOSTDATA") | nc 127.0.0.1 60002

How to test pipelining
1. (echo -en "GET /index.html HTTP/1.1\r\nHost: localhost\r\nConnection:
Keep-alive\r\n\r\n"; sleep 10; echo -en "GET /index.html HTTP/1.1\r\nHost:
localhost\r\n\r\n") | nc 127.0.0.1 60002