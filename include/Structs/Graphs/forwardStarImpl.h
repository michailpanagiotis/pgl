#ifndef FORWARDSTARIMPL_H
#define FORWARDSTARIMPL_H

#include <Structs/Graphs/dynamicGraph.h>
#include <Structs/Sets/bucketSet.h>
#include <Utilities/mersenneTwister.h>
#include <list>

template<typename Vtype, typename Etype>
class FSNode;
template<typename Vtype, typename Etype>
class FSEdge;
template<typename Vtype, typename Etype>
class FSInEdge;


template<typename Vtype, typename Etype>
class ForwardStarImpl
{
public:
    
    typedef unsigned int                                                SizeType;
    typedef typename std::list< FSNode< Vtype, Etype> >::iterator       NodeIterator;
    typedef typename std::vector< FSEdge< Vtype, Etype> >::iterator     EdgeIterator;
    typedef typename std::vector< FSInEdge< Vtype, Etype> >::iterator   InEdgeIterator;
    typedef NodeIterator*                                               NodeDescriptor;

    ForwardStarImpl():m_numNodes(0),m_numEdges(0)
    {
    }

    ~ForwardStarImpl() 
    {
        NodeIterator end = m_nodes.end();
        EdgeIterator e, end_edges;

        for( NodeIterator u = m_nodes.begin(); u != end; ++u)
        {
            delete u->getDescriptor();
        }        
    }

    EdgeIterator beginEdges( const NodeIterator& u) 
    { 
        return u->m_edges.begin();
    }
    
    InEdgeIterator beginInEdges( const NodeIterator& u) 
    { 
        return u->m_inEdges.begin();
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
        return u->m_inEdges.end();
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

        (*v).m_inEdges.erase(k);
        (*u).m_edges.erase(e);

        --m_numEdges;
    }
    
    void eraseEdge( const InEdgeIterator& k)
    {
        eraseEdge( getEdgeIterator(k));
    }
    
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

    EdgeIterator getEdgeIterator( const InEdgeIterator& k)
    { 
        NodeIterator u = getAdjacentNodeIterator(k);
        return beginEdges(u) + k->m_edge;
    }
    
    SizeType getId( const NodeIterator& u) 
    {
        return distance( m_nodes.begin(), u);
    }
    
    InEdgeIterator getInEdgeIterator( const EdgeIterator& e)
    {
        NodeIterator u = getAdjacentNodeIterator(e);
        return beginInEdges(u) + e->m_InEdge;
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
        return !(*u).m_inEdges.empty();
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
        FSNode<Vtype,Etype> newNode;
        //create descriptor
        NodeDescriptor m_auxNodeDescriptor = new NodeIterator();
        newNode.setDescriptor( m_auxNodeDescriptor);

        //insert node
        m_nodes.push_back( newNode);
        NodeIterator m_auxNodeIterator = m_nodes.end();
        --m_auxNodeIterator;
        
        m_edgeSet.registerBucket(&(m_auxNodeIterator->m_edges));
        m_inEdgeSet.registerBucket(&(m_auxNodeIterator->m_inEdges));

        //update descriptor
        *m_auxNodeDescriptor = m_auxNodeIterator;
        ++m_numNodes;
        return m_auxNodeDescriptor;
    }

    NodeDescriptor insertNodeBefore( const NodeDescriptor& uD) 
    { 
        //create empty node
        FSNode<Vtype,Etype> newNode;
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
        FSEdge<Vtype, Etype> newEdge;
        //create empty back edge
        FSInEdge<Vtype, Etype> newInEdge;
    
        //insert forward edge
        (*u).m_edges.push_back( newEdge);
        
        //insert backward edge
        (*v).m_inEdges.push_back( newInEdge);
        
        //get iterators on edges
        EdgeIterator e = u->m_edges.end();
        --e;
        InEdgeIterator k = v->m_inEdges.end();
        --k;

        //insert edge info
        e->m_adjacentNode = v;
        e->m_InEdge = distance( v->m_inEdges.begin(), k);

        k->m_adjacentNode = u;        
        k->m_edge = distance( u->m_edges.begin(), e);

        ++m_numEdges;
    }
    
    SizeType memUsage()   
    { 
        std::cout << "Graph mem Usage\t\tNodes\tEdges\tInEdges\n";
        std::cout << "\tNumber:\t\t" << m_numNodes << "\t" << m_numEdges << "\t" << m_numEdges << std::endl;
        std::cout << "\tSize:\t\t" << FSNode<Vtype,Etype>::memUsage() << "\t" << FSEdge<Vtype,Etype>::memUsage() << "\t" << FSInEdge<Vtype,Etype>::memUsage() << std::endl;
        
        /*std::cout << "\tNodes:\t\t" << m_numNodes << "\n"; 
        std::cout << "\tEdges:\t\t" << m_numEdges << "\n"; 
        std::cout << "\tInEdges:\t" << m_numEdges << "\n";
        std::cout << "\tNode Size:\t" << FSNode< Vtype, Etype>::memUsage() << "bytes" << "\t";
        std::cout << "\tEdge Size:\t" << FSEdge< Vtype, Etype>::memUsage() << "bytes" << "\t";
        std::cout << "\tBWEdge Size:\t" << FSInEdge< Vtype, Etype>::memUsage() << "bytes" << std::endl;*/

        return m_numNodes * FSNode< Vtype, Etype>::memUsage() + m_numEdges * (FSEdge< Vtype, Etype>::memUsage() + FSInEdge< Vtype, Etype>::memUsage());
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
    std::list< FSNode< Vtype, Etype> >  m_nodes;
    MersenneTwister                     m_random;
    SizeType                            m_numNodes;
    SizeType                            m_numEdges;
    BucketSet< FSEdge< Vtype, Etype> >  m_edgeSet;
    BucketSet< FSInEdge< Vtype, Etype> >  m_inEdgeSet;
};


template<typename Vtype, typename Etype>
class FSEdge : public Etype
{

public:
    typedef typename ForwardStarImpl<Vtype,Etype>::InEdgeIterator InEdgeIterator;
    typedef typename ForwardStarImpl<Vtype,Etype>::NodeIterator NodeIterator;
    typedef typename ForwardStarImpl<Vtype,Etype>::NodeDescriptor NodeDescriptor;
    typedef typename ForwardStarImpl<Vtype,Etype>::SizeType SizeType;

    FSEdge( unsigned int init = 0): Etype()
    {
    }
   
    FSEdge( NodeIterator adjacentNode , SizeType oppositeEdge , Etype data = Etype()):
            Etype(),
		    m_adjacentNode(adjacentNode),
			m_InEdge(oppositeEdge)
    {
    }
    
    static unsigned int memUsage() 
    {   
        //std::cout << GraphElement< Etype, EdgeDescriptor>::memUsage() + sizeof(NodeIterator) + sizeof(EdgeIterator) + sizeof(bool) << std::endl;
        return sizeof(FSEdge);
    }

    NodeIterator    m_adjacentNode;
    SizeType        m_InEdge;
};


template<typename Vtype, typename Etype>
class FSInEdge : public Etype
{

public:
    typedef typename ForwardStarImpl<Vtype,Etype>::EdgeIterator EdgeIterator;
    typedef typename ForwardStarImpl<Vtype,Etype>::NodeIterator NodeIterator;
    typedef typename ForwardStarImpl<Vtype,Etype>::NodeDescriptor NodeDescriptor;
    typedef typename ForwardStarImpl<Vtype,Etype>::SizeType SizeType;

    FSInEdge( unsigned int init = 0): Etype()
    {
    }
   
    FSInEdge( NodeIterator adjacentNode , SizeType oppositeEdge ,  Etype data = Etype()):
            Etype(),
		    m_adjacentNode(adjacentNode),
			m_edge(oppositeEdge)
    {
    }

    FSInEdge( const FSEdge<Vtype,Etype>& edge):Etype(edge)
    {
    }
    
    static unsigned int memUsage() 
    {   
        //std::cout << GraphElement< Etype, EdgeDescriptor>::memUsage() + sizeof(NodeIterator) + sizeof(EdgeIterator) + sizeof(bool) << std::endl;
        return sizeof(FSInEdge);
    }

    NodeIterator    m_adjacentNode;
    SizeType        m_edge;
};

template<typename Vtype, typename Etype>
class FSNode : public GraphElement< Vtype, typename ForwardStarImpl<Vtype,Etype>::NodeDescriptor>
{
public:
    typedef typename ForwardStarImpl<Vtype,Etype>::NodeDescriptor NodeDescriptor;
    typedef typename std::list<FSEdge< Vtype, Etype> >::const_iterator iterator;
    typedef typename std::list<FSInEdge< Vtype, Etype> >::const_iterator backIterator;

    FSNode():GraphElement< Vtype, NodeDescriptor>()
    {
    }

    FSNode( NodeDescriptor descriptor):GraphElement< Vtype, NodeDescriptor>( descriptor)
    {
    }
    
    /*FSNode( const FSNode& other):GraphElement< Vtype, NodeDescriptor>(other)
    {
        for( iterator it = other.m_edges.begin(); it != m_edges.end(); ++it)
        {
            m_edges.push_back( *it);
        }
        
        for( backIterator it = other.m_inEdges.begin(); it != m_inEdges.end(); ++it)
        {
            m_inEdges.push_back( *it);
        }
    }*/

    static unsigned int memUsage() 
    {
        return sizeof(FSNode);
    }

    typename BucketSet<FSEdge< Vtype, Etype> >::Bucket         m_edges;
    typename BucketSet<FSInEdge< Vtype, Etype> >::Bucket       m_inEdges;
    
};



#endif //FORWARDSTARIMPL_H
