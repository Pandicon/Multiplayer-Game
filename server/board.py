import math
import random
from configHandler import loadConfigData

class Board():
	def __init__(self):
		self.boardLayout = []
		
	def generateBoard(self, wallsToBool = True):
		configData = loadConfigData()
		gameboardTemplate = configData["gameboard"]
		shuffledGameboardParts = gameboardTemplate
		random.shuffle(shuffledGameboardParts)
		generatedBoard = [[], [], [], []]
		for i in range(len(shuffledGameboardParts)):
			quarter = shuffledGameboardParts[i]
			if quarter == 0:
				continue
			for n in range(len(quarter)):
				quarter[n] = spinTile(quarter[n], i)
				if wallsToBool:
					quarter[n][1][1] = tileWallsStrToBool(quarter[n][1][1])
			generatedBoard[i] = quarter
		self.boardLayout = generatedBoard
		return self.boardLayout

	def getBoard(self):
		return self.boardLayout
	
	def getWalls(self) -> list:
		# 2D array (16x16) of empty tiles
		w = [[[False, False, False, False] for _ in range(16)] for _ in range(16)]
		for i, q in enumerate(self.boardLayout):
			for t in q:
				w[t[0][0]+(i//2*8)][t[0][1]+(i%2*8)] = t[1][1]
		return w

def spinLetters(input: str, spin: int) -> str:
	output = ""
	letters = ["t", "r", "b", "l"]
	for letter in input:
		output += letters[(letters.index(letter)+spin)%4]
	return output

def spinCoords(input: list, spin: int) -> list:
	max = 7
	x = input[0] - (max/2)
	y = input[1] - (max/2)
	tempX = x
	for i in range(spin%4):
		x = y
		y = -tempX
		tempX = x
	x += 3.5
	y += 3.5
	return [int(x), int(y)]

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