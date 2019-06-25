class Node:
    op = None
    operands = []
    results = []
    latency = 0
    value = 0
    position = []
    def __init__(self, op, operands, results, latency = None, value = None, position = []):
        self.operands = operands if isinstance(operands, list) else [operands]
        self.results = results if isinstance(results, list) else [results]
        self.value = None
        if op in latencyList:
            self.latency = latencyList[op] if latency == None else latency
        else:
            if latency == None:
                print("Latency of the operation is not defined. Assigned 100")
                latency = 100
            self.latency = latency
        self.op = op

    def __str__(self):
        return repr([self.op, self.operands, self.results, self.latency])


latencyList = {'-' : 1 , '+' : 1, '*' : 5, '/' : 10, 'mac' : 5, 'memCall' : 4, 'cp' : 0}

class CCS:
    def __init__(self, colPtr = [], rowInd = [], val = [], numRows = 0, numCols = 0):
        self.colPtr = colPtr
        self.rowInd = rowInd
        self.val = val
        self.numCols = numCols
        self.numRows = numRows
        self.nnz = len(val)

    def __str__(self):
        return 'colPtr : ' + repr(self.colPtr) + '\n' + 'rowInd : ' + repr(self.rowInd) + '\n' + 'val    : ' + repr(self.val)

    def printMatToFile(self, fileName = 'mat'):
        file = open(fileName + '.csv', 'w')
        for i in range(self.numRows):
            file.write(repr(self.getDenseRow(i))[1:-1] + '\n')
        file.close()
    
    def getDenseCol(self, colNum):
        col = [0 for i in range(self.numRows)]
        for i in range(self.colPtr[colNum], self.colPtr[colNum + 1]):
            col[self.rowInd[i]] = self.val[i]
        return col

    def getDenseRow(self, rowNum):
        row = [0 for i in range(self.numCols)]
        for j in range(self.numCols):
            currCol = self.rowInd[self.colPtr[j] : self.colPtr[j+1]]
            if rowNum in currCol:
                row[j] = self.val[self.colPtr[j] + currCol.index(rowNum)]
        return row
        
    def getSparseRow(self, rowNum, upToColumn = None):
        if upToColumn == None:
            upToColumn = self.numCols
        row = [[], [], []]              # column no, valPtr, values
        for j in range(upToColumn):
            currCol = self.rowInd[self.colPtr[j] : self.colPtr[j+1]]
            if rowNum in currCol:
                row[0].append(j)
                row[1].append(self.colPtr[j] + currCol.index(rowNum))
                row[2].append(self.val[row[1][-1]])
        return row


    def getSparseCol(self, colNum, upToRow = None):
        if upToRow == None:
            upToRow = self.numRows
        col = [[], [], []]              # row number, ptr, values
        for i in range(self.colPtr[colNum], self.colPtr[colNum + 1]):
            if self.rowInd[i] < upToRow:
                col[0].append(self.rowInd[i])
                col[1].append(i)
                col[2].append(self.val[i])
        # col[1] = [i for i in range(self.colPtr[colNum], self.colPtr[colNum + 1])]
        # col[0] = [self.rowInd[col[1][j]] for j in range(len(col[1]))]
        # col[2] = [self.val[col[1][j]] for j in range(len(col[1]))]
        return col

    def addElement(self, rowNum, colNum, val):
        rowInd = self.rowInd[self.colPtr[colNum] : self.colPtr[colNum+1]]
        entryIndex = self.colPtr[colNum] + rowInd.index(min([i for i in rowInd if i > rowNum]))
        self.rowInd.insert(entryIndex, rowNum)
        for i in range(colNum+1, len(self.colPtr)):
            self.colPtr[i] += 1
        self.val.insert(entryIndex, val)

def spEye(n, scale = 1):
    colPtr = [i for i in range(n+1)]
    rowInd = [i for i in range(n)]
    val = [scale for i in range(n)]
    return CCS(colPtr, rowInd, val, n, n)

def name(mat, row, col):
    return mat + '_' + str(row) + '_' + str(col)

def sparseLUOplistGen(A):
    opList = {}
    opCount = {op : 0 for op in ['-', '+', '*', '/', 'mac', 'cp', 'memCall']}
    L = spEye(A.numRows)
    U = spEye(A.numRows)

    # Columnwise operations
    # ---------------------
    for col in range(L.numRows):
         # Solving lower triangular system LjUj = Aj
        # -----------------------------------------

        # list of multiply operations for element
        Ucol = [[] for i in range(col)]

        # generate mem call nodes for vector Aj
        currCol = A.getSparseCol(col, col)
        for (index, i , val) in zip(currCol[1], currCol[0], currCol[2]) :
                Ucol[i].append(Node('memCall', ['spA_' + str(index)],
                            [name('A', i, col)], value = val, position = [i,col]))

        # multiply nodes
        for j in range(col):
            if len(Ucol[j]):
                Lcol = L.getSparseCol(j, col)
                Uname = name('U', j, col)
                for i in Lcol[0]:
                    Ucol[i].append(Node('*', [Uname, name('L', i, j)], [Uname + name('L', i, j)]))
        
        print("Column", col)
        for col in Ucol:
            for ele in col:
                print(ele)
            print()






def gpLU(spMat):
    L = CCS()
    U = CCS()
    pass
    


# def gpLU(spMat):



if __name__ == '__main__' :
    # colPtr = [0, 3, 5, 7, 9, 11]
    # rowInd = [0, 2, 4, 0, 3, 1, 4, 0, 3, 1, 4]
    # val    = [1, 2, 5, -3, 4, -2, -5, -1, -4, 3, 6]

    colPtr = [0, 3, 5, 7, 9, 11]
    rowInd = [0, 2, 3, 1, 3, 0, 4, 1, 3, 0, 4]
    val    = [5, 2, 1, 4, -3, -5, -2, -4, -1, 6, 3]

    spMat = CCS(colPtr, rowInd, val, 5, 5)
    spMat.printMatToFile('first')
    # spMat.addElement(2,3, 8)
    # spMat.printMatToFile('second')
    # print(spMat.getSparseCol(3, 3))
    # print(spMat.getSparseRow(3, 3))
    sparseLUOplistGen(spMat)
