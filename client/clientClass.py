import socket
import pickle
import sys

class Client():
    def __init__(self, header, port, server_ip, encode_format, disconnect_message, server_address) -> None:
        self.client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.header = header
        self.port = port
        self.server_ip = server_ip
        self.encode_format = encode_format
        self.disconnect_message = disconnect_message
        self.server_address = server_address
        self.client.connect(self.server_address)

    def send(self, message):
        message = pickle.dumps(message)
        msg_length = sys.getsizeof(message)
        send_length = str(msg_length).encode(self.encode_format)
        send_length += b' ' * (self.header - len(send_length))
        self.client.send(send_length)
        self.client.send(message)

        response_length = int(self.client.recv(self.header).decode(self.encode_format))
        response = pickle.loads(self.client.recv(response_length))
        return response