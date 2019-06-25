#include "stdafx.h"
#include "sparseLU.h"

//****************************************************************************//

void LUGetOperationList(CCS &A, CCS &L, CCS &U, Graph &LUDAG)
{
    L = spEye(A.numCols);
    U = L;
    L.name = "L";
    U.name = "U";
    CS Lj, Aj, Uj;
    int i, j, k, numCols;

    numCols = A.numCols;

    vector<int> nzUij(numCols);
    vector<vector<pair<string, string>>> MACs(numCols);
    pair<string, string> MACpair;
    
    Node emptyNode, tmpNode, *Ujj = nullptr, *nodePtr;
    unordered_map<string, Node>::iterator graphIt;

    // Reset the the graph
    LUDAG.clearGraph();
    // add 0 node
    tmpNode = emptyNode;
    tmpNode.val = 0;
    tmpNode.op = "const";
    tmpNode.name = "0";
    tmpNode.delay = opDelay["const"];
    tmpNode.matPointer = nullptr;
    LUDAG.nodes["0"] = tmpNode;


    #ifdef PRINT_DEBUG_1
    debugFile << "sparseLU : LUGetOperationList: Init completed" << endl;
    #endif

    for(j = 0; j < numCols; j++)
    {
        // solving Lx Uj = Aj;
        // reset nzUij and MACs for both steps
        std::fill(nzUij.begin(), nzUij.end(), 0);
        for(i = 0; i < numCols; i++)
        {
            MACs[i].clear();
        }

        // add non-zero Aj read nodes (No dependencies) (for both steps)
        A.getSparseColumn(j, Aj);
        for(i = 0; i < Aj.index.size(); i++)
        {
            // push nodes to oplist
            tmpNode = emptyNode;
            createRdNode(tmpNode, Aj, A, j, i);
            LUDAG.nodes[tmpNode.name] = tmpNode;
            nzUij[Aj.index[i]] += 1;
        }

        // find non zero Uij, Lij (i < j) locations
        // j is column index and i is row index
        for(i = 0; i < j; i++)
        {
            // if Uij is non-zero
            if(nzUij[i])
            {
                MACpair.first = getMatNodeName(U.name, i, j);
                L.getSparseColumn(i, Lj, i+1);

                #ifdef PRINT_DEBUG_2
                debugFile << "non zero L" << i << endl;
                #endif

                for(k = 0; k < Lj.index.size(); k++)
                {
                    // toggle non zero status of affected nodes
                    nzUij[Lj.index[k]] += 1;
                    MACpair.second = getMatNodeName(L.name, Lj.index[k], i);
                    MACs[Lj.index[k]].push_back(MACpair);
                    #ifdef PRINT_DEBUG_2
                    debugFile << MACpair.first << " * " << MACpair.second <<' '; 
                    #endif
                }
                #ifdef PRINT_DEBUG_2
                debugFile << endl;
                #endif
            }
        }

        #ifdef PRINT_DEBUG_1
        debugFile << "sparseLU : LUGetOperationList: Found non-zero Uij, Lij" 
                    << endl;
        for(i = 0; i < numCols; i++)
        {
            if(nzUij[i])
                debugFile << i << "->" << nzUij[i] << ' ';
        }
        debugFile << endl;
        #endif

        // add non-zero Uij to U
        for(i = 0; i < j; i++)
        {
            if(nzUij[i])
                U.addElement(i, j, 0);
        }

        // add non-zero Lij to L
        for( i = j+1; i < numCols; i++)
        {
            if(nzUij[i])
                L.addElement(i, j, 0);
        }

        // create nodes for each non-zero Uij in order
        U.getSparseColumn(j, Uj, 0, j+1);
        for(i = 0; i < j+1; i++)
        {
            if(i == j)
            {
                // if diagonal element is zero
                if(nzUij[i] == 0)
                {
                    cout << "Error: Zero diagonal element at U" << i << ',' << i << endl;
                    exit(0);
                }
            }
            // If non zero Uij
            if(nzUij[i])
            {
                #ifdef PRINT_DEBUG_1
                debugFile << "MACs[" << i << "]: " << endl;
                for(k = 0;  k < MACs[i].size(); k++)
                    debugFile << MACs[i][k].first << ' ' << MACs[i][k].second 
                                << endl;
                #endif

                addMACNode(LUDAG, Uj, U, A.name, i, j, MACs);
                #ifdef PRINT_DEBUG_1
                debugFile << "Created U" << i << j << " node" << endl;
                debugFile << "LUDAG" << endl;
                LUDAG.printToFile(debugFile);
                #endif
                // If Uii -> save pointer to node fordivision operation
                if(i == j)
                {
                    graphIt = LUDAG.nodes.find(getMatNodeName(U.name, i, j));
                    Ujj = & graphIt->second;
                }
            }
        }

        #ifdef PRINT_DEBUG_1
        debugFile << "sparseLU : LUGetOperationList: created U" << j 
                    << " column" << endl;
        #endif

        // create non-zero Lij nodes
        L.getSparseColumn(j, Uj, j);

        for(i = j+1; i < numCols; i++)
        {
            #ifdef PRINT_DEBUG_2
            debugFile << "MACs[" << i << "]: " << endl;
            for(k = 0;  k < MACs[i].size(); k++)
                debugFile << MACs[i][k].first << ' ' << MACs[i][k].second 
                            << endl;
            #endif
            //Non zero Lij(pointer name is Uj, just trying to save variable)
            if(nzUij[i])
            {
                addMACNode(LUDAG, Uj, L, A.name,i, j, MACs, Ujj);

                #ifdef PRINT_DEBUG_1
                debugFile << "Created L" << i << j << " node" << endl;
                debugFile << "LUDAG" << endl;
                LUDAG.printToFile(debugFile);
                #endif
            }
        }

        #ifdef PRINT_DEBUG_1
        debugFile << "sparseLU : LUGetOperationList: Created L" << j << " nodes" << endl;
        #endif
        #ifdef PRINT_DEBUG_1
        debugFile << "******************* LUDAG *****************" << endl;
        LUDAG.printToFile(debugFile, true);
        debugFile << "******************* LUDAG *****************" << endl;
        #endif
    }

    #ifdef PRINT_DEBUG_1
    debugFile << "******************* LUDAG *****************" << endl;
    // LUDAG.printToFile(debugFile, true);
    debugFile << "******************* LUDAG *****************" << endl;
    #endif
}

//****************************************************************************//

void createRdNode(Node &node, CS &spVec, CCS &mat, int colNum, int vecLoc)
{
    node.val = spVec.val[vecLoc];
    node.op = "rd";
    // Aij will be read only once
    node.name = getMatNodeName(mat.name, spVec.index[vecLoc], colNum);
    node.delay = opDelay["rd"];
    node.matPointer = & mat;
    node.matName = mat.name;
    node.memMapIndex = spVec.pointer[vecLoc];
}

//****************************************************************************//

inline string getMatNodeName(string matName, int row, int col)
{
    return(matName + "_" + to_string(row) + "_" + to_string(col));
}

//****************************************************************************//

// void addMACNode(Graph &LUDAG, CS &Uj, string matName, string A_matName, int i,
//             int j, vector<vector<pair<string, string>>> &MACs, Node *Ujj)
// {
//     int k;void assignAddresses(Graph &lGraph, CCS & A, CCS & L, CCS & U);
//     // div node missing for L type node
//     if(i > j && Ujj == nullptr)
//     {
//         cout << "Error: normalizing factor missing";
//         exit(0);
//     }
//     // variable declaration
//     unordered_map<string, Node>::iterator graphIt0;
//     Node emptyNode, tmpNode, *nodePtrMul0, *nodePtrMul1, *nodePtrSub, *nodePtrMul;

//     #ifdef PRINT_DEBUG_2
//     debugFile << "sparseLU : addMACNode: init local variables for " << matName << i << j << endl;
//     #endif
    
//     // Find Aij node
//     graphIt0 = LUDAG.nodes.find(getMatNodeName(A_matName, i, j));
//     // if Aij is zero use zero node
//     graphIt0 = (graphIt0 == LUDAG.nodes.end()) ? LUDAG.nodes.find("0") : graphIt0;
    
//     // L nodes (non Lxx nodes)
//     if(i > j)
//     {
//         tmpNode = emptyNode;
//         // create Lij node
//         tmpNode.name = getMatNodeName(matName, i, j);
//         // pointer to division node
//         nodePtrMul = LUDAG.insertNode(tmpNode);
//         // create subtraction node
//         tmpNode.name = "sub" + getMatNodeName(matName, i, j);
//         // pointer to subtraction node Uij
//         nodePtrSub = LUDAG.insertNode(tmpNode);

//         // connect divion node with children
//         nodePtrMul->children.push_back(nodePtrSub);
//         nodePtrMul->children.push_back(Ujj);
//         Ujj->parents.push_back(nodePtrMul);
//         nodePtrSub->parents.push_back(nodePtrMul);

//         // set node properties
//         nodePtrMul->matName = matName;
//         nodePtrMul->matIndex[0] = i;
//         nodePtrMul->matIndex[1] = j;
//         nodePtrMul->memIndex = Uj.getPtrOfIndex(i);
//         nodePtrMul->op = "/";
//         nodePtrMul->delay = opDelay["/"];

//         #ifdef PRINT_DEBUG_2
//         debugFile << "sparseLU : addMACNode: created div node for U" << i << j << endl;
//         #endif

//     }
//     // Uij nodes
//     else
//     {
//         tmpNode = emptyNode;
//         // create subtraction node
//         tmpNode.name = getMatNodeName(matName, i, j);
        
//         // pointer to subtraction node Uij
//         nodePtrSub = LUDAG.insertNode(tmpNode);

//         // set memory location data
//         nodePtrSub->matName = matName;
//         nodePtrSub->matIndex[0] = i;
//         nodePtrSub->matIndex[1] = j;
//         nodePtrSub->memIndex = Uj.getPtrOfIndex(i);
//     }

//     #ifdef PRINT_DEBUG_2
//     debugFile << "sparseLU : addMACNode: created div node " << matName << i << j << endl;
//     #endif
//     // connect Aij node and sub node
//     graphIt0->second.parents.push_back(nodePtrSub);
//     nodePtrSub->children.push_back(&graphIt0->second);
    
//     // No MAC operations
//     if(MACs[i].size() == 0)
//     {
//         nodePtrSub->op = "pass";
//         nodePtrSub->delay = opDelay["pass"];
//         return;
//     }
//     // assign node operation
//     nodePtrSub->op = "-";
//     nodePtrSub->delay = opDelay["-"];
//     // create MAC node
//     tmpNode = emptyNode;
//     tmpNode.name = "mac" + nodePtrSub->name;
//     nodePtrMul = LUDAG.insertNode(tmpNode);
//     // connect with sub node
//     nodePtrMul->parents.push_back(nodePtrSub);
//     nodePtrSub->children.push_back(nodePtrMul);
//     // assign mac node properties
//     nodePtrMul->op = "mac";
//     // interchange pointers
//     nodePtrSub = nodePtrMul;

//     #ifdef PRINT_DEBUG_2
//     debugFile << "sparseLU : addMACNode: created MAC node for "<< matName << i << j << endl;
//     #endif

    
//     for(k = 0; k < MACs[i].size(); k++)
//     {
//         // get pointers to multiplicands
//         graphIt0 = LUDAG.nodes.find(MACs[i][k].first);
//         nodePtrMul0 = & graphIt0->second;
//         graphIt0 = LUDAG.nodes.find(MACs[i][k].second);
//         nodePtrMul1 = & graphIt0->second;
//         // create MUL node
//         tmpNode = emptyNode;
//         tmpNode.name = "mul_" + nodePtrMul0->name + "_" + nodePtrMul1->name;
//         nodePtrMul = LUDAG.insertNode(tmpNode);
//         // assign node properties
//         nodePtrMul->op = "*";
//         nodePtrMul->delay = opDelay["*"];
//         nodePtrMul->mac = true;
//         // attach node to MAC node
//         nodePtrMul->parents.push_back(nodePtrSub);
//         nodePtrSub->children.push_back(nodePtrMul);
//         // attach to multiplicands' node
//         nodePtrMul->children.push_back(nodePtrMul0);
//         nodePtrMul->children.push_back(nodePtrMul1);
//         nodePtrMul0->parents.push_back(nodePtrMul);
//         nodePtrMul1->parents.push_back(nodePtrMul);
//         #ifdef PRINT_DEBUG_2
//         debugFile << "sparseLU : addMACNode: created MUL child " << nodePtrMul->name << endl;
//         #endif
//     }
//     #ifdef PRINT_DEBUG_2
//     debugFile << "sparseLU : addMACNode: created MAC children for "<< matName << i << j << endl;
//     #endif
// }

//****************************************************************************//

void addMACNode(Graph &LUDAG, CS &Uj, CCS &mat, string A_matName, int i,
            int j, vector<vector<pair<string, string>>> &MACs, Node *Ujj)
{
    int k;
    // div node missing for L type node
    if(i > j && Ujj == nullptr)
    {
        cout << "Error: normalizing factor missing";
        exit(0);
    }
    // variable declaration
    unordered_map<string, Node>::iterator graphIt0;
    Node emptyNode, tmpNode, *nodePtrMul0, *nodePtrMul1, *nodePtrSub;

    #ifdef PRINT_DEBUG_2
    debugFile << "sparseLU : addMACNode: init local variables for " << matName << i << j << endl;
    #endif
    
    // Find Aij node
    graphIt0 = LUDAG.nodes.find(getMatNodeName(A_matName, i, j));
    // if Aij is zero use zero node
    graphIt0 = (graphIt0 == LUDAG.nodes.end()) ? LUDAG.nodes.find("0") : graphIt0;
    
    // L nodes (non Lxx nodes)
    if(i > j)
    {
        tmpNode = emptyNode;
        // create Lij node
        tmpNode.name = getMatNodeName(mat.name, i, j);
        // pointer to division node
        nodePtrMul0 = LUDAG.insertNode(tmpNode);
        // create subtraction node
        tmpNode.name = "mac_sub" + getMatNodeName(mat.name, i, j);
        // pointer to subtraction node Uij
        nodePtrSub = LUDAG.insertNode(tmpNode);
        nodePtrSub->matName = mat.name;
        nodePtrSub->matPointer = & mat;
        nodePtrSub->memMapIndex = Uj.getPtrOfIndex(i);

        // connect divion node with children
        nodePtrMul0->children.push_back(nodePtrSub);
        nodePtrMul0->children.push_back(Ujj);
        Ujj->parents.push_back(nodePtrMul0);
        nodePtrSub->parents.push_back(nodePtrMul0);

        // set node properties
        nodePtrMul0->matName = mat.name;
        nodePtrMul0->matPointer = & mat;
        nodePtrMul0->memMapIndex = Uj.getPtrOfIndex(i);
        nodePtrMul0->op = "/";
        nodePtrMul0->delay = opDelay["/"];
        nodePtrMul0->store = true;

        #ifdef PRINT_DEBUG_2
        debugFile << "sparseLU : addMACNode: created div node for U" << i << j << endl;
        #endif

    }
    // Uij nodes
    else
    {
        tmpNode = emptyNode;
        // create subtraction node
        tmpNode.name = getMatNodeName(mat.name, i, j);
        
        // pointer to subtraction node Uij
        nodePtrSub = LUDAG.insertNode(tmpNode);

        // set memory location data
        nodePtrSub->matName = mat.name;
        nodePtrSub->matPointer = &mat;
        nodePtrSub->memMapIndex = Uj.getPtrOfIndex(i);
        nodePtrSub->store = true;
    }

    #ifdef PRINT_DEBUG_2
    debugFile << "sparseLU : addMACNode: created div node " << matName << i << j << endl;
    #endif
    // connect Aij node and sub node
    graphIt0->second.parents.push_back(nodePtrSub);
    nodePtrSub->children.push_back(&graphIt0->second);
    
    // No MAC operations => pass node and return
    if(MACs[i].size() == 0)
    {
        nodePtrSub->op = "pass";
        nodePtrSub->delay = opDelay["pass"];
        return;
    }
    // assign node operation
    nodePtrSub->op = "mac_sub";
    nodePtrSub->delay = opDelay["mac_sub"];

    #ifdef PRINT_DEBUG_2
    debugFile << "sparseLU : addMACNode: created MAC node for "<< matName << i << j << endl;
    #endif

    // generate children's status vector
    // order of status flags is important
    nodePtrSub->childrenStatus.assign(MACs[i].size() + 1, NOT_READY);
    nodePtrSub->totalMACCount = MACs[i].size();
    // connect multiplicands to MAC node
    for(k = 0; k < MACs[i].size(); k++)
    {
        // get pointers to multiplicands
        graphIt0 = LUDAG.nodes.find(MACs[i][k].first);
        nodePtrMul0 = & graphIt0->second;
        graphIt0 = LUDAG.nodes.find(MACs[i][k].second);
        nodePtrMul1 = & graphIt0->second;
        // attach multiplicands' to MAC node in pairs
        nodePtrSub->children.push_back(nodePtrMul0);
        nodePtrSub->children.push_back(nodePtrMul1);
        nodePtrMul0->parents.push_back(nodePtrSub);
        nodePtrMul1->parents.push_back(nodePtrSub);
        #ifdef PRINT_DEBUG_2
        debugFile << "sparseLU : addMACNode: created MUL child " << nodePtrMul->name << endl;
        #endif
    }
    #ifdef PRINT_DEBUG_2
    debugFile << "sparseLU : addMACNode: created MAC children for "<< matName << i << j << endl;
    #endif
}

/******************************************************************************/

void assignGraphValuesToMats(Graph &exeGraph, CCS &U, CCS &L)
{
    for(auto node: exeGraph.nodes)
    {
        if(node.second.store)
        {
            node.second.matPointer->val[node.second.memMapIndex] = node.second.val;
        }
    }
    for(auto same : exeGraph.musthaveSameMemLoc)
    {
        std::get<0>(same)->val[std::get<1>(same)] = std::get<2>(same)->val[std::get<3>(same)];
    }
}

//****************************************************************************//
