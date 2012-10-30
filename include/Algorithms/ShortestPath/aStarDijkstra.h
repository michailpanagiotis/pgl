#ifndef ASTARDIJKSTRA_H
#define ASTARDIJKSTRA_H

#include <Structs/Trees/priorityQueue.h>
#include <Utilities/geographic.h>


/**
 * @class AStarDijkstra
 *
 * @brief The A* Variant of Dijkstra's algorithm using cartesian distances
 *
 * This class supports running queries between source and target nodes
 *
 * @tparam GraphType The type of the graph to run the algorithm on
 * @author Panos Michail
 *
 */

template<class GraphType>
class AStarDijkstra
{
public:
    typedef typename GraphType::NodeIterator                        NodeIterator;
    typedef typename GraphType::NodeDescriptor                      NodeDescriptor;
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
    AStarDijkstra( GraphType& graph, unsigned int* timestamp):G(graph),m_timestamp(timestamp),m_maxSpeed(0)
    {
		NodeIterator u,v,lastNode;
        EdgeIterator e,lastEdge;
		for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
        {
			for( e = G.beginEdges(u), lastEdge = G.endEdges(u); e != lastEdge; ++e)
            {
				v = G.target(e);
				double speed = euclideanDistance( u->x, u->y, v->x, v->y)/e->weight;
				if( speed > m_maxSpeed)
				{
					m_maxSpeed = speed;
				}	
			}
		}
		//m_maxSpeed *= 1.1;
		std::cout << "Max speed = " << m_maxSpeed << "\n"; 
    }
    
    /**
     * @brief Checks if the graph has feasible potentials
     *
     * @param t The target node
     * @return True if the potentials are feasible, false otherwise
     */
    bool hasFeasiblePotentials( const typename GraphType::NodeIterator& t)
    {
        NodeIterator u,v,lastNode;
        EdgeIterator e,lastEdge;

        WeightType potential_u, potential_v;
        int reducedCost;

        for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
        {
            for( e = G.beginEdges(u), lastEdge = G.endEdges(u); e != lastEdge; ++e)
            {
                v = G.target(e);
                potential_u = getHeuristic( u, t);
                potential_v = getHeuristic( v, t);
                reducedCost = e->weight - potential_u + potential_v;
                if( reducedCost < 0)
                {
					std::cout << "(" << G.getRelativePosition(u) << "," << G.getRelativePosition(v) << ") -> " << G.getRelativePosition(t) << "\n";
					std::cout << "p(u)  = " << potential_u << "\n";
					std::cout << "p(v)  = " << potential_v << "\n";
					std::cout << "wt(e) = " << e->weight << "\n";
					std::cout << "r(e)  = " << reducedCost << "\n";
                    return false;
                }
            }       
        }
        return true;
    }
    
	unsigned int getHeuristic( const NodeIterator& u, const NodeIterator& t)
	{
		return euclideanDistance( u->x, u->y, t->x, t->y)/m_maxSpeed;
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
        
        WeightType potential_u, potential_v;
        WeightType reducedCost;
        
        assert(hasFeasiblePotentials(t));
        
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
            if( u == t) break;
            
            potential_u = getHeuristic( u, t);
            for( e = G.beginEdges(u), lastEdge = G.endEdges(u); e != lastEdge; ++e)
            {
                v = G.target(e);
                potential_v = getHeuristic( v, t);
                reducedCost = e->weight + potential_v - potential_u; 
                
                if( v->timestamp < (*m_timestamp))
                {
                    v->pred = u->getDescriptor();
                    v->timestamp = (*m_timestamp);
                    
                    pq.insert( u->dist + reducedCost, v, &(v->pqitem));
                    v->dist = u->dist + reducedCost;
                }
                else if( v->dist > u->dist + reducedCost )
                {
                    v->pred = u->getDescriptor();
                    
                    pq.decrease( u->dist + reducedCost, &(v->pqitem));
                    v->dist = u->dist + reducedCost;
                }
            }
        }

        u = t;
        t->dist = 0;
        while( u->pred != G.nilNodeDescriptor())
        {
            v = G.getNodeIterator(u->pred);
            e = G.getEdgeIterator( v , u);
            t->dist += e->weight;
            u = v;
        }
        return t->dist;
    }
    
private:
    GraphType& G;
    unsigned int* m_timestamp;
    PriorityQueueType pq;
    unsigned int m_settled;
	double m_maxSpeed;
};

#endif//ASTARDIJKSTRA_H

