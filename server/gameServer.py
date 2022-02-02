from time import sleep
import bots
from server import Server
from packet import Packet
from configHandler import loadConfigData
from utils import getPublicIP, getLocalIP

from game import Game

class GameServer(Server):
	def __init__(self):
		self.game = Game(self)
		self.colors = bots.all
		self.connections = []
		mainConfig = loadConfigData("../config.json")
		serverMode = mainConfig["SERVER_MODE"]
		PORT = mainConfig["PORT"]
		if serverMode == "localhost":
				SERVER_IP = "127.0.0.1"
		elif serverMode == "local":
				SERVER_IP = getLocalIP()
		elif serverMode == "public":
				SERVER_IP = getPublicIP()
		else:
			print("Unknown server mode! (", serverMode, ")")
			return
		super().__init__(SERVER_IP, PORT)
		self.conCallback = self.clientConnect
		self.clientThrCallback = self.listenFromClient
		self.disconCallback = self.clientDisconnect
		print("[STARTING] Server is starting...")
		print("[LISTENING] Server is listening on IP " + str(SERVER_IP) + " and port " + str(PORT))
		self.acceptClientsAsync()
		self.game.start()
	def clientConnect(self, sck, address, id):
		print("[NEW CONNECTION] " + str(address) + " connected with ID " + str(id) + ".")
		print("[ACTIVE CONNECTIONS] " + str(len(self.clients)+1))
	def listenFromClient(self, sck, address, id):
		pl = self.game.newPlayer("Player" + str(id), id)
		plref = str(address) + pl.name
		sleep(0.1)
		self.send(sck, 1, self.game.board.generator.getWalls()) # send walls
		# TODO: send bot positions
		connected = True
		while connected:
			messageType, message = self.receive(sck, 1024)
			if messageType == 0: # disconnect
				print("[DISCONNECT] Message to disconnect received, disconnecting " + plref)
				connected = False
				break
			if messageType == 1: # TODO: log all messages (with their own format by type)
				pl.name = message
				plref = str(address) + pl.name
			elif messageType == 2:
				if not self.game.showing == None:
					self.send(sck, "Cannot be done now. Please use an official client.", 255) # TODO: I am too lazy to make checks in client, do not send message saying "please use an official client"
					continue
				self.game.wayFound(pl, message)
			elif messageType == 3:
				if self.game.showing == pl:
					self.game.move() # FIXME: move needs arguments (bot and direction)
				else:
					self.send(sck, "You are not showing right now. Please use an official client.", 255)
			elif messageType == 4:
				if self.game.showing == pl:
					self.game.revert()
				else:
					self.send(sck, "You are not showing right now. Please use an official client.", 255)
			elif messageType == 5:
				if self.game.showing == pl:
					self.game.reset()
				else:
					self.send(sck, "You are not showing right now. Please use an official client.", 255)
			elif messageType == 6:
				if self.game.showing == pl:
					self.game.move()
				else:
					self.send(sck, "You are not showing right now. Please use an official client.", 255)
			elif messageType == 255:
				print("[CHAT] Message from [" + plref + "]: " + message)
				self.broadcast(255, "[" + pl.name + "] " + message)
			else:
				print("received unknown packet from [" + str(address) + "]: " + str(message))
	def clientDisconnect(self, sck, address, id):
		print("[DISCONNECTION] " + str(address) + "(ID " + str(id) + ") disconnected.")
		print("[ACTIVE CONNECTIONS] " + str(len(self.clients) - 1))
	def send(self, sck, data, msgdata=None):
		if msgdata is not None:
			p = Packet(data, msgdata)
			self.send(sck, data=p.pack())
		else:
			super().send(sck, data)
	def broadcast(self, data, msgdata=None):
		if msgdata is not None:
			p = Packet(data, msgdata)
			self.broadcast(data=p.pack())
		else:
			super().broadcast(data)
	def receive(self, sck, length):
		received_packet = sck.recv(length)
		pckt = Packet.unpack(received_packet)
		return pckt.type, pckt.extract()
