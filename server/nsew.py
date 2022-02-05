N = (0, -1)
E = (1, 0)
S = (0, 1)
W = (-1, 0)
NSEW = [N,E,S,W]

def addTuples(a, b):
    return tuple([i+j for i, j in zip(a, b)])
