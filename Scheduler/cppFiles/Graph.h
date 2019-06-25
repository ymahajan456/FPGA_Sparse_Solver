#ifndef _GRAPH_H_
#define _GRAPH_H_

#include "stdafx.h"
#include "CCS.h"

// Shared variable map of delays for each operation
// These are typical values (Most Expected) for particular operation
EXTERN map<string, int> opDelay INITIALIZER({{"+", 12}, {"-", 12}, {"*",9}, {"/", 28}, {"rd", 1}, 
    {"wr", 1}, {"mac_add", 8}, {"mac_sub", 8}, {"const", 0}, {"pass", 0}});


/*
* Name: Node
* Provides:
*   a. name     Name of the node
*   b. op       Operation
*   c. val      Value
*   d. children Pointers to the children nodes
*   e. parents  Pointers to the parents nodes
*   f. delay    Delay of the operation in terms of clock cycles
*   g. priority Priority of the node in scheduling
*   i. matIndex Node output's location in matrix in terms of row and column
*   j. ccsIndex Node output's location index in CCS format
*   k. matName  Node output's matrix name
*   l. done     Flag to indicate the computation is done and available in memory
*   m. ready    Ready status of the node for scheduling and execution
    n. dirty    Value present in memory is correct
    o. mulCount Count of remaining MAC operation
* Methods:
*   a. Node()
*         Constructor to assign default values
*/
enum MACChildStatus
{
    NOT_READY   = 0,
    ONE_READY   = 1,
    READY       = 2,
    DONE        = 3
};

struct Node
{
    f_type val;
    vector<Node *> children, parents;
    string op, name;
    int delay, priority, level, memMapIndex;
    int nodeId = nodeIdNULL;
    string matName;
    CCS* matPointer;
    bool ready = false;
    bool done = false;
    bool dirty = false;
    bool store = false;
    vector<MACChildStatus> childrenStatus;
    vector<int> readyMULPairIndex;
    vector<int> schedulableMULPairIndex;
    int totalMACCount = 0;
    int doneMACCount = 0;

    Node()
    {
        this->val = 0;
        this->level = levelNULL;
        this->priority = priorityNULL;
    }
};

/*
* Name: Graph
* Provides:
*   a. nodes    : List of all the nodes in the graph pair(name, Node)
*   b. roots    : List of the pointers to root nodes
*   c. leaves   : List of the pointers to leaf nodes
* Methods:
*   a. clearGraph()
*       removes all the nodes in the graph
*   b. insertNode()
*       insert name, node pair in the graph
*   c. printToFile()
*       print the graph in to a file [dot file too if MAKE_GRAPH is defined]
*   d. executeGraph()
*       find values of all the nodes in the graph
*   e. findLeavesAndRoots()
*       Find leaves and roots of the graph
*   f. removeUselessNodes()
*       remove useless nodes in the graph 
*       (Uselessness is defined in function removeType)
*/

class Graph
{
    public:
    // data
    unordered_map<string, Node> nodes;
    vector<vector<Node *>> levels;
    vector<Node *> roots, leaves;
    int maxLevel = 0;
    vector<std::tuple<CCS *, int, CCS *, int>> musthaveSameMemLoc;
    // methods
    void clearGraph();
    Node* insertNode(Node & node);
    void printToFile(ofstream & outFile, bool leavesAndRoots = false);
    void executeGraph(string const dotFile  = "exeTree");
    void findLeavesAndRoots();
    void removeUselessNodes();
    void assignLevelsAndIds();
    void executeGraphUsingLevels(string const dotFileName = "exeTree");
};


// Calls graphViz to generate PNG from dot file
void convertDotToPng(string dotFileName);

#endif