#ifndef EDGEARRAY_H
#define EDGEARRAY_H

#include <Structs/Graphs/dynamicGraph.h>
#include <Structs/Maps/pmMap.h>

template< typename dataType, typename GraphType>
class EdgeArray
{
public:

    typedef GraphType                       Graph;
    typedef typename Graph::NodeIterator    node;
    typedef typename Graph::EdgeIterator    edge;
    typedef typename Graph::SizeType        sizeType;

    EdgeArray():m_G(0)
    {
    }

    EdgeArray( const Graph* G, dataType data = dataType()):m_G(G)
    {
        init(G, data);
    }

    ~EdgeArray()
    {
    }

    void init( const Graph* G, dataType data = dataType())
    {
        m_map.clear();
        m_G = G;
        
        node u, lastNode;
        edge e, lastEdge;
        for( u = G->beginNodes(), lastNode = G->endNodes(); u != lastNode; ++u)
        {
            for( e = G->beginNodes(), lastEdge = G->endNodes(); e != lastEdge; ++e)
            {   
                m_map[e->getDescriptor()] = data;
            }
        }
    }

    dataType& operator[] ( const edge& e )
    {
        return  m_map[ e->getDescriptor()];
    }
	
	const dataType& operator[] ( const edge& e ) const
    {
		return  m_map[ e->getDescriptor()];
    }

private:
    const GraphType*                                        m_G;
    PMMap< typename GraphType::EdgeDescriptor, dataType>    m_map;
};



#endif //EDGEARRAY_H
