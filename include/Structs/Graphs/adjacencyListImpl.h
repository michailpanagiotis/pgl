#ifndef ADJACENCYLISTIMPL_H
#define ADJACENCYLISTIMPL_H

#include <Structs/Graphs/dynamicGraph.h>
#include <Utilities/mersenneTwister.h>
#include <list>

template<typename Vtype, typename Etype>
class ALNode;
template<typename Vtype, typename Etype>
class ALEdge;
template<typename Vtype, typename Etype>
class ALInEdge;


template<typename Vtype, typename Etype>
class AdjacencyListImpl
{
public:
    
    typedef unsigned int                                                SizeType;
    typedef typename std::list< ALNode< Vtype, Etype> >::iterator       NodeIterator;
    typedef typename std::list< ALEdge< Vtype, Etype> >::iterator       EdgeIterator;
    typedef typename std::list< ALInEdge< Vtype, Etype> >::iterator     InEdgeIterator;
    typedef NodeIterator*                                               NodeDescriptor;

    AdjacencyListImpl():m_numNodes(0),m_numEdges(0)
    {
    }

    ~AdjacencyListImpl() 
    {
        NodeIterator end = m_nodes.end();
        EdgeIterator e, end_edges;

        for( NodeIterator u = m_nodes.begin(); u != end; ++u)
        {
            delete getDescriptor(u);
        }        
    }

    EdgeIterator beginEdges( const NodeIterator& u) 
    { 
        return u->m_edges.begin();
    }
    
    InEdgeIterator beginInEdges( const NodeIterator& u) 
    { 
        return u->m_InEdges.begin();
    }
    
    NodeIterator beginNodes()
    { 
        return m_nodes.begin();
    }
    
    NodeIterator chooseNode()  
	{ 
	    double random = m_random.getRandomNormalizedDouble();
        SizeType pos = m_nodes.size() * random;
        
        NodeIterator m_auxNodeIterator = m_nodes.begin();
        
        std::advance( m_auxNodeIterator, pos);
        return m_auxNodeIterator;
	}
	
	void clear()
    {
        m_nodes.clear();
        m_numNodes = 0;
        m_numEdges = 0;
    }
    
    void compress()
    {   
    }
    
    EdgeIterator endEdges( const NodeIterator& u) 
    { 
        return u->m_edges.end();
    }
    
    InEdgeIterator endInEdges( const NodeIterator& u) 
    { 
        return u->m_InEdges.end();
    }
    
    NodeIterator endNodes() 
    { 
        return m_nodes.end();
    }

    void eraseNode( NodeDescriptor& descriptor) 
    { 
        NodeIterator u = getNodeIterator(descriptor);
        delete descriptor;
        descriptor = 0;
        m_nodes.erase(u);
        --m_numNodes;
    }
    
    void eraseEdge( NodeDescriptor& uD, NodeDescriptor& vD) 
    { 
        EdgeIterator e = getEdgeIterator( uD, vD);
        NodeIterator v = getNodeIterator( vD);
        InEdgeIterator k = getInEdgeIterator( e);
        NodeIterator u = getNodeIterator( uD);

        (*v).m_InEdges.erase(k);
        (*u).m_edges.erase(e);

        --m_numEdges;
    }
    
    /*void eraseEdge( const InEdgeIterator& k)
    {
        eraseEdge( getEdgeIterator(k));
    }*/
    
    void expand()
    {
    }
    
    NodeIterator getAdjacentNodeIterator( const EdgeIterator& e)
    {
        return e->m_adjacentNode;
    }

    NodeIterator getAdjacentNodeIterator( const InEdgeIterator& k)
    {
        return k->m_adjacentNode;
    }
    
    
    NodeDescriptor getDescriptor( const NodeIterator& u) const
    {
        return u->getDescriptor();
    }
	
    EdgeIterator getEdgeIterator( const NodeDescriptor& uD, const NodeDescriptor& vD)
    {
        NodeIterator u = getNodeIterator( uD);
        NodeIterator v = getNodeIterator( vD);
        EdgeIterator e, end;
        NodeIterator neigh;        
        for( e = beginEdges(u), end = endEdges(u); e != end; ++e)
        {
            neigh = getAdjacentNodeIterator(e);
            if( neigh == v)
            {
                break;
            }
        }
        return e;
    }

    
    EdgeIterator getEdgeIterator( const InEdgeIterator& k) const
    { 
        return k->m_edge;
    }
    
    SizeType getId( const NodeIterator& u) 
    {
        return distance( m_nodes.begin(), u);
    }
    
    InEdgeIterator getInEdgeIterator( const EdgeIterator& e)
    {
        return e->m_InEdge;
    }

    NodeIterator getNodeIterator( const NodeDescriptor& descriptor) const
    { 
        return *descriptor;
    }
    
    bool hasEdge( const NodeDescriptor& uD, const NodeDescriptor& vD)
    {
        if( hasNode(uD) && hasNode(vD))
        {
            NodeIterator u = getNodeIterator( uD);
            NodeIterator v = getNodeIterator( vD);
            EdgeIterator e, end;
            NodeIterator neigh;        
            for( e = beginEdges(u), end = endEdges(u); e != end; ++e)
            {
                neigh = target(e);
                if( neigh == v)
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool hasEdges( const NodeIterator& u)
    {
        return !(*u).m_edges.empty();
    }

    bool hasInEdges( const NodeIterator& u)
    {
        return !(*u).m_InEdges.empty();
    }
  
    bool hasNode( const NodeDescriptor& descriptor)
    {
        return descriptor != 0;
    }
    
    SizeType indeg( const NodeIterator& u)
    {
        return distance( beginInEdges(u), endInEdges(u));
    }
    
    NodeDescriptor insertNode() 
    { 
        //create empty node
        ALNode<Vtype,Etype> newNode;
        //create descriptor
        NodeDescriptor m_auxNodeDescriptor = new NodeIterator();
        newNode.setDescriptor( m_auxNodeDescriptor);

        //insert node
        m_nodes.push_back( newNode);
        NodeIterator m_auxNodeIterator = m_nodes.end();
        --m_auxNodeIterator;
        
        //update descriptor
        *m_auxNodeDescriptor = m_auxNodeIterator;
        ++m_numNodes;
        return m_auxNodeDescriptor;
    }

    NodeDescriptor insertNodeBefore( const NodeDescriptor& uD) 
    { 
        //create empty node
        ALNode<Vtype,Etype> newNode;
        //create descriptor
        NodeDescriptor m_auxNodeDescriptor = new NodeIterator();
        newNode.setDescriptor( m_auxNodeDescriptor);

        NodeIterator u = getNodeIterator(uD);
        //insert node
        NodeIterator m_auxNodeIterator = m_nodes.insert( u, newNode);
        
        //update descriptor
        *m_auxNodeDescriptor = m_auxNodeIterator;
        ++m_numNodes;
        return m_auxNodeDescriptor;
    }
    
    void insertEdge( const NodeDescriptor& uD, const NodeDescriptor& vD) 
    { 
        NodeIterator u = getNodeIterator( uD);
        NodeIterator v = getNodeIterator( vD);
        
        //create empty edge
        ALEdge<Vtype, Etype> newEdge;
        //create empty back edge
        ALInEdge<Vtype, Etype> newInEdge;
    

        //insert forward edge
        (*u).m_edges.push_back( newEdge);
        
        //insert backward edge
        (*v).m_InEdges.push_back( newInEdge);
        
        //get iterators on edges
        EdgeIterator e = u->m_edges.end();
        --e;
        InEdgeIterator k = v->m_InEdges.end();
        --k;

        //insert edge info
        e->m_adjacentNode = v;
        e->m_InEdge = k;

        k->m_adjacentNode = u;        
        k->m_edge = e;

        ++m_numEdges;
    }
    
    SizeType memUsage()   
    { 
        std::cout << "Graph mem Usage\t\tNodes\tEdges\tInEdges\n";
        std::cout << "\tNumber:\t\t" << m_numNodes << "\t" << m_numEdges << "\t" << m_numEdges << std::endl;
        std::cout << "\tSize:\t\t" << ALNode<Vtype,Etype>::memUsage() << "\t" << ALEdge<Vtype,Etype>::memUsage() << "\t" << ALInEdge<Vtype,Etype>::memUsage() << std::endl;
        
        /*std::cout << "\tNodes:\t\t" << m_numNodes << "\n"; 
        std::cout << "\tEdges:\t\t" << m_numEdges << "\n"; 
        std::cout << "\tInEdges:\t" << m_numEdges << "\n";
        std::cout << "\tNode Size:\t" << ALNode< Vtype, Etype>::memUsage() << "bytes" << "\t";
        std::cout << "\tEdge Size:\t" << ALEdge< Vtype, Etype>::memUsage() << "bytes" << "\t";
        std::cout << "\tBWEdge Size:\t" << ALInEdge< Vtype, Etype>::memUsage() << "bytes" << std::endl;*/

        return m_numNodes * ALNode< Vtype, Etype>::memUsage() + m_numEdges * (ALEdge< Vtype, Etype>::memUsage() + ALInEdge< Vtype, Etype>::memUsage());
    }
    
    
    EdgeIterator nilEdge() const 
    { 
        return (*m_nodes.begin()).m_edges.end();
    }

    NodeIterator nilNode() const 
    {
        return m_nodes.end();
    }   
    
    SizeType outdeg( const NodeIterator& u)
    {
        return distance( beginEdges(u), endEdges(u));
    }
    
    void pushEdge( const NodeDescriptor& uD, const NodeDescriptor& vD) 
    {
        insertEdge(uD,vD);
    }

    void reserve( const SizeType& numNodes, const SizeType& numEdges)
    {
        return;
    }    

    void sanitizeDescriptor( const NodeIterator& u)
    {
        NodeDescriptor descriptor = getDescriptor(u);
        *descriptor = u;
    }


    void setDescriptor( NodeIterator& u, NodeDescriptor& uD)
    {
        u->setDescriptor( uD);
        *uD = u;
    }


private:
    std::list< ALNode< Vtype, Etype> >  m_nodes;
    MersenneTwister                     m_random;
    SizeType                            m_numNodes;
    SizeType                            m_numEdges;
};


template<typename Vtype, typename Etype>
class ALEdge : public Etype
{

public:
    typedef typename AdjacencyListImpl<Vtype,Etype>::InEdgeIterator InEdgeIterator;
    typedef typename AdjacencyListImpl<Vtype,Etype>::NodeIterator NodeIterator;
    typedef typename AdjacencyListImpl<Vtype,Etype>::NodeDescriptor NodeDescriptor;

    ALEdge( unsigned int init = 0): Etype()
    {
    }
   
    ALEdge( NodeIterator adjacentNode , InEdgeIterator oppositeEdge , Etype data = Etype()):
            Etype(),
		    m_adjacentNode(adjacentNode),
			m_InEdge(oppositeEdge)
    {
    }
    
    static unsigned int memUsage() 
    {   
        //std::cout << GraphElement< Etype, EdgeDescriptor>::memUsage() + sizeof(NodeIterator) + sizeof(EdgeIterator) + sizeof(bool) << std::endl;
        return sizeof(ALEdge);
    }

    NodeIterator              m_adjacentNode;
    InEdgeIterator              m_InEdge;
};


template<typename Vtype, typename Etype>
class ALInEdge : public Etype
{

public:
    typedef typename AdjacencyListImpl<Vtype,Etype>::EdgeIterator EdgeIterator;
    typedef typename AdjacencyListImpl<Vtype,Etype>::NodeIterator NodeIterator;
    typedef typename AdjacencyListImpl<Vtype,Etype>::NodeDescriptor NodeDescriptor;

    ALInEdge( unsigned int init = 0): Etype()
    {
    }
   
    ALInEdge( NodeIterator adjacentNode , EdgeIterator oppositeEdge ,  Etype data = Etype()):
            Etype(),
		    m_adjacentNode(adjacentNode),
			m_edge(oppositeEdge)
    {
    }

    ALInEdge( const ALEdge<Vtype,Etype>& edge):Etype(edge)
    {
    }
    
    static unsigned int memUsage() 
    {   
        //std::cout << GraphElement< Etype, EdgeDescriptor>::memUsage() + sizeof(NodeIterator) + sizeof(EdgeIterator) + sizeof(bool) << std::endl;
        return sizeof(ALInEdge);
    }

    NodeIterator              m_adjacentNode;
    EdgeIterator              m_edge;
};

template<typename Vtype, typename Etype>
class ALNode : public GraphElement< Vtype, typename AdjacencyListImpl<Vtype,Etype>::NodeDescriptor>
{
public:
    typedef typename AdjacencyListImpl<Vtype,Etype>::NodeDescriptor NodeDescriptor;
    typedef typename std::list<ALEdge< Vtype, Etype> >::const_iterator iterator;
    typedef typename std::list<ALInEdge< Vtype, Etype> >::const_iterator backIterator;

    ALNode():GraphElement< Vtype, NodeDescriptor>()
    {
    }

    ALNode( NodeDescriptor descriptor):GraphElement< Vtype, NodeDescriptor>( descriptor)
    {
    }
    
    /*ALNode( const ALNode& other):GraphElement< Vtype, NodeDescriptor>(other)
    {
        for( iterator it = other.m_edges.begin(); it != m_edges.end(); ++it)
        {
            m_edges.push_back( *it);
        }
        
        for( backIterator it = other.m_InEdges.begin(); it != m_InEdges.end(); ++it)
        {
            m_InEdges.push_back( *it);
        }
    }*/

    static unsigned int memUsage() 
    {
        return sizeof(ALNode);
    }

    std::list< ALEdge< Vtype, Etype> >            m_edges;
    std::list< ALInEdge< Vtype, Etype> >          m_InEdges;
    
};



#endif //ADJACENCYLISTIMPL_H
