from math import *

def hwConfigHelper(numBrams, numMACs, numDIVs):
    auSelSize = ceil(log2(1 + numMACs + numDIVs + 4*numBrams))
    bramSelSize = ceil(log2(1 + numMACs + numDIVs))
    ctrlSignalSize = 3 * auSelSize * numMACs + 2 * auSelSize * numDIVs + 4 * bramSelSize * numBrams + 11 * 4 * numBrams

    print("AU MUX Sel port width: ", auSelSize)
    print("BRAM MUX Sele port width: ", bramSelSize)
    print("CTRL signal width: ", ctrlSignalSize)

if __name__ == "__main__":
    hwConfigHelper(4, 4, 4)
