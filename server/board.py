from nsew import *
from boardGenerator import BoardGenerator

class Board:

    def __init__ (self):
        self.generator = BoardGenerator()
        self.tiles = self.generator.generateBoard()
        self.placeBots()

    def placeBots(self):
        self.botCoords = self.generator.generateBots()
        for bot in botCoords:
            while botCoords[bot][0] != "":
                botCoords[bot] = (random.randint(0, 15), random.randint(0, 15))
