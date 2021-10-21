import random
from configHandler import loadConfigData

def main():
    configData = loadConfigData()
    generatedBoard = generateBoard()

def generateBoard():
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
        generatedBoard[i] = quarter
    return generatedBoard

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

if __name__ == "__main__":
    main()