from board import Board
import random
import time

class Game():
	def __init__(self, server):
		self.board = Board()
		self.server = server
		#self.server.sendToAll(, 2) TODO send robot positions?
		self.targets = []
		for tile in self.board.tiles: # FIXME: middle squares have alse text "middle"
			if not self.board.tiles[tile][0] == "":
				self.targets.append(self.board.tiles[tile][0])
		self.players = []
		self.giveUp = False
		while not input() == "START":
			pass
		self.startTurn()
	def newPlayer(self, name, clientID):
		p = Player(name, clientID)
		self.players.append(p)
		return p
	def startTurn(self):
		self.target = random.choice(self.targets)
		self.found = []
		self.showing = None
		while len(self.found) == 0:
			time.sleep(1)
		time.sleep(60)
		self.showtime()
	def wayFound(self, player, lenght):
		self.found.append(Way(player, lenght))
		#self.server.broadcast(16, ) TODO: send found way
	def showtime(self):
		i = self.found.index(min(map(lambda x : x.lenght, self.found))) # FIXME: player who found the path first goes first on a tie
		self.showing = self.found[i].player
		self.found.pop(i)
		self.server.send(self.showing.id, 32, b"")
		j = 0
		while j < 60 and self.giveUp == False:
			time.sleep(1)
		self.stop()
	def move(self, bot, direction):
		self.board.moveBot(bot, direction)
		if self.board.tiles[self.board.bots[self.target.split("-", 1)[0]]][0] == self.target:# or self.board.tiles[self.board.bots[black]] == self.target: # FIXME: black robot isn't universal, but black target is; also FIXME: black is not defined?
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
	def __init__(self, player, lenght):
		self.player = player
		self.lenght = lenght
