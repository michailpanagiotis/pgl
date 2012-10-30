#ifndef NAMOASTAR_H
#define NAMOASTAR_H

#include <Structs/Trees/priorityQueue.h>
#include <Utilities/geographic.h>
#include <Algorithms/ShortestPath/dijkstra.h>

template<class GraphType>
class GreatCircleHeuristic
{
public:
    
    typedef typename GraphType::NodeIterator    NodeIterator;
	typedef typename GraphType::EdgeIterator    EdgeIterator;
    GreatCircleHeuristic( GraphType& graph):G(graph)
    {
        m_maxSpeed = 0;
	    NodeIterator u,v,lastnode;
	    EdgeIterator e,lastedge;	
	    for( u = G.beginNodes(), lastnode = G.endNodes(); u != lastnode; ++u)
	    {
	   	    for( e = G.beginEdges(u), lastedge = G.endEdges(u); e != lastedge; ++e)
		    {
	            if( double( e->criteriaList[0])/double( e->criteriaList[1]) > m_maxSpeed)
			    {
				    m_maxSpeed = double( e->criteriaList[0])/double( e->criteriaList[1]);
			    }
		    }
	    }
    } 
    
    void init(const NodeIterator& s,const NodeIterator& t)
	{
		NodeIterator u, lastNode;
		for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
		{
			u->heuristicList[0] = greatCircle( double(u->x)/100000, double(u->y)/100000, double(t->x)/100000, double(t->y)/100000);
			u->heuristicList[1] = double(u->heuristicList[0])/ (m_maxSpeed);	
		}
	}
    
private:
    GraphType& G;
    double m_maxSpeed;
};



template<class GraphType>
class BlindHeuristic
{
public:
    
    typedef typename GraphType::NodeIterator    NodeIterator;
	typedef typename GraphType::EdgeIterator    EdgeIterator;
    BlindHeuristic( GraphType& graph):G(graph)
    {
    } 
    
    void init(const NodeIterator& s,const NodeIterator& t)
	{
		NodeIterator u, lastNode;
		for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
		{
			u->heuristicList[0] = 0;
			u->heuristicList[1] = 0;	
		}
	}
    
private:
    GraphType& G;
};


template<class GraphType>
class TCHeuristic
{
public:
    
    typedef typename GraphType::NodeIterator    NodeIterator;
	typedef typename GraphType::EdgeIterator    EdgeIterator;
	typedef typename GraphType::InEdgeIterator  InEdgeIterator;
	typedef PriorityQueue< unsigned int, NodeIterator, HeapStorage>   PriorityQueueType;
	
    TCHeuristic( GraphType& graph):G(graph)
    {
        m_timestamp = new unsigned int();
        *m_timestamp = 0;
    } 
    
    ~TCHeuristic()
    {
        delete m_timestamp;
    }

    void init( const typename GraphType::NodeIterator& s, const typename GraphType::NodeIterator& t)
    {
        NodeIterator u,v,lastNode;
        *m_timestamp = 1;
        unsigned int EMPTY_HEURISTIC = std::numeric_limits<unsigned int>::max();
        for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
		{
			u->heuristicList[0] = EMPTY_HEURISTIC;
			u->heuristicList[1] = EMPTY_HEURISTIC;	
            u->timestamp = 0;
		}
        buildTree(t,0);
        *m_timestamp = 2;
        buildTree(t,1);
    }

    void buildTree( const typename GraphType::NodeIterator& t, unsigned int criterionIndex)
    {
        NodeIterator u,v,lastNode;
        InEdgeIterator k,lastInEdge;
        
        pq.clear();
        ++(*m_timestamp);
        t->heuristicList[criterionIndex] = 0;
        t->timestamp = (*m_timestamp);
        t->succ = G.nilNodeDescriptor();;

        pq.insert( t->heuristicList[criterionIndex], t, &(t->pqitem));

        while( !pq.empty())
        {
            u = pq.minItem();
            pq.popMin();
            for( k = G.beginInEdges(u), lastInEdge = G.endInEdges(u); k != lastInEdge; ++k)
            {
                v = G.source(k);

                if( v->timestamp < (*m_timestamp))
                {
                    v->succ = u->getDescriptor();
                    v->heuristicList[criterionIndex] = u->heuristicList[criterionIndex] + k->criteriaList[criterionIndex];
                    v->timestamp = (*m_timestamp);
                    pq.insert( v->heuristicList[criterionIndex], v, &(v->pqitem));
                }
                else if( v->heuristicList[criterionIndex] > u->heuristicList[criterionIndex] + k->criteriaList[criterionIndex] )
                {
                    v->succ = u->getDescriptor();
                    v->heuristicList[criterionIndex] = u->heuristicList[criterionIndex] + k->criteriaList[criterionIndex];
                    pq.decrease( v->heuristicList[criterionIndex], &(v->pqitem));
                }
            }
        }
    }    
    
private:
    GraphType& G;
    PriorityQueueType pq;
    PriorityQueueType secondary_pq;
    unsigned int* m_timestamp;
};



template<class GraphType>
class BoundedTCHeuristic
{
public:
    
    typedef typename GraphType::NodeIterator    NodeIterator;
	typedef typename GraphType::EdgeIterator    EdgeIterator;
	typedef typename GraphType::InEdgeIterator  InEdgeIterator;
	typedef PriorityQueue< unsigned int, NodeIterator, HeapStorage>   PriorityQueueType;
	
    BoundedTCHeuristic( GraphType& graph):G(graph)
    {
        m_timestamp = new unsigned int();
        *m_timestamp = 0;
    } 
    
    ~BoundedTCHeuristic()
    {
        delete m_timestamp;
    }
     
    
    void buildTree( const typename GraphType::NodeIterator& t, unsigned int bound)
    {
        NodeIterator u,v,lastNode;
        InEdgeIterator k,lastInEdge;
        
        secondary_pq.clear();
        t->heuristicList[1] = 0;
        t->marked = true;
        t->succ = G.nilNodeDescriptor();;
        t->dist = 0;
        secondary_pq.insert( t->heuristicList[1], t, &(t->secondary_pqitem));

        while( !secondary_pq.empty())
        {
            if( secondary_pq.minKey() > bound) break;
            u = secondary_pq.minItem();
            secondary_pq.popMin();
            for( k = G.beginInEdges(u), lastInEdge = G.endInEdges(u); k != lastInEdge; ++k)
            {
                v = G.source(k);

                if( !v->marked)
                {
                    v->succ = u->getDescriptor();
                    v->heuristicList[1] = u->heuristicList[1] + k->criteriaList[1];
                    v->dist = u->dist + k->criteriaList[0];
                    v->marked = true;
                    secondary_pq.insert( v->heuristicList[1], v, &(v->secondary_pqitem));
                }
                else if( v->heuristicList[1] > u->heuristicList[1] + k->criteriaList[1] )
                {
                    v->succ = u->getDescriptor();
                    v->heuristicList[1] = u->heuristicList[1] + k->criteriaList[1];
                    v->dist = u->dist + k->criteriaList[0];
                    secondary_pq.decrease( v->heuristicList[1], &(v->secondary_pqitem));
                }
            }
        }
    }    

    void buildSingleTree( const typename GraphType::NodeIterator& t, unsigned int criterionIndex, unsigned int bound)
    {
        NodeIterator u,v,lastNode;
        InEdgeIterator k,lastInEdge;
        
        while( !pq.empty())
        {
            if( pq.minKey() > bound) break;
            u = pq.minItem();
            pq.popMin();
            for( k = G.beginInEdges(u), lastInEdge = G.endInEdges(u); k != lastInEdge; ++k)
            {
                v = G.source(k);

                if( v->timestamp < (*m_timestamp))
                {
                    v->succ = u->getDescriptor();
                    v->heuristicList[criterionIndex] = u->heuristicList[criterionIndex] + k->criteriaList[criterionIndex];
                    v->timestamp = (*m_timestamp);
                    pq.insert( v->heuristicList[criterionIndex], v, &(v->pqitem));
                }
                else if( v->heuristicList[criterionIndex] > u->heuristicList[criterionIndex] + k->criteriaList[criterionIndex] )
                {
                    v->succ = u->getDescriptor();
                    v->heuristicList[criterionIndex] = u->heuristicList[criterionIndex] + k->criteriaList[criterionIndex];
                    pq.decrease( v->heuristicList[criterionIndex], &(v->pqitem));
                }
            }
        }
    }   

    void init( const typename GraphType::NodeIterator& s, const typename GraphType::NodeIterator& t)
    {
        NodeIterator u,v,lastNode;
        *m_timestamp = 1;
        unsigned int EMPTY_HEURISTIC = std::numeric_limits<unsigned int>::max();
        for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
		{
			u->heuristicList[0] = EMPTY_HEURISTIC;
			u->heuristicList[1] = EMPTY_HEURISTIC;	
            u->timestamp = 0;
            u->marked = false;
		}

        runQuery( s, t, 0);
        unsigned int bound1 = s->heuristicList[1];

        buildTree(t,bound1);

        unsigned int bound0 = s->dist;
        buildSingleTree(t,0,bound0);
        
    }

    void runQuery( const typename GraphType::NodeIterator& s, const typename GraphType::NodeIterator& t, unsigned int criterionIndex)
    {
        NodeIterator u,v,lastNode;
        InEdgeIterator k,lastInEdge;
        
        pq.clear();
        ++(*m_timestamp);
        t->heuristicList[criterionIndex] = 0;
        t->timestamp = (*m_timestamp);
        t->succ = G.nilNodeDescriptor();;

        pq.insert( t->heuristicList[criterionIndex], t, &(t->pqitem));

        while( !pq.empty())
        {
            u = pq.minItem();
            if( u == s) break;

            pq.popMin();
            for( k = G.beginInEdges(u), lastInEdge = G.endInEdges(u); k != lastInEdge; ++k)
            {
                v = G.source(k);

                if( v->timestamp < (*m_timestamp))
                {
                    v->succ = u->getDescriptor();
                    v->heuristicList = u->heuristicList + k->criteriaList;
                    v->timestamp = (*m_timestamp);
                    pq.insert( v->heuristicList[criterionIndex], v, &(v->pqitem));
                }
                else if( v->heuristicList[criterionIndex] > u->heuristicList[criterionIndex] + k->criteriaList[criterionIndex] )
                {
                    v->succ = u->getDescriptor();
                    v->heuristicList = u->heuristicList + k->criteriaList;
                    pq.decrease( v->heuristicList[criterionIndex], &(v->pqitem));
                }
            }
        }
    } 
    
private:
    GraphType& G;
    PriorityQueueType pq;
    PriorityQueueType secondary_pq;
    unsigned int* m_timestamp;
};


template<class GraphType, template <typename graphType> class HeuristicType>
class NamoaStarDijkstra
{
public:
	typedef typename GraphType::NodeIterator    NodeIterator;
	typedef typename GraphType::EdgeIterator    EdgeIterator;
	typedef typename GraphType::SizeType        SizeType;
	typedef typename GraphType::NodeData        NodeData;

	typedef PriorityQueue< CriteriaList, NodeIterator, HeapStorage> PriorityQueueType;
	typedef typename PriorityQueueType::PQItem PQItem;   
	
    /**
     * @brief Constructor
     *
     * @param graph The graph to run the algorithm on
     * @param timestamp An address containing a timestamp. A timestamp must be given in order to check whether a node is visited or not
     */
    NamoaStarDijkstra( GraphType& graph, unsigned int numCriteria, unsigned int* timestamp):G(graph),m_numCriteria(numCriteria),m_timestamp(timestamp),m_heuristicEngine(graph)
    {
    }
    
	void init(const NodeIterator& s, const NodeIterator& t)
	{
		NodeIterator u, lastNode;
		for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
		{
		    u->labels.clear();
		}
        m_heuristicEngine.init(s,t);
		pq.clear();
	}

    /**
     * @brief Builds a shortest path tree routed on a source node
     *
     * @param s The source node
     */
    void runQuery( const typename GraphType::NodeIterator& s, const typename GraphType::NodeIterator& t)
    {
		NodeIterator u,v,lastNode;
		EdgeIterator e,lastEdge;

        m_generatedLabels = 1;
		assert( hasFeasiblePotentials(t));
		++(*m_timestamp);
		

		unsigned int* pqitem = new unsigned int();
		s->labels.push_back(Label( CriteriaList(m_numCriteria), 0, pqitem));
		pq.insert( CriteriaList(m_numCriteria) + s->heuristicList, s, pqitem);

		while( !pq.empty())
		{
		    CriteriaList minCriteria = pq.min().key;
		    u = pq.minItem();
		    pq.popMin();

			CriteriaList g_u = minCriteria - u->heuristicList;
			
			if( u == t)
            {
                eraseAllDominatedLabels( G, t, g_u);
            }
			
			moveToClosed( g_u, u);

			if ( isDominatedByNodeLabels( t, minCriteria)) continue;

		    //std::cout << "extracting " << u->id << " with label ";
		    //label.print(std::cout, G);
		    //std::cout << std::endl; 

		  	for( e = G.beginEdges(u), lastEdge = G.endEdges(u); e != lastEdge; ++e)
		    {
				v = G.target(e);
				
				if ( v->timestamp != (*m_timestamp))
				{
					v->labels.clear();
					v->timestamp = (*m_timestamp);
				}
				
				CriteriaList g_v = g_u + e->criteriaList;
				CriteriaList heuristicCost = g_v + v->heuristicList;

				if ( distanceExistsInNode( v, g_v))
				{
					v->labels.push_back( Label( g_v, u->getDescriptor(), 0) );
                    ++m_generatedLabels;
				}
				else	
				{
					if( isDominatedByNodeLabels( v, g_v)) continue;
					eraseDominatedLabels( G, v, g_v);
					if( isDominatedByNodeLabels(t, heuristicCost)) continue;
					unsigned int* pqitem = new unsigned int();
					v->labels.push_back( Label( g_v, u->getDescriptor(), pqitem) );
				    ++m_generatedLabels;
					pq.insert( heuristicCost, v, pqitem);
				}
		    }
		}
    }

    const unsigned int& getGeneratedLabels()
    {
        return m_generatedLabels;
    }
    
    /**
     * @brief Runs a shortest path query between a source node s and a target node t
     *
     * @param s The source node
     * @param t The target node
     * @return The distance of the target node
     *
    WeightType runQuery( const typename GraphType::NodeIterator& s, const typename GraphType::NodeIterator& t)
    {

    }*/

private:
    GraphType& G;
    PriorityQueueType pq;
    unsigned int m_generatedLabels;
	unsigned int m_numCriteria;
	unsigned int* m_timestamp;
    HeuristicType<GraphType> m_heuristicEngine;

	bool distanceExistsInNode( const NodeIterator& v, const CriteriaList& g_v)
	{
		for ( std::vector<Label>::iterator it = v->labels.begin(); it != v->labels.end(); ++it)
		{
			if ( it->getCriteriaList() == g_v )
		    {
				return true;
		    }
		}
		return false;		
	}

	void eraseDominatedLabels( GraphType& G, const NodeIterator& v, const CriteriaList& g_v)
	{
        EdgeIterator e, lastEdge;
        NodeIterator w;
		std::vector<Label>::iterator it = v->labels.begin();
		while ( it != v->labels.end() )
		{
			if ( it->getCriteriaList().isDominatedBy(g_v) )
		    {
				if( it->isInQueue())
				{
					pq.remove( it->getPQitem());
					moveToClosed( it->getCriteriaList(), v);
				}
				else
				{
                    // GO FORWARD IN SEARCH SPACE TO FIND MORE DOMINATED LABELS
				    for( e = G.beginEdges(v), lastEdge = G.endEdges(v); e != lastEdge; ++e)
		            {
                        //std::cout << "Reducing search space...\n";
				        w = G.target(e);
                        eraseDominatedLabels( G, w, g_v + e->criteriaList);
			        }
			    }
				it = v->labels.erase( it );
		    }
		    else 
		    {
			    ++it;
		    }
		}
	}
	
	void eraseAllDominatedLabels(GraphType& G, const NodeIterator& t, const CriteriaList& g_v)
	{
        NodeIterator u, lastNode;
        for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
        {
            if( u->labels.empty()) continue;
            if( u == t) continue;
            eraseDominatedLabels( G, u, g_v);
        }
    }


    bool hasFeasiblePotentials( const typename GraphType::NodeIterator& t)
    {
        NodeIterator u,v,lastNode;
        EdgeIterator e,lastEdge;

        for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
        {
            for( e = G.beginEdges(u), lastEdge = G.endEdges(u); e != lastEdge; ++e)
            {
                v = G.target(e);
				CriteriaList c_e = e->criteriaList;
				CriteriaList c_v = v->heuristicList;
				CriteriaList c_u = u->heuristicList;
                if( e->criteriaList + v->heuristicList < u->heuristicList)
                {
                    return false;
                }
            }       
        }
        return true;
    }

	

	bool isDominatedByNodeLabels( const NodeIterator& v, const CriteriaList& g_v)
	{
		for ( std::vector<Label>::iterator it = v->labels.begin(); it != v->labels.end(); ++it)
		{
			if ( it->getCriteriaList().dominates(g_v) )
		    {
				return true;
		    }
		}
		return false;
	}

	void moveToClosed( const CriteriaList& g_u, const NodeIterator& u)
	{
		for ( std::vector<Label>::iterator it = u->labels.begin(); it != u->labels.end(); ++it)
		{
			if ( (it->isInQueue()) && (it->getCriteriaList() == g_u) )
		    {
				it->deletePQitem();
		    }
		}		
	}
	
};


#endif//NAMOASTAR_H

