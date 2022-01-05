import socket
import threading

from packet import Packet
from configHandler import loadConfigData
from utils import getPublicIP, getLocalIP

from game import Game

class Server:
    def __init__(self):
            self.game = Game(self)
            self.colors = loadConfigData()["bots"]
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
            DISCONNECT_MESSAGE = mainConfig["DISCONNECT_MESSAGE"]
            SERVER_ADDRESS = (SERVER_IP, PORT)

            server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            server.bind(SERVER_ADDRESS)

            print("[STARTING] Server is starting...")

            server.listen()
            print("[LISTENING] Server is listening on IP " + str(SERVER_IP) + " and port " + str(PORT))

            thread = threading.Thread(target = acceptClients)
            thread.start()

    def acceptClients(self):
            while True:
                    connection, address = server.accept()
                    self.connections.append(connection)
                    thread = threading.Thread(target=handleClient, args=(connection, address, DISCONNECT_MESSAGE))
                    thread.start()
                    print("[ACTIVE CONNECTIONS] " + str(threading.activeCount() - 1))

    def handleClient(self, connection: socket.socket, address, disconnect_message):
            print("[NEW CONNECTION] " + str(address) + " connected.")
            
            connected = True

            player = self.game.newPlayer("Player{}".format(threading.activveCount()-1), connection)

            send(connection, , 1)
            send(connection, , 2)
            
            while connected:
                    messageType, message = receive(connection, 1024)
                    if messageType == 0:
                            print("[DISCONNECT] Message to disconnect received, disconnecting " + str(address))
                            print("[ACTIVE CONNECTIONS] " + str(threading.activeCount() - 1))
                            connected = False
                            send(connection, "Disconnecting due to such request", 255)
                            break
                    if messageType == 1:
                        player.name = message
                    if messageType == 2:
                        if(not self.game.showing == None):
                            send(connection, "Cannot be done now. Please use an official client.", 255)
                            continue
                        self.game.wayFound(player, message)
                    if messageType == 3:
                        if(self.game.showing == player):
                            self.game.move()
                        else:
                            send(connection, "You are not showing right now. Please use an official client.", 255)
                    if messageType == 4:
                        if(self.game.showing == player):
                            self.game.revert()
                        else:
                            send(connection, "You are not showing right now. Please use an official client.", 255)
                    if messageType == 5:
                        if(self.game.showing == player):
                            self.game.reset()
                        else:
                            send(connection, "You are not showing right now. Please use an official client.", 255)
                    if messageType == 6:
                        if(self.game.showing == player):
                            self.game.move()
                        else:
                            send(connection, "You are not showing right now. Please use an official client.", 255)

                    print("[" + str(address) + "] " + str(message))
                    send(connection, "Message received", 255)

            connection.close()

    def sendToAll(self, message, messageType: int):
        for connection in connections:
            send(connection, message, messageType)

    def send(self, connection: socket.socket, message, messageType: int):
            packet = Packet(messageType, message)
            connection.send(packet.pack())

    def receive(self, connection: socket.socket, length: int):
            received_packet = connection.recv(length)
            pckt = Packet.unpack(received_packet)
        
            return pckt.type, pckt.extract()
