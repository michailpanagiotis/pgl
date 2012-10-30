#ifndef FORWARDSTARARRAY_H
#define FORWARDSTARARRAY_H

#include <Utilities/mersenneTwister.h>
#include <set>
#include <deque>
#include <assert.h>
#include <limits>
#include <memory>

/**
 * @class ForwardStarArray
 *
 * @brief 
 * 
 * @author Panos Michail
 *
 */

template <typename dataType>
class ForwardStarArray
{
public:
    typedef unsigned int SizeType;


    class Iterator;    
    class Bucket;
    class InlineStream; 
    class ConstantStream; 
    class CopyingStream;  
    class Observer;

    ForwardStarArray(): m_emptyElement(std::numeric_limits<unsigned int>::max()), 
                            m_numElements(0),
                            m_poolSize(4)
    {    
        // Because when doubling we do not want to be *too* empty
        assert( 2*minEmptinessPercentage <= maxFullnessPercentage);
        m_bucketSize = 4;
        init();
        std::fill( m_pool, m_pool + m_poolSize, m_emptyElement);    
        m_auxIter.reset( m_pool);
        m_oldPool = 0;
    }

    ~PackedMemoryArray()
    {
        delete[]    m_pool;
    }

    Iterator atAddress( dataType* addr)
    {
        assert( addr < getPool() + m_poolSize);
        if( addr) return Iterator( this, addr);
        return m_end;
    }

    Iterator atIndex ( const SizeType& poolIndex )
    {
        if( poolIndex < m_poolSize)
		    return Iterator( this, m_pool + poolIndex);
        return m_end;
    }
    
    Iterator begin ()
    {
        dataType* ptr = m_pool;
        while( (ptr != m_end.m_ptr) && (*ptr == m_emptyElement))
        {
            ptr += m_bucketSize;
        }
        return Iterator( this, ptr);
    }
    
    SizeType capacity () const
    {
        return m_poolSize;
    }

    bool empty () const
    {
        return ( m_numElements == 0);
    }
    
    void clear()
    {
        m_numElements = 0;
        m_poolSize = 4;
        m_bucketSize = 4;

        delete[] m_pool;

        init();
        std::fill( m_pool, m_pool + m_poolSize, m_emptyElement);    
        m_auxIter.reset( m_pool);
        m_oldPool = 0;
    }

    Iterator chooseCell()
    {
        double random = m_gen.getRandomNormalizedDouble();
        SizeType pos = m_poolSize * random;
        Iterator it(this, m_pool + pos);
        it.sanitize();
        if( (it == m_end) && (m_numElements > 0))
        {
            --it;
        }
        return it;
    }

    void compress( )
    {
    }

    const Iterator& end ()
    {
        return m_end;
    }

    void erase( const Iterator& iter)
    {         
        m_auxBucket = getBucketAt( iter);
        m_auxBucket.erase(iter);
    }

    void expand( )
    {
    }

    Iterator findBucket( const dataType& data)
    {
        Iterator it;
        TreeNode u, left, right;
        u = m_helper.getRoot();
        while( !u.isLeaf())
        {
            left = u;
            left.goLeft();
            right = u;
            right.goRight();

            if( !u->m_cardinality)
            {
                while( !u.isLeaf())
                {
                    u.goRight();
                }
                break;
            }

            if( !right->m_cardinality)
            {
                u = left;
                continue;
            }

            it = atIndex( m_helper.getIndexUnderNode(right) );

            if( data < (*it))
            {
                u = left;
                continue;
            }
            else
            {
                u = right;
                continue;
            }
        }

        return atIndex( m_helper.getIndexUnderNode(u) );
    }
    
    Iterator find( const dataType& data)
    {
        Iterator it = findBucket( data);
        Iterator end = Iterator( this, it.getAddress() + m_bucketSize);
        it.sanitize();
        end.sanitize();
        while( it != end)
        {
            if( (*it) == data)
            {
                return it;
            }
            ++it;
        }
        return m_end;
    }

    Iterator lower_bound( const dataType& data)
    {
        Iterator it = findBucket( data);
        it.sanitize();
        
        while( it != m_end)
        {
            if( ((*it) > data) || ((*it) == data))
            {
                return it;
            }
            ++it;
        }
        return m_end;
    }
    
    inline const dataType& getEmptyElement() const
    {
        return m_emptyElement;
    }    

    SizeType getPoolIndex( const Iterator& it) const
    {
        return it.m_ptr - m_pool;
    }
    
    SizeType getMemoryUsage() 
    {
        SizeType poolMem = m_poolSize * sizeof(dataType);
        SizeType propertiesMem = 4 * sizeof(SizeType) + sizeof(dataType);       
        SizeType auxMem = m_auxNode.getMemoryUsage() + m_end.getMemoryUsage() + m_auxBucket.getMemoryUsage() + m_auxNode.getMemoryUsage();
        SizeType observersMem = m_observerSet.size() * sizeof( Observer*);

        SizeType elements = m_numElements * sizeof(dataType);

        std::cout << "\n\tPMA Mem:";
        std::cout << "\n\t\telements:\t"<< elements << "(" << double(elements)/1048576 << "Mb)";
        std::cout << "\n\t\tpool:\t\t" << poolMem << "(" << double(poolMem)/1048576 << "Mb)";
        std::cout << "\n\t\tproperties:\t" << propertiesMem << "(" << double(propertiesMem)/1024 << "Kb)";
        std::cout << "\n\t\tauxiliary:\t" << auxMem << "(" << double(auxMem)/1024 << "Kb)";
        std::cout << "\n\t\tobservers:\t" << observersMem << "(" << double(observersMem)/1048576 << "Mb)";
        std::cout << "\n";

        return poolMem + propertiesMem + auxMem + helperMem + observersMem;
    }
    
    dataType* getPool()
    {
        if( m_oldPool)
        {
            return m_oldPool;
        }
        return m_pool;
    }

    inline SizeType getPoolIndexOf( const Iterator& it) const
    {
        return it.getAddress() - m_pool;
    }
  
    inline const SizeType& getPoolSize() const
    {
        return m_poolSize;
    }

    const Iterator& insert( const Iterator& iter, const dataType& data)
    {         
        m_auxBucket = getBucketAt( iter);

        if( !m_auxBucket.hasRoom())
        {
            push_back(m_auxBucket)  
        }

        m_auxBucket.insert ( iter, data);
        ++m_numElements;

        m_helper.increaseCardinality( m_auxNode);

        return m_auxIter;
    }

    bool isValidIterator( const Iterator& it) const
    {
        return ( it.getAddress() >= m_pool) && ( it.getAddress() < m_end.getAddress());
    }

    /* @brief Moves an element. All observers are notified right before a move is performed
     */
    void move( dataType* source, dataType* destination, const dataType& data)
    {
        //assert( source >= m_pool && ( source < m_pool + m_poolSize) );
        assert( destination >= m_pool && ( destination < m_pool + m_poolSize) );

        if( m_auxIter.m_ptr == source)
        {
            m_newIteratorIndex = destination - m_pool;
        }

        if( source == destination)
        {
            *destination = data;
            return;
        }

        typename std::set<Observer*>::iterator obs, obsEnd; 
        for( obs = m_observerSet.begin(), obsEnd = m_observerSet.end(); obs != obsEnd; obs++)
        {
            (*obs)->move( source, m_oldPool?m_oldPool:m_pool, destination, m_pool, data);
        }

        *destination = data;
    }
    
    void move( const Iterator& source, const Iterator& destination)
    {
        class SourceMonitor: public PackedMemoryArray<dataType>::Observer
        {
            public:
                SourceMonitor( dataType* source):m_source(source),m_destination(source)
                {
                }
                
                dataType* getDestination()
                {
                    return m_destination;
                }
                
	            void move( dataType* source, dataType* sourcePool, dataType* destination, dataType* destinationPool, const dataType& data)
	            {
	                if( source == m_source)
                        m_destination = destination;
	            }
	            
	        private:
                dataType* m_source;
                dataType* m_destination;
        };
        SourceMonitor sourceMonitor( source.getAddress());
        registerObserver(&sourceMonitor);
        insert( destination, *source);
        source.reset( this, sourceMonitor.getDestination());
        unregisterObserver(&sourceMonitor);        
        erase(source);
    }
    
    dataType& operator[] ( SizeType elementIndex )
    {
        assert( elementIndex < m_numElements);
        return * ( begin() + elementIndex);
    }
	
	const dataType& operator[] ( SizeType elementIndex ) const
    {
        assert( elementIndex < m_numElements);
		return  *( begin() + elementIndex);
    }

	Iterator optimalInsert( const dataType& data )
	{
		m_auxNode = m_helper.findEmptiestNode();
		Iterator it = atIndex( m_auxNode.getHorizontalIndex() * m_helper.getCapacity(m_auxNode));
		return insert( it, data);
	}

    void printDot( std::ostream& out, const std::string& name = "", const std::string& next = "")
    {
        //out << "digraph BST {\n\tnode [fontname=\"Arial\"]\n";
        out << "\nsubgraph cluster_" << name << "{\ncolor=blue\n";
        m_helper.printDot( out, name);

        for( SizeType j = 0; j < m_poolSize; j += m_bucketSize)
        {
            out << name << "_bucket" << j << "[ shape = \"record\", label = \"";
            for( SizeType i = j; i < j + m_bucketSize; ++i)
            {                
                out << "{" << i << "-" << m_pool + i << "|";
                //out << "{" << i << "|";                
                if( m_pool[ i] != getEmptyElement())
                {
                    out << m_pool[ i];
                }
                out << "}|";
            }
            out << "\"]\n";
        
            m_auxNode = m_helper.getNodeOverIndex( j);
            out << name << m_auxNode.getPoolIndex() << " -> " << name << "_bucket" << j << "\n";
            /*if( j+m_bucketSize < m_poolSize)
            {
                out << name << "_bucket" << j << " -> " << name << "_bucket" << j+m_bucketSize << "\n";
            }*/
        }
        out << "}\n";

        if( !next.empty())
        {
            for( SizeType j = 0; j < m_poolSize; j += m_bucketSize)
            {
                out << name << "_bucket" << j <<" -> " << next << "0 [color=white]\n";
            }
        }

    }
    
    void printDotRange( std::ostream& out, const Iterator& first, const Iterator& last, const std::string& name = "", const std::string& next = "")
    {
        //out << "digraph BST {\n\tnode [fontname=\"Arial\"]\n";
        out << "\nsubgraph cluster_" << name << "{\ncolor=blue\n";
        
        out << name << "[ shape = \"record\", label = \"";
        dataType* ptr = first.getAddress();
        dataType* lastPtr;
        Iterator end = last;
        ++end;
        end == m_end?lastPtr = m_pool+m_poolSize:lastPtr = end.getAddress(); 
        while( ptr < lastPtr)
        {                
            out << "{" << ptr << "|";
            if( *ptr != getEmptyElement())
            {
                out << *ptr;
            }
            out << "}|";
            ++ptr;
        }
        out << "\"]\n";
        out << "}\n";
        
        if( !next.empty())
        {
            out << name << " -> " << next <<"\n";
        }
    }

    void push_back( const dataType& data)
    {
        resetObservers();
        if( !m_helper.affordsInsertion())
        {
            doubleArraySize();         
        }

        m_auxNode = m_helper.getNodeOverIndex( m_poolSize - 1);
        
        if( !m_helper.affordsElementInsertionAt(m_auxNode))
        {
            TreeNode sparseNode = m_auxNode;
            m_auxNode = m_helper.getParentForInsertion( m_auxNode);
            InlineStream stream( this);
            m_helper.rearrangeOver( m_auxNode, sparseNode, stream);
            resetObservers();
            m_auxIter.reset( m_pool + m_newIteratorIndex);
            m_auxNode = m_helper.getNodeOverIndex( m_poolSize - 1);
        }
       
        m_auxBucket.setAt( m_poolSize - 1);
        m_auxBucket.push_back ( data);
        ++m_numElements;

        m_helper.increaseCardinality( m_auxNode);
    }

    void registerObserver( Observer* observer)
    {
        m_observerSet.insert( observer);
    }
    
    void reserve( SizeType numElements)
    {
        
        m_poolSize = nextPowerOf2( numElements);
        m_bucketSize = nextPowerOf2( floorLog2( m_poolSize));

		if( 64 % sizeof( dataType) == 0)
		{
			m_bucketSize = 64;
		} 

        m_oldPool = m_pool;

        m_maxFullnessPercentage = double( (numElements * 100 / m_poolSize) + 1)/100;
               
        //m_maxFullnessPercentage = 0.8; 
        std::cout << "Density:\t" << m_maxFullnessPercentage << "\n";

        //std::cout << "Doubling Array at "<< m_numElements <<"\n";
        init();

        CopyingStream stream( this, m_oldPool);
        m_auxNode = m_helper.getRoot();
        m_auxNode->m_cardinality = m_numElements;
        m_helper.rearrangeOver( m_auxNode, stream);

        resetObservers(); 
        m_auxIter.reset( m_pool + m_newIteratorIndex);
        delete[]    m_oldPool;
        m_oldPool = 0;
    }

    void resetObservers()
    {
        typename std::set<Observer*>::iterator obs, obsEnd;
        for( obs = m_observerSet.begin(), obsEnd = m_observerSet.end(); obs != obsEnd; obs++)
        {
            (*obs)->reset();
        }
    }
    
    SizeType size() const
    {
        return m_numElements;
    }

    void unregisterObserver( Observer* observer)
    {
        typename std::set<Observer*>::iterator pos = m_observerSet.find( observer);
        if( pos != m_observerSet.end())
        {
            m_observerSet.erase(pos);
        }
    }    
    
private:
    double                  m_minEmptinessPercentage;
    double                  m_maxFullnessPercentage;
    dataType*               m_pool;
    dataType                m_emptyElement;    
    SizeType                m_numElements;
    SizeType                m_poolSize;
    SizeType                m_bucketSize;
    Iterator                m_auxIter;
    Iterator                m_end;
    Bucket                  m_auxBucket;
    Helper                  m_helper;
    TreeNode                m_auxNode;
    SizeType                m_newIteratorIndex;
    std::set<Observer*>     m_observerSet;    
    

    SizeType                m_bucketMask;
    dataType*               m_oldPool;
    MersenneTwister         m_gen;

    void doubleArraySize()
    {
        m_poolSize <<= 1;
        m_bucketSize = nextPowerOf2( floorLog2( m_poolSize));
        m_oldPool = m_pool;

        //std::cout << "Doubling Array at "<< m_numElements <<"\n";
        init();
        CopyingStream stream( this, m_oldPool);
        m_helper.rearrangeOver( m_auxNode, stream);
        resetObservers(); 
        m_auxIter.reset( m_pool + m_newIteratorIndex);
        delete[]    m_oldPool;
        m_oldPool = 0;
    }

    void halveArraySize()
    {     
        if( m_poolSize > 2) 
        {   
            m_poolSize >>= 1;
            m_bucketSize = nextPowerOf2( floorLog2( m_poolSize));
        }
        else
        {
            m_poolSize = 1;
            m_bucketSize = m_poolSize;
        }
        m_oldPool = m_pool;
        
        //std::cout << "Halving Array at " << m_numElements << "\n";
        init();
        CopyingStream stream( this, m_oldPool);
        m_helper.rearrangeOver( m_auxNode, stream); 
        resetObservers();
        m_auxIter.reset( m_pool + m_newIteratorIndex);
        delete[]    m_oldPool;
        m_oldPool = 0;
    }

    void init()
    {
        assert( isPowerOf2( m_poolSize));
        //std::cout << "Pool size is " << m_poolSize << ", Bucket size is " << m_bucketSize << "\n";
        m_bucketMask = ~(m_bucketSize - 1);
        assert( m_bucketSize);
        m_pool = new dataType[ m_poolSize];
        //m_auxIter = Iterator( this, m_pool);
        m_end = Iterator( this, m_pool + m_poolSize);
        m_helper.reset( floorLog2(m_poolSize / m_bucketSize) , m_bucketSize, m_numElements, m_minEmptinessPercentage, m_maxFullnessPercentage);
        m_auxNode = m_helper.getRoot();
        m_auxBucket.setContainer( this);
        m_auxBucket.reset();
    }
};


template <typename dataType>
class PackedMemoryArray<dataType>::Iterator
{
    friend class PackedMemoryArray;

    Iterator( PackedMemoryArray* PMA, dataType* ptr) : 
            m_PMA( PMA), 
            m_ptr(ptr)
    {
        //assert( isValid());
        //assert( *m_ptr != m_PMA->m_emptyElement);
    }

public:
    Iterator():m_PMA(0)
    {    
    }

    Iterator( const Iterator& other):m_PMA(other.m_PMA),m_ptr(other.m_ptr)
    {
    }

    ~Iterator()
    {
    }

/*
    inline void align()
    {
        assert( isValid() || *this == m_PMA->m_end);
        if( *this != m_PMA->m_end)
        {
            if( (*m_ptr) != m_PMA->m_emptyElement )
            {
                return;
            }
            do
            {
                m_ptr += m_PMA->m_bucketSize - modulusPow2( m_ptr - m_PMA->m_pool, m_PMA->m_bucketSize);
            }while( (*this != m_PMA->m_end) && ( (*m_ptr) == m_PMA->m_emptyElement ) );
        } 
    }
*/

    dataType* getAddress() const
    {
        return m_ptr;
    }

/*
    bool isAtBucket() const
    {
        return ( (m_ptr-m_PMA->m_pool != m_PMA->m_poolSize) && (modulusPow2( m_ptr-m_PMA->m_pool, m_PMA->m_bucketSize) == 0));
    }

    bool isIteratorOf( const PackedMemoryArray* otherPMA) const
    {
        return m_PMA == otherPMA;
    }
*/

    SizeType getMemoryUsage()
    {
        return sizeof(dataType*) + sizeof(PackedMemoryArray*);
    }

    bool isEmpty() const
    {
        return *m_ptr == m_PMA->m_emptyElement;
    }

    inline bool operator==( const Iterator& other) const
    {
        return ( other.m_ptr == m_ptr);
    }

    inline bool operator!=( const Iterator& other) const
    {
        return !( other.m_ptr == m_ptr);
    }

    inline bool operator < ( const Iterator& other) const
    {
        return m_ptr < other.m_ptr;
    }

    inline bool operator > ( const Iterator& other) const
    {
        return m_ptr > other.m_ptr;
    }
    
    Iterator& operator--() // prefix
    {
        if( m_ptr == m_PMA->m_pool)
        {
            return *this;
        }
        
        --m_ptr;

        while( (m_ptr != m_PMA->m_pool) && ( (*m_ptr) == m_PMA->m_emptyElement ) )
        {
            m_ptr -= 1;
        }
        return *this;
    }
   

    Iterator operator--(int unused) // postfix
    {
        Iterator result = *this;
        --(*this);
        return result;
    }
    

    Iterator& operator++() // prefix
    {
        ++m_ptr;

        if( (*this == m_PMA->m_end) || (*m_ptr) != m_PMA->m_emptyElement)
        {
            return *this;
        }

        m_ptr = m_PMA->m_pool + m_PMA->m_bucketSize + ( (m_ptr - m_PMA->m_pool) & m_PMA->m_bucketMask);

        assert( (*this == m_PMA->m_end) ||  (*m_ptr) != m_PMA->m_emptyElement);

        return *this;

        /*if( *this != m_PMA->m_end)
        {
            m_ptr = m_PMA->m_pool + m_PMA->m_bucketSize + ( (m_ptr - m_PMA->m_pool) & m_PMA->m_bucketMask);
            while( (*this != m_PMA->m_end) && ( (*m_ptr) == m_PMA->m_emptyElement ) )
            {
                m_ptr += m_PMA->m_bucketSize;
            };
            return *this;
        }

        return *this;*/
    }
    
    Iterator operator++(int unused) // postfix
    {
        Iterator result = *this;
        ++(*this);
        return result;
    }
    
    Iterator operator+( const SizeType& step) // advance
    {
        if( step)
        {
            SizeType elementIndex = step + m_PMA->getElementIndexOf( *this);
            assert( elementIndex < m_PMA->m_numElements);
            if( elementIndex < m_PMA->m_numElements)
            {
                return Iterator( m_PMA, m_PMA->m_pool + m_PMA->m_helper.findIndexContainingElement( elementIndex));
            }
            return m_PMA->m_end;
        }
        return *this;
    }
    
    SizeType operator - ( const Iterator& other) 
    {
        return m_PMA->getElementIndexOf(*this) - m_PMA->getElementIndexOf( other);
    }
    
    dataType& operator*() const {
        assert( (*m_ptr) != m_PMA->m_emptyElement);
        return *m_ptr;
    }

	dataType* operator->() const {
        //assert( (*m_ptr) != m_PMA->m_emptyElement);
		return m_ptr;
	}

    void reset( dataType* ptr)
    {
        m_ptr = ptr; 
    }

    void sanitize()
    {
        if( *this == m_PMA->m_end) return;
        if( (*this != m_PMA->m_end) && ((*m_ptr) == m_PMA->m_emptyElement))
        {
            m_ptr = m_PMA->m_pool + m_PMA->m_bucketSize + ( (m_ptr - m_PMA->m_pool) & m_PMA->m_bucketMask);
            assert( (*this == m_PMA->m_end) ||  (*m_ptr) != m_PMA->m_emptyElement);
        }
    }

private:
    PackedMemoryArray* m_PMA;
    dataType* m_ptr;
};




template <typename dataType>
class PackedMemoryArray<dataType>::Bucket
{
public:
    Bucket():m_PMA(0),m_begin(0),m_end(0),m_head(0){}

    Bucket( PackedMemoryArray* PMA): m_PMA(PMA), 
                                            m_begin( m_PMA->m_pool), 
                                            m_end( m_begin + PMA->getBucketSize()),
                                            m_head( m_begin)
    {
    }

    void erase( const Iterator& it) 
    {
        assert(m_PMA);
        m_head = m_PMA->m_pool + m_PMA->getPoolIndexOf(it);
        assert( m_head >= m_begin && m_head < m_end);
        *m_head = m_PMA->getEmptyElement();    
        ++m_head;  
        while( (m_head != m_end) && ( *m_head != m_PMA->getEmptyElement()) )
        {
            m_PMA->move( m_head, m_head - 1, *m_head);
            ++m_head;
        }
        *(m_head-1) = m_PMA->getEmptyElement();
    }

    typename PackedMemoryArray<dataType>::Iterator getIterator()
    {
        return m_PMA->atAddress(m_head);
    }

    void insert( const Iterator& it, const dataType& data) 
    {
        assert(m_PMA);
        dataType temp;
        m_head = m_PMA->m_pool + m_PMA->getPoolIndexOf(it);
        assert( m_head >= m_begin && m_head < m_end);

        //Find last element
        --m_end;
        while( *m_end == m_PMA->getEmptyElement() && m_end != m_head)
        {
            --m_end;
        }    

        //Shift elements to the right
        while( m_end != m_head)
        {
            m_PMA->move( m_end, m_end + 1, *m_end);
            --m_end;
        }

        if( *m_end != m_PMA->getEmptyElement())
        {
            m_PMA->move(m_end, m_end + 1, *m_end);
        }
        *m_head = data;
        m_end = m_begin + m_PMA->getBucketSize();      
    }
        
    SizeType getMemoryUsage() const
    {
        return 3 * sizeof(dataType*) + sizeof( PackedMemoryArray*);
    }

    void push_back( const dataType& data)
    {
        assert(m_PMA);
        m_head = m_end - 1;
        while( (m_head != m_begin))
        {
            if ( *m_head != m_PMA->getEmptyElement())
            {
                break;
            }
            --m_head;        
        }

        if( *m_head != m_PMA->getEmptyElement())
        {
            ++m_head;
        }

        assert( m_head != m_end);
        *m_head = data;
    }

    void reset( SizeType index = 0)
    {
        assert(m_PMA);
        m_begin = m_PMA->m_pool + index * m_PMA->getBucketSize();
        m_end = m_begin + m_PMA->getBucketSize();
        m_head = m_begin;
    }

    void setAt( const SizeType& position)
    {
        m_begin = m_PMA->m_pool + position - modulusPow2( position, m_PMA->getBucketSize());
        m_end = m_begin + m_PMA->getBucketSize();
        m_head = m_begin;
    }

    void setContainer( PackedMemoryArray* PMA)
    {
        m_PMA = PMA;
    }

private:
    PackedMemoryArray*  m_PMA;
    dataType*           m_begin;
    dataType*           m_end;
    dataType*           m_head;
};





template <typename dataType>
class PackedMemoryArray<dataType>::InlineStream
{
public:
    InlineStream( PackedMemoryArray* PMA):m_PMA(PMA)
    {
        setAt(0);
    }
    
    void setAt( const SizeType& writehead)
    {
        m_readhead = m_PMA->m_pool + writehead;
        m_writehead = m_readhead;
        m_Q.clear();
    }
 
    void emptyOut( PackedMemoryArray<dataType>::SizeType n)
    {
        while ( n > 0)
        {
            while( m_readhead <= m_writehead)
            {
                if( *m_readhead != m_PMA->getEmptyElement())
                {
                    m_Q.push_back( std::pair< dataType, dataType*>( *m_readhead, m_readhead));
                    *m_readhead = m_PMA->getEmptyElement();
                }
                ++m_readhead;
            } 
            ++m_writehead;
            --n;
        }
    }

    void writeOut( PackedMemoryArray<dataType>::SizeType n)
    {
        while ( n > 0)
        {
            while( m_readhead <= m_writehead)
            {
                assert( m_readhead < m_PMA->m_pool + m_PMA->m_poolSize);
                if( *m_readhead != m_PMA->getEmptyElement())
                {
                    m_Q.push_back( std::pair< dataType, dataType*>( *m_readhead, m_readhead));
                    *m_readhead = m_PMA->getEmptyElement();
                }
                ++m_readhead;
            }
            if( m_Q.empty())
            {
                assert( m_readhead < m_PMA->m_pool + m_PMA->m_poolSize);
                while( *m_readhead == m_PMA->getEmptyElement()) 
                { 
                    ++m_readhead;
                }
                if( m_readhead != m_writehead)
                {
                    m_PMA->move( m_readhead, m_writehead, *m_readhead);
                    *m_readhead = m_PMA->getEmptyElement();
                }
                ++m_readhead;
            }
            else
            {
                std::pair<dataType,dataType*> p = m_Q.front();
	            m_PMA->move( p.second, m_writehead, p.first);
                m_Q.pop_front();
            }  
            ++m_writehead;
            --n;        
        }
    }
private: 
    std::deque< std::pair< dataType, dataType*> > m_Q;
    PackedMemoryArray* m_PMA;
    dataType* m_readhead;
    dataType* m_writehead;
};


template <typename dataType>
class PackedMemoryArray<dataType>::ConstantStream
{
public:
    ConstantStream( PackedMemoryArray* PMA , const dataType& value):m_PMA(PMA),m_constantValue(value)
    {
        setAt(0);
    }
    
    void setAt( const SizeType& writehead)
    {
        m_writehead = m_PMA->m_pool + writehead;
    }
 
    void emptyOut( PackedMemoryArray<dataType>::SizeType n) 
    {
        while ( n > 0)
        {  
            *m_writehead = m_PMA->getEmptyElement();
            ++m_writehead;
            --n;        
        }
    }

    void writeOut( PackedMemoryArray<dataType>::SizeType n)
    {
        while ( n > 0)
        {  
            *m_writehead = m_constantValue;
            ++m_writehead;
            --n;        
        }
    }
private: 
    PackedMemoryArray* m_PMA;
    dataType* m_writehead;
    dataType  m_constantValue;
};




template <typename dataType>
class PackedMemoryArray<dataType>::CopyingStream
{
public:
    CopyingStream( PackedMemoryArray* PMA , dataType* source):m_PMA(PMA),m_source(source)
    {
        setAt(0);
    }
    
    void setAt( const SizeType& writehead)
    {
        m_writehead = m_PMA->m_pool + writehead;
    }
 
    void emptyOut( PackedMemoryArray<dataType>::SizeType n) 
    {
        while ( n > 0)
        {  
            assert( m_writehead < m_PMA->m_pool + m_PMA->m_poolSize);
            *m_writehead = m_PMA->getEmptyElement();
            ++m_writehead;
            --n;        
        }
    }

    void writeOut( PackedMemoryArray<dataType>::SizeType n)
    {
        while ( n > 0)
        {  
            while( *m_source == m_PMA->getEmptyElement()) 
            { 
                ++m_source;
            }
            m_PMA->move( m_source, m_writehead, *m_source);
            ++m_writehead;
            ++m_source;
            --n;        
        }
    }

private: 
    PackedMemoryArray* m_PMA;
    dataType* m_writehead;
    dataType* m_source;
};


template< typename dataType>
class PackedMemoryArray<dataType>::Observer
{
public:
	virtual void move( dataType* source, dataType* sourcePool, dataType* destination, dataType* destinationPool, const dataType& data) {}
	virtual void remove( PackedMemoryArray::SizeType source) {}
	virtual void reset() {}
};




#endif //PACKEDMEMORYARRAY_H
