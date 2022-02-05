import socket

from packet import Packet

class Client():
	def __init__(self, port, server_ip, server_address) -> None:
		self.client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.port = port
		self.server_ip = server_ip
		self.server_address = server_address
		self.client.connect(self.server_address)

	def send(self, message: str, messageType: int = 255) -> None:
		packet = Packet(messageType, message)
		packed = packet.pack()
		self.client.send(packed)

	def receive(self, length: int = 1024) -> tuple:
		received_packet = self.client.recv(length)
		pckt = Packet.unpack(received_packet)
		return pckt.type, pckt.extract()