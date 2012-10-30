#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <Structs/Trees/priorityQueue.h>
#include <Structs/Graphs/nodeSelection.h>

/**
 * @class Dijkstra
 *
 * @brief Plain Dijkstra algorithm implementation
 *
 * This class supports building a full shortest path tree from a source node s, or running queries between source and target nodes
 *
 * @tparam GraphType The type of the graph to run the algorithm on
 * @author Panos Michail
 *
 */

template<class GraphType>
class Dijkstra
{
public:
    typedef typename GraphType::NodeIterator                        NodeIterator;
    typedef typename GraphType::EdgeIterator                        EdgeIterator;
    typedef typename GraphType::SizeType                            SizeType;
    typedef unsigned int                                            WeightType;
    typedef PriorityQueue< WeightType, NodeIterator, HeapStorage>   PriorityQueueType;
    
    /**
     * @brief Constructor
     *
     * @param graph The graph to run the algorithm on
     * @param timestamp An address containing a timestamp. A timestamp must be given in order to check whether a node is visited or not
     */
    Dijkstra( GraphType& graph, unsigned int* timestamp):G(graph),m_timestamp(timestamp)
    {
    }
    
    /**
     * @brief Builds a shortest path tree routed on a source node
     *
     * @param s The source node
     */
    void buildTree( const typename GraphType::NodeIterator& s)
    {
        NodeIterator u,v,lastNode;
        EdgeIterator e,lastEdge;
        
        pq.clear();
        m_settled = 1;
        ++(*m_timestamp);
        s->dist = 0;
        s->timestamp = (*m_timestamp);
        s->pred = G.nilNodeDescriptor();

        pq.insert( s->dist, s, &(s->pqitem));

        while( !pq.empty())
        {
            u = pq.minItem();
            pq.popMin();
            ++m_settled;
    
            for( e = G.beginEdges(u), lastEdge = G.endEdges(u); e != lastEdge; ++e)
            {
                v = G.target(e);

                if( v->timestamp < (*m_timestamp))
                {
                    v->pred = u->getDescriptor();
                    v->dist = u->dist + e->weight;
                    v->timestamp = (*m_timestamp);
                    pq.insert( v->dist, v, &(v->pqitem));
                }
                else if( v->dist > u->dist + e->weight )
                {
                    v->pred = u->getDescriptor();
                    v->dist = u->dist + e->weight;
                    pq.decrease( v->dist, &(v->pqitem));
                }
            }
        }
    }

    void buildSubTree( const typename GraphType::NodeIterator& s, NodeSelection<GraphType>& targets)
    {
        NodeIterator u,v,lastNode;
        EdgeIterator e,lastEdge;
        
        pq.clear();
        m_settled = 1;
        ++(*m_timestamp);
        s->dist = 0;
        s->timestamp = (*m_timestamp);
        s->pred = G.nilNodeDescriptor();

        pq.insert( s->dist, s, &(s->pqitem));

        unsigned int numTargets = targets.size();

        while( !pq.empty())
        {
            u = pq.minItem();
            pq.popMin();
            ++m_settled;

            if( targets.isMember(u))
            {
                --numTargets;    
                if( !numTargets) break;
            }
            
            for( e = G.beginEdges(u), lastEdge = G.endEdges(u); e != lastEdge; ++e)
            {
                v = G.target(e);

                if( v->timestamp < (*m_timestamp))
                {
                    v->pred = u->getDescriptor();
                    v->dist = u->dist + e->weight;
                    v->timestamp = (*m_timestamp);
                    pq.insert( v->dist, v, &(v->pqitem));
                }
                else if( v->dist > u->dist + e->weight )
                {
                    v->pred = u->getDescriptor();
                    v->dist = u->dist + e->weight;
                    pq.decrease( v->dist, &(v->pqitem));
                }
            }
        }
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
        
        _MEMSTATS( 
            MemTransfersRecorder nodeRecorder( nodeMemTransfersFile);
            MemTransfersRecorder edgeRecorder( edgeMemTransfersFile);            
        )
        pq.clear();
        m_settled = 1;
        ++(*m_timestamp);
        s->dist = 0;
        s->timestamp = (*m_timestamp);
        s->pred = G.nilNodeDescriptor();;

        pq.insert( s->dist, s, &(s->pqitem));

        while( !pq.empty())
        {
            u = pq.minItem();
            pq.popMin();
            ++m_settled;
            if( u == t) break;
            for( e = G.beginEdges(u), lastEdge = G.endEdges(u); e != lastEdge; ++e)
            {
                v = G.target(e);

                if( v->timestamp < (*m_timestamp))
                {
                    _MEMSTATS( 
                        nodeRecorder.recordJumpAt( (char*)&(*v));
                        edgeRecorder.recordJumpAt( (char*)&(*e));            
                    )
                    v->pred = u->getDescriptor();
                    v->dist = u->dist + e->weight;
                    v->timestamp = (*m_timestamp);
                    pq.insert( v->dist, v, &(v->pqitem));
                }
                else if( v->dist > u->dist + e->weight )
                {
                    _MEMSTATS( 
                        nodeRecorder.recordJumpAt( (char*)&(*v));
                        edgeRecorder.recordJumpAt( (char*)&(*e));            
                    )
                    v->pred = u->getDescriptor();
                    v->dist = u->dist + e->weight;
                    pq.decrease( v->dist, &(v->pqitem));
                }
            }
        }
        return t->dist;
    }

private:
    GraphType& G;
    unsigned int* m_timestamp;
    PriorityQueueType pq;
    unsigned int m_settled;
};



/**
 * @class BackwardDijkstra
 *
 * @brief Plain Dijkstra algorithm backwards implementation
 *
 * This class supports building a full shortest path tree towards a target node t, or running queries between source and target nodes
 *
 * @tparam GraphType The type of the graph to run the algorithm on
 * @author Panos Michail
 *
 */

template<class GraphType>
class BackwardDijkstra
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
    BackwardDijkstra( GraphType& graph, unsigned int* timestamp):G(graph),m_timestamp(timestamp)
    {
    }
    
    /**
     * @brief Builds a backwards shortest path tree routed on a target node
     *
     * @param t The target node
     */
    void buildTree( const typename GraphType::NodeIterator& t)
    {
        NodeIterator u,v,lastNode;
        InEdgeIterator k,lastInEdge;
        
        pqBack.clear();
        ++(*m_timestamp);
        t->distBack = 0;
        t->timestamp = (*m_timestamp);
        t->succ = G.nilNodeDescriptor();;

        pqBack.insert( t->distBack, t, &(t->pqitemBack));

        while( !pqBack.empty())
        {
            u = pqBack.minItem();
            pqBack.popMin();
            for( k = G.beginInEdges(u), lastInEdge = G.endInEdges(u); k != lastInEdge; ++k)
            {
                v = G.source(k);

                if( v->timestamp < (*m_timestamp))
                {
                    v->succ = u->getDescriptor();
                    v->distBack = u->distBack + k->weight;
                    v->timestamp = (*m_timestamp);
                    pqBack.insert( v->distBack, v, &(v->pqitemBack));
                }
                else if( v->distBack > u->distBack + k->weight )
                {
                    v->succ = u->getDescriptor();
                    v->distBack = u->distBack + k->weight;
                    pqBack.decrease( v->distBack, &(v->pqitemBack));
                }
            }
        }
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
        InEdgeIterator k,lastInEdge;
        
        pqBack.clear();
        ++(*m_timestamp);
        t->distBack = 0;
        t->timestamp = (*m_timestamp);
        t->succ = G.nilNodeDescriptor();;

        pqBack.insert( t->distBack, t, &(t->pqitemBack));

        while( !pqBack.empty())
        {
            u = pqBack.minItem();
            pqBack.popMin();
            if( u == s) break;
            for( k = G.beginInEdges(u), lastInEdge = G.endInEdges(u); k != lastInEdge; ++k)
            {
                v = G.source(k);

                if( v->timestamp < (*m_timestamp))
                {
                    v->succ = u->getDescriptor();
                    v->distBack = u->distBack + k->weight;
                    v->timestamp = (*m_timestamp);
                    pqBack.insert( v->distBack, v, &(v->pqitemBack));
                }
                else if( v->distBack > u->distBack + k->weight )
                {
                    v->succ = u->getDescriptor();
                    v->distBack = u->distBack + k->weight;
                    pqBack.decrease( v->distBack, &(v->pqitemBack));
                }
            }
        }
        
        u = s;
        t->dist = 0;
        EdgeIterator e;
        while( u->succ != G.nilNodeDescriptor())
        {
            v = G.getNodeIterator( u->succ);
            e = G.getEdgeIterator( u, v);
            t->dist += e->weight;
            v->pred = G.getNodeDescriptor(u);
            u = v;
        }
        return t->dist;
    }
    
private:
    GraphType& G;
    unsigned int* m_timestamp;
    PriorityQueueType pqBack;
};

#endif//DIJKSTRA_H

