from nsew import *
import bots
from boardGenerator import BoardGenerator

class Board:
	def __init__ (self):
		self.generator = BoardGenerator()
		self.tiles = self.generator.generateBoard()
		self.botCoords = []
		self.defaultCoords = []
		self.history = []
		self.placeBots()
		self.history = [self.defaultCoords]
	def placeBots(self):
		self.botCoords = self.generator.generateBots()
		for bot in bots.all:
			while not self.checkPlacement(bot):
				self.botCoords[bot] = self.generator.genBotPos()
		self.defaultCoords = [x for x in self.botCoords]
		self.restart()
	def checkPlacement(self, bot):
		if len([x for x  in self.botCoords if self.botCoords[bot] == x]) > 1:
			return False
		if self.tiles[self.botCoords[bot]][0] != "":
			return False
		for d in NSEW:
			if self.tiles[self.move(self.botCoords[bot], d)][0].split("-", 1)[0] == bots.botstr(bot):
				return False
		return True

	def moveBot(self, bot, direction):
		self.botCoords[bot] = self.move(self.botCoords[bot], direction)

	def revert(self):
		self.botCoords = self.history[-1]
		self.history.pop(-1)

	def restart(self):
		self.botCoords = [x for x in self.defaultCoords]
		self.history = [[x for x in self.defaultCoords]]
		
	def move(self, coords, direction, collisions=True):
		self.history.append([x for x in self.botCoords]);
		while not self.tiles[coords][1][NSEW.index(direction)]:
			prev = coords
			coords = addTuples(coords, direction)
			if collisions and coords in self.botCoords:
				return prev
		return coords
