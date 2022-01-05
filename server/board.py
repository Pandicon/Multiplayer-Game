from nsew import *
from boardGenerator import BoardGenerator

class Board:
    def __init__ (self):
        self.generator = BoardGenerator()
        self.tiles = self.generator.generateBoard()
        self.placeBots()
        self.history = [defaultCoords]

    def placeBots(self):
        self.botCoords = self.generator.generateBots()
        self.defaultCoords = self.botCoords
        for bot in self.botCoords:
            while(not checkPlacement(bot)):
                self.botCoords[bot] = (random.randint(0, 15), random.randint(0, 15))

    def checkPlacement(self, bot):
        if(self.tiles[self.botCoords[bot]][0] != ""):
            return False
        for d in NSEW:
            if(not self.tiles[move(self.botCoords[bot], d)][0].split("-", 1)[0] == bot):
                return False
        return True

    def moveBot(self, bot, direction):
        botCoords[bot] = move(botCoords[bot], direction)

    def revert(self):
        self.history.pop(-1)
        self.botCoords = self.history[-1]

    def restart(self):
        self.botCoords = self.defaultCoords
        self.history = [defaultCoords]
        
    def move(self, coords, direction):
        while(not(self.tiles[coords][1][NSEW.index(direction)])):
            coords = addTuples(coords, direction)
        return coords
