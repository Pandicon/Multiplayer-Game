import socket
import threading

class Server:
	def __init__(self, ip, port):
		self.sck = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.sck.bind((ip, port))
		self.sck.listen()
		self.conCallback = None
		self.clientThrCallback = None
		self.disconCallback = None
		self.clients = { }
		self.nextClID = 0
	def acceptClients(self):
		while True:
			sck, address = self.sck.accept()
			if self.conCallback is not None:
				self.conCallback(sck, address, self.nextClID)
			thr = threading.Thread(target=self.clientThr, args=(sck, address, self.nextClID))
			self.nextClID += 1
			thr.start()
	def acceptClientsAsync(self):
		thr = threading.Thread(target=self.acceptClients)
		thr.start()
		return thr
	def clientThr(self, sck, address, id):
		self.clients[id] = (sck, address)
		try:
			if self.clientThrCallback is not None:
				self.clientThrCallback(sck, address, id)
		except Exception as e:
			del self.clients[id]
			sck.close()
			raise e
		if self.disconCallback is not None:
			self.disconCallback(sck, address, id)
		del self.clients[id]
		sck.close()
	def send(self, client, data):
		if type(client) == int:
			self.clients[client][0].send(data)
		client.send(data)
	def broadcast(self, data):
		for id, cl in self.clients.items():
			cl[0].send(data)
