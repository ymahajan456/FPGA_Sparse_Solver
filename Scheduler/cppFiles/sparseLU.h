#ifndef _SPARSE_LU_H_
#define _SPARSE_LU_H_

#include "stdafx.h"
#include "CCS.h"
#include "Graph.h"

#ifdef DEFINE_VARIABLES
#define EXTERN                  
#define INITIALIZER(...)        = __VA_ARGS__
#else
#define EXTERN                  extern
#define INITIALIZER(...)       
#endif 

// Generate graph(LUDAG) for the LU decomposition(L, U) of the matrix(A)
void LUGetOperationList(CCS &A, CCS &L, CCS &U, Graph &LUDAG);
// Create read type node
void createRdNode(Node &node, CS &spVec, CCS &mat, int colNum, int vecLoc);
// get node name from matrix name, row and column
string getMatNodeName(string matName, int row, int col);
// generate LU node and child nodes
void addMACNode(Graph &LUDAG, CS &Uj, CCS &mat, string A_matName, int i, int j, 
        vector<vector<pair<string, string>>> &MACs, Node *Ujj = nullptr);
// Assign values from executed graph back to matrix
void assignGraphValuesToMats(Graph &exeGraph, CCS &U, CCS &L);

#endif