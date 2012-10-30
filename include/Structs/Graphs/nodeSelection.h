#ifndef NODESELECTION_H
#define NODESELECTION_H

#include <Structs/Graphs/dynamicGraph.h>


template<typename GraphType>
class NodeSelection
{
public:

    typedef GraphType                       Graph;
    typedef typename Graph::NodeIterator    NodeIterator;
    typedef typename Graph::NodeDescriptor  NodeDescriptor;
    typedef typename Graph::SizeType        SizeType;


    NodeSelection( Graph* G):m_G(G),m_numNodes(0)
    {
    }

    ~NodeSelection()
    {
    }

	typename std::vector<NodeDescriptor>::iterator begin()
	{
		return m_dense.begin();
	}

    void clear()
    {
        m_numNodes = 0;
    }

    bool empty()
    {
        return m_numNodes == 0;
    }

	typename std::vector<NodeDescriptor>::iterator end()
	{
		return m_dense.end();
	}


    const std::vector<NodeDescriptor>& getMembers()
    {
        return m_dense;
    }

    bool isMember( const NodeDescriptor& uD)
    {
        NodeIterator u = m_G->getNodeIterator(uD);
        return (u->selectionID < m_numNodes) && ( m_dense[u->selectionID] == uD);
    }
    
    bool isMember( const NodeIterator& u)
    {
        NodeDescriptor uD = m_G->getNodeDescriptor(u);
        return (u->selectionID < m_numNodes) && ( m_dense[u->selectionID] == uD);
    }
    
    SizeType size()
    {
        return m_numNodes;
    }
    
    void select( const NodeDescriptor& uD)
    {
        if (isMember(uD)) return;
        assert(!isMember( uD));
        NodeIterator u = m_G->getNodeIterator(uD);
        u->selectionID = m_numNodes;

        if( m_dense.size() > m_numNodes)
        {
            m_dense[m_numNodes] = uD;
        }
        else
        {
            m_dense.push_back(uD);
        }
        ++m_numNodes;
    }
    
    void select( const NodeIterator& u)
    {
        NodeDescriptor uD = m_G->getNodeDescriptor(u);
        if (isMember(uD)) return;
        assert(!isMember( uD));
        u->selectionID = m_numNodes;

        if( m_dense.size() > m_numNodes)
        {
            m_dense[m_numNodes] = uD;
        }
        else
        {
            m_dense.push_back(uD);
        }
        ++m_numNodes;
    }

private:
    GraphType*    m_G;
    std::vector<NodeDescriptor>     m_dense;
    SizeType                        m_numNodes;
};


#endif // NODESELECTION_H
