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
		self.pathlen = 0
		self.roundend = False
	def start(self):
		while input() != "START":
			pass
		self.server.broadcast(1, self.board.generator.getWalls())
		self.server.broadcast(2, self.board.defaultCoords)
		self.started = True
		while True:
			self.startTurn()
	def newPlayer(self, name, clientID):
		p = Player(name, clientID)
		self.players.append(p)
		if self.started:
			self.server.send(clientID, 1, self.board.generator.getWalls())
			time.sleep(0.05) # helps to prevent packet mixing
			self.server.send(clientID, 2, self.board.defaultCoords)
			time.sleep(0.05)
			targetpos = next(pos for pos, tile in self.board.tiles.items() if tile[0] == self.target)
			self.server.send(clientID, 3, (targetpos, self.target))
		return p
	def startTurn(self):
		self.target = random.choice(self.targets)
		targetpos = next(pos for pos, tile in self.board.tiles.items() if tile[0] == self.target)
		self.server.broadcast(3, (targetpos, self.target))
		print(self.target)
		self.found = []
		self.showing = None
		self.roundend = False
		while len(self.found) == 0:
			time.sleep(1)
		time.sleep(60)
		self.server.broadcast(17, b"")
		self.showtime()
	def wayFound(self, player, length):
		self.found.append(Way(player, length))
		print(player.name + " has found a way with length " + str(length))
		self.server.broadcast(16, (player.name, length))
	def showtime(self):
		if len(self.found) == 0:
			self.endTurn(None)
			return
		i, way = min(enumerate(self.found), key=lambda x: x[1].length)
		self.showing = self.found[i].player
		if self.showing.clientID not in self.server.clients:
			self.found.pop(i)
			self.showtime()
			return
		self.pathlen = way.length
		self.found.pop(i)
		self.server.send(self.showing.clientID, 32, b"")
		for pl in self.players:
				self.server.send(pl.clientID, 34, b"")
			if pl.clientID != self.showing.clientID and pl.clientID in self.server.clients:
		j = 0
		while j < 600 and not self.giveUp and not self.roundend:
			time.sleep(0.1)
			j += 1
		if self.roundend:
			return
		self.stop()
	def move(self, bot, direction):
		self.board.moveBot(bot, direction)
		self.server.broadcast(18, (bot, self.board.botCoords[bot]))
		relevantbots = [b for i, b in enumerate(self.board.botCoords) if i == bot and (self.target == "spiral" or self.target.startswith(bots.botstr(i)))]
		if self.target in [self.board.tiles[b][0] for b in relevantbots] and len(self.board.history) - 1 <= self.pathlen:
			self.endTurn(self.showing)
	def revert(self):
		self.board.revert()
		self.server.broadcast(19, b"")
	def reset(self):
		self.board.restart()
		self.server.broadcast(20, b"")
	def stop(self):
		self.board.restart()
		if self.showing.clientID in self.server.clients:
			self.server.send(self.showing.clientID, 33, b"")
		self.showing = None
		self.giveUp = False
		self.showtime()
	def endTurn(self, winner):
		self.board.restart()
		if winner is not None:
			winner.points += 1
			self.server.broadcast(64, winner.name)
			self.targets.remove(self.target)
		if len(self.targets) == 0:
			print("Game ends!")
			mostpts = max(self.players, key=lambda pl: pl.points).points
			winners = [pl.name for pl in self.players if pl.points == mostpts]
			if len(winners) > 1:
				winners[-2] = winners[-2] + " and " + winners[-1]
				del winners[-1]
			self.server.broadcast(65, ", ".join(winners))
			self.board = Board()
			self.targets = []
			for tile in self.board.tiles:
				if self.board.tiles[tile][0] != "" and self.board.tiles[tile][0] != "middle":
					self.targets.append(self.board.tiles[tile][0])
			for pl in self.players:
				pl.points = 0
		self.roundend = True

class Player():
	def __init__(self, name, clientID):
		self.name = name
		self.clientID = clientID
		self.points = 0

class Way():
	def __init__(self, player, length):
		self.player = player
		self.length = length
