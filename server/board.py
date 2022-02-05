from nsew import *
import bots
import random
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
				self.botCoords[bot] = (random.randint(0, 15), random.randint(0, 15)) # NOTE: you generate bot placements on two places, propably shoul make a function for that in board generator...
		self.defaultCoords = self.botCoords # FIXME: you propably want to copy the list (not just reference)
	def checkPlacement(self, bot):
		if self.tiles[self.botCoords[bot]][0] != "":
			return False
		for d in NSEW:
			if self.tiles[self.move(self.botCoords[bot], d)][0].split("-", 1)[0] == bots.botstr(bot):
				return False
		return True

	def moveBot(self, bot, direction):
		self.botCoords[bot] = self.move(self.botCoords[bot], direction)

	def revert(self):
		self.history.pop(-1)
		self.botCoords = self.history[-1]

	def restart(self):
		self.botCoords = self.defaultCoords
		self.history = [self.defaultCoords]
		
	def move(self, coords, direction):
		# TODO: add to history
		while not self.tiles[coords][1][NSEW.index(direction)]: # TODO: add robot collisions
			coords = addTuples(coords, direction)
		return coords
