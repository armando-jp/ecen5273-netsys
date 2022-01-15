from socket import *

# Configure server socket for UDP
serverPort = 12000
serverSocket = socket(AF_INET, SOCK_DGRAM)
serverSocket.bind(('', serverPort))
print ("The server is ready to receive")


while True:
    # Receive message from client and generate uppercase version
    message, clientAddress = serverSocket.recvfrom(2048)
    modifiedMessage = message.decode().upper()
    print(modifiedMessage)

    # Send modified message to client.
    serverSocket.sendto(modifiedMessage.encode(), clientAddress)

