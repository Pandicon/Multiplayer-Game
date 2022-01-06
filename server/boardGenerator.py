import math
import random
from configHandler import loadConfigData
import bots

class BoardGenerator():
	def __init__(self):
		self.boardLayout = []
		self.configData = loadConfigData()
		
	def generateBoard(self, wallsToBool = True):
		gameboardTemplate = self.configData["gameboard"]
		shuffledGameboardParts = gameboardTemplate
		random.shuffle(shuffledGameboardParts)
		generatedBoard = [[], [], [], []]
		for i in range(len(shuffledGameboardParts)):
			quarter = shuffledGameboardParts[i]
			for n in range(len(quarter)):
				quarter[n] = spinTile(quarter[n], i)
				if wallsToBool:
					quarter[n][1][1] = tileWallsStrToBool(quarter[n][1][1])
			generatedBoard[i] = quarter
		self.boardLayout = generatedBoard
		return self.getBoard()
	def generateBots(self):
		return [(random.randint(0, 15), random.randint(0, 15)) for _ in bots.all]
	def getBoard(self):
		w = {}
		for i, q in enumerate(self.boardLayout):
			for t in q:
				d = math.floor(abs(i - 1.5))
				x = t[0][0]+((1-d)*8)
				y = t[0][1]+(i//2*8)
				w[(x, y)] = t[1]
		for x in range(16):
			for y in range(16):
				if (x, y) not in w:
					w[(x, y)] = ["", [False, False, False, False]]
		return w

	
	def getWalls(self) -> list:
		# 2D array (16x16) of empty tiles
		w = [[[False, False, False, False] for _ in range(16)] for _ in range(16)]
		for i, q in enumerate(self.boardLayout):
			for t in q:
				d = math.floor(abs(i - 1.5))
				x = t[0][0]+((1-d)*8)
				y = t[0][1]+(i//2*8)
				w[x][y] = t[1][1]
		return w

def spinLetters(input: str, spin: int) -> str:
	output = ""
	letters = ["t", "r", "b", "l"]
	for letter in input:
		output += letters[(letters.index(letter)+spin)%4]
	return output

def spinCoords(input: list, spin: int) -> list:
	max = 7
	x = input[0]
	y = input[1]
	for i in range(spin%4):
		tempX = x
		x = max-y
		y = tempX
	return [x, y]

def spinTile(input: list, spin: int) -> list:
	coords = input[0]
	tileInfo = input[1]
	letters = tileInfo[1]
	coords = spinCoords(coords, spin)
	letters = spinLetters(letters, spin)
	tileInfo[1] = letters
	return [coords, tileInfo]

def tileWallsStrToBool(walls: str) -> list:
	output = []
	letters = ["t", "r", "b", "l"]
	for i in range(len(letters)):
		if letters[i] in walls:
			output.append(True)
		else:
			output.append(False)
	return output

def tileWallsBoolToStr(walls: list) -> str:
	output = ""
	letters = ["t", "r", "b", "l"]
	for i in range(len(walls)):
		if walls[i]:
			output += letters[i]
	return output
