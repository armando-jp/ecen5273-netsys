from socket import *

# Configure server to listen on port 12000
serverPort = 12000
serverSocket = socket(AF_INET,SOCK_STREAM)
serverSocket.bind(('',serverPort))
serverSocket.listen(1)
print('The server is ready to receive')


while True:
     # Accept incoming connection
     connectionSocket, addr = serverSocket.accept()

     # Decode incoming message and create uppercase version
     sentence = connectionSocket.recv(1024).decode()
     capitalizedSentence = sentence.upper()
     print(capitalizedSentence)

     # Send the capitalized sentence back to client
     connectionSocket.send(capitalizedSentence.encode())

     # Close the connection to client
     connectionSocket.close()

