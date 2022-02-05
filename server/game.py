import bots
from board import Board
import random
import time

class Game():
	def __init__(self, server):
		self.board = Board()
		self.server = server
		self.targets = []
		for tile in self.board.tiles:
			if self.board.tiles[tile][0] != "" and self.board.tiles[tile][0] != "middle":
				self.targets.append(self.board.tiles[tile][0])
		self.players = []
		self.giveUp = False
		self.target = None
		self.found = []
		self.showing = None
		self.started = False
	def start(self):
		while input() != "START":
			pass
		self.server.broadcast(1, self.board.generator.getWalls())
		self.server.broadcast(2, self.board.defaultCoords)
		self.started = True
		self.startTurn()
	def newPlayer(self, name, clientID):
		p = Player(name, clientID)
		self.players.append(p)
		if self.started:
			self.server.send(clientID, 1, self.board.generator.getWalls())
			self.server.send(clientID, 2, self.board.defaultCoords)
			targetpos = next(pos for pos, tile in self.board.tiles.items() if tile[0] == self.target)
			self.server.send(clientID, 3, bytearray([targetpos[0] << 4 | targetpos[1], bots.all_str.index(self.target.split("-")[0])]))
		return p
	def startTurn(self):
		self.target = random.choice(self.targets)
		targetpos = next(pos for pos, tile in self.board.tiles.items() if tile[0] == self.target)
		self.server.broadcast(3, bytearray([targetpos[0] << 4 | targetpos[1], bots.all_str.index(self.target.split("-")[0])]))
		self.found = []
		self.showing = None
		while len(self.found) == 0:
			time.sleep(1)
		time.sleep(60)
		self.showtime()
	def wayFound(self, player, length):
		self.found.append(Way(player, length))
		#self.server.broadcast(16, ) TODO: send found way
	def showtime(self):
		shortestpath = min(map(lambda x : x.length, self.found))
		i = next(i for i, x in enumerate(self.found) if x.length == shortestpath) # FIXME: player who found the path first goes first on a tie
		self.showing = self.found[i].player
		self.found.pop(i)
		self.server.send(self.showing.clientID, 32, b"")
		j = 0
		while j < 60 and self.giveUp == False:
			time.sleep(1)
		self.stop()
	def move(self, bot, direction):
		self.board.moveBot(bot, direction)
		if self.board.tiles[self.board.bots[self.target.split("-", 1)[0]]][0] == self.target:# or self.board.tiles[self.board.bots[bots.black]] == self.target: # FIXME: black robot isn't universal, but black target is
			self.endTurn(self.showing)
		#self.server.broadcast(18, ) TODO: send move
	def revert(self):
		self.board.revert()
		#self.server.broadcast(19, ) TODO: send revert
	def reset(self):
		self.board.reset()
		#self.server.broadcast(20, ) TODO: send reser
	def stop(self):
		self.server.send(self.showing.connection, 0, 33)
		self.showing = None
		self.giveUp = False
		self.showtime()
	def endTurn(self, winner):
		if winner is not None:
			winner.points += 1
		if len(self.targets) == 0:
			self.__init__() # FIXME: regenerating board every time??? (the board should be same until you run out of targets)

class Player():
	def __init__(self, name, clientID):
		self.name = name
		self.clientID = clientID
		self.points = 0

class Way():
	def __init__(self, player, length):
		self.player = player
		self.length = length
