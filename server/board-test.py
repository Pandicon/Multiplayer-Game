from board import *

def tst(what, res):
	if eval(what) != res:
		print("!: ", what, " != ", res, " (", eval(what), ")")

tst("spinLetters('tr', 1)", "rb")
tst("spinLetters('tr', 2)", "bl")
tst("spinLetters('tr', 3)", "lt")
tst("spinLetters('tr', 0)", "tr")
tst("spinLetters('tbr', 1)", "rlb")
tst("spinLetters('tbr', 2)", "btl")
tst("spinLetters('tbr', 3)", "lrt")
tst("spinLetters('tbr', 0)", "tbr")
tst("spinCoords([0, 0], 0)", [0, 0])
tst("spinCoords([0, 0], 1)", [7, 0])
tst("spinCoords([0, 0], 2)", [7, 7])
tst("spinCoords([0, 0], 3)", [0, 7])
tst("spinCoords([0, 0], 4)", [0, 0])
tst("spinCoords([1, 3], 0)", [1, 3])
tst("spinCoords([1, 3], 1)", [4, 1])
tst("spinCoords([1, 3], 2)", [6, 4])
tst("spinCoords([1, 3], 3)", [3, 6])

