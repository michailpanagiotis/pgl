#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <Structs/Trees/completeBinaryTree.h>
#include <limits>

typedef unsigned int PQSizeType;

template <typename KeyType, typename DataType>
class HeapItem
{
public:
    typedef PQSizeType* DescriptorType;
    
    HeapItem( unsigned int init = 0):key(std::numeric_limits<KeyType>::max()),data(),ptr()
    {
    }
    HeapItem( const KeyType& k, const DataType& d, const DescriptorType& p):key(k),data(d),ptr(p)
    {
    }
    
    const KeyType& getKey() const
    {
        return key;
    }
    
    const DataType& getData() const
    {
        return data;
    }
    
    HeapItem operator = ( const HeapItem& other)
    {
        if( this != &other)
        {
            key = other.key;
            data = other.data;
            ptr = other.ptr;
        }
        return *this;
    }
    
    void swapWith( HeapItem& other)
    {
        std::swap(this->key, other.key);    
        std::swap(this->data, other.data);
        std::swap( (this->ptr), other.ptr);
    }
    
    KeyType key;
    DataType data;
    DescriptorType ptr;
    //HeapItem* parent;
    //HeapItem* leftChild;
    //HeapItem* rightChild;
};

/*template <typename KeyType, typename DataType>
void std::swap( HeapItem<KeyType,DataType>& a, HeapItem<KeyType,DataType>& b )
{
    std::cout << "Hi!\n";
}*/


template <typename KeyType, typename DataType, template <typename datatype> class StorageType>
class PriorityQueue
{

public:
    typedef PQSizeType SizeType;
    typedef PQSizeType* DescriptorType;
    typedef CompleteBinaryTree< HeapItem<KeyType,DataType>, StorageType> TreeType;
    typedef HeapItem<KeyType,DataType> PQItem;
    typedef typename TreeType::Node Node;

	class PQVisitor
	{
	public:
		PQVisitor()
		{
		}
	
		virtual void visit( Node& u)
		{
		}
	};

    PriorityQueue():m_numItems(0)
    {
        m_auxNode = m_T.getRootNode();
        m_lastNode = m_T.getRootNode();
    }
    
    ~PriorityQueue()
    {
    }
    
    void clear()
    {
        while( !empty())
            popMin();
    }
    
    bool contains( const DescriptorType ptr)
    {
        return ptr && ((*ptr) <= m_numItems);
    }
    
    void decrease( const KeyType& key, const DescriptorType ptr)
    {
        if( ptr)
        {
            assert( (*ptr) <= m_numItems);
            m_auxNode.setAtBfsIndex( *ptr);
            m_auxNode->key = key;
            upheap(m_auxNode);
        }
    }
    
    bool empty() const
    {
        return m_numItems == 0;
    }

    /**
     * @brief Insert a key-value pair to the priority queue
     * @param key The key of the new element
     * @param data The assorted data for the new element
     * @param ptr An address that holds a pointer to track the element in the queue. This address must be malloced/freed externally. If it is not given, 
     * you will not be able to decrease the key of an element once it is in the queue.
     */
    void insert( const KeyType& key, const DataType& data, const DescriptorType ptr = 0)
    {
        increaseSize();
        m_auxNode.setAtBfsIndex( lastItemBfsIndex());
        m_auxNode->key = key;
        m_auxNode->data = data;
        m_auxNode->ptr = ptr;
        if( ptr)
        {
            *ptr = m_numItems;
        }
        upheap(m_auxNode);
        assert( (!ptr) || (*(m_auxNode->ptr) == m_auxNode.getBfsIndex()) );
    }
    
	bool isMember( const DescriptorType ptr)
    {
		return bool(ptr) && ( (*ptr) <= m_numItems );
	}

    const PQItem& min()
    {   
        m_auxNode.setAtRoot();
        return *m_auxNode;
    }

    const KeyType& minKey()
    {
        m_auxNode.setAtRoot();
        return m_auxNode->key;
    }

    const DataType& minItem()
    {   
        return min().data;
    }

	const KeyType& getKey( const DescriptorType ptr)
    {
        assert( (ptr) && ((*ptr) <= m_numItems));
        m_auxNode.setAtBfsIndex( *ptr);
        return m_auxNode->key;
    }

    const DataType& getItem( const DescriptorType ptr)
    {   
        assert( (ptr) && ((*ptr) <= m_numItems));
        m_auxNode.setAtBfsIndex( *ptr);
        return m_auxNode->data;
    }
    
	void popMin()
    {		
        assert( m_numItems > 0);
        m_auxNode.setAtRoot();
        
        certainDownheap( m_auxNode);
        m_lastNode.setAtBfsIndex( lastItemBfsIndex() );
        if( m_auxNode != m_lastNode)
        {
            swap( m_auxNode, m_lastNode);
            upheap( m_auxNode);
        }
        if( m_lastNode->ptr)
            *(m_lastNode->ptr) = std::numeric_limits<PQSizeType>::max();
        decreaseSize();
    }
    
    /*void printGraphviz( const std::string& filename)
    {
        std::ofstream out( filename.c_str());
        m_T.printGraphviz(out);
        out.close();
    }*/

    void printGraphviz( const std::string& filename)
    {        
        std::ofstream out( filename.c_str());
        out << "digraph BFS {\n\tedge [len=3]\n\tnode  [fontname=\"Arial\"]\n";
                
        std::stack<Node> S;
        m_auxNode.setAtRoot();
        S.push(m_auxNode);
        while( !S.empty())
        {
            m_auxNode = S.top();
            S.pop();      	 
            out << m_auxNode.getPoolIndex();
            out << "[shape=record,label=\"{";
		    out << m_auxNode.getPoolIndex() ;//<< "|";
            //out <<  m_auxNode->key << "|";
            //out <<  (m_auxNode->ptr);
	        out << "}\"]\n";
            out << "\t"; 
            if( !m_auxNode.isLeaf())
            {
                out <<  m_auxNode.getPoolIndex();    
                out << " -> ";  
                m_auxNode.goLeft();
                out <<  m_auxNode.getPoolIndex();    
                out << "\n"; 
                S.push(m_auxNode);
                m_auxNode.goUp();
                out << "\t";        
                out <<  m_auxNode.getPoolIndex(); 
                out << " -> ";     
                m_auxNode.goRight();
                out <<  m_auxNode.getPoolIndex();  
                S.push(m_auxNode);
            }
            out << "\n";
        }
        out << "}";
        out.close();
    }
    
	void remove( const DescriptorType ptr)
    {
        if( ptr)
        {
            assert( (*ptr) <= m_numItems);
			assert( m_numItems > 0);
        	
			m_auxNode.setAtBfsIndex( *ptr);
        
        	certainDownheap( m_auxNode);
        	m_lastNode.setAtBfsIndex( lastItemBfsIndex() );
        	if( m_auxNode != m_lastNode)
        	{
            	swap( m_auxNode, m_lastNode);
            	upheap( m_auxNode);
        	}
        	if( m_lastNode->ptr)
            	*(m_lastNode->ptr) = std::numeric_limits<PQSizeType>::max();
        	decreaseSize();
		}
	}

    const SizeType& size()
    {
        return m_numItems;
    }

    void update( const KeyType& key, const DescriptorType ptr)
    {
        if( ptr)
        {
            assert( (*ptr) <= m_numItems);
            m_auxNode.setAtBfsIndex( *ptr);

            if( key < m_auxNode->key)
            {
                m_auxNode->key = key;
                upheap(m_auxNode);
            }

            if( key > m_auxNode->key)
            {
                m_auxNode->key = key;
                downheap(m_auxNode);
            }
        }
    }

	void visit( PQVisitor* vis)
    {        
        std::stack<Node> S;
        m_auxNode.setAtRoot();
        S.push(m_auxNode);
        while( !S.empty())
        {
            m_auxNode = S.top();
            S.pop();      	 
			vis->visit( m_auxNode);            

            if( !m_auxNode.isLeaf())
            {               
                m_auxNode.goLeft();
                S.push(m_auxNode);
                m_auxNode.goUp();                   
                m_auxNode.goRight();
                S.push(m_auxNode);
            }
        }
    }

    TreeType m_T;
    
private:
    
    SizeType m_numItems;
    enum pos { PARENT, LEFT, RIGHT} m_minNodePos;
    KeyType m_minKey;
    Node m_auxNode, m_parentNode, m_lastNode, m_left, m_right;
    PQItem m_tempItem;
    
    void certainDownheap( Node& u)
    {
        while( !u.isLeaf())
        {
            m_left = u;
            m_left.goLeft();
            m_right = u;
            m_right.goRight();
            if( !isInHeap( m_left)) return;
            
            if( !isInHeap( m_right))
            {
                swap( u, m_left);
                u = m_left;
                return;
            }
            
            if( m_left->key < m_right->key)
            {
                swap( u, m_left);
                u = m_left;
            }
            else
            {
                swap( u, m_right);
                u = m_right;
            }
        }
    }
    
    void decreaseSize()
    {
        if( m_numItems < 9)
        {
            --m_numItems;
            return;
        }
        
        if( isPowerOf2( m_numItems))
        {
            m_T.decreaseHeight();
        }
        --m_numItems;
    }
    
    void downheap( Node& u)
    {
        while( !u.isLeaf())
        {
            m_minKey = u->key;
            m_minNodePos = PARENT;
            m_left = u;
            m_left.goLeft();
            if( isInHeap(m_left) && ( m_left->key < m_minKey) )
            {
                m_minKey = m_left->key;
                m_minNodePos = LEFT;
            }   
            
            m_right = u;
            m_right.goRight();
            if( isInHeap(m_right) && ( m_right->key < m_minKey) )
            {
                m_minKey = m_right->key;
                m_minNodePos = RIGHT;
            }
        
            if( m_minNodePos == PARENT) return;
            
            if( m_minNodePos == LEFT)
            {
                swap( u, m_left);
                u = m_left;
            }
            else
            {
                swap( u, m_right);
                u = m_right;
            }
        }
    }
    
    void increaseSize()
    {
        ++m_numItems;
        
        //(  isPowerOf2( m_numItems))
        if( m_numItems > m_T.getNumNodes())
        {
            //std::cout << "Increasing to " << m_numItems << std::endl;
            m_T.increaseHeight();
        }
    }
    
    bool isInHeap( const Node& u) const
    {
        return u.getBfsIndex() <= lastItemBfsIndex(); 
    }
    
    const SizeType& lastItemBfsIndex() const
    {
        return m_numItems;
    }
    
    void swap( Node& u, Node& v)
    {
        //m_tempItem = *v;
        //*v = *u;
        //*u = m_tempItem;

        //std::swap(*u,*v);
        
        u->swapWith(*v);
        
        if( u->ptr) *(u->ptr) = u.getBfsIndex();
        if( v->ptr) *(v->ptr) = v.getBfsIndex();

        //*(u->ptr) = u.getBfsIndex();
        //*(v->ptr) = v.getBfsIndex();
    }
    
    void upheap( Node& u)
    {
        m_parentNode = u;
        while( !u.isRoot())
        {
            m_parentNode.goUp();
            if( m_parentNode->key > u->key)
            {
                swap( u, m_parentNode);
                u = m_parentNode;
            }
            else
            {
                return;
            }
        }
    }
    	
};


#endif //PRIORITY_QUEUE_H
