#ifndef PACKEDMEMORYARRAYHELPER_H
#define PACKEDMEMORYARRAYHELPER_H

#include <Structs/Trees/completeBinaryTree.h>
#include <assert.h>


class PmaTreeData{
public:
	PmaTreeData( unsigned int cardinality = 0, unsigned int offsetElements = 0):m_cardinality(cardinality), m_offsetElements(offsetElements)
	{
	}
	
	friend std::ostream& operator << ( std::ostream & os, const PmaTreeData& data)
	{
		os << "( " << data.m_cardinality << ", " << data.m_offsetElements << ")";
		return os;	
	}

	unsigned int m_cardinality;
	unsigned int m_offsetElements;
};


template <typename DataType>
class PackedMemoryArrayHelper
{
public:
    typedef CompleteBinaryTree<PmaTreeData, ExplicitVebStorage> TreeType;
    typedef TreeType::SizeType SizeType;
    typedef TreeType::Node Node;


    PackedMemoryArrayHelper():m_densityTree(0)
    {       
        //m_densityTree = new CompleteBinaryTree<PmaTreeData>( 0); 
    }

    ~PackedMemoryArrayHelper()
    {   
        assert(m_densityTree);
        delete m_densityTree;
        delete [] m_maxDensity;
        delete [] m_minDensity;
        m_densityTree = 0;
        m_maxDensity = 0;
        m_minDensity = 0;
    }

    bool affordsElementErasureAt( Node& u)
    {
        return u->m_cardinality - 1 >= m_minDensity[u.getHeight()] * getCapacity(u);
    }

    bool affordsElementInsertionAt( Node& u)
    {
        return u->m_cardinality + 1 <= m_maxDensity[u.getHeight()] * getCapacity(u);
    }

    bool affordsErasure()
    {
        assert(m_densityTree);
        m_auxNode.setAtRoot();
        return affordsElementErasureAt(m_auxNode);
    }  
    
    bool affordsInsertion() 
    {
        assert(m_densityTree);
        m_auxNode.setAtRoot();
        return affordsElementInsertionAt(m_auxNode);
    }

    template< typename StreamType>
    void compressOver( const Node& u, StreamType& stream)
    {
        assert(m_densityTree);
        std::stack<Node> S;
        SizeType childCapacity,rightCardinality, leftCardinality;
        S.push(u);
        stream.setAt( u.getHorizontalIndex() * getCapacity(u) );
        while( !S.empty())
        {
            m_auxNode = S.top();
            S.pop();      	 
            if( !m_auxNode.isLeaf())
            {
                childCapacity = (getCapacity(m_auxNode) >> 1);
                if( m_auxNode->m_cardinality > childCapacity)
                {
                    leftCardinality = childCapacity;
                    rightCardinality = m_auxNode->m_cardinality - childCapacity;
                }
                else
                {
                    leftCardinality = m_auxNode->m_cardinality;
                    rightCardinality = 0;
                }
                
                m_auxNode.goRight();

                m_auxNode->m_cardinality = rightCardinality;
                      
                S.push(m_auxNode);

                m_auxNode.goUp();
                m_auxNode->m_offsetElements = leftCardinality;
                
                m_auxNode.goLeft();
                m_auxNode->m_cardinality = leftCardinality;

                S.push(m_auxNode);
            }
            else
            {
                m_auxNode->m_offsetElements = 0;
                if( m_auxNode->m_cardinality)
                {
                    stream.writeOut( m_auxNode->m_cardinality);
                }
                stream.emptyOut( getCapacity(m_auxNode) - m_auxNode->m_cardinality);
            }
        }
    }

    void decreaseCardinality( const Node& u)
    {
        assert(m_densityTree);
        m_auxNode = u;
        bool changedParentOffset = false;
        while( !m_auxNode.isRoot())
        {
            --(m_auxNode->m_cardinality);
            if( changedParentOffset)
            {
                --(m_auxNode->m_offsetElements);
            }
            // Only left children change a parent's offset
            changedParentOffset = !m_auxNode.isRightChild();
            m_auxNode.goUp();
        }
        --(m_auxNode->m_cardinality);
        if( changedParentOffset)
        {
            --(m_auxNode->m_offsetElements);
        }
    }

    const Node& findEmptiestNode()
    {
        assert(m_densityTree);
        m_auxNode.setAtRoot();
        while( !m_auxNode.isLeaf())
        {
            if( m_auxNode->m_offsetElements <= (m_auxNode->m_cardinality >> 1))
            {
                m_auxNode.goLeft();
            }   
            else
            {
                m_auxNode.goRight();
            }
        }
        return m_auxNode;
    }

    SizeType findIndexContainingElement( const SizeType& elementIndex)
    {
        assert(m_densityTree);
        m_auxNode.setAtRoot();
        SizeType aggregateOffset = 0;
        SizeType aggregateCapacity = 0;

        while( !m_auxNode.isLeaf())
        {
		    if( elementIndex < aggregateOffset + m_auxNode->m_offsetElements)
		    {
			    m_auxNode.goLeft();
		    }
		    else
		    {
			    aggregateOffset += m_auxNode->m_offsetElements;
                aggregateCapacity += (getCapacity(m_auxNode) >> 1);
			    m_auxNode.goRight();
		    }
        }

        return aggregateCapacity + ( elementIndex - aggregateOffset);
    }

    const Node& findNodeContainingElement( const SizeType& elementIndex)
    {
        assert(m_densityTree);
        m_auxNode.setAtRoot();
        SizeType aggregateOffset = 0;

        while( !m_auxNode.isLeaf())
        {
		    if( elementIndex < aggregateOffset + m_auxNode->m_offsetElements)
		    {
			    m_auxNode.goLeft();
		    }
		    else
		    {
			    aggregateOffset += m_auxNode->m_offsetElements;
			    m_auxNode.goRight();
		    }
        }

        return m_auxNode;
    }

    SizeType getAggregateOffsetOver( const SizeType& index)
    {
        SizeType aggregateOffset = 0;
        m_auxNode.setAtPos( 0, index / m_leafSize);
        while( !m_auxNode.isRoot())
        {
            if( m_auxNode.isRightChild())
            {
                m_auxNode.goUp();
                aggregateOffset += m_auxNode->m_offsetElements;
            }
            else
            {
                m_auxNode.goUp();
            }
        }
        return aggregateOffset;
    }

    inline SizeType getCapacity( const Node& u)
    {
        return m_leafSize << u.getHeight();
    }

    inline SizeType getCardinality( const Node& u)
    {
        return u->m_cardinality;
    }
    
    SizeType getIndexUnderNode( const Node& u)
    {
        assert(m_densityTree);
        return m_leafSize * u.getHorizontalIndex();
    }

    SizeType getMemoryUsage()
    {
        return m_densityTree->getMemoryUsage() + m_auxNode.getMemoryUsage() + sizeof( SizeType);
    }

    const Node& getNodeOverIndex( const SizeType& index)
    {
        assert(m_densityTree);
        m_auxNode.setAtPos( 0, index / m_leafSize);
        return m_auxNode;
    }

    const Node& getParentForErasure( const Node& u)
    {
        assert(m_densityTree);
        m_auxNode = u;
        while( (!m_auxNode.isRoot()) && (!affordsElementErasureAt(m_auxNode)))
        {
            m_auxNode.goUp();
        }
        return m_auxNode;
    }

    const Node& getParentForInsertion( const Node& u)
    {
        assert(m_densityTree);
        m_auxNode = u;
        while( !affordsElementInsertionAt(m_auxNode))
        {
            m_auxNode.goUp();
        }
        return m_auxNode;
    }

    std::pair<SizeType,SizeType> getRange( const Node& u)
    {
        assert(m_densityTree);
        SizeType index = (m_leafSize << u.getHeight()) * u.getHorizontalIndex();
        return std::pair<SizeType,SizeType>( index, index + getCapacity(u)) ;
    }

    const Node& getRoot()
    {   
        assert(m_densityTree);
        m_auxNode.setAtRoot();
        return m_auxNode;
    }

    void increaseCardinality( const Node& u)
    {
        assert(m_densityTree);
        m_auxNode = u;
        bool changedParentOffset = false;
        while( !m_auxNode.isRoot())
        {
            ++(m_auxNode->m_cardinality);
            if( changedParentOffset)
            {
                ++(m_auxNode->m_offsetElements);
            }
            // Only left children change a parent's offset
            changedParentOffset = !m_auxNode.isRightChild();
            m_auxNode.goUp();
        }
        ++(m_auxNode->m_cardinality);
        if( changedParentOffset)
        {
            ++(m_auxNode->m_offsetElements);
        }
    }

    

    void printDot( std::ostream& out, const std::string& name = "")
    {
        if( m_densityTree->getHeight() > 10)
        {
            SizeType size = (1 << m_densityTree->getHeight());
            std::cout << "The tree is too big to print (>" << size << ")\n";
            return;
        }
        //printDataVisitor<PmaTreeData>* visitor = new printDataVisitor<PmaTreeData>(out, name);  
        //dfs( m_densityTree, &m_auxNode, visitor);
        
        m_auxNode.setAtRoot();
                
        std::stack<Node> S;
        S.push(m_auxNode);
        while( !S.empty())
        {
            m_auxNode = S.top();
            S.pop();      	 
            out << name << m_auxNode.getPoolIndex();
            out << "[shape=record,label=\"{";
            out << name << m_auxNode.getPoolIndex() << "|";
		    //out << name << m_auxNode.getPoolIndex() << "|" << m_auxNode.getBfsIndex() << "|";
		    //m_out << "{" << u->getData().m_offsetElements << "|" << u->getData().m_cardinality << "}|";
		    out << m_auxNode->m_cardinality << "/" << getCapacity(m_auxNode) << "|";
		    out << m_minDensity[m_auxNode.getHeight()] << "-" << m_maxDensity[m_auxNode.getHeight()] ;
     	    //out << m_minDensity[m_auxNode.getHeight()] * getCapacity(m_auxNode) << "-" << m_maxDensity[m_auxNode.getHeight()] * getCapacity(m_auxNode);
	        out << "}\"]\n";
            out << "\t"; 
            if( !m_auxNode.isLeaf())
            {
                out << name << m_auxNode.getPoolIndex();    
                out << " -> ";  
                m_auxNode.goLeft();
                out << name << m_auxNode.getPoolIndex();    
                out << "\n"; 
                S.push(m_auxNode);
                m_auxNode.goUp();
                out << "\t";        
                out << name << m_auxNode.getPoolIndex(); 
                out << " -> ";     
                m_auxNode.goRight();
                out << name << m_auxNode.getPoolIndex();  
                S.push(m_auxNode);
            }
            out << "\n";
        }
        //delete visitor;  
    }

    template< typename StreamType>
    void rearrangeOver( const Node& u, const Node& sparseNode, StreamType& stream)
    {
        assert(m_densityTree);
        std::stack<Node> S;
        SizeType remainder, baseCardinality, leftCardinality;
        S.push(u);
        stream.setAt( u.getHorizontalIndex() * getCapacity(u) );
        while( !S.empty())
        {
            m_auxNode = S.top();
            S.pop();      	 
            if( !m_auxNode.isLeaf())
            {
                remainder = modulusPow2( m_auxNode->m_cardinality, 2);
                baseCardinality = m_auxNode->m_cardinality >> 1;    

                m_auxNode.goRight();

                if( sparseNode.isToTheLeftOf( m_auxNode))
                {
                    m_auxNode->m_cardinality = baseCardinality + remainder;
                    leftCardinality = baseCardinality;
                }
                else
                {
                    m_auxNode->m_cardinality = baseCardinality;
                    leftCardinality = baseCardinality + remainder;
                }  
                      
                S.push(m_auxNode);

                m_auxNode.goUp();
                m_auxNode->m_offsetElements = leftCardinality;
                
                m_auxNode.goLeft();
                m_auxNode->m_cardinality = leftCardinality;

                S.push(m_auxNode);
            }
            else
            {
                m_auxNode->m_offsetElements = 0;
                //assert( m_auxNode->m_cardinality > 0);
                if( m_auxNode->m_cardinality)
                {
                    stream.writeOut( m_auxNode->m_cardinality);
                }
                stream.emptyOut( getCapacity(m_auxNode) - m_auxNode->m_cardinality);
            }
        }
    }

    template< typename StreamType>
    void rearrangeOver( const Node& u, StreamType& stream)
    {
        assert(m_densityTree);
        std::stack<Node> S;
        SizeType remainder, baseCardinality, leftCardinality;
        S.push(u);
        stream.setAt( u.getHorizontalIndex() * getCapacity(u) );
        while( !S.empty())
        {
            m_auxNode = S.top();
            S.pop();      	 
            if( !m_auxNode.isLeaf())
            {
                remainder = modulusPow2( m_auxNode->m_cardinality, 2);
                baseCardinality = m_auxNode->m_cardinality >> 1;    

                m_auxNode.goRight();

                m_auxNode->m_cardinality = baseCardinality;
                leftCardinality = baseCardinality + remainder;

                S.push(m_auxNode);

                m_auxNode.goUp();
                m_auxNode->m_offsetElements = leftCardinality;
                
                m_auxNode.goLeft();
                m_auxNode->m_cardinality = leftCardinality;

                S.push(m_auxNode);
            }
            else
            {
                m_auxNode->m_offsetElements = 0;
                //assert( m_auxNode->m_cardinality > 0);
                if( m_auxNode->m_cardinality)
                {
                    stream.writeOut( m_auxNode->m_cardinality);
                }
                stream.emptyOut( getCapacity(m_auxNode) - m_auxNode->m_cardinality);
            }
        }
    }

    void reset( const SizeType& treeHeight, const SizeType& leafSize, const SizeType& cardinality, double minEmptinessPercentage = 0.5, double maxFullnessPercentage = 0.75)
    {
        if( m_densityTree)
        {
            delete m_densityTree;
            delete [] m_maxDensity;
            delete [] m_minDensity;
            m_densityTree = 0;
            m_maxDensity = 0;
            m_minDensity = 0;
        }
        m_densityTree = new TreeType( treeHeight, leafSize * pow2(treeHeight) );  
        m_auxNode = m_densityTree->getRootNode();
        m_auxNode->m_cardinality = cardinality; 
        m_leafSize = leafSize;
        
        m_maxDensity = new double[ treeHeight + 1];
	    m_minDensity = new double[ treeHeight + 1];
	    m_maxDensity[treeHeight] = maxFullnessPercentage;
	    m_minDensity[treeHeight] = minEmptinessPercentage;
	    m_maxDensity[0] = 0.9;
	    m_minDensity[0] = 0.1;
	    // calculate densities		
	    for( SizeType h = 1; h < treeHeight; h++)
	    {
	        m_maxDensity[h] = m_maxDensity[treeHeight] + ( m_maxDensity[0] - m_maxDensity[treeHeight]) * ( treeHeight - h) / treeHeight;
	        m_minDensity[h] = m_minDensity[treeHeight] - ( m_minDensity[treeHeight] - m_minDensity[0]) * ( treeHeight - h) / treeHeight;
	    }
    }

private:
    TreeType*   m_densityTree;
    Node        m_auxNode;
    SizeType    m_leafSize;
    double*     m_maxDensity;
    double*     m_minDensity;

};


#endif //PACKEDMEMORYARRAYHELPER_H
