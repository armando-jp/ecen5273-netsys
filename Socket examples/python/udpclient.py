from socket import *

# Server paramters
# Connect to server using UDP
serverName = 'localhost'
serverPort = 12000
clientSocket = socket(AF_INET, SOCK_DGRAM)

# Obtain user input and send to server
message = input('Input lowercase sentence:')
clientSocket.sendto(message.encode(), (serverName, serverPort))

# Receive response from server and print
modifiedMessage, serverAddress = clientSocket.recvfrom(2048)
print(modifiedMessage.decode())

# Close the client socket.
clientSocket.close()
