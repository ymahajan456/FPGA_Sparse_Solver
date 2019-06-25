#ifndef _CCS_H_
#define _CCS_H_

#include "stdafx.h"

/*
* Name: Column Sparse [CS]
* Provides: 
*   a. index    column/row entry index for each element (Mxi/ Mix)
*   b. pointer  stores the index for the elements in the CCS matrix from which 
*               the column/row is taken
*   c. val      value of the element
* Methods:
*   a. getPtrOfIndex()
*               returns the CS Pointer for the element from CS index
*/
class CS
{
    public:
    vector<int> index, pointer;
    vector<f_type> val;
    int getPtrOfIndex(int index);
};

/*
* Name: Compressed column Sparse
* Provides:
*   a. numRos   : number of rows in a matrix (mandatory)
*   b. numRCols : number of columns in a matrix (mandatory)
*   c. name     : name of the matrix (mandatory)
*   d. colPtr   : column pointer vector
*   e. rowInd   : row index vector
*   f. val      : value vector
* Methods:
*   a. CCS()
*           Constructors
*   b. print()
            Print matrix in CCS format (prints to console)
*   c. printToFile()
*           Print complete matrix in csv file
*   d. getDenseColumn()
*           Returns a dense vector for given column
*   e. getDenseRow()
*           Returns a dense vector for given row
*   f. getSparseRow()
*           Returns CS object for given column
*   g. getSparseRow()
*           Returns CS object for given row
*   h. addElement()
*           Add element to CCS matrix if it doesn't exists
*/
class CCS
{
    public:

    int numRows, numCols;
    vector<int> colPtr, rowInd, BRAMInd, BRAMAddr;
    vector<f_type> val;
    string name = "";

    CCS(vector<int> colPtr, vector<int> rowInd, vector<f_type> val, int numRows, string name);
    CCS(int &colPtr, int &rowInd, f_type  &val, int numRows, string name);
    CCS(int numRows, int numCols, string name);
    CCS(string ccsFile, string matName = "A" );
    void print(bool printMemMapVector = false);
    void printToFile(string fileName);
    void getDenseColumn(int colNum, vector<f_type> &column);
    void getDenseRow(int rowNum, vector<f_type> &row);
    void getSparseColumn(int colNum, CS &column, int fromRow = 0, int toRow = 0);
    void getSparseRow(int rowNum, CS &row, int fromColumn = 0, int toColumn = 0);
    bool addElement(int row, int column, f_type value);
    bool removeElement(int row, int col);
    bool removeElement(int entryIndex);
    bool ifNZElementExists(int row, int column, int &loc);
    bool operator==(CCS &y);
    void removeZeroes(f_type threshold);
    void removeZeroes();
    void resizeMemMap();
    void generateBRAMCoeFiles(const vector<string> bramNames);
};
// Returns a CCS identity matrix with name "eye" 
CCS spEye(int size, f_type scale = (f_type) 1);




#endif