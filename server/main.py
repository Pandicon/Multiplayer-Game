import socket
import pickle
import threading
import sys

from board import Board
from configHandler import loadConfigData

def main():
    board = Board()
    board.generateBoard()
    mainConfig = loadConfigData("../config.json")
    HEADER = mainConfig["HEADER"]
    PORT = mainConfig["PORT"]
    SERVER_IP = socket.gethostbyname(socket.gethostname())
    ENCODE_FORMAT = mainConfig["ENCODE_FORMAT"]
    DISCONNECT_MESSAGE = mainConfig["DISCONNECT_MESSAGE"]
    SERVER_ADDRESS = (SERVER_IP, PORT)

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(SERVER_ADDRESS)

    print("[STARTING] Server is starting...")

    server.listen()
    print(f"[LISTENING] Server is listening on IP {SERVER_IP} and port {PORT}")

    while True:
        connection, address = server.accept()
        thread = threading.Thread(target=handleClient, args=(connection, address, HEADER, ENCODE_FORMAT, DISCONNECT_MESSAGE))
        thread.start()
        print(f"[ACTIVE CONNECTIONS] {threading.activeCount() - 1}")

def handleClient(connection, address, header, encode_format, disconnect_message):
    print(f"[NEW CONNECTION] {address} connected.")

    connected = True
    while connected:
        message_length = connection.recv(header).decode(encode_format)
        if not message_length:
            print(f"[MESSAGE ERROR] Didn't receive a message, disconnecting client {address}")
            connected = False
            break
        message_length = int(message_length)
        message = pickle.loads(connection.recv(message_length))
        if message == disconnect_message:
            print(f"[DISCONNECT] Message to disconnect received, disconnecting {address}")
            connected = False
            send(connection, "Disconnecting due to such request", header, encode_format)
            break

        print(f"[{address}] {message}")
        send(connection, "Message received", header, encode_format)

    connection.close()

def send(connection, message, header, encode_format):
    message = pickle.dumps(message)
    msg_length = sys.getsizeof(message)
    send_length = str(msg_length).encode(encode_format)
    send_length += b' ' * (header - len(send_length))
    connection.send(send_length)
    connection.send(message)

if __name__ == "__main__":
    main()