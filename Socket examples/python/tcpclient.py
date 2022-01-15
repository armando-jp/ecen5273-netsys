from socket import *

# Server Info
# Name = 'localhost'
# Port = 12000
# Connection type: TCP
serverName = 'localhost'
serverPort = 12000
clientSocket = socket(AF_INET, SOCK_STREAM)

# Connect to server
clientSocket.connect((serverName,serverPort))

#Obtain user input and send to server
sentence = input('Input lowercase sentence:')
clientSocket.send(sentence.encode())

# Receive the modified sentence from the server and print
modifiedSentence = clientSocket.recv(1024)
print ('From Server:', modifiedSentence.decode())

# Close the socket
clientSocket.close()

