#include "Schedule.h"

inline bool isMACType(Node * node)
{
    return node->op == "mac_sub" || node->op == "mac_add";
}

inline int ceilIntDiv(int x, int y)
{
    return (x + y - 1)/y;
}

inline void removeNodes(vector<Node *> & list, vector<Node *> & removeList)
{
    list.erase( remove_if( begin(list),end(list),
    [&](Node * x){return find(begin(removeList),end(removeList),x)!=end(removeList);}), end(list) );
}

inline int rollingIndex(int index,int size, int cycle)
{
    // return((cycle % size + index) % size);
    return((cycle + index) % size);
}


inline bool isLive(const int varId, const vector<int> liveVariables)
{
    return(std::find(liveVariables.begin(), liveVariables.end(), varId) != liveVariables.end());
}

inline string vecToString(const vector<int> vec)
{
    string s = "";
    for(auto n : vec)
    {
        s += to_string(n) + ' ';
    }
    return(s);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Merge Selections
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------


inline bool merrgeSelections(int i, int j, vector<vector<int>> & selectionPriorityTable,
                    vector<vector<int>> & selectionReadPortTable,  
                    vector<vector<int>> & selectionWriteMACPortTable,
                    vector<vector<int>> & selectionWriteDivPortTable,
                    vector<int> freeBRAMPortsCount, vector<int> BRAMPortsCount,
                    vector<int> freeBRAMMACWritePortsCount, 
                    int readyCount, int BRAMCount)
{
    int k;
    // check port count
    //-----------------
    for(k = 0; k < BRAMCount; k++)
    {
        // read port count
        if((selectionReadPortTable[i][k] + selectionReadPortTable[j][k]) > freeBRAMPortsCount[k])
            return(false);

        // write MAC port count

        if((selectionWriteMACPortTable[i][k] + selectionWriteMACPortTable[j][k]) > freeBRAMMACWritePortsCount[k])
            return(false);

        // write Div port count

        if((selectionWriteDivPortTable[i][k] + selectionWriteDivPortTable[j][k]) > BRAMPortsCount[k])
            return(false);
    }

    // merge data
    //-----------
    for(k = 0; k < BRAMCount; k++)
    {
        //read port
        selectionReadPortTable[j][k] += selectionReadPortTable[i][k];
        // write MAC port
        selectionWriteMACPortTable[j][k] += selectionWriteMACPortTable[i][k];
        // write Div port
        selectionWriteDivPortTable[j][k] += selectionWriteDivPortTable[i][k];
    }
    for(k = 0; k < readyCount; k++)
    {
        selectionPriorityTable[j][k] += selectionPriorityTable[i][k];
    }

	return(true);

}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Get priority of Assignment
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------


// 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃
// Incomplete

inline bool compareNodePriorityTuple(const tuple<Node*, int, int> a, const tuple<Node*, int, int> b)
{
    // assending order
    return( std::get<1>(a) > std::get<1>(b)) ;

    //
    // Just bored 🤬 🤬
    //
}

inline int getAssignment(const vector<int> priorityLine,
                const int macUnitCount, const int divUnitCount, Node * macAssignments[], 
                Node * divAssignments[] , int macAssignmentsPairIndex[],
                const vector<Node *> readyNodes, const vector<int> mulPairs, 
                const vector<int> auGroup
                )
{
    // Node , priority, MUP pair
    vector<tuple<Node*, int, int>> macNodes, divNodes;
    int i, j, priority = 0;
    // fill the vectors
    for(i = 0; i < readyNodes.size(); i++)
    {
        if(priorityLine[i] != 0)
        {
            // has possible operation
            if(auGroup[i] == 2)
            {
                // divide node
                divNodes.push_back(std::make_tuple(readyNodes[i], priorityLine[i], 0));
            }
            else
            {
                // Mac node
                macNodes.push_back(std::make_tuple(readyNodes[i], priorityLine[i], mulPairs[i]));
            }
        }
    }
    // sort the vectors to find most prior assignments
    std::sort(macNodes.begin(), macNodes.end(), compareNodePriorityTuple);
    std::sort(divNodes.begin(), divNodes.end(), compareNodePriorityTuple);

    // fill the return table
    j = 0;
    for(i = 0; i < macUnitCount; i ++)
    {
        macAssignments[i] = (j < macNodes.size()) ? std::get<0>(macNodes[j]) : nullptr;
        macAssignmentsPairIndex[i] = (j < macNodes.size()) ? std::get<2>(macNodes[j]) : -1;
        priority += (j < macNodes.size()) ? std::get<1>(macNodes[j]) : 0;
        j ++;
    }

    j = 0;
    for(i = 0; i < divUnitCount; i++)
    {
        divAssignments[i] = (j < divNodes.size()) ? std::get<0>(divNodes[j]) : nullptr;
        priority += (j < divNodes.size()) ? std::get<1>(divNodes[j]) : 0;
        j ++;
    }

    return priority;
}
// 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Assign priority
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------




void assignPriority(Graph & lGraph)
{
    for(int level = lGraph.maxLevel; level > 0; level --)
    {
        for(auto currNode : lGraph.levels[level])
        {
            for(auto parent : currNode->parents)
                currNode->priority += parent->priority;
            if(isMACType(currNode))
                currNode->priority += opDelay[currNode->op] * (currNode->childrenStatus.size() - 1);
            else
                currNode->priority += opDelay[currNode->op];
        }
    }

    for(auto currNode : lGraph.levels[lGraph.maxLevel])
    {
        currNode->priority += 10000000;
    }
}



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Assign Addresses
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void assignAddresses(Graph &lGraph, CCS & A, CCS & L, CCS & U, HardwareConfig & hwConfig)
{
    int requiredMemoryDepth, availableMemoryDepth = 0, currBRAM = 0;
    int i, j, k;
    vector<int> BRAMDepths(hwConfig.BRAMs.size());
    vector<int> numAssigned(hwConfig.BRAMs.size(), 0);
    vector<int>::iterator entryIndex;

    CS csL, csA;

    // nnz(L) + nnz(U) - diagonal entries of L; Lxx = 1 known
    requiredMemoryDepth = L.rowInd.size() + U.rowInd.size() - L.numCols;
    for(i = 0; i < BRAMDepths.size(); i++)
    {
        BRAMDepths[i] = hwConfig.BRAMs[i]->size;
        availableMemoryDepth += BRAMDepths[i];
    }
    
    if(availableMemoryDepth < requiredMemoryDepth)
    {
        cout << "🚧 LIMIT : Insufficient memory space" << endl <<
            "Requires at least " << requiredMemoryDepth << " memory units" << endl;
        exit(1);
    }

    // L elements
    // ------------------
    
    for(i = 0; i < L.numCols; i++)
    {
        L.getSparseColumn(i, csL);
        A.getSparseColumn(i, csA);

        for(j = 1; j < csL.index.size() ; j++) // dont assign memory for Lxx
        {
            // Assign L element
            L.BRAMInd[csL.pointer[j]] = currBRAM;
            L.BRAMAddr[csL.pointer[j]] = numAssigned[currBRAM] ++;

            // assign A element if exists
            entryIndex = std::lower_bound(csA.index.begin(), csA.index.end(), csL.index[j]);
            if(entryIndex != csA.index.end())
            {
                // element exits and assign memory location
                k = std::distance(csA.index.begin(), entryIndex);
                A.BRAMInd[csA.pointer[k]] = L.BRAMInd[csL.pointer[j]];
                A.BRAMAddr[csA.pointer[k]] = L.BRAMAddr[csL.pointer[j]];
            }
        }

        // Lxx Node
        L.BRAMInd[csL.pointer[0]] = BRAMIndConst;
        L.BRAMAddr[csL.pointer[0]] = BRAMAddrConst1;

    // U elements
    // ------------------

        U.getSparseColumn(i, csL);

        for(j = 0; j < csL.index.size(); j++)
        {

            // Assign L element
            U.BRAMInd[csL.pointer[j]] = currBRAM;
            U.BRAMAddr[csL.pointer[j]] = numAssigned[currBRAM] ++;

            // assign A element if exists
            entryIndex = std::lower_bound(csA.index.begin(), csA.index.end(), csL.index[j]);
            if(entryIndex != csA.index.end())
            {
                // element exits and assign memory location
                k = std::distance(csA.index.begin(), entryIndex);
                A.BRAMInd[csA.pointer[k]] = U.BRAMInd[csL.pointer[j]];
                A.BRAMAddr[csA.pointer[k]] = U.BRAMAddr[csL.pointer[j]];
            }
        }

        // next BRAM
        currBRAM = (currBRAM == hwConfig.BRAMs.size() - 1) ? 0 : currBRAM + 1;
    }
}


void assignScatteredAddresses(Graph &lGraph, CCS & A, CCS & L, CCS & U, HardwareConfig & hwConfig)
{
    int requiredMemoryDepth, availableMemoryDepth = 0, currBRAM = 0;
    int i, j, k;
    vector<int> BRAMDepths(hwConfig.BRAMs.size());
    vector<int> numAssigned(hwConfig.BRAMs.size(), 0);
    vector<int>::iterator entryIndex;

    CS csL, csA;

    // nnz(L) + nnz(U) - diagonal entries of L; Lxx = 1 known
    requiredMemoryDepth = L.rowInd.size() + U.rowInd.size() - L.numCols;
    for(i = 0; i < BRAMDepths.size(); i++)
    {
        BRAMDepths[i] = hwConfig.BRAMs[i]->size;
        availableMemoryDepth += BRAMDepths[i];
    }
    
    if(availableMemoryDepth < requiredMemoryDepth)
    {
        cout << "🚧 LIMIT : Insufficient memory space" << endl <<
            "Requires at least " << requiredMemoryDepth << " memory units" << endl;
        exit(1);
    }

    // scatter L elements
    // ------------------
    
    for(i = 0; i < L.numCols; i++)
    {
        L.getSparseColumn(i, csL);
        A.getSparseColumn(i, csA);

        for(j = 1; j < csL.index.size() ; j++) // dont assign memory for Lxx
        {
            // find free memory location
            while( numAssigned[currBRAM] >= BRAMDepths[currBRAM])
                currBRAM = (currBRAM == hwConfig.BRAMs.size() -1 ) ? 0 : currBRAM + 1;

            // Assign L element
            L.BRAMInd[csL.pointer[j]] = currBRAM;
            L.BRAMAddr[csL.pointer[j]] = numAssigned[currBRAM] ++;
            currBRAM = (currBRAM == hwConfig.BRAMs.size() - 1) ? 0 : currBRAM + 1;

            // assign A element if exists
            entryIndex = std::lower_bound(csA.index.begin(), csA.index.end(), csL.index[j]);
            if(entryIndex != csA.index.end())
            {
                // element exits and assign memory location
                k = std::distance(csA.index.begin(), entryIndex);
                A.BRAMInd[csA.pointer[k]] = L.BRAMInd[csL.pointer[j]];
                A.BRAMAddr[csA.pointer[k]] = L.BRAMAddr[csL.pointer[j]];
            }
        }

        L.BRAMInd[csL.pointer[0]] = BRAMIndConst;
        L.BRAMAddr[csL.pointer[0]] = BRAMAddrConst1;

    // scatter U elements
    // ------------------

        U.getSparseColumn(i, csL);

        for(j = 0; j < csL.index.size(); j++)
        {
            // find free memory location
            while( numAssigned[currBRAM] >= BRAMDepths[currBRAM])
                currBRAM = (currBRAM == hwConfig.BRAMs.size() - 1) ? 0 : currBRAM + 1;

            // Assign L element
            U.BRAMInd[csL.pointer[j]] = currBRAM;
            U.BRAMAddr[csL.pointer[j]] = numAssigned[currBRAM] ++;
            currBRAM = (currBRAM == hwConfig.BRAMs.size() - 1) ? 0 : currBRAM + 1;

            // assign A element if exists
            entryIndex = std::lower_bound(csA.index.begin(), csA.index.end(), csL.index[j]);
            if(entryIndex != csA.index.end())
            {
                // element exits and assign memory location
                k = std::distance(csA.index.begin(), entryIndex);
                A.BRAMInd[csA.pointer[k]] = U.BRAMInd[csL.pointer[j]];
                A.BRAMAddr[csA.pointer[k]] = U.BRAMAddr[csL.pointer[j]];
            }
        }
    }
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Print Schedular Window
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------


void printAUWindow(const vector<vector<AU_Operation>> AUData, const int startIndex)
{
    int i, j;
    i = startIndex;
    do
    {
        debugFile << "--------------------------------------------------------------------------------------" << endl;
        debugFile << "0️⃣ Line " << i << " Window Size " << AUData.size() << endl;
        // print AU index
        debugFile << endl << setw(10) << "AU Index" << " | ";
        for(j = 0; j < AUData[i].size(); j++)
        {
            debugFile << setw(15) << AUData[i][j].AUIndex << " | ";
        }

        // Print OP and Node ID
        debugFile << endl << setw(10) << "OP : Id" << " | ";
        for(j = 0; j < AUData[i].size(); j++)
        {
            debugFile << setw(15) << AUData[i][j].op + " : " + to_string(AUData[i][j].outputId) << " | ";
        }
        // Input Ids 
        debugFile << endl << setw(10) << "In Id" << " | ";
        for(j = 0; j < AUData[i].size(); j++)
        {
            debugFile << setw(15) << vecToString(AUData[i][j].inputIds) << " | ";
        }
        // Input Loc 
        debugFile << endl << setw(10) << "In Loc" << " | ";
        for(j = 0; j < AUData[i].size(); j++)
        {
            debugFile << setw(15) << vecToString(AUData[i][j].inputLocs) << " | ";
        }
        // Input Loc 
        debugFile << endl << setw(10) << "BRAM Index" << " | ";
        for(j = 0; j < AUData[i].size(); j++)
        {
            debugFile << setw(15) << AUData[i][j].BRAMIndex << " | ";
        }
        // Retire Clock 
        debugFile << endl << setw(10) << "Retire At" << " | ";
        for(j = 0; j < AUData[i].size(); j++)
        {
            debugFile << setw(15) << AUData[i][j].retireClock << " | ";
        }
        debugFile << endl;
        // update line index
        i++;
        i = (i == AUData.size()) ? 0 : i ;
    }while(i != startIndex);
    debugFile << endl;
}

void printMemWindow(const vector<vector<BRAM_Operation>> memData, const int startIndex)
{
        int i, j;
    i = startIndex;
    do
    {
        debugFile << "--------------------------------------------------------------------------------------" << endl;
        debugFile << "0️⃣ Line " << i << endl;
        // print BRAM index
        debugFile << endl << setw(10) << "BRAM Index" << " | ";
        for(j = 0; j < memData[i].size(); j++)
        {
            debugFile << setw(15) << memData[i][j].BRAMIndex << " | ";
        }

        // Print OP and Node ID
        debugFile << endl << setw(10) << "OP : Id" << " | ";
        for(j = 0; j < memData[i].size(); j++)
        {
            debugFile << setw(15) << to_string(memData[i][j].op) + " : " + to_string(memData[i][j].nodeId) << " | ";
        }
        // Port Index
        debugFile << endl << setw(10) << "Port Id" << " | ";
        for(j = 0; j < memData[i].size(); j++)
        {
            debugFile << setw(15) << memData[i][j].portIndex << " | ";
        }
        // Addresses
        debugFile << endl << setw(10) << "Addr" << " | ";
        for(j = 0; j < memData[i].size(); j++)
        {
            debugFile << setw(15) << memData[i][j].addr << " | ";
        }
        // Din Loc
        debugFile << endl << setw(10) << "Din Loc" << " | ";
        for(j = 0; j < memData[i].size(); j++)
        {
            debugFile << setw(15) << memData[i][j].dinLoc << " | ";
        }
        debugFile << endl;
        // update line index
        i++;
        i = (i == memData.size()) ? 0 : i ;
    }while(i != startIndex);
    debugFile << endl;
}

void printScheduleWindow(const vector<vector<AU_Operation>> AUAssignData,
                const vector<vector<AU_Operation>> AURetireData,
                const vector<vector<BRAM_Operation>> memData, 
                const int AURetireWindowStart, const int memWindowStart, const int clockCycle)
{
    int i, j;
    debugFile << "🏗 🏗 🏗 🏗 🏗 Schedule Window : " << clockCycle << " 🏗 🏗 🏗 🏗 🏗" << endl;
    debugFile << endl << "💶 💶 💶 💶 💶 💶 💶 💶 💶 AU Retire Window 💶 💶 💶 💶 💶 💶 💶 💶 💶" << endl << endl;
    printAUWindow(AURetireData, AURetireWindowStart);
    debugFile << endl << "💶 💶 💶 💶 💶 💶 💶 💶 💶 AU Assign Window 💶 💶 💶 💶 💶 💶 💶 💶 💶" << endl << endl;
    printAUWindow(AUAssignData, memWindowStart);
    debugFile << endl << "💶 💶 💶 💶 💶 💶 💶 💶 💶 Memory Data Window 💶 💶 💶 💶 💶 💶 💶 💶 💶" << endl << endl;
    printMemWindow(memData, memWindowStart);
    debugFile << endl << endl;
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Update Done status
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

int updateDoneStatus(vector<Node *> & retiringNodes, vector<Node *> & mayBeReadyNodes)
{
    int tmpInt, doneCount = 0;
    mayBeReadyNodes.clear();

    // clear ready MUl pair of all MAC nodes

    for(auto node : retiringNodes)
    {
        if(isMACType(node))
        {
            // #ifdef PRINT_DEBUG_0
            //     debugFile << "🦗 🐞 🦗 🐞 🦗"  << node->nodeId << endl;
            // #endif
            node->childrenStatus[0] = DONE;
            // eviction of dome MUL pair is done at the operation assignment
            node->doneMACCount++;

            #ifdef PRINT_DEBUG_1
                debugFile << "🦗 🐞 🦗 🐞 🦗 Form Done Update Function 🦗 🐞 🦗 🐞 🦗" << node->name << endl;
                debugFile << "Done MACs: " << node->doneMACCount << " Total MACs: " << node->totalMACCount << endl << endl;
            #endif
            // if all MACs done
            if(node->doneMACCount == node->totalMACCount)
            {
                // change done status
                node->done = true;
            }
            else if(node->doneMACCount == 1)
            {
                // change child 0 to itself
                node->children[0] = node;

                mayBeReadyNodes.push_back(node);
                // reduce priority of MAC node by one MAC
                //node->priority -= opDelay[node->op];
            }
            else
            {
                mayBeReadyNodes.push_back(node);
                // reduce priority of MAC node by one MAC
                node->priority -= opDelay[node->op];
            }
        }
        // non MAC type node
        else
        {
            // change status
            node->done = true;
        }

        #ifdef PRINT_DEBUG_3
            debugFile << "🦗 🐞 🦗 🐞 🦗 Retiring Node Name:" << node->name << endl;
            debugFile << "Parents : ";
            for(auto parent : node->parents)
            {
                debugFile << parent->name << ' ';
            }
            debugFile << endl << "Operation: " << node->op << " Done = " << node->done << endl;
        #endif

        // find ready parents
        if(node->done)
        {
            doneCount ++;
            #ifdef PRINT_DEBUG_1
                debugFile << "Done Node "  << node->name << endl;
            #endif
            for(auto parent : node->parents)
            {
                if(isMACType(parent))
                {
                    // update child status
                    // tmpInt is location of child
                    tmpInt = std::distance(parent->children.begin(), std::find(parent->children.begin(), parent->children.end(), node) );
                    if(tmpInt == parent->children.size())
                    {
                        // pain hai
                        cout << "🚨 ERROR: Graph is broken 🔗" << endl;
                        cout << "    " << node->name << " is not present in " << parent->name << "'s children list" << endl;
                    }
                    // location of status
                    tmpInt = ceilIntDiv(tmpInt, 2);

                    // when Leaf nodes retire => change child 0 status to DONE
                    if(tmpInt == 0)
                    {
                        parent->childrenStatus[0] = DONE;
                        continue;
                    }
                    // update status
                    switch(parent->childrenStatus[tmpInt])
                    {
                        case NOT_READY:
                            parent->childrenStatus[tmpInt] = ONE_READY;
                            break;
                        case ONE_READY:
                            // push parent to mayBeReadyNodes
                            parent->childrenStatus[tmpInt] = READY;
                            parent->readyMULPairIndex.push_back(tmpInt);
                            mayBeReadyNodes.push_back(parent);
                            break;
                        default:
                            cout << "🚨 ERROR: MAC operation is already DONE/ READY" << endl;
                            cout << "Parent Node: " << parent->name << "; Child Node: " << node->name << endl;
                            exit(1);
                    }
                }
                else
                {
                    // add parent to mayBeReadyNodes
                    mayBeReadyNodes.push_back(parent);
                }
            }
        }

        #ifdef PRINT_DEBUG_2
            debugFile << "🦗 🐞 🦗 🐞 🦗 May Be Ready Nodes Size:" << mayBeReadyNodes.size() << endl;
        #endif
    }

    // remove duplicates from mayBeReadyNodes
    set <Node *> s;
    for(tmpInt = 0; tmpInt < mayBeReadyNodes.size(); tmpInt++ ) s.insert( mayBeReadyNodes[tmpInt] );
    mayBeReadyNodes.assign( s.begin(), s.end() );

        #ifdef PRINT_DEBUG_1
            debugFile << "🦗 🐞 🦗 🐞 🦗 May Be Ready Nodes Size:" << mayBeReadyNodes.size() << endl;
        #endif

    return(doneCount);
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Find ready Nodes
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

inline bool liveOrNotDirtyAndDone(const Node * node, const vector<int> liveVariables)
{
    return(isLive(node->nodeId, liveVariables)  || (!node->dirty && node->done));
}

inline bool liveOrNotDirty(const Node * node, const vector<int> liveVariables)
{
    return(isLive(node->nodeId, liveVariables)  || (node->dirty == false));
}

inline bool isDone(const Node * node)
{
    return(node->done == true);
}

inline bool compareNodes(Node * a, Node * b)
{
    return(a->priority > b->priority);
}

void findReadyNodes(vector<Node *> & readyNodes, vector<Node *> & mayBeReadyNodes, vector<Node *> previousCycle, vector<int> liveVariables)
{
    bool ready = true;
    int i;
    // remove already scheduled nodes from ready nodes
    removeNodes(readyNodes, previousCycle);

    for(auto node : mayBeReadyNodes)
    {
        if(isMACType(node))
        {
            // node->schedulableMULPairIndex.clear();

            // if(node->name == "U_3_4")
            // {
            //     cout << "Add Break Point" << endl;
            // }
            if(node->childrenStatus[0] != DONE)
            {
                // node is not ready
                continue;
            }
            // // MUL pairs
            // for(auto j : node->readyMULPairIndex)
            // {
            //     // ready mul pair index => both are done
            //     if(liveOrNotDirtyAndDone(node->children[2 * j], liveVariables) && 
            //         liveOrNotDirtyAndDone(node->children[2 * j - 1], liveVariables))
            //     {
            //         node->schedulableMULPairIndex.push_back(j);
            //     }
            // }
            // some pair is ready ..
            if(node->readyMULPairIndex.size() > 0)
            {
                readyNodes.push_back(node);
            }
        }
        else
        {
            ready = true;
            for(auto child : node->children)
            {
                // ready = liveOrNotDirtyAndDone(child, liveVariables);
                ready = isDone(child);
                if(! ready)
                {
                    break;
                }
            }
            if(ready)
            {
                readyNodes.push_back(node);
            }
        }
    }

    // remove duplicates from mayBeReadyNodes
    set <Node *> s;
    int tmpInt;
    for(tmpInt = 0; tmpInt < readyNodes.size(); tmpInt++ ) s.insert( readyNodes[tmpInt] );
    readyNodes.assign( s.begin(), s.end() );

    // sort ready nodes in decreasing order
    std::sort(readyNodes.begin(), readyNodes.end(), compareNodes);

}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Update Dirty Memory Status
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void updateDirtyMemoryStatus(vector<Node *> completedWrites, vector<Node *> currCycle)
{
    // clear dirty bit for completed writes
    for(auto node : completedWrites )
    {
        node->dirty = false;
    }

    // set dirty bit for recently scheduled nodes
    for(auto node : currCycle)
    {
        node->dirty = true;
    }
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// simple schedule
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------


// only pipelined AUs wih operator groupings
void scheduleNoReg(Graph & lGraph, HardwareConfig & hwConfig, Schedule & schedule)
{
    #ifdef PRINT_DEBUG_1
        debugFile << "🥇 : Entering Schedular Function" << endl;
    #endif
    
    
    // variable declaration
    int numPorts = 0, i, j, k, tmpInt, maxPrioritySelection;
    int AUDataWindowSize, memDataWindowSize;
    int clockCycle = 0;
    int numScheduledNodes = 0;
    int numNodesInGraph = lGraph.nodes.size();
    int readLatency = hwConfig.BRAMs[0]->readLatency;
    bool tmpBool, done;
    int tmpIntArr[5], tmpIntArrSize = 0;
    Node * tmpNodePtrArr[5];
    int  tmpNodePtrArrSize = 0;
    vector<Node *> readyNodes, previousAssignedNodes, mayBeReadyNodes, retiringNodes, completedWrites;
    int BRAMIndex, BRAMAddr;
    AU_Operation emptyAUOp;
    BRAM_Operation emptyBRAMOp;
    int maxLivePair, maxLiveCount;

    // priority lookup table for selection
    vector<vector<int>> selectionPriorityTable(SELECTION_TABLE_SIZE, vector<int>(SELECTION_TABLE_SIZE, 0));
    // required read ports for selection
    vector<vector<int>> selectionReadPortTable(SELECTION_TABLE_SIZE, vector<int>(MAX_BRAMS, 0));
    // required write MAC ports for selection
    vector<vector<int>> selectionWriteMACPortTable(SELECTION_TABLE_SIZE, vector<int>(MAX_BRAMS, 0));
    vector<int> selectionWriteMACPortFreeCount(MAX_BRAMS, 0);
    // required write DIV ports for selection
    vector<vector<int>> selectionWriteDivPortTable(SELECTION_TABLE_SIZE, vector<int>(MAX_BRAMS, 0));
    // MUL pair index in selection table
    vector<int> selectionPairIndexTable(SELECTION_TABLE_SIZE, -1);
    // selection table node pointer table
    vector <Node *> selectionNodeTable(SELECTION_TABLE_SIZE);
    int activeSelectionTableSize = 0;
    
    // AU assignment location
    vector<int> auAssignLocation(hwConfig.AUs.size());
    

    // if number of BRAMs > MAX_BRAMS
    if(hwConfig.BRAMs.size() > MAX_BRAMS)
    {
        cout << "🚧 LIMIT : Maximum number of BRAMS is set to " << MAX_BRAMS << endl;
        exit(1);
    }
    // schedular data
    // memory ports
    // check if read and wr pipeLatencies same
    map<int, pair<int, int>> liveToBRAMLookup;
    vector<vector<int>> BRAMToLiveLookup(hwConfig.BRAMs.size());
    vector<int> BRAMPortsCount(hwConfig.BRAMs.size());

    for(i = 0; i < hwConfig.BRAMs.size(); i++)
    {
        BRAMPortsCount[i] = hwConfig.BRAMs[i]->ports;
        for(j = 0; j < BRAMPortsCount[i]; j++)
        {
            liveToBRAMLookup.insert(pair<int, pair<int, int>>
                (hwConfig.AUs.size() + numPorts +  j, pair<int, int>(i, j)));

            BRAMToLiveLookup[i].push_back(hwConfig.AUs.size() + numPorts + j);
        }
        numPorts += BRAMPortsCount[i];
    }

        // #ifdef PRINT_DEBUG_0
        // debugFile << "🦗 🐞 🦗 🐞 🦗"  << i << endl;
        // #endif

    // schedule Au window
    int maxAuDelay = 0;
    for(i = 0; i < hwConfig.AUs.size(); i++)
    {
        if(! hwConfig.AUs[i]->pipelined)
        {
            cout << "🚨 ERROR: Only pipelined AUs supported!" << endl;
            exit(1);
        }
        if(maxAuDelay < hwConfig.AUs[i]->pipeLatency)
        {
            maxAuDelay = hwConfig.AUs[i]->pipeLatency;
        }
        auAssignLocation[i] = readLatency + hwConfig.AUs[i]->pipeLatency;
    }

    // operator based AU data
    // 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃
    // Incomplete:
    int macUnitCount = 0;
    int divUnitCount = 0;
    vector<int> auGroup(SELECTION_TABLE_SIZE,0);
    for(auto au : hwConfig.AUs)
    {
        if(au->numOperations == 5)
            macUnitCount ++;
        else
            divUnitCount ++;
    }

	Node * * macAssignments = new Node *[macUnitCount];
	Node * * divAssignments = new Node *[divUnitCount];
	Node * * macAssignmentsTmp = new Node *[macUnitCount];
	Node * * divAssignmentsTmp = new Node *[divUnitCount];
	int *macAssignmentsPairIndex = new int[macUnitCount];
	int *macAssignmentsPairIndexTmp = new int[macUnitCount];
    // 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃




    // mem data
    memDataWindowSize = 1 + readLatency;
    vector<vector<BRAM_Operation>> memData(memDataWindowSize, vector<BRAM_Operation>(numPorts));
    vector<int> freeBRAMPortsCount(hwConfig.BRAMs.size(), 0);
    // AU retire data
    AUDataWindowSize = memDataWindowSize + maxAuDelay;
    vector<vector<AU_Operation>> AURetireData(AUDataWindowSize, vector<AU_Operation>(hwConfig.AUs.size()));
    // AU assign data
    vector<vector<AU_Operation>> AUAssignData(memDataWindowSize, vector<AU_Operation>(hwConfig.AUs.size()));
    vector<int> doneNodeCount(memDataWindowSize, 0);
    // live variable list
    vector<int> liveVariables;
    // vector<int> cycleZeroRetireNodes;
    vector<int> cycleZeroAssignNodes;
	// live variables map
	vector<int> liveVariablesMap(hwConfig.AUs.size() + numPorts + 1, 0);

    #ifdef PRINT_DEBUG_1
        debugFile << "🥇 : schedular Initialization Complete. Entering Schedular Loop" << endl;
    #endif

    // cout << "Mem Data Window Size: " << memDataWindowSize << endl;
    // cout << "AU Retire Window Size: " << AUDataWindowSize << endl;

    // ---------------------------
    // 🏐 🏐 🏐 the schedular loop
    // ---------------------------
    retiringNodes = lGraph.leaves;
    int FLOPS = 0;

    while(numScheduledNodes < numNodesInGraph)
    {
        #ifdef PRINT_DEBUG_0
            debugFile << endl << endl;
            debugFile << "⏰ ⏰ ⏰ ⏰ ⏰ ⏰ : Clock Cycle:" << clockCycle << 
                " ⏰ ⏰ ⏰ ⏰ ⏰ ⏰" << endl << endl << endl;
        #endif
        
        //---------------
        // reset data
        //----------------
        // std::fill(selectionWrCancellablePortTable.begin(), selectionWrCancellablePortTable.begin() + readyNodes.size(), vector<int>(MAX_BRAMS, 0));
        std::fill(selectionWriteMACPortTable.begin(), selectionWriteMACPortTable.end(), vector<int>(MAX_BRAMS, 0));
        std::fill(selectionWriteDivPortTable.begin(), selectionWriteDivPortTable.end(), vector<int>(MAX_BRAMS, 0));
        // std::fill(selectionReadPortTable.begin(), selectionReadPortTable.end(), vector<int>(MAX_BRAMS, 0));
        std::fill(selectionPriorityTable.begin(), selectionPriorityTable.end(), vector<int>(SELECTION_TABLE_SIZE, 0));
        std::copy(BRAMPortsCount.begin(), BRAMPortsCount.end(), freeBRAMPortsCount.begin());
        std::copy(BRAMPortsCount.begin(), BRAMPortsCount.end(), selectionWriteMACPortFreeCount.begin());
        
        // clear window data
        std::fill(liveVariablesMap.begin(), liveVariablesMap.end() - 1, nodeIdNULL);
        for(auto & op : AUAssignData[rollingIndex(readLatency, memDataWindowSize, clockCycle)])
        {
            op.op = "NO_OP";
        }
        for(auto & op : memData[rollingIndex(readLatency, memDataWindowSize, clockCycle)])
        {
            op.op = BRAM_NOTHING;
        }

        #ifdef PRINT_DEBUG_3
        if(clockCycle == 74)
        {
            debugFile << rollingIndex(0, memDataWindowSize, clockCycle) << endl;
            printScheduleWindow(AUAssignData, AURetireData, memData, rollingIndex(0, AUDataWindowSize, clockCycle),
                rollingIndex(0, memDataWindowSize, clockCycle), clockCycle);
        }
        #endif

        #ifdef PRINT_DEBUG_0
        if(clockCycle == 70)
        {
            cout << "Break" << endl;
        }
        debugFile << "👍 👍 👍 : Retiring Nodes :" << endl;
        tmpInt = 0;
        for(auto node : retiringNodes)
        {
            debugFile <<setw(2) << tmpInt << " Node Name: " << node->name << "; Node ID: " <<
                node->nodeId << endl;
            tmpInt ++;
        }
        #endif


        // -------------------------------------------------------------
        // update live variables => cycle 0 Write Nodes + cycle n retires
        //--------------------------------------------------------------
        liveVariables.assign({0});          // is is always present
        cycleZeroAssignNodes.clear();
        // cycleZeroRetireNodes.clear();
        // cycle n
        FLOPS += retiringNodes.size();
        for(auto node : retiringNodes)
        {
            if(clockCycle > 0)
            {
                liveVariables.push_back(node->nodeId);
            }
        }

        // #ifdef PRINT_DEBUG_0
        // debugFile << "🦗 🐞 🦗 🐞 🦗 AU Assign Data Rolling Index 0 :"  << rollingIndex(0, memDataWindowSize, clockCycle) << endl;
        // #endif

        // cycle 0 write nodes => (cycle 0 nodes)
        for(auto op : AUAssignData[rollingIndex(0, memDataWindowSize, clockCycle)])
        {
            if(op.op != "NO_OP")
                cycleZeroAssignNodes.push_back(op.nodePtr->nodeId);
        }

        for(auto op : AURetireData[rollingIndex(0, AUDataWindowSize, clockCycle)])
        {
            if(op.op != "NO_OP")
            {
                // cycleZeroRetireNodes.push_back(op.nodePtr->nodeId);
                // if node is not present in assign 0
                if(cycleZeroAssignNodes.end() == std::find(cycleZeroAssignNodes.begin(), cycleZeroAssignNodes.end(), op.nodePtr->nodeId))
                {
                    // output must be written
                    liveVariables.push_back(op.outputId);
                    freeBRAMPortsCount[op.BRAMIndex] -= 1;

                    // make memdata and live variable entry

                    // location of live variable
                    i = BRAMToLiveLookup[op.BRAMIndex][freeBRAMPortsCount[op.BRAMIndex]];
                    liveVariablesMap[i] = op.outputId;
                    // update mem operation object
                    emptyBRAMOp.BRAMIndex = op.BRAMIndex;
                    emptyBRAMOp.addr = op.BRAMAddress;
                    emptyBRAMOp.dinLoc = op.AUIndex;
                    emptyBRAMOp.nodeId = op.outputId;
                    emptyBRAMOp.op = BRAM_WRITE;
                    emptyBRAMOp.portIndex = freeBRAMPortsCount[op.BRAMIndex];
                    emptyBRAMOp.nodePtr = op.nodePtr;

                    // deep copy operation to table
                    memData[rollingIndex(readLatency, memDataWindowSize, clockCycle)][i - hwConfig.AUs.size()] = emptyBRAMOp;
                }
            }
        }
        


        #ifdef PRINT_DEBUG_0
        // if(clockCycle == 33)
        // {
            debugFile << endl;
            debugFile << "💃 💃 💃 : Live Variables :" << endl;
            for(auto var : liveVariables)
            {
                debugFile <<setw(4) << var << ' ';
            }
            debugFile << endl;
        // }
        #endif
        
        // clear previous cycle
        mayBeReadyNodes.clear();
        // update done status
        doneNodeCount[rollingIndex(readLatency, memDataWindowSize, clockCycle)] = updateDoneStatus(retiringNodes, mayBeReadyNodes);

        #ifdef PRINT_DEBUG_1
        // if(clockCycle == 33)
        // {
            debugFile << endl;
            debugFile << "🤷 🤷 🤷 : May be Ready Nodes :" << endl;
            tmpInt = 0;
            for(auto node : mayBeReadyNodes)
            {
                debugFile <<setw(2) << tmpInt << " Node Name: " << node->name << "; Node ID: " <<
                    node->nodeId << endl;
                tmpInt ++;
            }
        // }
        #endif
        // find ready nodes
        findReadyNodes(readyNodes, mayBeReadyNodes, previousAssignedNodes, liveVariables);

        #ifdef PRINT_DEBUG_0
        // if(clockCycle == 33)
        // {
            debugFile << endl << "🙋 🙋 🙋 : Ready Nodes :" << endl;
            tmpInt = 0;
            for(auto node : readyNodes)
            {
                debugFile <<setw(2) << tmpInt << " Node Name: " << node->name << "; Node ID: " <<
                    node->nodeId << endl;
                tmpInt ++;
            }
        // }
        #endif

        // -------------------------
        // fill the selection table
        //--------------------------
        // freeMACPorts table
        i = 0;
        for(auto node : readyNodes)
        {
            if(isMACType(node))
            {
                #ifdef PRINT_DEBUG_1
                    if(node->name == "U_121_121")
                    {
                        cout << "Add Debug Point Here" << endl;
                    }
                #endif

                // update schedulable pairs
                node->schedulableMULPairIndex.clear();
                for(auto j : node->readyMULPairIndex)
                {
                    // ready mul pair index => both are done
                    if(liveOrNotDirty(node->children[2 * j], liveVariables) && 
                        liveOrNotDirty(node->children[2 * j - 1], liveVariables))
                    {
                        node->schedulableMULPairIndex.push_back(j);
                    }
                }

                
                // child 0
                tmpNodePtrArr[0] = node->children[0];
                tmpIntArr[0] = tmpNodePtrArr[0]->nodeId;

                maxLiveCount = 0;
                maxLivePair = 0;
                
                // find the schedulable pair with most live inputs
                for(int pair : node->schedulableMULPairIndex)
                {
                    // MUL Pair
                    tmpNodePtrArr[1] = node->children[pair * 2];
                    tmpIntArr[1] = tmpNodePtrArr[1]->nodeId;
                    tmpNodePtrArr[2] = node->children[pair * 2 - 1];
                    tmpIntArr[2] = tmpNodePtrArr[2]->nodeId;

                    tmpInt = 3;

                    // clear the row
                    std::fill(selectionReadPortTable[i].begin(), selectionReadPortTable[i].end(), 0);

                    // checking live inputs
                    for(j = 0; j < 3; j++)
                    {
                        if(! isLive(tmpIntArr[j], liveVariables))
                        {
                            // variable is not live; add to read list
                            // tmpNodePtrArr[j]->matPointer->BRAMInd
                            selectionReadPortTable[i][ tmpNodePtrArr[j]->matPointer->BRAMInd[tmpNodePtrArr[j]->memMapIndex] ] += 1;
                            tmpInt -- ;
                        }
                    }
                    // check if node itself can be scheduled in this cycle
                    tmpBool = true;
                    for(j = 0; j < freeBRAMPortsCount.size(); j++)
                    {
                        if(selectionReadPortTable[i][j] > freeBRAMPortsCount[j])
                        {
                            // node itself is not schedulable in this cycle
                            tmpBool = false;
                            break;
                        }
                    }

                    if((tmpBool == true) && (tmpInt >= maxLiveCount))
                    {
                        maxLiveCount = tmpInt;
                        maxLivePair = pair;
                        if(maxLiveCount == 3)
                        {
                            // 3 variable function
                            break;
                        }
                    }
                }

                if(maxLivePair == 0)
                {
                    // nothing can be scheduled; so continue
                    debugFile << "Rejecting " << node->name << " : No Schedulable Pair" << endl;
                    continue;
                }
                // fill selectionPairIndexTable
                selectionPairIndexTable[i] = maxLivePair;
                
                // MUL Pair
                tmpNodePtrArr[1] = node->children[maxLivePair * 2];
                tmpIntArr[1] = tmpNodePtrArr[1]->nodeId;
                tmpNodePtrArr[2] = node->children[maxLivePair * 2 - 1];
                tmpIntArr[2] = tmpNodePtrArr[2]->nodeId;

                // clear the row
                std::fill(selectionReadPortTable[i].begin(), selectionReadPortTable[i].end(), 0);
                // checking live inputs
                for(j = 0; j < 3; j++)
                {
                    if(! isLive(tmpIntArr[j], liveVariables))
                    {
                        // variable is not live; add to read list
                        // tmpNodePtrArr[j]->matPointer->BRAMInd
                        selectionReadPortTable[i][ tmpNodePtrArr[j]->matPointer->BRAMInd[tmpNodePtrArr[j]->memMapIndex] ] += 1;
                    }
                }

                // fill selection priority
                selectionPriorityTable[i][i] = node->priority;

                // fill write data
                // std::fill(selectionWritePortTable[i].begin(), selectionWritePortTable[i].end(), 0);
                selectionWriteMACPortTable[i][node->matPointer->BRAMInd[node->memMapIndex]] += 1;

                // fill selectionNodeTable
                selectionNodeTable[i] = node;

                // auGroupType
                // 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃
                // incomplete
                auGroup[i] = (node->op == "/") ? 2 : 1;
                // 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃

                // increment the pointer
                i++ ;
            }
            // Non MAC type node
            else
            {
                // fill selection priority
                selectionPriorityTable[i][i] = node->priority;

                // fill selectionNodeTable
                selectionNodeTable[i] = node;
                
                // clear the read table row
                std::fill(selectionReadPortTable[i].begin(), selectionReadPortTable[i].end(), 0);
                
                tmpBool = true;
                for(auto child : node->children)
                {
                    if(liveOrNotDirty(child, liveVariables))
                    {
                        if(! isLive(child->nodeId, liveVariables))
                        {
                            int lookup = child->matPointer->BRAMInd[child->memMapIndex];
                            selectionReadPortTable[i][lookup] += 1;
                        }
                    }
                    else
                    {
                        // someone is done but dirty and not live
                        tmpBool = false;
                        break;
                    }
                }

                // continue case
                if( ! tmpBool)
                {
                    // move to next node
                    debugFile << "Rejecting " << node->name << " : dirty inputs" << endl;
                    continue;
                }

                // check if node itself can be scheduled in this cycle
                tmpBool = true;
                for(j = 0; j < freeBRAMPortsCount.size(); j++)
                {
                    if(selectionReadPortTable[i][j] > freeBRAMPortsCount[j])
                    {
                        // node itself is not schedulable in this cycle
                        tmpBool = false;
                        debugFile << "Rejecting " << node->name << " : insufficient ports" << endl;
                        break;
                    }
                }

                // fill write data
                selectionWriteDivPortTable[i][node->matPointer->BRAMInd[node->memMapIndex]] += 1;

                // auGroupType
                // 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃
                // incomplete
                auGroup[i] = (node->op == "/") ? 2 : 1;
                // 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃


                i = tmpBool ? i + 1 : i;
            }

            if(i == SELECTION_TABLE_SIZE)
            {
                break;
            }
        }

        activeSelectionTableSize = i;

        
        #ifdef PRINT_DEBUG_1
        debugFile << endl << "🥇 Selection Table Filled. Active Width: " << activeSelectionTableSize << endl;

        for(i = 0; i < activeSelectionTableSize; i++)
        {
            debugFile << selectionNodeTable[i]->name << ' ';
        }
        debugFile << endl;
        #endif
        
        
        // ------------------------
        // select assignments
        // ------------------------

        for(auto op : AURetireData[rollingIndex(auAssignLocation[0], AUDataWindowSize, clockCycle)])
        {
            if( op.op == "/" )
            {
                selectionWriteMACPortFreeCount[op.BRAMIndex] -= 1;
            }
        }

        // selection merge loop
        for(i = 0; i < activeSelectionTableSize -1 ; i++)
        {
            for(j = i + 1; j < activeSelectionTableSize; j++)
            {
                #ifdef PRINT_DEBUG_1
                if(clockCycle == 33)
                {
                    debugFile << "Read Port Status before Merger of " << i << ' ' << j << endl;
                    for(int tt : selectionReadPortTable[j])
                    {
                        debugFile << ' ' << tt;
                    }
                    debugFile << endl;
                    for(int tt : selectionReadPortTable[i])
                    {
                        debugFile << ' ' << tt;
                    }
                    debugFile << endl << "-------------------------------------------------------" << endl;
                }
                #endif
                // inline function, so will be replaced
                bool tatti;
                tatti = merrgeSelections(i, j, selectionPriorityTable, selectionReadPortTable,
                selectionWriteMACPortTable, selectionWriteDivPortTable, freeBRAMPortsCount, BRAMPortsCount,
                selectionWriteMACPortFreeCount, activeSelectionTableSize, BRAMPortsCount.size());

                #ifdef PRINT_DEBUG_1
                if(clockCycle == 33)
                {
                    for(int tt : selectionReadPortTable[j])
                    {
                        debugFile << ' ' << tt;
                    }
                    debugFile << endl << "-------------------- " << ((tatti) ? "MERGED"  : "  ") << " -------------------" << endl;
                    for(int tt : freeBRAMPortsCount)
                    {
                        debugFile << ' ' << tt;
                    }
                    debugFile << endl;
                }
                #endif

            }
        }

        #ifdef PRINT_DEBUG_1
        debugFile << endl << "🥇 Selection Table Merged" << endl;
        #endif

        // selection select loop
        tmpInt = 0;                             // max priority
        maxPrioritySelection = 0;              // max priority index
        
        for(i = 0; i < activeSelectionTableSize; i++)
        {
            j = getAssignment(selectionPriorityTable[i], macUnitCount, divUnitCount, 
                macAssignmentsTmp, divAssignmentsTmp, macAssignmentsPairIndexTmp, 
                selectionNodeTable, selectionPairIndexTable, auGroup);
            
            // if selection has higher priority
            if(j > tmpInt)
            {
                maxPrioritySelection = i;
                tmpInt = j;
                std::copy(macAssignmentsTmp, macAssignmentsTmp + macUnitCount, macAssignments);
                std::copy(divAssignmentsTmp, divAssignmentsTmp + divUnitCount, divAssignments);
                std::copy(macAssignmentsPairIndexTmp, macAssignmentsPairIndexTmp + macUnitCount, macAssignmentsPairIndex);
            }
        }
        // if active table size is zero
        if(activeSelectionTableSize == 0)
        {
            std::fill(macAssignments, macAssignments + macUnitCount, nullptr);
            std::fill(divAssignments, divAssignments + divUnitCount, nullptr);
        }

        
        
        // -------------------------
        // 🥇 Pre Assignment Data
        // -------------------------
        
        
        #ifdef PRINT_DEBUG_1
            debugFile << "Selection Table Index : " << maxPrioritySelection << endl;
            for(int tt : selectionReadPortTable[maxPrioritySelection])
            {
                debugFile << ' ' << tt;
            }
            debugFile << endl << "---------------------------------------" << endl;
            for(int tt : freeBRAMPortsCount)
            {
                debugFile << ' ' << tt;
            }
            debugFile << endl;
            
            // printScheduleWindow(AUAssignData, AURetireData, memData, rollingIndex(0, AUDataWindowSize, clockCycle),
            //     rollingIndex(0, memDataWindowSize, clockCycle), clockCycle);
        #endif
        
        // -------------------------
        // 🥇 Process Assignments
        // -------------------------

        #ifdef PRINT_DEBUG_0
        debugFile << endl << "🥇 Operations Selected" << endl;
        #endif

        // 🥈 create live variable map
        // ---------------------------

        // 🥉 variables on PE output ports
		i = 0;
        for(auto op : AURetireData[rollingIndex(readLatency, AUDataWindowSize, clockCycle)])
        {
            if(op.op != "NO_OP")
            {
                liveVariablesMap[i] = op.outputId;
            }
            i++;
        }

        #ifdef PRINT_DEBUG_1
        debugFile << endl << "🥉 Output Port added to Live Variables" << endl;
        #endif

        // 🥉 Variable on write ports from cycle 0 are already updated

        // 🥈 consolidate all assignments in previousAssignedNodes
        //      previousAssignedNodes is just for convenience in finding ready nodes
        // --------------------------------------------------------------------
        previousAssignedNodes.clear();

        #ifdef PRINT_DEBUG_0
        // debugFile << "🐞 🦗 🐞 🦗 🐞 Mac Unit Count " << macUnitCount << endl;
        debugFile << endl << "💻 💻 💻 : Selected MAC Nodes :" << endl;
        #endif
        for(i = 0; i < macUnitCount; i++)
        {
            if(macAssignments[i])            // contains nullprt 
            {
                // if(macAssignments[i]->name == "U_110_110")
                // {
                //     cout << "Add Break Point" << endl;
                // }
                
                previousAssignedNodes.push_back(macAssignments[i]);
                // keep only the selected mul pair
                macAssignments[i]->schedulableMULPairIndex[0] = macAssignmentsPairIndex[i];

                #ifdef PRINT_DEBUG_0
                    debugFile << "Node Name: " << macAssignments[i]->name;
                    debugFile << " Selected MUL Pair Nodes: " << macAssignments[i]->children[macAssignments[i]->schedulableMULPairIndex[0] * 2]->nodeId;
                    debugFile << macAssignments[i]->children[macAssignments[i]->schedulableMULPairIndex[0] * 2 -1]->nodeId << endl;
                #endif
            }
        }

        #ifdef PRINT_DEBUG_0
        // debugFile << "🐞 🦗 🐞 🦗 🐞 Div Unit Count " << divUnitCount << endl;
        debugFile << endl << "💿 💿 💿 : Selected DIV Nodes :" << endl;
        #endif
        for(i = 0; i < divUnitCount; i++)
        {
            if(divAssignments[i])        // contains nullprt
            {
                previousAssignedNodes.push_back(divAssignments[i]);

                #ifdef PRINT_DEBUG_0
                    debugFile << "Selected Div Node " << divAssignments[i]->name << endl;
                #endif
            }
        }

        // 🥈 Create operation nodes for the cycle
        //-----------------------------------------
        // create MAC nodes
        i = -1; // MAC unit ID
        j = -1;  // div unit id
        for(auto node : previousAssignedNodes)
        {
            // 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃
            // find MAC unit
            if(isMACType(node))
            {
                i++;
                if(i == hwConfig.AUs.size())
                {
                    // unable to find MAC unit
                    cout << "🚨 ERROR: Can't assign MAC unit in clock cycle " << clockCycle << endl;
                    exit(1);
                }
                while(hwConfig.AUs[i]->numOperations != 5)
                {
                    i++;
                    if(i == hwConfig.AUs.size())
                    {
                        // unable to find MAC unit
                        cout << "🚨 ERROR: Can't assign MAC unit in clock cycle " << clockCycle << endl;
                        exit(1);
                    }
                }
            }
            else
            {
                j++;
                if(j == hwConfig.AUs.size())
                {
                    // unable to find div unit
                    cout << "🚨 ERROR: Can't assign DIV unit in clock cycle " << clockCycle << endl;
                    exit(1);
                }
                while(hwConfig.AUs[j]->numOperations != 1)
                {
                    j++;
                    if(j == hwConfig.AUs.size())
                    {
                        // unable to find div unit
                        cout << "🚨 ERROR: Can't assign DIV unit in clock cycle " << clockCycle << endl;
                        exit(1);
                    }
                }
            }
            // 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃 🎃

            emptyAUOp.assignClock = clockCycle + readLatency;
            emptyAUOp.AUIndex = isMACType(node) ? i : j;
            emptyAUOp.BRAMAddress = node->matPointer->BRAMAddr[node->memMapIndex];
            emptyAUOp.BRAMIndex = node->matPointer->BRAMInd[node->memMapIndex];
            emptyAUOp.nodePtr = node;
            emptyAUOp.op = node->op;
            emptyAUOp.outputId = node->nodeId;
            emptyAUOp.retireClock = clockCycle + auAssignLocation[isMACType(node) ? i : j];

            #ifdef PRINT_DEBUG_1
            if(clockCycle == 33)
            {
                debugFile << "💶 💶 Assigning Node: " << node->name << ' ' << node->op  << endl 
                                << "Free BRAM Port Count: ";
                for(auto tt : freeBRAMPortsCount)
                    debugFile << ' ' << tt;
                debugFile << endl << endl;
            }
            #endif
            

            // process inputs
            if(isMACType(node))
            {
                emptyAUOp.inputIds.resize(3);
                emptyAUOp.inputLocs.resize(3);
                // child 0
                emptyAUOp.inputIds[0] = node->children[0]->nodeId;
                tmpNodePtrArr[0] = node->children[0];
                // MUL pair
                tmpNodePtrArr[1] = node->children[node->schedulableMULPairIndex[0] * 2 - 1];
                tmpNodePtrArr[2] = node->children[node->schedulableMULPairIndex[0] * 2];
                emptyAUOp.inputIds[1] = tmpNodePtrArr[1]->nodeId;
                emptyAUOp.inputIds[2] = tmpNodePtrArr[2]->nodeId;
                tmpNodePtrArrSize = 3;
            }
            else
            {
                tmpNodePtrArrSize = node->children.size();
                emptyAUOp.inputIds.resize(tmpNodePtrArrSize);
                emptyAUOp.inputLocs.resize(tmpNodePtrArrSize);

                for(k = 0; k < tmpNodePtrArrSize; k++ )
                {
                    tmpNodePtrArr[k] = node->children[k];
                    emptyAUOp.inputIds[k] = tmpNodePtrArr[k]->nodeId;
                }
            }

            // assign variable location and create mem read node
            for(k = 0; k < tmpNodePtrArrSize; k++)
            {
                // check if variable is live
                tmpInt = distance(liveVariablesMap.begin(), std::find(liveVariablesMap.begin(),
                    liveVariablesMap.end(), emptyAUOp.inputIds[k]));

                if(tmpInt != liveVariablesMap.size() )
                {
                    emptyAUOp.inputLocs[k] = tmpInt;


                    #ifdef PRINT_DEBUG_0
                    // if(clockCycle == 33)
                    // {
                        debugFile << "Input Variable " << k << ' ' <<  emptyAUOp.inputIds[k] << " is live at " << tmpInt << endl;
                    // }
                    #endif
                }
                else
                {
                    // have to read
                    //-------------

                    // find read port
                    BRAMIndex = tmpNodePtrArr[k]->matPointer->BRAMInd[tmpNodePtrArr[k]->memMapIndex];
                    freeBRAMPortsCount[BRAMIndex] -= 1;

                    #ifdef PRINT_DEBUG_0
                    // if(clockCycle == 33)
                    // {
                        debugFile << "Input Variable " << k << ' ' << emptyAUOp.inputIds[k] 
                            << " is assigned to be assigned at BRAM " << BRAMIndex << " at Port " << freeBRAMPortsCount[BRAMIndex] << endl;
                    // }
                    #endif
                    // store port in tmpint
                    tmpInt = freeBRAMPortsCount[BRAMIndex];

                    // find live variable location
                    tmpInt = BRAMToLiveLookup[BRAMIndex][tmpInt];
                    // update live variable map
                    liveVariablesMap[tmpInt] = emptyAUOp.inputIds[k];

                    // create read operation
                    emptyBRAMOp.op = BRAM_READ;
                    emptyBRAMOp.addr = tmpNodePtrArr[k]->matPointer->BRAMAddr[tmpNodePtrArr[k]->memMapIndex];
                    emptyBRAMOp.BRAMIndex = BRAMIndex;
                    emptyBRAMOp.nodeId = emptyAUOp.inputIds[k];
                    emptyBRAMOp.portIndex = freeBRAMPortsCount[BRAMIndex];
                    emptyBRAMOp.nodePtr = tmpNodePtrArr[k];

                    emptyAUOp.inputLocs[k] =  tmpInt;
                    
                    // add read node to memData (deep copy)
                    memData[rollingIndex(readLatency, memDataWindowSize, clockCycle)][tmpInt - hwConfig.AUs.size()] = emptyBRAMOp;

                    #ifdef PRINT_DEBUG_0
                    // if(clockCycle == 33)
                    // {
                        debugFile << "Input Variable Id: " << k << " Live Location: " << emptyAUOp.inputIds[k] 
                            << " is assigned to BRAM " << BRAMIndex << " at Port " << freeBRAMPortsCount[BRAMIndex] << endl;
                    // }
                    #endif
                }
            }   // assign input location to AU op
        
            // add node to the AU assign table
            AUAssignData[rollingIndex(readLatency, memDataWindowSize, clockCycle)][isMACType(node) ? i : j] = emptyAUOp;

            // add node to AU retire table
            AURetireData[rollingIndex(emptyAUOp.retireClock - clockCycle, AUDataWindowSize, clockCycle)][isMACType(node) ? i : j] = emptyAUOp;

        }   // crate AU op 


        #ifdef PRINT_DEBUG_0
        debugFile << endl << "🥇 Assignments Done" << endl;
        #endif
        // -------------------------
        // 🥇 Post Assignment Work
        // -------------------------

        // retiring nodes = cycle[readlatency] for next clock cycle
        retiringNodes.clear();
        for(auto op : AURetireData[rollingIndex(readLatency + 1, AUDataWindowSize, clockCycle)])
        {
            if(op.op != "NO_OP")
                retiringNodes.push_back(op.nodePtr);
        }

        // update data for scheduled nodes
        Node * node;
        for(i = 0; i < macUnitCount; i++)
        {
            node = macAssignments[i];
            if(node)
            {
                // if(node->name == "U_110_110")
                // {
                //     cout << "Add Break Point" << endl;
                // }
                node->childrenStatus[0] = NOT_READY;
                node->childrenStatus[node->schedulableMULPairIndex[0]] = DONE;
                node->readyMULPairIndex.erase(std::find(node->readyMULPairIndex.begin(), node->readyMULPairIndex.end(), node->schedulableMULPairIndex[0]));
            }
        }

        // update dirty status
        //----------------------

        completedWrites.clear();
        for(auto op : memData[rollingIndex(0, memDataWindowSize, clockCycle)])
        {
            if(op.op == BRAM_WRITE)
            {
                completedWrites.push_back(op.nodePtr);
            }
        }

        updateDirtyMemoryStatus(completedWrites, previousAssignedNodes);

        #ifdef PRINT_DEBUG_1
        debugFile << endl << "🥇 Post Assignment Work Done" << endl;
        #endif
        // -------------------------
        // 🥇 Save Schedule
        // -------------------------
        schedule.AUSchedule.push_back(vector<AU_Operation>());
        for(auto op : AUAssignData[rollingIndex(0, memDataWindowSize, clockCycle)])
        {
            if(op.op != "NO_OP")
            {
                schedule.AUSchedule[clockCycle].push_back(op);
            }
        }
        schedule.memSchedule.push_back(vector<BRAM_Operation>());
        for(auto op : memData[rollingIndex(readLatency, memDataWindowSize, clockCycle)])
        {
            if(op.op != BRAM_NOTHING)
            {
                schedule.memSchedule[clockCycle].push_back(op);
            }
        }

        numScheduledNodes += doneNodeCount[rollingIndex(0, memDataWindowSize, clockCycle)];

        // cout << "Clock Cycle: " << setw(4) << clockCycle << " Done Nodes: " << numScheduledNodes << " / " << numNodesInGraph << " FLOPS: " << FLOPS << endl;

        // #ifdef PRINT_DEBUG_0
        // debugFile << endl << "🥇 Schedule Saved" << endl;
        // #endif

        // clear AU retire 0 objects
        for(auto & op : AURetireData[rollingIndex(0, AUDataWindowSize, clockCycle)])
        {
            op.op = "NO_OP";
        }

        #ifdef PRINT_DEBUG_0
        debugFile << endl << "🥇 Schedule Saved" << endl;
        #endif


        #ifdef PRINT_DEBUG_1
        printScheduleWindow(AUAssignData, AURetireData, memData, rollingIndex(0, AUDataWindowSize, clockCycle),
            rollingIndex(0, memDataWindowSize, clockCycle), clockCycle);
        #endif


        clockCycle ++;

        if(clockCycle > 100000)
        {
            cout << "🚧   Limit : Maximum number clock cycles reached " << endl;
            exit(1);
        }
    }   // schedular loop end

    
    // deletete dynamically allocated elements
    delete[] macAssignments;
    delete[] macAssignmentsTmp;
    delete[] divAssignments;
    delete[] divAssignmentsTmp;
    delete[] macAssignmentsPairIndex;
    delete[] macAssignmentsPairIndexTmp;

    #ifdef PRINT_DEBUG_1
        debugFile << endl << "🥇 DOne BItch" << endl;
    #endif

}

// void printReadableSchedule(const struct Schedule sch)
// {
//     int numCycles, i, j;

//     numCycles = sch.AUSchedule.size();
//     for(int cycle = 0; cycle < numCycles; cycle++)
//     {
//         debugFile << "----------------------------------------------------------" << endl;
//         debugFile << "----------------------------------------------------------" << endl;
//         debugFile << "⏰ ⏰ ⏰ ⏰ ⏰ ⏰ ⏰ ⏰ ⏰ ⏰ Clock Cycle "<< i << "⏰ ⏰ ⏰ ⏰ ⏰ ⏰ ⏰ ⏰ ⏰ ⏰" << endl;
//         debugFile << "----------------------------------------------------------" << endl;
//         // Au Data
//         debugFile << "🏷 🏷 Arithmetic Unit Assignments 🏷 🏷" << endl;
//         // print AU index
//         debugFile << endl << setw(10) << "AU Index" << " | ";
//         for(j = 0; j < sch.AUSchedule[cycle].size(); j++)
//         {
//             debugFile << setw(15) << sch.AUSchedule[cycle][j].AUIndex << " | ";
//         }

//         // Print OP and Node ID
//         debugFile << endl << setw(10) << "OP : Id" << " | ";
//         for(j = 0; j < sch.AUSchedule[cycle].size(); j++)
//         {
//             debugFile << setw(15) << sch.AUSchedule[cycle][j].op + " : " + to_string(sch.AUSchedule[cycle][j].outputId) << " | ";
//         }
//         // Input Ids 
//         debugFile << endl << setw(10) << "In Id" << " | ";
//         for(j = 0; j < sch.AUSchedule[cycle].size(); j++)
//         {
//             debugFile << setw(15) << vecToString(sch.AUSchedule[cycle][j].inputIds) << " | ";
//         }
//         // Input Loc 
//         debugFile << endl << setw(10) << "In Loc" << " | ";
//         for(j = 0; j < sch.AUSchedule[cycle].size(); j++)
//         {
//             debugFile << setw(15) << vecToString(sch.AUSchedule[cycle][j].inputLocs) << " | ";
//         }
//         // Input Loc 
//         debugFile << endl << setw(10) << "BRAM Index" << " | ";
//         for(j = 0; j < sch.AUSchedule[cycle].size(); j++)
//         {
//             debugFile << setw(15) << sch.AUSchedule[cycle][j].BRAMIndex << " | ";
//         }
//         // Retire Clock 
//         debugFile << endl << setw(10) << "Retire At" << " | ";
//         for(j = 0; j < sch.AUSchedule[cycle].size(); j++)
//         {
//             debugFile << setw(15) << sch.AUSchedule[cycle][j].retireClock << " | ";
//         }
//         debugFile << endl;
//         debugFile << "----------------------------------------------------------" << endl;
//     }
// }

void printReadableSchedule(const struct Schedule sch)
{
    debugFile << "----------------------------------------------------------" << endl;
    debugFile << "🤯 🤯 🤯 🤯 🤯 🤯 Arithmetic Unit Schedule 🤯 🤯 🤯 🤯 🤯 🤯" << endl;
    debugFile << "----------------------------------------------------------" << endl;
    printAUWindow(sch.AUSchedule, 0);

    debugFile << "----------------------------------------------------------" << endl;
    debugFile << "🤯 🤯 🤯 🤯 🤯 🤯 Memory Unit Schedule 🤯 🤯 🤯 🤯 🤯 🤯" << endl;
    debugFile << "----------------------------------------------------------" << endl;
    printMemWindow(sch.memSchedule, 0);

}

// scheduleNoReg function end

// v.erase(std::remove_if(
//     v.begin(), v.end(),
//     [](const int& x) { 
//         return x > 10; // put your condition here
//     }), v.end());

void convertScheduleToInstructionStream(const struct HardwareConfig hwConfig, const struct Schedule sch, const string instructionFileName)
{
    ofstream insFile;
    insFile.open(instructionFileName);

    int macSelInsWidth = hwConfig.AUMuxSelWidth * 3 * hwConfig.numMACUnits;
    int divSelInsWidth = hwConfig.AUMuxSelWidth * 2 * (hwConfig.AUs.size() - hwConfig.numMACUnits);
    int bramSelInslWidth = hwConfig.BRAMMuxSelWidth * hwConfig.BRAMs[0]->ports * hwConfig.BRAMs.size();
    int bramAddrInsWidth = (hwConfig.BRAMAddressWidth + 1) * hwConfig.BRAMs[0]->ports * hwConfig.BRAMs.size();

    int insWidth = macSelInsWidth + divSelInsWidth + bramSelInslWidth + bramAddrInsWidth;

    // // limit checker
    // if(insWidth > INS_BITSET_WIDTH)
    // {
    //     cout << "🚧 : Limit Exceeded: Instruction field exceeded the limit; Requires at least " 
    //         << insWidth << " bits" << endl;
    //     exit(0);
    // }
    if(hwConfig.AUMuxSelWidth > SEL_BITSET_WIDTH)
    {
        cout << "🚧 : Limit Exceeded: AU Mux selection field exceeded the limit; Requires at least " 
            << hwConfig.AUMuxSelWidth << " bits" << endl;
        exit(0);
    }
    if(hwConfig.BRAMAddressWidth > ADDR_BITSET_WIDTH)
    {
        cout << "🚧 : Limit Exceeded: BRAM address field exceeded the limit; Requires at least " 
            << hwConfig.BRAMAddressWidth << " bits" << endl;
        exit(0);
    }

    string currIns('0', insWidth);

    insFile << "memory_initialization_radix=2;" << endl <<
        "memory_initialization_vector=" << endl;

    int cycle  = 0;
    int startLoc, endLoc, portLoc, offset;
    int numBRAMPorts = hwConfig.BRAMs[0]->ports;
    int macInLoc;

    for(cycle = 0; cycle < sch.AUSchedule.size(); cycle++)
    {
        currIns.assign(insWidth, '0');

        offset = bramAddrInsWidth + macSelInsWidth + divSelInsWidth;
        for(auto memOp : sch.memSchedule[cycle])
        {
            portLoc = memOp.BRAMIndex * numBRAMPorts + memOp.portIndex;
            startLoc = portLoc * (hwConfig.BRAMAddressWidth + 1);
            endLoc = startLoc + hwConfig.BRAMAddressWidth - 1;
            currIns.replace(startLoc, hwConfig.BRAMAddressWidth, bitset<ADDR_BITSET_WIDTH>(memOp.addr).to_string().substr(ADDR_BITSET_WIDTH - hwConfig.BRAMAddressWidth));
            currIns[endLoc + 1] = (memOp.op == 1) ? '1' : '0' ;
            // write operation -> assign read location
            if(memOp.op == BRAM_WRITE)
            {
                // debugFile << "location: " << memOp.dinLoc << " conv: " << bitset<SEL_BITSET_WIDTH>(memOp.dinLoc).to_string().substr(SEL_BITSET_WIDTH - hwConfig.BRAMMuxSelWidth) << endl;
                startLoc = offset + (memOp.BRAMIndex * 4 + memOp.portIndex) * hwConfig.BRAMMuxSelWidth;
                // debugFile << "start location: " << startLoc << " offset: " << offset << endl;
                currIns.replace(startLoc, hwConfig.BRAMMuxSelWidth, bitset<SEL_BITSET_WIDTH>(memOp.dinLoc).to_string().substr(SEL_BITSET_WIDTH - hwConfig.BRAMMuxSelWidth));
            }
        }

        for( auto auOp : sch.AUSchedule[cycle])
        {
            offset = bramAddrInsWidth;
            if(auOp.AUIndex >= hwConfig.numMACUnits)
            {
                startLoc = offset + macSelInsWidth + (auOp.AUIndex - hwConfig.numMACUnits) * 2 * hwConfig.AUMuxSelWidth;
                for(auto inputLoc : auOp.inputLocs)
                {
                    currIns.replace(startLoc, hwConfig.AUMuxSelWidth, bitset<SEL_BITSET_WIDTH>(inputLoc).to_string().substr(SEL_BITSET_WIDTH - hwConfig.AUMuxSelWidth));
                    startLoc += hwConfig.AUMuxSelWidth;
                }
            }
            else
            {
                startLoc = offset + auOp.AUIndex * 3 * hwConfig.AUMuxSelWidth;
                macInLoc = auOp.inputLocs[1];
                currIns.replace(startLoc, hwConfig.AUMuxSelWidth, bitset<SEL_BITSET_WIDTH>(macInLoc).to_string().substr(SEL_BITSET_WIDTH - hwConfig.AUMuxSelWidth));
                startLoc += hwConfig.AUMuxSelWidth;
                macInLoc = auOp.inputLocs[2];
                currIns.replace(startLoc, hwConfig.AUMuxSelWidth, bitset<SEL_BITSET_WIDTH>(macInLoc).to_string().substr(SEL_BITSET_WIDTH - hwConfig.AUMuxSelWidth));
                startLoc += hwConfig.AUMuxSelWidth;
                macInLoc = auOp.inputLocs[0];
                currIns.replace(startLoc, hwConfig.AUMuxSelWidth, bitset<SEL_BITSET_WIDTH>(macInLoc).to_string().substr(SEL_BITSET_WIDTH - hwConfig.AUMuxSelWidth));
            }
            
        }

        insFile << currIns << ",\n";
        #ifdef PRINT_DEBUG_1
        debugFile << "Instruction Cycle " << cycle << endl << currIns << endl;
        #endif
    }

    // last extra zero puts the machine into halt
    insFile << "0;\n";
    insFile.close();
}
