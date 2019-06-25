#define DEFINE_VARIABLES
#include "stdafx.h"
#include "CCS.h"

bool readAndSaveCCS(string matFileName)
{
    ofstream ccsFile((matFileName.substr(0, matFileName.size()-2) + "ccs").c_str(), ios::out | ios::binary);
    ifstream matlabFile((matFileName).c_str(), ios::in | ios::binary);

    int row, col, nnz, nRows, nCols;
    double reader;

    matlabFile.read((char*)& nnz, sizeof(nnz));
    matlabFile.read((char*)& row, sizeof(row));
    matlabFile.read((char*)& col, sizeof(col));

    ccsFile.write((char*)& nnz, sizeof(nnz));
    ccsFile.write((char*)& row, sizeof(row));
    ccsFile.write((char*)& col, sizeof(col));

    nRows = row;
    nCols = col;
    // ccsSizeFile.close();

    cout << "rows: " << row << " cols: " << col << " nnz: " << nnz << endl;
    // create CCS object
    CCS mat(row, col, "A");

    for(int i = 0; i < nnz; i++)
    {
        matlabFile.read((char*)& row, sizeof(row));
        matlabFile.read((char*)& col, sizeof(col));
        matlabFile.read((char*)& reader, sizeof(reader));
        mat.addElement(row-1, col-1, (f_type) reader);
    }

    ccsFile.write(reinterpret_cast<char*>(mat.colPtr.data()) , mat.colPtr.size() * sizeof(int));
    ccsFile.write(reinterpret_cast<char*>(mat.rowInd.data()), mat.rowInd.size() * sizeof(int));
    ccsFile.write(reinterpret_cast<char*>(mat.val.data()), mat.val.size() * sizeof(f_type));

    ccsFile.close();
    matlabFile.close();

    // check if matrix has zero diagonal entries
    for(int i = 0; i < row; i++)
    {
        if(mat.ifNZElementExists(i,i, nnz) == false) return false;
    }
    return true;
}

int main(int argc, char* argv[])
{
    bool nonZeroDiagonal = false;

    #ifdef PRINT_DEBUG_MSG
    debugFile.open("debugProcessTests.txt");
    #else
    ofstream debugFile("matGraph.txt");
    #endif

    ofstream nzDiagMatList("../spMats/processData/nonZeroDiagonalMatrices.txt");
    if(argc == 1)
        cout << "provide path of the files to be converted" << endl;

    for(int i = 1; i < argc; i++)
    {
        cout << "converting " << argv[i] << endl;
        if(readAndSaveCCS(argv[i]))
        {
            // nzDiagMatList << argv[i] << endl;
            cout << argv[i] << " is NOT a non zero diagonal matrix" << endl;
        }
    }
    nzDiagMatList.close();
    debugFile.close();
}