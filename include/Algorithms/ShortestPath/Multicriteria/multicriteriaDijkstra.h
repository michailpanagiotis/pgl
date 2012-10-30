#ifndef MULTICRITERIADIJKSTRA_H
#define MULTICRITERIADIJKSTRA_H

#include <Structs/Trees/priorityQueue.h>
class CriteriaList
{
public:
    typedef unsigned int WeightType;
    typedef std::vector< WeightType>::iterator Iterator;
    typedef std::vector< WeightType>::const_iterator ConstIterator;    

    CriteriaList( const unsigned int& numCriteria = 0, const unsigned int& defaultValue = 0): m_criteria( numCriteria, defaultValue)
    {
    }

    CriteriaList( const CriteriaList& other): m_criteria( other.m_criteria)
    {
    }

	CriteriaList( const std::vector<WeightType>& other)
    {
        for( unsigned int i = 0; i < other.size(); ++i)
        {
            m_criteria.push_back( other[i]);
        }
    }
    
    void clear()
    {
        for ( Iterator criterion = m_criteria.begin(), endCriterion = m_criteria.end(); criterion != endCriterion; ++criterion)
        {
            *criterion = 0;
        }
    }

    bool dominates(const CriteriaList& other) const
    {
        assert( m_criteria.size() == other.m_criteria.size());
        if ( *this == other) return true;
        
        ConstIterator criterion = m_criteria.begin(), endCriterion = m_criteria.end(), otherCriterion = other.m_criteria.begin();
        for ( ; criterion != endCriterion; ++criterion, ++otherCriterion)
        {
            if ( (*criterion) > (*otherCriterion))  
            {
                return false;
            }
        }
        return true;
        //return dominatesTight( other);
    }
    
    bool dominatesTight(const CriteriaList& other) const
    {
        assert( m_criteria.size() == other.m_criteria.size());
        assert( m_criteria.size() > 1);
           
        //std::cout << (double) m_criteria[0]/other.m_criteria[0] << std::endl;
        if( (double) (1/0.999) * m_criteria[0] < (double)other.m_criteria[0]) return true;
        return false;

        ConstIterator criterion = m_criteria.begin(), endCriterion = m_criteria.end(), otherCriterion = other.m_criteria.begin();
        WeightType mysum = 0;
        WeightType othersum = 0;
        for ( ; criterion != endCriterion; ++criterion, ++otherCriterion)
        {
            if ( criterion == m_criteria.begin()) continue;
            mysum += (*criterion);
            othersum += (*criterion);
        }
        criterion = m_criteria.begin();
        otherCriterion = other.m_criteria.begin();
        return ((double)othersum / mysum) > ( (double) (*criterion)/ (*otherCriterion)) * gamma;
    }

    bool isDominatedBy(const CriteriaList& other) const
    {
        return other.dominates(*this);
    }

    WeightType& operator [] ( unsigned int pos)
    {
        assert ( pos < m_criteria.size());
        Iterator it = m_criteria.begin();
        std::advance( it, pos);
        return *it;
    }

    bool operator < (const CriteriaList& other) const
    {
        assert( m_criteria.size() == other.m_criteria.size());
        if ( *this == other) return false;

        ConstIterator criterion = m_criteria.begin(), endCriterion = m_criteria.end(), otherCriterion = other.m_criteria.begin();
        for ( ; criterion != endCriterion; ++criterion, ++otherCriterion)
        {
            if ( (*criterion) < (*otherCriterion))  
            {
                return true;
            }
            if ( (*criterion) > (*otherCriterion))  
            {
                return false;
            }
        }
        return false;
    }

    bool operator > (const CriteriaList& other) const
    {       
        return other < (*this);
    }

    bool operator == (const CriteriaList& other) const
    {
        assert( m_criteria.size() == other.m_criteria.size());
        if ( this == &other) return true;

        ConstIterator criterion = m_criteria.begin(), endCriterion = m_criteria.end(), otherCriterion = other.m_criteria.begin();
        for ( ; criterion != endCriterion; ++criterion, ++otherCriterion)
        {
            if ( (*criterion) != (*otherCriterion))  
            {
                return false;
            }
        }
        return true;
    }

    CriteriaList operator + (const CriteriaList& other) const
    {
        assert( m_criteria.size() == other.m_criteria.size());
        CriteriaList sum( m_criteria.size());

        ConstIterator criterion = m_criteria.begin(), endCriterion = m_criteria.end(), otherCriterion = other.m_criteria.begin();
        Iterator sumCriterion = sum.m_criteria.begin();
        for ( ; criterion != endCriterion; ++criterion, ++otherCriterion, ++ sumCriterion)
        {
            *sumCriterion = *criterion + *otherCriterion;
        }
        return sum;
    }

    CriteriaList operator - (const CriteriaList& other) const
    {
        assert( m_criteria.size() == other.m_criteria.size());
        CriteriaList diff( m_criteria.size());

        ConstIterator criterion = m_criteria.begin(), endCriterion = m_criteria.end(), otherCriterion = other.m_criteria.begin();
        Iterator diffCriterion = diff.m_criteria.begin();
        for ( ; criterion != endCriterion; ++criterion, ++otherCriterion, ++ diffCriterion)
        {
			assert( *criterion >= *otherCriterion);
            *diffCriterion = *criterion - *otherCriterion;
        }
        return diff;
    }

    void print (std::ostream& out, const std::string& delimiter = ", " )
    {
		unsigned int i = 0;
        for ( Iterator criterion = m_criteria.begin(), endCriterion = m_criteria.end(); criterion != endCriterion; ++criterion)
        {
            if ( criterion != m_criteria.begin()) out << delimiter;
            out << "c" << i++ << ": " << *criterion;
        }
    }
private:
    std::vector< WeightType> m_criteria;
    static const double gamma = 1.1;
    static const double epsilon = 0.005;
};

class Label
{
public:

    Label(): m_criteriaList( 0), m_pred(0), m_data(0)
    {        
    }

    Label( const unsigned int& numCriteria): m_criteriaList( numCriteria), m_pred(0), m_data(0) 
    {        
    }

    Label( const CriteriaList& criteriaList, void* pred, void* data): 
                                        			m_criteriaList( criteriaList),  
                                                    m_pred(pred),
													m_data(data)
    {        
    }

    Label( const Label& other): 
                        m_criteriaList( other.m_criteriaList), 
                        m_pred(other.m_pred),
						m_data(other.m_data)
    {        
    }

	void deletePQitem()
	{
		delete m_data.pqitem;
		m_data.pqitem = 0;
	}

    bool dominates(const Label& other) const
    {
        return m_criteriaList.dominates( other.m_criteriaList);
    }

    bool dominatesUnique(const Label& other) const
    {
        if( m_data.boundaryNode == other.m_data.boundaryNode) return m_criteriaList.dominates( other.m_criteriaList);
        return false;
    }
    
    void* getPredecessor() const
    {
        return m_pred;
    }

    const CriteriaList& getCriteriaList() const
    {
        return m_criteriaList;
    }

	unsigned int* getPQitem() const
	{
		return m_data.pqitem;
	}

    bool isDominatedBy(const Label& other) const
    {
        return other.dominates(*this);
    }

    bool isDominatedUniqueBy(const Label& other) const
    {
        if( m_data.boundaryNode == other.m_data.boundaryNode) return other.dominates(*this);
        return false;
    }

	bool isInQueue() const
	{
		return m_data.pqitem != 0;
	}

    bool operator < (const Label& other) const
    {
        return m_criteriaList < other.m_criteriaList;
    }

    bool operator > (const Label& other) const
    {       
        return other < (*this);
    }

    template <class GraphType>
    void print (std::ostream& out, GraphType& G)
    {
        out << "( ";
        m_criteriaList.print(out, ", ");
 
        if ( m_pred )
        {
            typename GraphType::NodeIterator u = G.getNodeIterator((typename GraphType::NodeDescriptor)m_pred);
            //out <<  ", " << u->id;
        }
        else
        {
            out << ", nil";
        }
        out << ")";
    }

private:
    CriteriaList m_criteriaList;
    void* m_pred; 
	 
    union extra_info
    {
        extra_info(void* init):boundaryNode(init)
        {
        }
        unsigned int* pqitem;
        void * boundaryNode;
    };

    extra_info m_data;
};

template<class GraphType>
class MulticriteriaDijkstra
{
public:
	typedef typename GraphType::NodeIterator    node;
	typedef typename GraphType::EdgeIterator    edge;
	typedef typename GraphType::SizeType        SizeType;
	typedef typename GraphType::NodeData        NodeData;


	typedef PriorityQueue< Label, node, HeapStorage> PriorityQueueType;
	typedef typename PriorityQueueType::PQItem PQItem;   

    /**
     * @brief Constructor
     *
     * @param graph The graph to run the algorithm on
     * @param timestamp An address containing a timestamp. A timestamp must be given in order to check whether a node is visited or not
     */
    MulticriteriaDijkstra( GraphType& graph, unsigned int numCriteria, unsigned int* timestamp):G(graph),m_numCriteria(numCriteria),m_timestamp(timestamp)
    {
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

    /**
     * @brief Builds a shortest path tree routed on a source node
     *
     * @param s The source node
     */
    void runQuery( const typename GraphType::NodeIterator& s, const typename GraphType::NodeIterator& t)
    {
		node u,v,lastNode;
		edge e,lastEdge;

		bool isDominated;

        m_generatedLabels = 1;
		pq.insert( Label( CriteriaList(m_numCriteria), 0, 0), s);

		while( !pq.empty())
		{
		    Label label = pq.min().key;
		    u = pq.minItem();
		    pq.popMin();

		    //std::cout << "extracting " << u->id << " with label ";
		    //label.print(std::cout, G);
		    //std::cout << std::endl; 

		    for( e = G.beginEdges(u), lastEdge = G.endEdges(u); e != lastEdge; ++e)
		    {
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

		        //std::cout << "push into queue " << v->id << " with label ";
		        //newLabel.print(std::cout, G);
		        //std::cout << std::endl;
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

		        //std::cout << "push into node vector " << v->id << " label ";
		        //newLabel.print(std::cout, G);
		        //std::cout << std::endl;

		        v->labels.push_back( newLabel );
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
};


#endif//MULTICRITERIADIJKSTRA_H

