N = (1, 0)
E = (0, 1)
S = (-1, 0)
W = (0, -1)

NSEW = [N,E,S,W]

def addTuples(a, b):
    result = list(a)

    for i in range(len(result)):
        result[i] += b[i]

    return tuple(result)
