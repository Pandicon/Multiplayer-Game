from board import Board
import random
import time

class Game():
    def __init__(self, server):
        self.board = Board()
        self.server = server
        self.server.sendToAll(, 2)
        self.targets = []
        for tile in self.board.tiles:
            if(not self.board.tiles[tile][0]==""):
                self.targets.append(self.board.tiles[tile][0])
        self.players = []
        self.giveUp = False
        while(not input() == "START"):
            pass
        startTurn()

    def newPlayer(self, name, connection: socket.socket):
        p = Player(name, connection)
        self.players.append(p)
        return p

    def startTurn(self):
        self.target = self.targets[random.randint(0, len(self.targets)-1)
        self.found = []
        self.showing = None
        while(self.found.lenght == 0):
            time.sleep(1)
        time.sleep(60)
        showtime()

    def wayFound(self, player, lenght):
        self.found.append(Way(player, lenght)
        self.server.sendToAll(, 16)

    def showtime(self):
        i = self.found.index(min(map(lambda x : x.lenght, self.found)))
        self.showing = self.found[i].player
        self.found.pop(i)
        self.server.send(self.showing.connection, 0, 32)
        
        j = 0
        while(j < 60 and self.giveUp == False):
            time.sleep(1)
        stop()

    def move(self, bot, direction):
        self.board.moveBot(bot, direction)
        if(self.board.tiles[self.board.bots[self.target.split("-", 1)[0]]][0] == self.target or self.board.tiles[self.board.bots[black]] == self.target):
            endTurn(self.showing)
        self.server.sendToAll(, 3)

    def revert(self):
        self.board.revert()
        self.server.sendToAll(, 2)

    def reset(self):
        self.board.reset()
        self.server.sendToAll(, 2)

    def stop(self):
        self.server.send(self.showing.connection, 0, 33)
        self.showing = None
        self.giveUp = False
        showtime()

    def endTurn(self, winner):
        if(not winner == None):
            winner.points += 1
        if(len(self.targets) == 0)
            __init__()
        

class Player():
    def __init__(self, name, connection: socket.socket):
        self.name = name
        self.connection = connection
        self.points = 0

class Way():
    def __init__(self, player, lenght):
        self.player = player
        self.lenght = lenght
