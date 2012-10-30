/**
 * @brief An example of using a packed memory graph to solve a problem. 
 * Usage: ./a.out [path to folder containing DIMACS10 maps] [map name]
 *
 * @author Panos Michail
 *
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <Structs/Graphs/dynamicGraph.h>
#include <Structs/Graphs/packedMemoryGraphImpl.h>
#include <iomanip>
#include <Utilities/geographic.h>
#include <Utilities/graphGenerators.h>
#include <Utilities/timer.h>
#include <boost/program_options.hpp>
#include <queue>

/* PROBLEM: Find the edge distance of each node and each edge of a graph to a specific node s. Finally ouput the max distance of a node from s.
   INPUT: A directed graph G, a random node s
   SOLUTION: We will run a Breadth First Search (BFS) from node s on graph G and record the distance from s for each node and each edge 
*/


/* the struct for the information on the nodes */
struct NodeInfo: DefaultGraphItem
{
	NodeInfo():marked(false),distance(0),x(0),y(0)
	{	
	}
    
    /* we need a boolean, to check if BFS has visited the node or not */
    bool marked;

    /* we need an integer to keep the distance from s */
    unsigned int distance;

    /* we know that we will read a DIMACS10 map, and this map provides node coordinates, so we need two integers to store them. */
    unsigned int x,y;
};

/* the struct for the information on the edges */
struct EdgeInfo: DefaultGraphItem
{
    EdgeInfo():distance(0)
    {
    }
    /* we need an integer to keep the distance from s */
    unsigned int distance;
};


/* we define our graph type: it is a Packed Memory Graph with NodeInfo and EdgeInfo on the nodes and edges respectively */
typedef DynamicGraph< PackedMemoryGraphImpl, NodeInfo, EdgeInfo>    Graph;


/* we rename some of the graph's types, easier to use */
typedef Graph::NodeIterator     NodeIterator;
typedef Graph::EdgeIterator     EdgeIterator;
typedef Graph::InEdgeIterator   InEdgeIterator;
typedef Graph::NodeDescriptor   NodeDescriptor;
typedef Graph::EdgeDescriptor   EdgeDescriptor;
typedef Graph::SizeType         SizeType;


/* we will use a reader to read the map data. we rename it as 'Reader' */
typedef GraphReader< Graph>     Reader;

/* this is the BFS routine. Obviously, it needs a graph G and a starting node s */
void calcDistances( Graph& G, NodeIterator& s)
{  
    /* we clear the marked flag for all nodes */
    for( NodeIterator u = G.beginNodes(), v = G.endNodes(); u != v; ++u) 
    {
        u->marked = false;
    }
    
    /* we will need a queue of nodes for BFS */
    std::queue<NodeIterator> Q;

    /* we set the marked flag for s, its distance to zero, and put it in the queue */
    s->marked = true;
    s->distance = 0;
    Q.push(s);

    /* BFS LOOP */
    while( !Q.empty())
    {
        /* we get the first node in the queue */
        NodeIterator u = Q.front();
        Q.pop();      

        /* for each of its outgoing edges */
        for( EdgeIterator e = G.beginEdges(u), end = G.endEdges(u); e != end; ++e)
        {
            /* we get the neighboring node and visit it */
            NodeIterator v = G.target( e);
            if( ! v->marked)
            {
                /* we set the distance of each edge e, and its opposite edge k */
                InEdgeIterator k = G.getInEdgeIterator(e);
                e->distance = u->distance;   
                k->distance = u->distance;

                /* we set the node as marked, set its distance and put it in the queue */
                v->marked = true;
                v->distance = u->distance + 1;
                Q.push(v);
            }
        }
    } 
}

/* this routine finds the maximum distance of an edge */
unsigned int findMaxEdgeDistance( Graph& G)
{  
    unsigned int max = 0;
    for( NodeIterator u = G.beginNodes(), v = G.endNodes(); u != v; ++u)
    {
        for( EdgeIterator e = G.beginEdges(u), end = G.endEdges(u); e != end; ++e)
        {
            if ( e->distance > max)
            {
                max = e->distance;
            }
        }
    }
    return max;
}

/* this routine finds the maximum distance of a node */
unsigned int findMaxNodeDistance( Graph& G)
{  
    unsigned int max = 0;
    for( NodeIterator u = G.beginNodes(), v = G.endNodes(); u != v; ++u)
    {
        if ( u->distance > max)
        {
            max = u->distance;
        }
    }
    return max;
}

int main( int argc, char* argv[])
{
    Graph G;
	Reader* reader = 0;

    std::string basePath(argv[1]);
    std::string mapname(argv[2]);
    std::string mapfile = basePath + mapname + std::string(".osm.graph");
    std::string coordinatesfile = basePath + mapname + std::string(".osm.xyz");

    /* we create a reader on the map file */
    reader = new DIMACS10Reader<Graph>( mapfile, coordinatesfile);
	
    /* we read the map onto the graph */
    G.read(reader);

    /* we choose a starting node s */
    NodeIterator s = G.chooseNode();

    /* we run the calculation routine */
    calcDistances( G, s);

    /* ouput max distances */
    std::cout << "Max edge distance: " << findMaxEdgeDistance( G) << std::endl;
    std::cout << "Max node distance: " << findMaxNodeDistance( G) << std::endl;
    return 0;
}
