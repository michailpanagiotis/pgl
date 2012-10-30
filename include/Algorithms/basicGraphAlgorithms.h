#ifndef BASICGRAPHALGORITHMS_H
#define BASICGRAPHALGORITHMS_H

#include <Structs/Graphs/dynamicGraph.h>
#include <Structs/Graphs/nodeSelection.h>
#include <queue>

template<class GraphType>
class SearchVisitor
{
public:
    
    typedef typename GraphType::NodeIterator    node;

    SearchVisitor()
    {  
    }

    virtual void visitOnInit()
    {
    }

    virtual void visitOnFinding( const node& u)
    {
        //std::cout << "Default Visitor\n";
    }

    virtual void visitOnMarking( const node& u)
    {
        //std::cout << "Default Visitor\n";
    }

    virtual void visitOnExit()
    {
    }
};

/**
 * @brief This is the core Breadth First Search Algorithm
 * @param G The graph to search
 * @param root The node to start the search from
 * @param visitor A visitor that gets called for every node in the BFS tree
 *
 * @author Panos Michail
 *
 */
template<class GraphType>
void bfsCore( GraphType& G, typename GraphType::NodeIterator& root, SearchVisitor<GraphType>* visitor = 0)
{
    typedef typename GraphType::NodeIterator    node;
    typedef typename GraphType::EdgeIterator    edge;
    typedef typename GraphType::SizeType        SizeType;
    typedef typename GraphType::NodeData        NodeData;

    node u,v;
    edge e,end;
    std::queue<node> Q;
    Q.push(root);

    visitor->visitOnInit();

    for( u = G.beginNodes(), v = G.endNodes(); u != v; ++u) u->marked = false;
    
    visitor->visitOnMarking(root);
    root->marked = true;
    while( !Q.empty())
    {
        u = Q.front();
        Q.pop();      

        for( e = G.beginEdges(u), end = G.endEdges(u); e != end; ++e)
        {
            v = G.target( e);
            if( ! v->marked)
            {
                visitor->visitOnMarking(v);
                v->marked = true;
                Q.push(v);
            }
        }
    } 
    
    visitor->visitOnExit();
}


/**
 * @brief This is the core Breadth First Search Algorithm on the reversed graph
 * @param G The graph to search
 * @param root The node to start the search from
 * @param visitor A visitor that gets called for every node in the BFS tree
 *
 * @author Panos Michail
 *
 */
template<class GraphType>
void reverseBfsCore( GraphType& G, typename GraphType::NodeIterator& root, SearchVisitor<GraphType>* visitor = 0)
{
    typedef typename GraphType::NodeIterator    node;
    typedef typename GraphType::EdgeIterator    edge;
    typedef typename GraphType::SizeType        SizeType;
    typedef typename GraphType::NodeData        NodeData;

    node u,v;
    edge e,end;
    std::queue<node> Q;
    Q.push(root);

    visitor->visitOnInit();

    for( u = G.beginNodes(), v = G.endNodes(); u != v; ++u) u->marked = false;

    visitor->visitOnMarking(root);
    root->marked = true;

    while( !Q.empty())
    {
        u = Q.front();
        Q.pop();      

        for( e = G.beginBackEdges(u), end = G.endBackEdges(u); e != end; ++e)
        {
            v = G.source( e);
            if( ! v->marked)
            {
                visitor->visitOnMarking(v);
                v->marked = true;
                Q.push(v);
            }
        }
    } 

    visitor->visitOnExit();
}


/**
 * @brief This is an undirected version of the core Breadth First Search Algorithm
 * @param G The graph to search
 * @param root The node to start the search from
 * @param visitor A visitor that gets called for every node in the BFS tree
 *
 * @author Panos Michail
 *
 */
template<class GraphType>
void undirectedBfsCore( GraphType& G, typename GraphType::NodeIterator& root, SearchVisitor<GraphType>* visitor = 0)
{
    typedef typename GraphType::NodeIterator    node;
    typedef typename GraphType::EdgeIterator    edge;
    typedef typename GraphType::BackEdgeIterator    backEdge;
    typedef typename GraphType::SizeType        SizeType;
    typedef typename GraphType::NodeData        NodeData;

    node u,v;
    edge e,end;
    backEdge k,kEnd;
    std::queue<node> Q;
    Q.push(root);

    bool status = false;

    visitor->visitOnInit();

    for( u = G.beginNodes(), v = G.endNodes(); u != v; ++u) u->marked = false;
    
    visitor->visitOnMarking(root);
    root->marked = true;

    while( !Q.empty())
    {
        u = Q.front();
        Q.pop();      

        status = false;

        for( e = G.beginEdges(u), end = G.endEdges(u); e != end; ++e)
        {
            status = true;
            v = G.target( e);
            if( ! v->marked)
            {
                visitor->visitOnMarking(v);
                v->marked = true;
                Q.push(v);
            }
        }
        
        for( k = G.beginBackEdges(u), kEnd = G.endBackEdges(u); k != kEnd; ++k)
        {
            status = true;
            v = G.source( k);
            if( ! v->marked)
            {
                visitor->visitOnMarking(v);
                v->marked = true;
                Q.push(v);
            }
        }
    } 

    visitor->visitOnExit();
}


/**
 * @brief This is the core Depth First Search Algorithm
 * @param G The graph to search
 * @param root The node to start the search from
 * @param visitor A visitor that gets called for every node in the BFS tree
 *
 * @author Panos Michail
 *
 */
template<class GraphType>
void dfsCore( GraphType& G, typename GraphType::NodeIterator& root, SearchVisitor<GraphType>* visitor = 0)
{
    typedef typename GraphType::NodeIterator    node;
    typedef typename GraphType::EdgeIterator    edge;
    typedef typename GraphType::SizeType        SizeType;
    typedef typename GraphType::NodeData        NodeData;

    node u,v;
    edge e,end;
    std::stack<node> S;
    S.push(root);

    visitor->visitOnInit();

    for( u = G.beginNodes(), v = G.endNodes(); u != v; ++u) u->marked = false;

    visitor->visitOnMarking(root);
    root->marked = true;

    while( !S.empty())
    {
        u = S.top();
        S.pop();      

        for( e = G.beginEdges(u), end = G.endEdges(u); e != end; ++e)
        {
            v = G.target( e);
            if( ! v->marked)
            {
                visitor->visitOnMarking(v);
                v->marked = true;
                S.push(v);
            }
        }
    } 

    visitor->visitOnExit();
}


/**
 * @brief This is the core Depth First Search Algorithm on the reversed graph
 * @param G The graph to search
 * @param root The node to start the search from
 * @param visitor A visitor that gets called for every node in the BFS tree
 *
 * @author Panos Michail
 *
 */
template<class GraphType>
void reverseDfsCore( GraphType& G, typename GraphType::NodeIterator& root, SearchVisitor<GraphType>* visitor = 0)
{
    typedef typename GraphType::NodeIterator    node;
    typedef typename GraphType::EdgeIterator    edge;
    typedef typename GraphType::SizeType        SizeType;
    typedef typename GraphType::NodeData        NodeData;

    node u,v;
    edge e,end;
    std::stack<node> S;
    S.push(root);

    visitor->visitOnInit();

    for( u = G.beginNodes(), v = G.endNodes(); u != v; ++u) u->marked = false;

    visitor->visitOnMarking(root);
    root->marked = true;

    while( !S.empty())
    {
        u = S.top();
        S.pop();      

        for( e = G.beginBackEdges(u), end = G.endBackEdges(u); e != end; ++e)
        {
            v = G.source( e);
            if( ! v->marked)
            {
                visitor->visitOnMarking(v);
                v->marked = true;
                S.push(v);
            }
        }
    } 

    visitor->visitOnExit();
}

/**
 * @brief This is an undirected version of the core Depth First Search Algorithm
 * @param G The graph to search
 * @param root The node to start the search from
 * @param visitor A visitor that gets called for every node in the BFS tree
 *
 * @author Panos Michail
 *
 */
template<class GraphType>
void undirectedDfsCore( GraphType& G, typename GraphType::NodeIterator& root, SearchVisitor<GraphType>* visitor = 0)
{
    typedef typename GraphType::NodeIterator    node;
    typedef typename GraphType::EdgeIterator    edge;
    typedef typename GraphType::SizeType        SizeType;
    typedef typename GraphType::NodeData        NodeData;

    node u,v;
    edge e,end;
    std::stack<node> S;
    S.push(root);

    visitor->visitOnInit();

    for( u = G.beginNodes(), v = G.endNodes(); u != v; ++u) u->marked = false;

    visitor->visitOnMarking(root);
    root->marked = true;

    while( !S.empty())
    {
        u = S.top();
        S.pop();      

        for( e = G.beginEdges(u), end = G.endEdges(u); e != end; ++e)
        {
            v = G.target( e);
            if( ! v->marked)
            {
                visitor->visitOnMarking(v);
                v->marked = true;
                S.push(v);
            }
        }
        
        for( e = G.beginBackEdges(u), end = G.endBackEdges(u); e != end; ++e)
        {
            v = G.source( e);
            if( ! v->marked)
            {
                visitor->visitOnMarking(v);
                v->marked = true;
                S.push(v);
            }
        }
    } 

    visitor->visitOnExit();
}


template<class GraphType>
unsigned int findStronglyConnectedComponents( GraphType& G)
{    
    typedef typename GraphType::NodeIterator    node;
    typedef typename GraphType::EdgeIterator    edge;
    typedef typename GraphType::BackEdgeIterator    backEdge;
    typedef typename GraphType::SizeType        SizeType;
    typedef typename GraphType::NodeData        NodeData;

    node u,v;
    edge e,end;
    backEdge k,kEnd;
    std::stack<node> S;
    std::stack<node> discovered;
    unsigned int component = 0;

    for( u = G.beginNodes(), v = G.endNodes(); u != v; ++u)
    {
        u->marked = false; 
    }

    /*NodeIterator u = G.beginNodes();
    discovered.push(u);
    S.push(u);*/

    while( discovered.size() != G.getNumNodes())
    {
        for( u = G.beginNodes(), v = G.endNodes(); u != v; ++u)
            if( !u->marked)
                break;
        
        //std::cout << u->getId() << std::endl;   
        
        u->marked = true;
        discovered.push(u);
        S.push(u);

        while( !S.empty())
        {
            u = S.top();
            S.pop();      

            //std::cout << u->getId() << std::endl;

            for( e = G.beginEdges(u), end = G.endEdges(u); e != end; ++e)
            {
                v = G.target( e);
                if( ! v->marked)
                {
                    v->marked = true;
                    S.push(v);
                    discovered.push(v);
                }
            }

           /* for( e = G.beginBackEdges(u), end = G.endBackEdges(u); e != end; ++e)
            {
                v = G.source( e);
                if( ! v->marked)
                {
                    v->marked = true;
                    S.push(v);
                    discovered.push(v);
                }
            }*/

        } 
    }

    for( u = G.beginNodes(), v = G.endNodes(); u != v; ++u) u->marked = false;

    while( !discovered.empty())
    {
        u = discovered.top();
        discovered.pop();

        if( u->marked)
        {
            continue;
        }
        component++;
        S.push(u);
        u->marked = true;

        while( !S.empty())
        {
            u = S.top();
            S.pop();      
            //std::cout << u->getId() << " " << u->marked << " " << S.size() << std::endl;
            for( k = G.beginBackEdges(u), kEnd = G.endBackEdges(u); k != kEnd; ++k)
            {
                v = G.source(k);
                if( ! v->marked)
                {
                    v->marked = true;
                    S.push(v);
                    v->component = component;
                }
            }

            /*for( e = G.beginEdges(u), end = G.endEdges(u); e != end; ++e)
            {
                v = G.target( e);
                if( ! v->marked)
                {
                    v->marked = true;
                    S.push(v);
                    v->component = component;
                }
            }*/
        } 
    }

    return component;
}


/**
 * @brief Check if a directed graph is strongly connected
 * @param G The graph to check
 *
 * @author Panos Michail
 *
 */
template<class GraphType>
bool isConnected( GraphType& G)
{
    typedef typename GraphType::NodeIterator    node;
    typedef typename GraphType::EdgeIterator    edge;
    typedef typename GraphType::SizeType        SizeType;
    typedef typename GraphType::NodeData        NodeData;
    
    class Visitor : public SearchVisitor<GraphType>
    {
    public:
        Visitor():m_numNodes(1){}
        
        const SizeType& getNumNodes()
        {
            return m_numNodes;   
        }
        
        virtual void visitOnMarking( const node& u)
        {
            //std::cout << "Connectivity Visitor\n";
            ++m_numNodes;
        }
        
        SizeType m_numNodes;
    };
    
    Visitor vis;
    node s = G.chooseNode();
    bfsCore( G, s, &vis);
    return vis.getNumNodes() == G.getNumNodes();
}


/**
 * @brief Check if a directed graph is weakly connected
 * @param G The graph to check
 *
 * @author Panos Michail
 *
 */
template<class GraphType>
bool isWeaklyConnected( GraphType& G)
{
    typedef typename GraphType::NodeIterator    node;
    typedef typename GraphType::EdgeIterator    edge;
    typedef typename GraphType::SizeType        SizeType;
    typedef typename GraphType::NodeData        NodeData;
    
    class Visitor : public SearchVisitor<GraphType>
    {
    public:
        Visitor():m_numNodes(1){}
        
        const SizeType& getNumNodes()
        {
            return m_numNodes;   
        }
        
        virtual void visitOnMarking( const node& u)
        {
            //std::cout << "Connectivity Visitor\n";
            ++m_numNodes;
        }
        
        SizeType m_numNodes;
    };
    
    Visitor vis;
    node s = G.chooseNode();
    undirectedBfsCore( G, s, &vis);
    return vis.getNumNodes() == G.getNumNodes();
}


template<class GraphType>
bool topologicalSort( GraphType& G)
{
    return true;
}


template<class GraphType>
void markKCore( GraphType& G, unsigned int k)
{
    typedef typename GraphType::NodeIterator        NodeIterator;
    typedef typename GraphType::NodeDescriptor      NodeDescriptor;
    typedef typename GraphType::EdgeIterator        EdgeIterator;
    typedef typename GraphType::BackEdgeIterator    BackEdgeIterator;
    typedef typename GraphType::EdgeDescriptor      EdgeDescriptor;
    typedef typename GraphType::SizeType            SizeType;

    for( NodeIterator u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
    {
        if( G.degree(u) >= k)
        {
            u->marked = true;
        }
    }
}


template < class GraphType>
std::vector<typename GraphType::NodeDescriptor> getNeighbors( GraphType& G, const typename GraphType::NodeIterator& u)
{
    typedef typename GraphType::NodeIterator        NodeIterator;
    typedef typename GraphType::EdgeIterator        EdgeIterator;
    typedef typename GraphType::InEdgeIterator      InEdgeIterator;
    
    EdgeIterator e, endEdges;
    InEdgeIterator f, endInEdges;
    NodeIterator v;
    
    NodeSelection<GraphType> selection(&G);
    
    for( e = G.beginEdges(u), endEdges = G.endEdges(u); e != endEdges; ++e)
    {
        v = G.target(e);
        selection.select(v);
    }
    
    for( f = G.beginInEdges(u), endInEdges = G.endInEdges(u); f != endInEdges; ++f)
    {
        v = G.source(f);
        selection.select(v);
    }
    
    return selection.getMembers();
}


template < class GraphType>
std::vector<typename GraphType::NodeDescriptor> getOutNeighbors( GraphType& G, const typename GraphType::NodeIterator& u)
{
    typedef typename GraphType::NodeIterator        NodeIterator;
    typedef typename GraphType::EdgeIterator        EdgeIterator;
    typedef typename GraphType::InEdgeIterator      InEdgeIterator;
    
    EdgeIterator e, endEdges;
    InEdgeIterator f, endInEdges;
    NodeIterator v;
    
    NodeSelection<GraphType> selection(&G);
    
    for( e = G.beginEdges(u), endEdges = G.endEdges(u); e != endEdges; ++e)
    {
        v = G.target(e);
        selection.select(v);
    }
    
    return selection.getMembers();
}

template < class GraphType>
std::vector<typename GraphType::NodeDescriptor> getInNeighbors( GraphType& G, const typename GraphType::NodeIterator& u)
{
    typedef typename GraphType::NodeIterator        NodeIterator;
    typedef typename GraphType::EdgeIterator        EdgeIterator;
    typedef typename GraphType::InEdgeIterator      InEdgeIterator;
    
    EdgeIterator e, endEdges;
    InEdgeIterator f, endInEdges;
    NodeIterator v;
    
    NodeSelection<GraphType> selection(&G);
    
    for( f = G.beginInEdges(u), endInEdges = G.endInEdges(u); f != endInEdges; ++f)
    {
        v = G.source(f);
        selection.select(v);
    }
    
    return selection.getMembers();
}

/**
 * @brief Compare two graphs in terms of underlying structure
 * @param first The original graph
 * @param second The graph to check against to
 * @return -1 if the first graph has more nodes or edges, 0 if they are exactly the same,
 * +1 if the second graph has more nodes or edges
 *
 * @author Panos Michail
 *
 */
template < class FirstGraphType, class SecondGraphType>
int compare( const FirstGraphType& first, const SecondGraphType& second)
{      
    typename FirstGraphType::NodeIterator f_u,f_end_nodes;
    typename FirstGraphType::EdgeIterator f_e,f_end_edges;
    typename SecondGraphType::NodeIterator s_u, s_end_nodes;  
    typename SecondGraphType::EdgeIterator s_e,s_end_edges;      

    bool foundNode,foundEdge;

    std::cout << "\nForward..." << std::flush;
    for( f_u = first.beginNodes(), f_end_nodes = first.endNodes(); f_u != f_end_nodes; ++f_u)
    {
        foundNode = false;
        for( s_u = second.beginNodes(), s_end_nodes = second.endNodes(); s_u != s_end_nodes; ++s_u)
        {
            if( second.getId(s_u) == first.getId(f_u))
            {
                foundNode = true;
                for( f_e = first.beginEdges(f_u), f_end_edges = first.endEdges(f_u); f_e != f_end_edges; ++f_e)
                {
                    foundEdge = false;
                    for( s_e = second.beginEdges(s_u), s_end_edges = second.endEdges(s_u); s_e != s_end_edges; ++s_e)
                    {
                        if( second.getId(s_e) == first.getId(f_e))
                        {
                            foundEdge = true;
                        }
                    }
                    if( !foundEdge) return -1;
                }
            }
        }
        if( !foundNode) return -1;
    }
    std::cout << "done!\nBackward..." << std::flush;
    for( s_u = second.beginNodes(), s_end_nodes = second.endNodes(); s_u != s_end_nodes; ++s_u)
    {
        foundNode = false;
        for( f_u = first.beginNodes(), f_end_nodes = first.endNodes(); f_u != f_end_nodes; ++f_u)
        {
            if( second.getId(s_u->getId) == first.getId(f_u))
            {
                foundNode = true;
                for( s_e = second.beginEdges(s_u), s_end_edges = second.endEdges(s_u); s_e != s_end_edges; ++s_e)
                {
                    foundEdge = false;
                    for( f_e = first.beginEdges(f_u), f_end_edges = first.endEdges(f_u); f_e != f_end_edges; ++f_e)
                    {
                        if( second.getId(s_e) == first.getId(f_e))
                        {
                            foundEdge = true;
                        }
                    }
                    if( !foundEdge) return -1;
                }
            }
        }
        if( !foundNode) return 1;
    }
    std::cout << "done!\n" << std::flush;    
    return 0;
}


#endif // BASICGRAPHALGORITHMS_H
