import threading

from board import Board
from packet import Packet
from configHandler import loadConfigData
from server import Server
from utils import getPublicIP, getLocalIP

def main():
	global board, server
	board = Board()
	board.generateBoard()
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
	server = Server(SERVER_IP, PORT)
	server.conCallback = clientConCallback
	server.clientThrCallback = recvFromClient
	server.disconCallback = clientDisconCallback
	print("[STARTING] Server is starting...")
	print("[LISTENING] Server is listening on IP " + str(SERVER_IP) + " and port " + str(PORT))
	server.acceptClients()
def clientConCallback(sck, address, id):
	print("[NEW CONNECTION] " + str(address) + " connected with ID " + str(id) + ".")
	print("[ACTIVE CONNECTIONS] " + str(threading.activeCount()))
def recvFromClient(sck, address, id):
	send(sck, 1, board.getWalls()) # walls
	connected = True
	while connected:
		messageType, message = receive(sck, 1024)
		if messageType == 0: # disconnect
			print("[DISCONNECT] Message to disconnect received, disconnecting " + str(address))
			connected = False
			break
		elif messageType == 255:
			print("[CHAT] Message from [" + str(address) + "]: " + message)
			broadcast(255, message)
		else:
			print("received unknown packet from [" + str(address) + "]: " + str(message))
def send(sck, messageType, message):
	p = Packet(messageType, message)
	server.send(sck, p.pack())
def broadcast(messageType, message):
	p = Packet(messageType, message)
	server.broadcast(p.pack())
def receive(sck, length: int):
	received_packet = sck.recv(length)
	pckt = Packet.unpack(received_packet)
	return pckt.type, pckt.extract()
def clientDisconCallback(sck, address, id):
	print("[DISCONNECTION] " + str(address) + "(ID " + str(id) + ") disconnected.")
	print("[ACTIVE CONNECTIONS] " + str(threading.activeCount() - 2))
if __name__ == "__main__":
	main()
