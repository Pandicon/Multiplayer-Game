import socket
import threading

from board import Board
from packet import Packet
from configHandler import loadConfigData

def main():
    board = Board()
    board.generateBoard()
    mainConfig = loadConfigData("../config.json")
    PORT = mainConfig["PORT"]
    SERVER_IP = socket.gethostbyname(socket.gethostname())
    DISCONNECT_MESSAGE = mainConfig["DISCONNECT_MESSAGE"]
    SERVER_ADDRESS = (SERVER_IP, PORT)

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(SERVER_ADDRESS)

    print("[STARTING] Server is starting...")

    server.listen()
    print(f"[LISTENING] Server is listening on IP {SERVER_IP} and port {PORT}")

    while True:
        connection, address = server.accept()
        thread = threading.Thread(target=handleClient, args=(connection, address, DISCONNECT_MESSAGE))
        thread.start()
        print(f"[ACTIVE CONNECTIONS] {threading.activeCount() - 1}")

def handleClient(connection: socket.socket, address: socket._RetAddress, disconnect_message):
    print(f"[NEW CONNECTION] {address} connected.")

    connected = True
    while connected:
        messageType, message = receive(connection, 1024)
        if message == disconnect_message:
            print(f"[DISCONNECT] Message to disconnect received, disconnecting {address}")
            connected = False
            send(connection, "Disconnecting due to such request", 255)
            break

        print(f"[{address}] {message}")
        send(connection, "Message received", 255)

    connection.close()

def send(connection: socket.socket, message: str, messageType: int):
    packet = Packet(messageType, message)
    packed = packet.pack()
    connection.send(packed)

def receive(connection: socket.socket, length: int):
    received_packet = connection.recv(length)
    packet = Packet()
    return packet.unpack(received_packet)

if __name__ == "__main__":
    main()