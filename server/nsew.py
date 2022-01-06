N = (1, 0)
E = (0, 1)
S = (-1, 0)
W = (0, -1)
NSEW = [N,E,S,W]

def addTuples(a, b):
    return tuple([i+j for i, j in zip(a, b)])
