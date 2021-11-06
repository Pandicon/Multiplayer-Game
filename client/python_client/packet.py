import math

class Packet:
	def __init__(self, ptype=0, data=b""):
		self.type = ptype
		if type(data) == str:
			self.data = bytearray(data, "utf-8")
		elif type(data) == list and self.type == 1: # walls packet
			tiles = []
			for row in data:
				for col in row:
					tiles.append(int("".join([str(int(w)) for w in col]), 2))
			for i in range(len(tiles)//2):
				tiles[i] = tiles[i] * 16 + tiles[i + 1]
				del tiles[i+1]
			self.data = bytearray(tiles)
		else:
			self.data = data
	def pack(self) -> bytearray:
		return self.type.to_bytes(1, "little") + self.data
	@classmethod
	def unpack(cls, pckt: bytearray):
		return cls(pckt[0], pckt[1:])
	def extract(self):
		if self.type == 1:
			tiles = []
			for c in self.data:
				tiles.extend([c // 16, c % 16])
			tiles = [tileIntToBools(t) for t in tiles]
			board_size = 16
			walls = [tiles[i:i+board_size] for i in range(0, len(tiles), board_size)]
			return walls
		elif self.type == 255:
			return self.data.decode("utf-8")
		else:
			return self.data
	def __repr__(self) -> str:
		return f"Packet({repr(self.type)}, {repr(self.data)})"

def tileIntToBools(tileValue: int) -> list:
	bools = []
	divisors = [8, 4, 2, 1]
	for divisor in divisors:
		i = math.floor(tileValue/divisor)
		bools.append(bool(i))
		tileValue -= i*divisor
	return bools