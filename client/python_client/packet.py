import math

class Packet:
    def __init__(self, type=0, data=""):
       self.type = type
       self.data = data
    def pack(self) -> bytearray:
        if type(self.data) == str:
            return bytearray(str(self.type) + "^" + self.data, "utf-8")
        if self.type == 1 and type(self.data) == list:
            walls = self.data
            tiles = []
            for row in walls:
                for col in row:
                    tiles.append(int("".join([str(int(w)) for w in col]), 2))
            cols = [str(len(row)) for row in walls]
            prefix = str(self.type) + "^" + ",".join(cols) + "|"
            tilesStr = [str(tile) for tile in tiles]
            data = prefix + ",".join(tilesStr)
            return bytearray(data, "utf-8")
    def unpack(self, pckt: bytearray) -> tuple:
        decoded = pckt.decode("utf-8")
        arr = decoded.split("^")
        self.type = int(arr[0])
        self.data = "^".join(arr[1:])
        if self.type == 255:
            return (self.type, self.data)
        if self.type == 1:
            data = self.data
            cols = (data.split("|")[0]).split(",")
            tilesRaw = (data.split("|")[1]).split(",")
            tiles = []
            i = 0
            for columnsCount in cols:
                column = []
                for _ in range(int(columnsCount)):
                    tileValue = int(tilesRaw[i])
                    tileBools = tileIntToBools(tileValue)
                    column.append(tileBools)
                    i += 1
                tiles.append(column)
            self.data = tiles
            return tiles
    def __repr__(self):
        return f"Packet({repr(self.type)}, {repr(self.data)})"

def tileIntToBools(tileValue: int) -> list[bool]:
    bools = []
    divisors = [8, 4, 2, 1]
    for divisor in divisors:
        i = math.floor(tileValue/divisor)
        bools.append(bool(i))
        tileValue -= i*divisor
    return bools