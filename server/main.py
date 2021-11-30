import socket
import threading

from board import Board
from packet import Packet
from configHandler import loadConfigData
from utils import getPublicIP, getLocalIP

def main():
	global board
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
	SERVER_ADDRESS = (SERVER_IP, PORT)

	server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	server.bind(SERVER_ADDRESS)

	print("[STARTING] Server is starting...")

	server.listen()
	print("[LISTENING] Server is listening on IP " + str(SERVER_IP) + " and port " + str(PORT))

	while True:
		connection, address = server.accept()
		thread = threading.Thread(target=handleClient, args=(connection, address))
		thread.start()
		print("[ACTIVE CONNECTIONS] " + str(threading.activeCount() - 1))

def handleClient(connection: socket.socket, address):
	print("[NEW CONNECTION] " + str(address) + " connected.")
	
	send(connection, board.getWalls(), 1) # walls

	connected = True
	while connected:
		messageType, message = receive(connection, 1024)
		if messageType == 0:
			print("[DISCONNECT] Message to disconnect received, disconnecting " + str(address))
			connected = False
			#send(connection, "Disconnecting due to such request", 255)
			break

		print("[" + str(address) + "] " + str(message))
		send(connection, "Message received", 255)

	connection.close()

def send(connection: socket.socket, message, messageType: int):
	packet = Packet(messageType, message)
	connection.send(packet.pack())

def receive(connection: socket.socket, length: int):
	received_packet = connection.recv(length)
	pckt = Packet.unpack(received_packet)
	return pckt.type, pckt.extract()

if __name__ == "__main__":
	main()
