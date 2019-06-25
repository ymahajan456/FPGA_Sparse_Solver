#include "CCS.h"

//****************************************************************************//

CCS::CCS(vector<int> colPtr, vector<int> rowInd, vector<f_type> val, int numRows, string name)
{
    this->numCols = colPtr.size() - 1;
    this->numRows = numRows;
    this->colPtr = colPtr;
    this->rowInd = rowInd;
    this->val = val;
    this->name = name;
}

//****************************************************************************//

CCS::CCS(int &colPtr, int &rowInd, f_type &val, int numRows, string name)
{
    this->numRows = numRows;
    this->colPtr = vector<int>(colPtr, colPtr + sizeof(colPtr)/sizeof(int));
    this->rowInd = vector<int>(rowInd, rowInd + sizeof(rowInd)/sizeof(int));
    this->val = vector<f_type>(val, val + sizeof(val)/sizeof(f_type));
    this->numCols = this->colPtr.size() - 1;
    this->name = name;
}


//****************************************************************************//

CCS::CCS(int numRows, int numCols, string name)
{
    this->numRows = numRows;
    this->numCols = numCols;
    this->colPtr = vector<int>(numCols+1, 0);
    this->name = name;
}

//****************************************************************************//
CCS::CCS(string ccsFileName, string matName)
{
    ifstream inFile(ccsFileName + ".ccs", ios::in | ios::binary);
    int row, col, nnz;

    inFile.read((char*)& nnz, sizeof(nnz));
    inFile.read((char*)& row, sizeof(row));
    inFile.read((char*)& col, sizeof(col));

    this->numRows = row;
    this->numCols = col;
    this->colPtr.resize(col+1);
    this->rowInd.resize(nnz);
    this->val.resize(nnz);
    this->name = matName;

    inFile.read(reinterpret_cast<char*>(this->colPtr.data()), this->colPtr.size() * sizeof(int));
    inFile.read(reinterpret_cast<char*>(this->rowInd.data()), this->rowInd.size() * sizeof(int));
    inFile.read(reinterpret_cast<char*>(this->val.data()), this->val.size() * sizeof(f_type));

    inFile.close();
}
//****************************************************************************//

void CCS::print(bool printMemMapVector)
{
    int i, j;
    #ifdef PRINT_DEBUG_MSG 
    debugFile 
    #else 
    cout 
    #endif
    << "***** " << this->name << " *****" << endl;

    for (i = 0; i < this->colPtr.size()-1; i++)
    {
        #ifdef PRINT_DEBUG_MSG 
        debugFile 
        #else 
        cout 
        #endif 
        << setw(5) << left << colPtr[i] << "Column " << i << endl;

        for(j = this->colPtr[i]; j < this->colPtr[i+1]; j++)
        {
            #ifdef PRINT_DEBUG_MSG 
            debugFile 
            #else 
            cout 
            #endif
            << "     " << setw(5) << right << this->rowInd[j] << ' '
                << setw(10) << right << this->val[j];
            if(printMemMapVector)
            {
                #ifdef PRINT_DEBUG_MSG 
                debugFile 
                #else 
                cout 
                #endif
                << " " << setw(5) << right << this->BRAMInd[j] << ' '
                    << this->BRAMAddr[j];
            }
            #ifdef PRINT_DEBUG_MSG 
            debugFile 
            #else 
            cout 
            #endif
            << endl;
        }
    }
}

//****************************************************************************//

void CCS::printToFile(string fileName)
{
    int i;
    vector<f_type> row(this->numCols);
    fileName += ".csv";
    ofstream matFile(fileName.c_str());

    for (i = 0; i < this->numRows; i++)
    {
        this->getDenseRow(i, row);
        for(auto &j : row)
        {
            matFile << j << ", ";
        }
        matFile << endl;
    }
    matFile.close();
}

//****************************************************************************//

void CCS::getDenseColumn(int colNum, vector<f_type> &column)
{
    // Size of the vector must be correct
    std::fill(column.begin(), column.end(), 0);
    for(int i = this->colPtr[colNum]; i < this->colPtr[i+1]; i++)
        column[i] = this->val[i];
}

//****************************************************************************//

void CCS::getDenseRow(int rowNum, vector<f_type> &row)
{
    int i, j;
    for(i = 0; i < this->numCols; i++)
    {
        row[i] = (f_type) 0;
        for(j = this->colPtr[i]; j < this->colPtr[i+1]; j++)
        {
            if(this->rowInd[j] == rowNum) 
                row[i] =this->val[j];
        }
    }
}

//****************************************************************************//

void CCS::getSparseColumn(int colNum, CS &column, int fromRow, int toRow)
{
    int i;
    toRow = toRow == 0 ? this->numRows : toRow;
    column.index.clear();
    column.pointer.clear();
    column.val.clear();
    for(i = this->colPtr[colNum]; i < this->colPtr[colNum + 1]; i++)
    {
        if(this->rowInd[i] < toRow && this->rowInd[i] >= fromRow)
        {
            column.index.push_back(this->rowInd[i]);
            column.pointer.push_back(i);
            column.val.push_back(this->val[i]);
        }
    }
}

//****************************************************************************//

void CCS::getSparseRow(int rowNum, CS &row, int fromColumn, int toColumn)
{
    int i, j;
    toColumn = toColumn == 0 ? this->numCols : toColumn;
    row.index.clear();
    row.pointer.clear();
    row.val.clear();
    for(i = fromColumn; i < toColumn; i++)
    {
        for(j = this->colPtr[i]; j < this->colPtr[i+1]; i++)
        {
            if(this->rowInd[j] == rowNum)
            {
                row.index.push_back(i);
                row.pointer.push_back(j);
                row.val.push_back(this->val[j]);
            }
        }
    }
}

//****************************************************************************//

bool CCS::addElement(int row, int column, f_type value)
{
    vector<int>::iterator entryIndex; 
    int j;

    if(this->colPtr[column] < this->colPtr[column + 1])
    {
        entryIndex = lower_bound(this->rowInd.begin() + this->colPtr[column], 
                        this->rowInd.begin() + this->colPtr[column+1], row);
    }
    else
    {
        entryIndex = this->rowInd.begin() + this->colPtr[column];
    }

    j = std::distance(this->rowInd.begin(), entryIndex);

    if(j-1 >= this->colPtr[column] && this->rowInd[j-1] == row)
    {
        cout << "Worning: Element Exists at " << this->name << " " << row << ", " << column
                << " , action ignored" << endl;
        return false;
    }


    this->rowInd.insert(entryIndex, row);
        
    this->val.insert(this->val.begin() + j, value);
    
    for(j = column +1 ; j < this->colPtr.size(); j++)
        this->colPtr[j] += 1;

    return true;
}

//****************************************************************************//

bool CCS::removeElement(int row, int column)
{
    vector<int>::iterator entryIndex; 
    int j;

    entryIndex = lower_bound(this->rowInd.begin() + this->colPtr[column], 
                        this->rowInd.begin() + this->colPtr[column+1], row);

    if(entryIndex == this->rowInd.end())
        return false;

    j = std::distance(this->rowInd.begin(), entryIndex);

    // remove value and row
    this->rowInd.erase(entryIndex);
    this->val.erase(this->val.begin() + j);
    // update column pointers
    for(j = column+1; j < this->numCols; j++)
        this->colPtr[j] -= 1;
}

//****************************************************************************//

// be carefull with the index
bool CCS::removeElement(int entryIndex)
{
    vector<int>::iterator column;
    // check if entryIndex is in valid range
    if(entryIndex >= this->rowInd.size())
        return false;

    // remove value and row
    this->rowInd.erase(this->rowInd.begin() + entryIndex);
    this->val.erase(this->val.begin() + entryIndex);
    // update column pointers
    column = lower_bound(this->colPtr.begin(), this->colPtr.end(), entryIndex);
    while(column != this->colPtr.end())
    {
        *column -= 1;
        column++;
    }
	return true;
}

//****************************************************************************//

CCS spEye(int size, f_type scale)
{
    vector<int> rowInd(size), colPtr(size + 1);
    vector<f_type> val(size, scale);
    int i;
    for(i = 0; i < size; i++)
    {
        rowInd[i] = i;
        colPtr[i] = i;
    }
    colPtr[size] = size;
    return(CCS(colPtr, rowInd, val, size, "eye"));
}

//****************************************************************************//

int CS::getPtrOfIndex(int index)
{
    int i;
    for(i=0; i < this->index.size(); i++)
    {
        if(this->index[i] == index)
            return(this->pointer[i]);
    }
	return(-1);
}

//****************************************************************************//

bool CCS::ifNZElementExists(int row, int column, int &loc)
{
    vector<int>::iterator entryIndex; 

    entryIndex = lower_bound(this->rowInd.begin() + this->colPtr[column], 
                    this->rowInd.begin() + this->colPtr[column+1], row);

    if(entryIndex == this->rowInd.end()) return false;

    loc = std::distance(this->rowInd.begin(), entryIndex);
    return true;
}

//****************************************************************************//

bool CCS::operator==(CCS &y)
{
    int i;
    // check number of rows and columns
    if(this->numCols != y.numCols || this->numRows != y.numRows)
    {
        #ifdef PRINT_DEBUG_MSG 
        debugFile 
        #else 
        cout 
        #endif
        << "Numbers of rows and columns doesn't match for " << this->name << endl;
        return false;
    }
    // remove additional zeroes (thresholded)
    this->removeZeroes();
    y.removeZeroes();

    if(this->val.size() != y.val.size())
    {
        #ifdef PRINT_DEBUG_MSG 
        debugFile 
        #else 
        cout 
        #endif
        << "Numbers of NNZ doesn't match for " << this->name << endl;
        return false;
    }
    // comparte column sizes ans row indices
    if(! std::equal(this->colPtr.begin(), this->colPtr.end(), y.colPtr.begin()))
    {
        #ifdef PRINT_DEBUG_MSG 
        debugFile 
        #else 
        cout 
        #endif
        << "Numbers of number of elements in columns doesn't match for " << this->name << endl;
        return false;
    }
    if(! std::equal(this->rowInd.begin(), this->rowInd.end(), y.rowInd.begin()))
    {
        #ifdef PRINT_DEBUG_MSG 
        debugFile 
        #else 
        cout 
        #endif
        << "Numbers of row iondices don't match for " << this->name << endl;
        return false;
    }
    for(i = 0; i < this->val.size(); i++)
    {
        if(abs(this->val[i] - y.val[i]) > abs(g_errThreshold * y.val[i]))
        {
            #ifdef PRINT_DEBUG_MSG 
            debugFile 
            #else 
            cout 
            #endif
            << "Values don't match for " << i << endl;
            return false;
        }
    }

    // after going through hell, Finally ....
    return true;

}

//****************************************************************************//

void CCS::removeZeroes(f_type threshold)
{
    vector<int> zeroes;
    int i,j;
    for(i = 0; i < this->val.size(); i++)
    {
        if(this->val[i] < threshold)
            zeroes.push_back(i - zeroes.size());
    }

    for(i = 0; i < zeroes.size(); i++)
        this->removeElement(zeroes[i]);
}

//****************************************************************************//

void CCS::removeZeroes()
{
    this->removeZeroes(g_nzThreshold);
}

//****************************************************************************//

void CCS::resizeMemMap()
{
    this->BRAMAddr.assign(this->rowInd.size(), BRAMAddrNULL);
    this->BRAMInd.assign(this->rowInd.size(), BRAMIndNULL);
}

// ***************************************************************************//

union
{
    f_type input;
    int output;
} typeConv;

void CCS::generateBRAMCoeFiles(const vector<string> bramNames)
{
    ofstream outFile;
    int maxMemAddress = * (max_element(this->BRAMAddr.begin(), this->BRAMAddr.end())) + 1;
    vector<vector<f_type>> bramData(bramNames.size(), vector<f_type>(maxMemAddress, 0.0));
    for(int i = 0; i < this->BRAMAddr.size(); i++)
    {
        // element is meant to be stored in the BRAM
        if(this->BRAMInd[i] >= 0)
        {
            bramData[this->BRAMInd[i]][this->BRAMAddr[i]] = this->val[i];
        }
    }
    // generate '.coe' files

    for(int i = 0; i < bramNames.size(); i++)
    {
        outFile.open("../outputFiles/bram_" + bramNames[i] + ".coe");
        outFile << "memory_initialization_radix=2;" << endl <<
            "memory_initialization_vector=" << endl;

        for(int j = 0; j < maxMemAddress; j++)
        {
            typeConv.input = bramData[i][j];
            outFile << bitset<sizeof(f_type) * CHAR_BIT>(typeConv.output) << ",\n";
        }
        outFile << "0;\n";
        outFile.close();
    }
}