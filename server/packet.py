import bots
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
		elif type(data) == list and self.type == 2:
			self.data = bytearray([x << 4 | y for x, y in data])
		elif type(data) == tuple and self.type == 3:
			pos, trg = data
			self.data = bytearray([pos[0] << 4 | pos[1], bots.black if trg == "spiral" else bots.all_str.index(trg.split("-")[0])])
		elif type(data) == tuple and self.type == 16:
			pl, ln = data
			self.data = ln.to_bytes(1, "little") + bytearray(pl, "utf-8")
		elif type(data) == tuple and self.type == 18:
			col, to = data
			self.data = bytearray([to[0] << 4 | to[1]]) + col.to_bytes(1, "little")
		else:
			self.data = data
	def pack(self) -> bytearray:
		return self.type.to_bytes(1, "little") + self.data
	@classmethod
	def unpack(cls, pckt: bytearray):
		if len(pckt) > 1:
			return cls(pckt[0], pckt[1:])
		elif len(pckt) == 1:
			return cls(pckt[0], b"")
		else:
			return cls(0, b"")
	def extract(self):
		if self.type == 1: # nick
			return self.data.decode("utf-8")
		elif self.type == 2:
			return self.data[0]
		elif self.type == 3:
			return (self.data[0] >> 4, self.data[0] & 0xf)
		elif self.type == 255: # chatmsg
			return self.data.decode("utf-8")
		else:
			return self.data
	def __repr__(self) -> str:
		return ("Packet(" + str(repr(self.type)) + str(repr(self.data)) + ")")

def tileIntToBools(tileValue: int) -> list:
	bools = []
	divisors = [8, 4, 2, 1]
	for divisor in divisors:
		i = math.floor(tileValue/divisor)
		bools.append(bool(i))
		tileValue -= i*divisor
	return bools