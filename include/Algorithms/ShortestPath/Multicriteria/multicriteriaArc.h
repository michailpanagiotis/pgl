#ifndef MULTICRITERIAARC_H
#define MULTICRITERIAARC_H

#include <Structs/Trees/priorityQueue.h>
#include <Utilities/geographic.h>
#include <Algorithms/basicGraphAlgorithms.h>

template<class GraphType>
class MulticriteriaArc;

template<class GraphType, template <typename graphType> class HeuristicType>
class NamoaStarArc
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
    NamoaStarArc( GraphType& graph, unsigned int numCriteria, unsigned int* timestamp):G(graph),m_numCriteria(numCriteria),m_timestamp(timestamp),m_heuristicEngine(graph),m_arcDijkstra(graph, numCriteria, timestamp)
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

        unsigned int mask = m_arcDijkstra.getPartition().getOnMask( m_arcDijkstra.getPartition().getCell( t->x, t->y));

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
                if( ! (e->flags & mask)) continue;

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
    MulticriteriaArc<GraphType> m_arcDijkstra;

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



template<class GraphType>
class MulticriteriaArc
{
public:
	typedef typename GraphType::NodeIterator    NodeIterator;
	typedef typename GraphType::NodeDescriptor  NodeDescriptor;
	typedef typename GraphType::EdgeIterator    EdgeIterator;
	typedef typename GraphType::InEdgeIterator  InEdgeIterator;
	typedef typename GraphType::SizeType        SizeType;
	typedef typename GraphType::NodeData        NodeData;
	typedef PriorityQueue< Label, NodeIterator, HeapStorage> PriorityQueueType;
	typedef typename PriorityQueueType::PQItem PQItem;   
	
    class Partition
    {
    public:
        
        Partition()
        {
        }
        
	    Partition( unsigned int xmin, unsigned int xmax, unsigned int ymin, unsigned int ymax, unsigned int div, unsigned int l = 1):
	                        m_xmin(xmin),
	                        m_xmax(xmax + 1),
	                        m_ymin(ymin),
	                        m_ymax(ymax + 1),
	                        m_div(div),
	                        m_numLevels(l)
	    {    
        }

        unsigned int getCell( unsigned int x, unsigned int y, unsigned int l = 0)
	    {   
	        unsigned int column = getOffsetX(x, l) / getStepXofLevel(l);
		    unsigned int row = getOffsetY(y, l) / getStepYofLevel(l);
		    return row*m_div + column;
        }
    
        unsigned int getMaxLevel () const
        {
            return getNumLevels() - 1;
        }
    
        unsigned int getNumCells ( const unsigned int& l = 0) const
        {
            return m_div * m_div;
        }
    
        unsigned int getNumLevels () const
        {
            return m_numLevels;
        }
	
	    unsigned int getOffsetX ( const unsigned int& x, unsigned int l)
        {
            if ( l == getMaxLevel())
                return x-m_xmin;
            else
                return (x-m_xmin)%getStepXofLevel(l+1);
        }
    
        unsigned int getOffsetY ( unsigned int y, unsigned int l)
        {
            if ( l == getMaxLevel())
                return y-m_ymin;
            else
                return (y-m_ymin)%getStepYofLevel(l+1);
        }
    
        unsigned int getOnMask( unsigned int index)
	    {
	        return 1 << index;
	    }
	
	    unsigned int getOffMask( unsigned int index)
	    {
            return ~getOnMask( index);
        }
	
	    unsigned int getStepXofLevel(unsigned int l)
        {
            //std::cout << "Step X at level " << l << " is " << (m_xmax-m_xmin)/pow( (double)m_div, (double)m_numLevels-l) << std::endl;
            return (m_xmax-m_xmin)/pow( (double)m_div, (double)m_numLevels-l);
        }
    
        unsigned int getStepYofLevel(unsigned int l)
        {
            //std::cout << "Step Y at level " << l << " is " << (m_ymax-m_ymin)/pow( (double)m_div, (double)m_numLevels-l) << std::endl;
            return (m_ymax-m_ymin)/pow( (double)m_div, (double)m_numLevels-l);
        }
        
        void reset( unsigned int xmin, unsigned int xmax, unsigned int ymin, unsigned int ymax, unsigned int div, unsigned int l = 1)
        {
            m_xmin = xmin;
            m_xmax = xmax + 1;
            m_ymin = ymin;
            m_ymax = ymax + 1;
            m_div = div;
            m_numLevels = l;
        }
	
    private:
        unsigned int m_xmin, m_xmax, m_ymin, m_ymax, m_div, m_numLevels;
    };


    MulticriteriaArc( GraphType& graph, unsigned int numCriteria, unsigned int* timestamp):
                    G(graph),
                    m_numCriteria(numCriteria),
                    m_timestamp(timestamp)
    {
        partition();
        preprocess();
    }

    void getBoundaryNodes( std::vector<NodeIterator>& boundary, const unsigned int& cell)
    {
        NodeIterator u, lastNode;
        boundary.clear();
        unsigned int sum = 0;
        for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
        {
            if( m_partition.getCell( u->x, u->y) != cell) continue;
            sum++;
            if( isBoundaryNodeInCell( u, cell)) boundary.push_back(u);
        }
        std::cout << "\thas " << sum << " nodes\n";
    }

    Partition& getPartition()
    {
        return m_partition;
    } 

    const unsigned int& getGeneratedLabels()
    {
        return m_generatedLabels;
    }

    void init(const typename GraphType::NodeIterator& s, const typename GraphType::NodeIterator& t)
	{
		typename GraphType::NodeIterator u, lastNode;
		for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
		{
		    u->labels.clear();
		}
		pq.clear();
	}   

    bool isBoundaryNodeInCell( typename GraphType::NodeIterator& u, const unsigned int& cell)
	{
		std::vector<NodeDescriptor> neighbors = getNeighbors( G, u);
		for( unsigned int i = 0; i < neighbors.size(); ++i)
		{
            NodeIterator v = G.getNodeIterator( neighbors[i]);
            //std::cout << "(" << v->x << "," << v->y << ") (" << u->x << "," << u->y << ")\n";
            //assert( m_partition.getCell( v->x, v->y) == cell);
			if ( m_partition.getCell( v->x, v->y) != cell )
				return true;
     	}
        return false;
	}

    void openFlagsLeadingTo( const std::vector< NodeIterator>& boundary, const unsigned int& cell)
    {
        NodeIterator u,v,lastNode;
		EdgeIterator e,lastEdge;
        InEdgeIterator k,lastInEdge;

        for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
		{
		    u->labels.clear();
		}
		PriorityQueueType queue;
        unsigned int onMask = m_partition.getOnMask( cell);

		bool isDominated;

        m_generatedLabels = boundary.size();

        for( unsigned int i = 0; i < boundary.size(); ++i)
        {
            queue.insert( Label( CriteriaList(m_numCriteria), 0, boundary[i]->getDescriptor()), boundary[i]);
        }

        std::cout << "\tBuilding boundary tree for cell " << cell << "\n";

		while( !queue.empty())
		{
		    Label label = queue.min().key;
		    u = queue.minItem();
		    queue.popMin();

		    for( k = G.beginInEdges(u), lastInEdge = G.endInEdges(u); k != lastInEdge; ++k)
		    {
		        isDominated = false;
		        v = G.source(k);

                if( m_partition.getCell( v->x, v->y) == cell ) continue;

		        Label newLabel( label.getCriteriaList() + k->criteriaList, u->getDescriptor(), label.getPredecessor() );

		        for ( std::vector<Label>::iterator it = v->labels.begin(); it != v->labels.end(); ++it)
		        {
		            if ( it->dominatesUnique(newLabel) )
		            {
		                isDominated = true;
		                break;
		            }
		        }
		        if ( isDominated )  continue;

                ++m_generatedLabels;
		        queue.insert( newLabel, v);

		        std::vector<Label>::iterator it = v->labels.begin();
		        while ( it != v->labels.end() )
		        {
		            if ( it->isDominatedUniqueBy(newLabel) )
		            {
		                it = v->labels.erase( it );
		            }
		            else 
		            {
		                ++it;
		            }
		        }

		        v->labels.push_back( newLabel );
		    }
		}

        std::cout << "\tSetting flags out of cell " << cell << "\n";

        for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
		{
            for ( std::vector<Label>::iterator it = u->labels.begin(); it != u->labels.end(); ++it)
		    {   
                e = G.getEdgeIterator( G.getNodeDescriptor(u), (NodeDescriptor)it->getPredecessor());
                k = G.getInEdgeIterator( e);
                e->flags |= onMask;
                k->flags |= onMask;
            }
            u->labels.clear();
        }

        std::cout << "\tSetting flags inside cell " << cell << "\n";

        for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
		{
            for( e = G.beginEdges(u), lastEdge = G.endEdges(u); e != lastEdge; ++e)
		    {
                v = G.target(e);
                if( m_partition.getCell( v->x, v->y) == cell )
                {
                    k = G.getInEdgeIterator( e);
                    e->flags |= onMask;
                    k->flags |= onMask;
                }
            }
		}
        std::cout << "\tGenerated labels: " << m_generatedLabels << "\n";
    }

    void partition()
    {
        std::cout << "Partitioning graph...\n";
        unsigned int xmax = 0, xmin = std::numeric_limits<unsigned int>::max(), ymax = 0, ymin = std::numeric_limits<unsigned int>::max();
        NodeIterator u, v, last;
        EdgeIterator e, lastEdge;
        InEdgeIterator k;
        for ( u = G.beginNodes(), last = G.endNodes(); u != last; ++u)
        {
            //for( e = G.beginEdges(u), lastEdge = G.endEdges(u); e != lastEdge; ++e)
            //{
            //    k = G.getInEdgeIterator(e);
                //e->flags &= 0;
                //k->flags &= 0;
            //}
            if ( u->x > xmax) xmax = u->x;
            if ( u->x < xmin) xmin = u->x;
            if ( u->y > ymax) ymax = u->y;
            if ( u->y < ymin) ymin = u->y;
        }
        m_partition.reset( xmin, xmax, ymin, ymax, 4);
        
    }


    void preprocess()
    {
        NodeIterator u, v, lastNode;
        EdgeIterator e, lastEdge;
        std::cout << "Preprocessing Arc Flags...\n";
        for( unsigned int c = 0; c < m_partition.getNumCells(); ++c)
        {
            std::cout << "Cell: " << c << "\n";
            std::vector< NodeIterator> boundary;
            getBoundaryNodes( boundary, c);
            std::cout << "\thas " << boundary.size() << " boundary nodes\n";
            openFlagsLeadingTo( boundary, c);
        }

        std::ofstream out("arc-flags");
        for( u = G.beginNodes(), lastNode = G.endNodes(); u != lastNode; ++u)
		{
            for( e = G.beginEdges(u), lastEdge = G.endEdges(u); e != lastEdge; ++e)
		    {
                v = G.target(e);
                out << G.getRelativePosition(u) + 1 << " " << G.getRelativePosition(v) + 1 << " " << e->flags << "\n";
            }
        }
        out.close();
    }

    void runQuery( const typename GraphType::NodeIterator& s, const typename GraphType::NodeIterator& t)
    {
		NodeIterator u,v,lastNode;
		EdgeIterator e,lastEdge;

        unsigned int mask = m_partition.getOnMask( m_partition.getCell( t->x, t->y));

		bool isDominated;

        m_generatedLabels = 1;
		pq.insert( Label( CriteriaList(m_numCriteria), 0, 0), s);

		while( !pq.empty())
		{
		    Label label = pq.min().key;
		    u = pq.minItem();
		    pq.popMin();

		    for( e = G.beginEdges(u), lastEdge = G.endEdges(u); e != lastEdge; ++e)
		    {
                if( ! (e->flags & mask)) continue;

		        isDominated = false;
		        v = G.target(e);
		        Label newLabel( label.getCriteriaList() + e->criteriaList, u->getDescriptor(), 0 );

		        for ( std::vector<Label>::iterator it = v->labels.begin(); it != v->labels.end(); ++it)
		        {
		            if ( it->dominates(newLabel) )
		            {
		                isDominated = true;
		                break;
		            }
		        }
		        if ( isDominated )  continue;

                ++m_generatedLabels;
		        pq.insert( newLabel, v);

		        std::vector<Label>::iterator it = v->labels.begin();
		        while ( it != v->labels.end() )
		        {
		            if ( it->isDominatedBy(newLabel) )
		            {
		                it = v->labels.erase( it );
		            }
		            else 
		            {
		                ++it;
		            }
		        }

		        v->labels.push_back( newLabel );
		    }
		}
    }
private:
    GraphType& G;
    PriorityQueueType pq;
    unsigned int m_generatedLabels;
	unsigned int m_numCriteria;
	unsigned int* m_timestamp;
    Partition m_partition;
};







#endif//MULTICRITERIAARC_H

