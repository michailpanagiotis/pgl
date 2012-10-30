#ifndef PACKEDMEMORYGRAPHIMPL_H
#define PACKEDMEMORYGRAPHIMPL_H

#include <Utilities/mersenneTwister.h>
#include <vector>

template<typename Vtype, typename Etype>
class PMGNode;
template<typename Vtype, typename Etype>
class PMGEdge;
template<typename Vtype, typename Etype>
class PMGInEdge;
template<typename Vtype, typename Etype>
class PMGNodeObserver;
template<typename Vtype, typename Etype>
class PMGEdgeObserver;
template<typename Vtype, typename Etype>
class PMGInEdgeObserver;


template<typename Vtype, typename Etype>
class EdgeBucket
{
    friend class PMGNodeObserver< Vtype, Etype>;
    friend class PMGEdgeObserver< Vtype, Etype>;
    friend class PMGInEdgeObserver< Vtype, Etype>;
};



template<typename Vtype, typename Etype>
class PackedMemoryGraphImpl
{

    friend class PMGNodeObserver< Vtype, Etype>;
    friend class PMGEdgeObserver< Vtype, Etype>;
    friend class PMGInEdgeObserver< Vtype, Etype>;

public:
    
    typedef unsigned int                                                        SizeType;
    typedef PMGNode<Vtype,Etype>**                                              NodeDescriptor;
    typedef typename PackedMemoryArray< PMGNode< Vtype, Etype> >::Iterator      NodeIterator;
    typedef typename PackedMemoryArray< PMGEdge< Vtype, Etype> >::Iterator      EdgeIterator;
    typedef typename PackedMemoryArray< PMGInEdge< Vtype, Etype> >::Iterator    InEdgeIterator;
    

    PackedMemoryGraphImpl()
    {
        m_nodeObserver = new PMGNodeObserver< Vtype, Etype>(this);
        m_nodes.registerObserver( m_nodeObserver);
        
        m_edgeObserver = new PMGEdgeObserver< Vtype, Etype>(this);
        m_edges.registerObserver( m_edgeObserver);       
   
        m_InEdgeObserver = new PMGInEdgeObserver< Vtype, Etype>(this);
        m_inEdges.registerObserver( m_InEdgeObserver);
    }

    ~PackedMemoryGraphImpl() 
    {
        delete m_nodeObserver;
        delete m_edgeObserver;
        delete m_InEdgeObserver;

        NodeIterator end = m_nodes.end();
        EdgeIterator end_edges = m_edges.end();

        for( NodeIterator u = m_nodes.begin(); u != end; ++u)
        {
            delete u->getDescriptor();
        }
    }
    
    EdgeIterator beginEdges()
    {
        return m_edges.begin();
    }
    
    EdgeIterator beginEdges( const NodeIterator& u)  
    { 
        return getEdgeIteratorAtAddress(u->m_firstEdge);
    }
    
    InEdgeIterator beginInEdges( const NodeIterator& u)  
    { 
        return getInEdgeIteratorAtAddress( u->m_firstInEdge);
    }
    
    NodeIterator beginNodes()
    { 
        return  m_nodes.begin();
    }
    
    NodeIterator chooseNode()  
	{ 
	    double random = m_random.getRandomNormalizedDouble();
        SizeType pos = m_nodes.size() * random;
        
        NodeIterator m_auxNodeIterator = m_nodes.begin() + pos;
        return m_auxNodeIterator;
	}
	
	void clear()
    {
        m_nodes.clear();
        m_edges.clear();
        m_inEdges.clear();
    }
    
    void compress()
    {
        std::cout << "Compressing graph...\n";
        m_nodes.compress();
        m_edges.compress();
        m_inEdges.compress();
    }
    
    EdgeIterator endEdges()
    {
        return m_edges.end();
    }
    
    EdgeIterator endEdges( const NodeIterator& u)  
    { 
        return getEdgeIteratorAtAddress( u->m_lastEdge);
    }
       
    InEdgeIterator endInEdges( const NodeIterator& u)  
    { 
        return getInEdgeIteratorAtAddress( u->m_lastInEdge);
    }
    
    NodeIterator endNodes()
    { 
        return m_nodes.end();
    }
    
    void eraseEdge( NodeDescriptor uD, NodeDescriptor vD) 
    { 
        EdgeIterator e = getEdgeIterator( uD, vD);
        InEdgeIterator k = getInEdgeIterator( e);
        NodeIterator u = getNodeIterator(uD);
        NodeIterator v = getNodeIterator(vD);
        
        //std::cout << "Deleting\t" << e->getDescriptor() << std::endl;
        //delete (e->getDescriptor());
        //descriptor = 0;
        
        // if we erase a node's first edge
        if( u->m_firstEdge == e.getAddress())
        {
            NodeIterator w,z;
            EdgeIterator f = e;
            ++f;
            
            // if it has no more edges
            if( f.getAddress() == u->m_lastEdge)
            {
                setFirstEdge(u,0);
            }
            else
            {
                setFirstEdge(u, f.getAddress());
            }
        }

        if( v->m_firstInEdge == k.getAddress() )
		{
		    NodeIterator w,z;
			InEdgeIterator f = k;
		    ++f;
			if( f.getAddress() == v->m_lastInEdge)
            {
                setFirstInEdge( v, 0);
            }
            else
            {
                setFirstInEdge( v, f.getAddress());
            }
		}

        e->m_InEdge = 0;
        m_edges.erase( e);
        k->m_edge = 0;
        m_inEdges.erase( k);   
    }

    void eraseNode( NodeDescriptor uD) 
    { 
        NodeIterator u = getNodeIterator( uD);
        m_nodes.erase(u);
        delete uD;
    }
    
    void expand()
    {
        std::cout << "Expanding graph...\n";
        m_nodes.expand();
        m_edges.expand();
        m_inEdges.expand();
    }
    
    NodeIterator findNextNodeWithEdges( NodeIterator u)
    {
        ++u;
        NodeIterator end = m_nodes.end();
        for( ; u != end; ++u)
        {
            if( u->hasEdges())
            {
                //std::cout << "Found next edge at distance: " << m_auxNodeIterator - u << "***" << std::endl;
                return u;
            }
        }
        return end;
    }

    NodeIterator findNextNodeWithInEdges( NodeIterator u)
    {
        ++u;
        NodeIterator end = m_nodes.end();
        for( ; u != end; ++u)
        {
            if( u->hasInEdges())
            {
                //std::cout << "Found next back edge at distance: " << m_auxNodeIterator - u  << std::endl;
                return u;
            }
        }
        return end;
    }

    NodeIterator findPreviousNodeWithEdges( const NodeIterator& u)
    {
        NodeIterator begin = m_nodes.begin();
        NodeIterator m_auxNodeIterator = u;
        if( u == begin) return m_nodes.end();
        --m_auxNodeIterator;
        while( m_auxNodeIterator != begin)
        {
            if( m_auxNodeIterator->hasEdges())
            {
                return m_auxNodeIterator;
            }   
            --m_auxNodeIterator;
        }
        return m_auxNodeIterator->hasEdges()? m_auxNodeIterator:m_nodes.end();
    }
    
    NodeIterator findPreviousNodeWithInEdges( const NodeIterator& u)
    {
        NodeIterator begin = m_nodes.begin();
        NodeIterator m_auxNodeIterator = u;
        if( u == begin) return m_nodes.end();
        --m_auxNodeIterator;
        while( m_auxNodeIterator != begin)
        {
            if( m_auxNodeIterator->hasInEdges())
            {
                return m_auxNodeIterator;
            }   
            --m_auxNodeIterator;
        }
        return m_auxNodeIterator->hasInEdges()? m_auxNodeIterator:m_nodes.end();
    }
    

    bool hasEdges( const NodeIterator& u)
    {        
        return u->hasEdges();
    }

    bool hasInEdges( const NodeIterator& u)
    {
        return u->hasInEdges();
    }
  
    bool hasNode( const NodeDescriptor& descriptor)
    {
        return descriptor != 0;
    }
    
    NodeIterator getAdjacentNodeIterator( const EdgeIterator& e)
    { 
        return m_nodes.atAddress( e->m_adjacentNode);
    }
    
    NodeIterator getAdjacentNodeIterator( const InEdgeIterator& k)
    {
        return m_nodes.atAddress( k->m_adjacentNode);
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
        return m_edges.atAddress(k->m_edge);
    }
    
    EdgeIterator getEdgeIteratorAtAddress( PMGEdge<Vtype,Etype>* addr)
    {
        return m_edges.atAddress(addr);
    } 
    
    EdgeIterator getEdgeIteratorAtIndex( const SizeType& position)
    {
        return m_edges.atIndex(position);
    } 
    
    SizeType getId( const NodeIterator& u)
    {
        return m_nodes.getElementIndexOf(u);
    }
    
    InEdgeIterator getInEdgeIterator( const EdgeIterator& e)
    {
        return m_inEdges.atAddress( e->m_InEdge);
    }
    
    InEdgeIterator getInEdgeIteratorAtAddress( PMGInEdge<Vtype,Etype>* addr)
    {
        return m_inEdges.atAddress(addr);
    }
    
    InEdgeIterator getInEdgeIteratorAtIndex( const SizeType& position)
    {
        return m_inEdges.atIndex(position);
    } 

    NodeIterator getNodeIterator( const NodeDescriptor& descriptor) 
    { 
        return m_nodes.atAddress(*descriptor);
    }

    NodeIterator getNodeIterator( const SizeType& position) 
    { 
        return m_nodes.atIndex(position);
    }

    NodeIterator getNodeIteratorAtAddress( PMGNode<Vtype,Etype>* addr) 
    { 
        return m_nodes.atAddress(addr);
    }

	SizeType indeg( const NodeIterator& u)
    {
        return endInEdges(u) - beginInEdges(u);
    }

    void insertEdge( NodeDescriptor uD, NodeDescriptor vD) 
    { 
        //assert( isValid());
        NodeIterator u = getNodeIterator( uD);
        NodeIterator v = getNodeIterator( vD);
        
        assert( u != v);
        assert( (!u->hasEdges()) || (u->m_firstEdge != u->m_lastEdge));
        assert( (!v->hasInEdges()) || (v->m_firstInEdge != v->m_lastInEdge));
        
        NodeIterator w;
        EdgeIterator e;
        InEdgeIterator k;
        
            w = findNextNodeWithEdges(u);
            if( w != m_nodes.end()) e = getEdgeIteratorAtAddress(w->m_firstEdge);
            else                    e = m_edges.end();

            w = findNextNodeWithInEdges(v);
            if( w != m_nodes.end()) k = getInEdgeIteratorAtAddress(w->m_firstInEdge);
            else                    k = m_inEdges.end();
            

        PMGEdge<Vtype, Etype> newEdge( v.getAddress());
        PMGInEdge<Vtype, Etype> newInEdge( u.getAddress());
        e = m_edges.insert( e, newEdge);
        k = m_inEdges.insert( k, newInEdge);        

        e->m_InEdge = k.getAddress();
        k->m_edge = e.getAddress();  

        assert( k->m_adjacentNode != 0);

        if( !u->hasEdges())
        {
            setFirstEdge( u, e.getAddress());
            w = findNextNodeWithEdges(u);
            if( w != m_nodes.end())
            {
                u->m_lastEdge = w->m_firstEdge;
            }
        } 
        
        if( !v->hasInEdges())
        {
            setFirstInEdge( v, k.getAddress());
            w = findNextNodeWithInEdges(v);
            if( w != m_nodes.end())
            {
                v->m_lastInEdge = w->m_firstInEdge;
            }
        } 

        assert( (u->m_lastEdge > u->m_firstEdge) || (!(u->m_lastEdge)));
        assert( (v->m_lastInEdge > v->m_firstInEdge) || (!(v->m_lastInEdge)));
        assert( u->hasEdges() && v->hasInEdges());

    }

	SizeType outdeg( const NodeIterator& u)
    {
        return endEdges(u) - beginEdges(u);
    }

    void setFirstEdge( NodeIterator u, PMGEdge<Vtype,Etype>* address)
    {
        u->m_firstEdge = address;
        if( address)
        {
            u = findPreviousNodeWithEdges(u);
            if( u != m_nodes.end())
            {
                u->m_lastEdge = address;
            }
        }
        else
        {
            u->m_lastEdge = 0;
            u = findPreviousNodeWithEdges(u);
            if( u != m_nodes.end())
            {
                NodeIterator v = findNextNodeWithEdges(u);
                if( v != m_nodes.end())
                {
                    u->m_lastEdge = v->m_firstEdge;
                }
                else
                {
                    u->m_lastEdge = 0;
                }
            }
        }
    }
    
    void setFirstInEdge( NodeIterator u, PMGInEdge<Vtype,Etype>* address)
    {
        u->m_firstInEdge = address;
        if( address)
        {
            u = findPreviousNodeWithInEdges(u);
            if( u != m_nodes.end())
            {
                u->m_lastInEdge = address;
            }
        }
        else
        {
            u->m_lastInEdge = 0;
            u = findPreviousNodeWithInEdges(u);
            if( u != m_nodes.end())
            {
                NodeIterator v = findNextNodeWithInEdges(u);
                if( v != m_nodes.end())
                {
                    u->m_lastInEdge = v->m_firstInEdge;
                }
                else
                {
                    u->m_lastInEdge = 0;
                }
            }
        }
    }

    NodeDescriptor insertNode() 
    { 
        NodeDescriptor m_auxNodeDescriptor = new PMGNode<Vtype,Etype>*();
        *m_auxNodeDescriptor = 0;
        //std::cout << "\nMalloced for node" << m_auxNodeDescriptor << std::endl;
        PMGNode<Vtype,Etype> newNode;
        newNode.setDescriptor( m_auxNodeDescriptor);
        NodeIterator m_auxNodeIterator = m_nodes.optimalInsert( newNode);
        //NodeIterator m_auxNodeIterator = m_nodes.insert( m_nodes.end(),newNode);
        //NodeIterator m_auxNodeIterator = m_nodes.insert( m_nodes.begin(),newNode);
        *m_auxNodeDescriptor = m_auxNodeIterator.getAddress();
        m_lastPushedNode = m_nodes.end();
        m_currentPushedNode = m_nodes.end();
        return m_auxNodeDescriptor;
    }

    NodeDescriptor insertNodeBefore( NodeDescriptor descriptor) 
    { 
        NodeDescriptor m_auxNodeDescriptor = new PMGNode<Vtype,Etype>*();
        
        *m_auxNodeDescriptor = 0;
        PMGNode<Vtype,Etype> newNode;
        newNode.setDescriptor( m_auxNodeDescriptor);


        NodeIterator m_auxNodeIterator = m_nodes.insert( getNodeIterator(descriptor), newNode);

        *m_auxNodeDescriptor = m_auxNodeIterator.getAddress();

        m_lastPushedNode = m_nodes.end();
        m_currentPushedNode = m_nodes.end();
        return m_auxNodeDescriptor;
    }
    
    bool isValid()
    {
        PMGEdge<Vtype,Etype>* lastEdge = 0;
        PMGInEdge<Vtype,Etype>* lastInEdge = 0;

        bool valid = true;
        for( NodeIterator u = beginNodes(), end = endNodes(); u != end; ++u)
        {
            if( u->hasEdges())
            {
                if( lastEdge > u->m_firstEdge)
                {
                    valid = false;
                }
                lastEdge = u->m_firstEdge;
            }
            if( u->hasInEdges())
            {
                if( lastInEdge > u->m_firstInEdge)
                {
                    valid = false;
                }
                lastInEdge = u->m_firstInEdge;
            }
        }        
        return valid;
    }
    
    SizeType memUsage()   
    { 
        std::cout << "Graph mem Usage\t\tNodes\tEdges\tInEdges\n";
        std::cout << "\tNumber:\t\t" << m_nodes.size() << "\t" << m_edges.size() << "\t" << m_inEdges.size() << std::endl;
        std::cout << "\tReserved:\t" << m_nodes.capacity() << "\t" << m_edges.capacity() << "\t" << m_inEdges.capacity() << std::endl;
        std::cout << "\tSize:\t\t" << PMGNode<Vtype,Etype>::memUsage() << "\t" << PMGEdge<Vtype,Etype>::memUsage() << "\t" << PMGInEdge<Vtype,Etype>::memUsage() << std::endl;
        
        m_nodes.getMemoryUsage();

        /*std::cout << "\tNodes:\t\t" << m_nodes.size() << "\tReserved:" << m_nodes.capacity() << "\n"; 
        std::cout << "\tEdges:\t\t" << m_edges.size() << "\tReserved:" << m_edges.capacity() << "\n"; 
        std::cout << "\tInEdges:\t" << m_inEdges.size() << "\tReserved:" << m_inEdges.capacity() << "\n"; 
        std::cout << "\tNode Size:\t" << PMGNode<Vtype,Etype>::memUsage() << "bytes" << "\t";
        std::cout << "\tEdge Size:\t" << PMGEdge<Vtype,Etype>::memUsage() << "bytes" << "\t";
        std::cout << "\tBWEdge Size:\t" << PMGInEdge<Vtype,Etype>::memUsage() << "bytes" << std::endl;*/

        return  PMGNode<Vtype,Etype>::memUsage() * m_nodes.capacity() + 
                PMGEdge<Vtype,Etype>::memUsage() * m_edges.capacity() + 
                PMGInEdge<Vtype,Etype>::memUsage() * m_inEdges.capacity();
    }
    
    EdgeIterator nilEdge() 
    { 
        return m_edges.end();
    }

    NodeIterator nilNode()
    {
        return m_nodes.end();
    }  
    
    void printDot(std::ostream& out)
    {
        out << "digraph BST {\n\tnode [fontname=\"Arial\"]\n";
        m_nodes.printDot( out, "n", "e");
        m_edges.printDot( out, "e", "b");
        m_inEdges.printDot( out,"b");
        out << "}";
    }
    
    void printDotRange( const char* filename, const NodeIterator& firstNode, const NodeIterator& lastNode)
    {
        std::ofstream out( filename);
        out << "digraph BST {\n\tnode [fontname=\"Arial\"]\n";
        m_nodes.printDotRange( out, firstNode, lastNode, "n", "e");
        m_edges.printDotRange( out, beginEdges(firstNode), endEdges(lastNode), "e", "b");
        m_inEdges.printDotRange( out, beginInEdges(firstNode), endInEdges(lastNode), "b");
        out << "}";
        out.close();
    }

    void pushEdge( NodeDescriptor uD, NodeDescriptor vD) 
    { 
        //assert( isValid());
        NodeIterator u = getNodeIterator( uD);
        NodeIterator v = getNodeIterator( vD);
        
        if( u != m_currentPushedNode)
        {
            m_lastPushedNode = m_currentPushedNode;
            m_currentPushedNode = u;
        }
        
        assert( u != v);
        assert( (!u->hasEdges()) || (u->m_firstEdge != u->m_lastEdge));
        assert( (!v->hasInEdges()) || (v->m_firstInEdge != v->m_lastInEdge));
        
        NodeIterator w;
        EdgeIterator e;
        InEdgeIterator k;

        w = v;
        ++w;
            w = findNextNodeWithInEdges(w);
            if( w != m_nodes.end()) k = getInEdgeIteratorAtAddress(w->m_firstInEdge);
            else                    k = m_inEdges.end();
            

        PMGEdge<Vtype, Etype> newEdge( v.getAddress());
        PMGInEdge<Vtype, Etype> newInEdge( u.getAddress());


        m_edges.push_back( newEdge);
        e = m_edges.end();
        --e;
        
        k = m_inEdges.insert( k, newInEdge);        

        //e->m_adjacentNode = v.getPoolIndex();
        e->m_InEdge = k.getAddress();

        //k->m_adjacentNode = u.getPoolIndex();
        k->m_edge = e.getAddress();  

        assert( k->m_adjacentNode != 0);

        bool uHadEdges = true;
        bool vHadInEdges = true;

        if( !u->hasEdges())
        {
            uHadEdges = false;
            u->m_firstEdge = e.getAddress();
        }
        
        if( !v->hasInEdges())
        {
            vHadInEdges = false;
            v->m_firstInEdge = k.getAddress();
        }

        if( !uHadEdges)
        {
            //w = findPreviousNodeWithEdges(u);
            w = m_lastPushedNode;
            assert( w != u);
            if( w != m_nodes.end())
            {
                w->m_lastEdge = e.getAddress();
                assert( w->m_lastEdge != w->m_firstEdge);
                assert( (w->m_lastEdge > w->m_firstEdge) || (!w->m_lastEdge));
            }
        } 
        
        if( !vHadInEdges)
        {
            w = findPreviousNodeWithInEdges(v);
            assert( w != v);
            if( w != m_nodes.end())
            {
                w->m_lastInEdge = k.getAddress();
                assert( w->m_lastInEdge != w->m_firstInEdge);
                assert( (w->m_lastInEdge > w->m_firstInEdge) || (!w->m_lastInEdge));
            }
            w = v;
            ++w;    
            w = findNextNodeWithInEdges(w);
            if( w != m_nodes.end())
            {
                v->m_lastInEdge = w->m_firstInEdge;
            }
        } 

        assert( (u->m_lastEdge > u->m_firstEdge) || (!(u->m_lastEdge)));
        assert( (v->m_lastInEdge > v->m_firstInEdge) || (!(v->m_lastInEdge)));

        assert( u->hasEdges() && v->hasInEdges());


        /*std::stringstream sf;
        sf << "/home/michai/Projects/Graphs/GraphViz/" << m_edges.size() << "f.dot";
        s = sf.str();
        out.open(s.c_str());
        printDot(out);
        out.close();*/

    }

    void reserve( const SizeType& numNodes, const SizeType& numEdges)
    {
        std::cout << "\tReserving space for nodes\t";
        m_nodes.reserve( numNodes);
        std::cout << "\tReserving space for edges\t";
        m_edges.reserve( numEdges);
        std::cout << "\tReserving space for inedges\t";
        m_inEdges.reserve( numEdges);
    }

    void sanitizeDescriptor( const NodeIterator& u)
    {
        NodeDescriptor descriptor = getDescriptor(u);
        *descriptor = u.getAddress();
    }


    void setDescriptor( NodeIterator& u, NodeDescriptor& uD)
    {
        u->setDescriptor( uD);
        *uD = u.getAddress();
    }


private:
    PackedMemoryArray< PMGNode< Vtype, Etype> >     m_nodes;
    PackedMemoryArray< PMGEdge< Vtype, Etype> >     m_edges; 
    PackedMemoryArray< PMGInEdge< Vtype, Etype> >   m_inEdges;
    PMGNodeObserver< Vtype, Etype>*                 m_nodeObserver;
    PMGEdgeObserver< Vtype, Etype>*                 m_edgeObserver;
    PMGInEdgeObserver< Vtype, Etype>*               m_InEdgeObserver;

    NodeIterator m_lastPushedNode, m_currentPushedNode;

    MersenneTwister m_random;
};

template<typename Vtype, typename Etype>
class PMGEdge : public Etype
{

public:
    //typedef typename PackedMemoryGraphImpl<Vtype,Etype>::NodeDescriptor   NodeDescriptor;
    typedef typename PackedMemoryGraphImpl<Vtype,Etype>::SizeType         SizeType;
    
    PMGEdge( unsigned int init = 0): Etype(),
                        m_adjacentNode(0), 
                        m_InEdge(0)
    {
    }
   
    PMGEdge( PMGNode<Vtype,Etype>* adjacentNode):
            Etype(),
			m_adjacentNode(adjacentNode),
			m_InEdge(0)
    {
    }

    static unsigned int memUsage() 
    {
        return sizeof(PMGEdge);
    }

    
    bool operator ==( const PMGEdge& other) const
	{
        return ( m_InEdge == other.m_InEdge) && (m_adjacentNode == other.m_adjacentNode);
	}
	
	bool operator !=( const PMGEdge& other) const
	{
		return (m_InEdge != other.m_InEdge) || (m_adjacentNode != other.m_adjacentNode);
	}


    friend std::ostream& operator << ( std::ostream& out, PMGEdge other)
	{
        if( other.m_adjacentNode != 0)
        {
            out << "{" << other.m_adjacentNode << "|";
        }
        else
        {
            out << "{-|";
        }
        /*if( other.m_InEdge != 0)
        {   
            out << other.m_InEdge << "|";
        }
        else
        {
            out << "-|";
        }*/
        //out << "}|" << other.m_descriptor << "}";
        out << "}";
		return out;
	}

    PMGNode<Vtype,Etype>*       m_adjacentNode;
    PMGInEdge<Vtype,Etype>*     m_InEdge;   
};


template<typename Vtype, typename Etype>
class PMGInEdge : public Etype
{

public:
    typedef typename PackedMemoryGraphImpl<Vtype,Etype>::NodeDescriptor   NodeDescriptor;
    typedef typename PackedMemoryGraphImpl<Vtype,Etype>::SizeType         SizeType;
   
    PMGInEdge( unsigned int init = 0): Etype(),
                        m_adjacentNode(0), 
                        m_edge(0)
    {
    }
   
    PMGInEdge( PMGNode<Vtype,Etype>* adjacentNode):
            Etype(),
			m_adjacentNode(adjacentNode),
			m_edge(0)
    {
    }

    static unsigned int memUsage() 
    {
        return sizeof(PMGInEdge);
    }

    bool operator ==( const PMGInEdge& other) const
	{
        return ( m_edge == other.m_edge) && (m_adjacentNode == other.m_adjacentNode);
	}
	
	bool operator !=( const PMGInEdge& other) const
	{
		return (m_edge != other.m_edge) || (m_adjacentNode != other.m_adjacentNode);
	}

    friend std::ostream& operator << ( std::ostream& out, PMGInEdge other)
	{
        if( other.m_adjacentNode != 0)
        {
            out << "{" << other.m_adjacentNode << "|";
        }
        else
        {
            out << "{-|";
        }
        /*if( other.m_edge != 0)
        {   
            out << other.m_edge << "|";
        }
        else
        {
            out << "-|";
        }*/
        //out << "}|" << other.m_descriptor;
        out << "}";
		return out;
	}

    PMGNode<Vtype,Etype>*    m_adjacentNode;
    PMGEdge<Vtype,Etype>*    m_edge;   
};


template<typename Vtype, typename Etype>
class PMGNode : public GraphElement< Vtype, typename PackedMemoryGraphImpl<Vtype,Etype>::NodeDescriptor>
{
public:
    typedef typename PackedMemoryGraphImpl<Vtype,Etype>::NodeDescriptor NodeDescriptor;
    typedef typename PackedMemoryGraphImpl<Vtype,Etype>::SizeType       SizeType;

    PMGNode( unsigned int init = 0):GraphElement< Vtype, NodeDescriptor>(), 
                                    m_firstEdge(0), 
                                    m_lastEdge(0),
                                    m_firstInEdge(0),
                                    m_lastInEdge(0)
    {
    }

    PMGNode( NodeDescriptor descriptor):GraphElement< Vtype, NodeDescriptor>(descriptor), 
                                    m_firstEdge(0), 
                                    m_lastEdge(0),
                                    m_firstInEdge(0),
                                    m_lastInEdge(0)
    {
    }
    
    bool hasEdges() const
    {
        return m_firstEdge != 0;
    }

    bool hasInEdges() const
    {
        return m_firstInEdge != 0;
    }

    static unsigned int memUsage() 
    {
        return sizeof(PMGNode);
    }

    friend std::ostream& operator << ( std::ostream& out, PMGNode other)
	{
        if( other.hasEdges())
        {
            out << "{" << other.m_firstEdge << "|" << other.m_lastEdge << "}";
        }
        else
        {
            out << "{-|-}";
        }
        /*if( other.hasInEdges())
        {   
            out << "|{" << other.m_firstInEdge << "|" << other.m_lastInEdge << "}";
        }
        else
        {
            out << "|{-|-}";
        }*/
        //out << "}";
        //out << "}|" << other.m_descriptor << "}";
		return out;
	}


    PMGEdge<Vtype,Etype>*     m_firstEdge;
    PMGEdge<Vtype,Etype>*     m_lastEdge;
	PMGInEdge<Vtype,Etype>*   m_firstInEdge;
    PMGInEdge<Vtype,Etype>*   m_lastInEdge;
};


template<typename Vtype, typename Etype>
class PMGNodeObserver : public PackedMemoryArray< PMGNode<Vtype,Etype> >::Observer
{
public:

    typedef typename PackedMemoryGraphImpl<Vtype,Etype>::EdgeIterator   EdgeIterator;
    typedef typename PackedMemoryGraphImpl<Vtype,Etype>::InEdgeIterator   InEdgeIterator;
    typedef typename PackedMemoryGraphImpl<Vtype,Etype>::NodeIterator   NodeIterator;
    typedef typename PackedMemoryGraphImpl<Vtype,Etype>::SizeType       SizeType;

    PMGNodeObserver( PackedMemoryGraphImpl<Vtype,Etype>* G): m_G(G)
    {
    }

    void move( PMGNode<Vtype,Etype>* source, PMGNode<Vtype,Etype>* sourcePool, PMGNode<Vtype,Etype>* destination, PMGNode<Vtype,Etype>* destinationPool, const PMGNode<Vtype,Etype>& node)
    {
        assert( node != m_G->m_nodes.getEmptyElement());
       
        if( node.hasEdges())
        {
            EdgeIterator e = m_G->getEdgeIteratorAtAddress( node.m_firstEdge);
            EdgeIterator endEdges = m_G->getEdgeIteratorAtAddress( node.m_lastEdge);
            while ( e != endEdges)
            {
                e->m_InEdge->m_adjacentNode = destination;
                ++e;
            }
        }

        if( node.hasInEdges())
        {
            InEdgeIterator k = m_G->getInEdgeIteratorAtAddress( node.m_firstInEdge);
            InEdgeIterator endInEdges = m_G->getInEdgeIteratorAtAddress( node.m_lastInEdge);
            while ( k != endInEdges)
            {
                k->m_edge->m_adjacentNode = destination;
                ++k;
            }
        }

        //std::cout << "Moving " << source << " to " << destination << std::endl;
		*(node.getDescriptor()) = destination;        
    }
private:
    PackedMemoryGraphImpl<Vtype,Etype>* m_G;
    EdgeIterator                        e, end;
    InEdgeIterator                      back_e, back_end;
};


template<typename Vtype, typename Etype>
class PMGEdgeObserver : public PackedMemoryArray< PMGEdge<Vtype,Etype> >::Observer
{
public:
    typedef typename PackedMemoryGraphImpl<Vtype,Etype>::SizeType       SizeType;
    typedef typename PackedMemoryGraphImpl<Vtype,Etype>::NodeIterator   NodeIterator;
    typedef typename PackedMemoryGraphImpl<Vtype,Etype>::InEdgeIterator   InEdgeIterator;

    PMGEdgeObserver( PackedMemoryGraphImpl<Vtype,Etype>* G): m_G(G),lastChangedNode(0),lastChangedTailNode(0)
    {
    }

    void move( PMGEdge<Vtype,Etype>* source, PMGEdge<Vtype,Etype>* sourcePool, PMGEdge<Vtype,Etype>* destination, PMGEdge<Vtype,Etype>* destinationPool, const PMGEdge<Vtype,Etype>& edge)
    {
        if( source == destination) return;
        assert( edge != m_G->m_edges.getEmptyElement());

        if( !(edge.m_InEdge)) return;      
        
        edge.m_InEdge->m_edge = destination;

        /*if( source == (PMGEdge<Vtype,Etype>*)0x139c910)
        {
            std::cout << "Hi!\n";
        }*/

        if( (edge.m_InEdge->m_adjacentNode->m_firstEdge == source) 
            && (edge.m_InEdge->m_adjacentNode != lastChangedNode))
        {
            NodeIterator u = m_G->getNodeIteratorAtAddress(edge.m_InEdge->m_adjacentNode);
            m_G->setFirstEdge( u, destination);   
            lastChangedNode = edge.m_InEdge->m_adjacentNode;
        }
        
    }

    void reset()
    {
        lastChangedNode = 0;
        lastChangedTailNode = 0;
    }
private:
    PackedMemoryGraphImpl<Vtype,Etype>* m_G;
    PMGNode<Vtype,Etype>*  lastChangedNode;
    PMGNode<Vtype,Etype>*  lastChangedTailNode;
};


template<typename Vtype, typename Etype>
class PMGInEdgeObserver : public PackedMemoryArray< PMGInEdge<Vtype,Etype> >::Observer
{
public:
    typedef typename PackedMemoryGraphImpl<Vtype,Etype>::SizeType       SizeType;
    typedef typename PackedMemoryGraphImpl<Vtype,Etype>::NodeIterator   NodeIterator;
    typedef typename PackedMemoryGraphImpl<Vtype,Etype>::EdgeIterator   EdgeIterator;

    PMGInEdgeObserver( PackedMemoryGraphImpl<Vtype,Etype>* G): m_G(G), lastChangedNode(0), lastChangedTailNode(0)
    {
    }

    void move( PMGInEdge<Vtype,Etype>* source, PMGInEdge<Vtype,Etype>* sourcePool, PMGInEdge<Vtype,Etype>* destination, PMGInEdge<Vtype,Etype>* destinationPool, const PMGInEdge<Vtype,Etype>& InEdge)
    {
        if( source == destination) return;

        assert( InEdge != m_G->m_inEdges.getEmptyElement());

        /*if( source == (PMGInEdge<Vtype,Etype>*)0x1735318)
        {
            std::cout << "Hi!\n";
        }
        
        if( source == (PMGInEdge<Vtype,Etype>*)0x1735324)
        {
            std::cout << "Hi!\n";
        }*/

        if( !(InEdge.m_edge)) return;

        InEdge.m_edge->m_InEdge = destination;

        PMGNode<Vtype,Etype>* adjacentNode = InEdge.m_edge->m_adjacentNode;
        PMGInEdge<Vtype,Etype>* firstInEdge = adjacentNode->m_firstInEdge;

        if( ( firstInEdge == source) && ( adjacentNode != lastChangedNode))
        {
            NodeIterator u = m_G->getNodeIteratorAtAddress(adjacentNode);
            m_G->setFirstInEdge( u, destination);
            lastChangedNode = adjacentNode;
        }
    }

    void reset()
    {
        lastChangedNode = 0;
        lastChangedTailNode = 0;
    }
private:
    PackedMemoryGraphImpl<Vtype,Etype>* m_G;
    PMGNode<Vtype,Etype>*  lastChangedNode;
    PMGNode<Vtype,Etype>*  lastChangedTailNode;
};


#endif //PACKEDMEMORYGRAPHIMPL_H
