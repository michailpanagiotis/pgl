#ifndef BIDIRECTIONALDIJKSTRA_H
#define BIDIRECTIONALDIJKSTRA_H

#include <Structs/Trees/priorityQueue.h>


/**
 * @class BidirectionalDijkstra
 *
 * @brief Bidirectional Dijkstra algorithm implementation
 *
 * This class supports running queries between source and target nodes by building two search trees, one starting at the source node and one ending at the target node.
 *
 * @tparam GraphType The type of the graph to run the algorithm on
 * @author Panos Michail
 *
 */
template<class GraphType>
class BidirectionalDijkstra
{
public:
    typedef typename GraphType::NodeIterator                        NodeIterator;
    typedef typename GraphType::NodeDescriptor                      NodeDescriptor;
    typedef typename GraphType::EdgeIterator                        EdgeIterator;
    typedef typename GraphType::InEdgeIterator                    InEdgeIterator;
    typedef typename GraphType::SizeType                            SizeType;
    typedef unsigned int                                            WeightType;
    typedef PriorityQueue< WeightType, NodeIterator, HeapStorage>   PriorityQueueType;
    
    /**
     * @brief Constructor
     *
     * @param graph The graph to run the algorithm on
     * @param timestamp An address containing a timestamp. A timestamp must be given in order to check whether a node is visited or not
     */
    BidirectionalDijkstra( GraphType& graph, unsigned int* timestamp):G(graph),m_timestamp(timestamp)
    {
    }
    
    const unsigned int& getSettledNodes()
    {
        return m_settled;
    }

    /**
     * @brief Runs a shortest path query between a source node s and a target node t
     *
     * @param s The source node
     * @param t The target node
     * @return The distance of the target node
     */
    WeightType runQuery( const typename GraphType::NodeIterator& s, const typename GraphType::NodeIterator& t)
    {
        NodeIterator u,v,lastNode;
        EdgeIterator e,lastEdge;
        
        pqFront.clear();
        pqBack.clear();
        ++(*m_timestamp);

        minDistance = std::numeric_limits<WeightType>::max();

        m_settled = 2;

        s->dist = 0;
        s->distBack = std::numeric_limits<WeightType>::max();//<// 
        s->timestamp = (*m_timestamp);
        s->pred = G.nilNodeDescriptor();

        t->dist = std::numeric_limits<WeightType>::max(); //<// 
        t->distBack = 0;
        t->timestamp = (*m_timestamp);
        t->succ = G.nilNodeDescriptor();

        pqFront.insert( s->dist, s, &(s->pqitem));
        pqBack.insert( t->distBack, t, &(t->pqitemBack));

        while( ! ( pqFront.empty() && pqBack.empty()))
        {
            curMin = 0;
            if( !pqFront.empty()) curMin += pqFront.minKey();
            if( !pqBack.empty()) curMin += pqBack.minKey();
            if( curMin > minDistance)
            {
                break;
            }
            searchForward();
            searchBackward();
        }
        
        u = viaNode;
        t->dist = u->dist;
        while( u->succ != G.nilNodeDescriptor())
        {
            v = G.getNodeIterator(u->succ);
            v->pred = G.getNodeDescriptor( u);
            e = G.getEdgeIterator( u, v);
            t->dist += e->weight;
            u = v;
        }

        return t->dist;
    }   
    
private:
    GraphType& G;
    unsigned int* m_timestamp;
    PriorityQueueType pqFront, pqBack;
    NodeIterator viaNode;
    WeightType curMin, minDistance;
    unsigned int m_settled;    

    bool isBackwardFound( const NodeIterator& u)
    {
        return (u->timestamp == (*m_timestamp)) && (u->distBack != std::numeric_limits<WeightType>::max());
    }
    
    bool isBackwardSettled( const NodeIterator& u)
    {
        return isBackwardFound(u) && (!isInBackQueue(u));
    }
    
    bool isForwardFound( const NodeIterator& u)
    {
        return (u->timestamp == (*m_timestamp)) && (u->dist != std::numeric_limits<WeightType>::max());
    }
    
    bool isForwardSettled( const NodeIterator& u)
    {
        return isForwardFound(u) && (!isInFrontQueue(u));
    }
    
    bool isInBackQueue( const NodeIterator& u)
    {
        return pqBack.contains( &(u->pqitemBack));
    }
    
    bool isInFrontQueue( const NodeIterator& u)
    {
        return pqFront.contains( &(u->pqitem));
    }
    
    void searchForward()
    {
        EdgeIterator e,lastEdge;
        NodeIterator u,v;
        if( !pqFront.empty())
        {
            u = pqFront.minItem();
            pqFront.popMin();
            ++m_settled;
            for( e = G.beginEdges(u), lastEdge = G.endEdges(u); e != lastEdge; ++e)
            {
                v = G.target(e);

                if( v->timestamp < (*m_timestamp))
                {
                    v->pred = u->getDescriptor();
                    v->dist = u->dist + e->weight;
                    v->timestamp = (*m_timestamp);
                    v->distBack = std::numeric_limits<WeightType>::max();
                    pqFront.insert( v->dist, v, &(v->pqitem));
                }
                else if( v->dist == std::numeric_limits<WeightType>::max())
                {
                    v->pred = u->getDescriptor();
                    v->dist = u->dist + e->weight;
                    pqFront.insert( v->dist, v, &(v->pqitem));
                }
                else if( v->dist > u->dist + e->weight )
                {
                    v->pred = u->getDescriptor();
                    v->dist = u->dist + e->weight;
                    pqFront.decrease( v->dist, &(v->pqitem));
                }

                
                if( isBackwardFound(v) && ( u->dist + e->weight + v->distBack < minDistance))
                {
                    minDistance = u->dist +e->weight + v->distBack;
                    //std::cout << "Settled " << G.getId(v) << " from front with " << minDistance << " (" << u->dist << "+" << v->distBack << ")!\n";
                    viaNode = v;
                }
            }
        }
    }
    
    void searchBackward()
    {
        InEdgeIterator k,lastInEdge;
        NodeIterator u,v;
        if( !pqBack.empty())
        {
            u = pqBack.minItem();
            pqBack.popMin();
            ++m_settled;
            for( k = G.beginInEdges(u), lastInEdge = G.endInEdges(u); k != lastInEdge; ++k)
            {
                v = G.source(k);

                if( v->timestamp < (*m_timestamp))
                {
                    v->succ = u->getDescriptor();
                    v->distBack = u->distBack + k->weight;
                    v->timestamp = (*m_timestamp);
                    v->dist = std::numeric_limits<WeightType>::max();
                    pqBack.insert( v->distBack, v, &(v->pqitemBack));
                }
                else if( v->distBack == std::numeric_limits<WeightType>::max())
                {
                    v->succ = u->getDescriptor();
                    v->distBack = u->distBack + k->weight;
                    pqBack.insert( v->distBack, v, &(v->pqitemBack));
                }
                else if( v->distBack > u->distBack + k->weight )
                {
                    v->succ = u->getDescriptor();
                    v->distBack = u->distBack + k->weight;
                    pqBack.decrease( v->distBack, &(v->pqitemBack));
                }

                if( isForwardFound(v) && ( v->dist + k->weight + u->distBack < minDistance))
                {
                    minDistance = v->dist +k->weight + u->distBack;
                    //std::cout << "Settled " << G.getId(v) << " from back with " << minDistance << " (" << v->dist << "+" << u->distBack << ")!\n";
                    viaNode = v;
                }
            }
        }
    }
};

#endif//BIDIRECTIONALDIJKSTRA_H

