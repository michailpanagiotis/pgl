#ifndef NODEARRAY_H
#define NODEARRAY_H

#include <Structs/Graphs/dynamicGraph.h>
#include <Structs/Maps/pmMap.h>


template< typename dataType, typename GraphType>
class NodeArray
{
public:

    typedef GraphType                       Graph;
    typedef typename Graph::NodeIterator    node;
    typedef typename Graph::SizeType        sizeType;

    NodeArray():m_G(0)
    {
    }

    NodeArray( const Graph* G, dataType data = dataType()):m_G(G)
    {
        init(G, data);
    }

    ~NodeArray()
    {
    }

    void init( const Graph* G, dataType data = dataType())
    {
        m_map.clear();
        m_G = G;
        
        node u, end;
        for( u = G->beginNodes(), end = G->endNodes(); u != end; ++u)
        {
            m_map[u->getDescriptor()] = data;
        }
    }

    dataType& operator[] ( const node& u )
    {
        return  m_map[ u->getDescriptor()];
    }
	
	const dataType& operator[] ( const node& u ) const
    {
		return  m_map[ u->getDescriptor()];
    }

private:
    const GraphType*                            m_G;
    PMMap< typename GraphType::NodeDescriptor, dataType> m_map;
};



#endif //NODEARRAY_H
