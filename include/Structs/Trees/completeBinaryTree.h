#ifndef COMPLETEBINARYTREE_H
#define COMPLETEBINARYTREE_H

#include <configuration.h>
#include <Utilities/binaryMath.h>
#include <Structs/Trees/treeStorageSchemes.h>
#include <assert.h>
#include <stack>
#include <fstream>

template <typename DataType>
class StatisticAnalyser
{
public:
    typedef unsigned int SizeType; 
    StatisticAnalyser() 
    {
        m_out.open( queueStatsFile.c_str(), std::fstream::out | std::fstream::app);
    }
    
    ~StatisticAnalyser()
    {
        m_out.close();
    }
    
    void recordJump(SizeType jump)
    {
        m_out << jump * sizeof(DataType) << "\n";
    }
    
private:
    std::ofstream m_out;
};


template <typename DataType, typename SizeType>
class CopyController
{
public:
    CopyController():bfsIndex(0),height(0),newHeight(0),ptr(0),newPtr(0)
    {   
    }
    
    CopyController( const SizeType& b, const SizeType& h, const SizeType& nh, DataType* p, DataType* np):
                    bfsIndex(b),height(h),newHeight(nh),ptr(p),newPtr(np)
    {   
    }
    
    CopyController operator = ( const CopyController& other)
    {
        if( this != &other)
        {
            bfsIndex = other.bfsIndex;
            height = other.height;
            newHeight = other.newHeight;
            ptr = other.ptr;
            newPtr = other.newPtr;
        }
        return *this;
    }
    
    void print()
    {
        ////std::cout << "Bfs: " << bfsIndex << "\tHeight: " << height << " -> " << newHeight << "\tAddress: " << ptr << " -> " << newPtr << std::endl;
    }
    
    void reset( const SizeType& b, const SizeType& h, const SizeType& nh, DataType* p, DataType* np)
    {
        bfsIndex = b;
        height = h;
        newHeight = nh;
        ptr = p;
        newPtr = np;
    }
    
    SizeType bfsIndex;
    SizeType height,newHeight;
    DataType* ptr;
    DataType* newPtr;
    
};



/**
 * @class CompleteBinaryTree
 *
 * @brief A complete binary tree
 *
 * @tparam DataType The data to associate with the nodes. It must be a class or struct
 * @tparam StorageStrategy The underlying storage implementation to use. Available implementations are 'HeapStorage', 'ExplicitHeapStorage', 'VebStorage', 'ExplicitVebStorage', 'ExplicitPowerVebStorage' and 'SplitStorage'
 * @author Panos Michail
 *
 */

template <typename DataType, template <typename datatype> class StorageStrategy = HeapStorage>
class CompleteBinaryTree
{

public:

    typedef unsigned int SizeType;
    typedef StorageStrategy<DataType> StorageScheme;

    class Node;

    /**
     * @brief Constructor
     * @param height The tree height ( maximum distance from root, i.e. a tree with
     * one node has height=0)
     * @param defaultValue The default value to initiallize the nodes' data
     */
    CompleteBinaryTree( const SizeType& height = 0, const DataType& defaultValue = DataType(0)):m_height(height),m_numNodes((1 << (m_height + 1)) - 1),m_defaultValue( defaultValue)
    {
        m_pool = new DataType[ m_numNodes ];
        m_storage = new StorageScheme( m_pool, height);

        for( SizeType i = 0; i < m_numNodes; i++)
        {   
            m_pool[i] = defaultValue;
        }
    }

    /**
     * @brief Destructor
     */
    ~CompleteBinaryTree()
    {
       delete[] m_pool;
       delete m_storage;
    }
    
    
    /**
     * @brief Remove the last level of the tree
     */
    void decreaseHeight()
    {
        //std::cout << "Halving\n";
        assert( m_height > 0);
        SizeType newHeight = m_height - 1;
        SizeType newNodes = pow2(newHeight + 1) - 1;
        ////std::cout << "Allocating " << toMb(newNodes) << "Mb\n";
        DataType* newPool = new DataType[ newNodes ];
        StorageScheme* newStorage = new StorageScheme( newPool, newHeight);
        for( SizeType i = 0; i < newNodes; i++)
        {   
            newPool[i] = m_defaultValue;
        }
        
        std::stack< CopyController<DataType,SizeType> > S;
        CopyController<DataType,SizeType> rightCtrl,leftCtrl;
        CopyController<DataType,SizeType> ctrl( 1, m_height, newHeight, m_pool, newPool);
        S.push( ctrl);
        while( !S.empty())
        {
            ctrl = S.top();
            S.pop();
            ctrl.print();
            *(ctrl.newPtr) = *(ctrl.ptr);
            if( (ctrl.bfsIndex << 1) <= newNodes )
            {
                rightCtrl.reset( (ctrl.bfsIndex << 1) + 1, ctrl.height - 1, ctrl.newHeight - 1,
                            m_storage->getRightChildAddress( ctrl.ptr, ctrl.bfsIndex, ctrl.height), 
                            newStorage->getRightChildAddress( ctrl.newPtr, ctrl.bfsIndex, ctrl.newHeight));
                S.push( rightCtrl);      
                 
                leftCtrl.reset( ctrl.bfsIndex << 1, ctrl.height - 1, ctrl.newHeight - 1,
                            m_storage->getLeftChildAddress( ctrl.ptr, ctrl.bfsIndex, ctrl.height), 
                            newStorage->getLeftChildAddress( ctrl.newPtr, ctrl.bfsIndex, ctrl.newHeight));
                S.push( leftCtrl);
            }
        }
        
        --m_height;
        
        ////std::cout << "Deallocating " << toMb(m_numNodes) << "Mb\n";
        m_numNodes = newNodes;
        delete [] m_pool;
        delete m_storage;
        
        m_pool = newPool;
        m_storage = newStorage;
    }

    /**
     * @brief Returns the height of the tree
     * @return The height of the tree
     */
    const SizeType& getHeight() const
    {
        return m_height;
    }
    
    SizeType getMemoryUsage()
    {
        SizeType sum = (m_numNodes + 1) * sizeof( DataType) + m_storage->getMemoryUsage() + 2 * sizeof( SizeType);
        return sum;
    }
    
    /**
     * @brief Returns the number of nodes of the tree
     * @return The number of nodes of the tree
     */
    const SizeType& getNumNodes() const
    {
        return m_numNodes;
    }
    
    /**
     * @brief Returns the root node
     * @return The root node
     */
    Node getRootNode() 
    {
        return Node(this,m_pool,1);
    }
    
    /**
     * @brief Returns a pointer to the root node
     * @return A pointer to the root node
     */
    Node* getRootPointer() const;

    /**
     * @brief Add one more level to the tree
     */
    void increaseHeight()
    {
        SizeType newHeight = m_height + 1;
        SizeType newNodes = pow2(newHeight + 1) - 1;
        
        ////std::cout << "Allocating " << toMb(newNodes * sizeof(DataType)) << "Mb for " << newNodes << " nodes\n";
        DataType* newPool = new DataType[ newNodes ];
        StorageScheme* newStorage = new StorageScheme( newPool, newHeight);
        for( SizeType i = 0; i < newNodes; i++)
        {   
            newPool[i] = m_defaultValue;
        }
        
        std::stack< CopyController<DataType,SizeType> > S;
        CopyController<DataType,SizeType> rightCtrl,leftCtrl;
        CopyController<DataType,SizeType> ctrl( 1, m_height, newHeight, m_pool, newPool);
        S.push( ctrl);
        while( !S.empty())
        {
            ctrl = S.top();
            S.pop();
            ctrl.print();
            *(ctrl.newPtr) = *(ctrl.ptr);
            if( (ctrl.bfsIndex << 1) <= m_numNodes )
            {
                rightCtrl.reset( (ctrl.bfsIndex << 1) + 1, ctrl.height - 1, ctrl.newHeight - 1,
                            m_storage->getRightChildAddress( ctrl.ptr, ctrl.bfsIndex, ctrl.height), 
                            newStorage->getRightChildAddress( ctrl.newPtr, ctrl.bfsIndex, ctrl.newHeight));
                S.push( rightCtrl);      
                 
                leftCtrl.reset( ctrl.bfsIndex << 1, ctrl.height - 1, ctrl.newHeight - 1,
                            m_storage->getLeftChildAddress( ctrl.ptr, ctrl.bfsIndex, ctrl.height), 
                            newStorage->getLeftChildAddress( ctrl.newPtr, ctrl.bfsIndex, ctrl.newHeight));
                S.push( leftCtrl);
            }
        }
        
        ++m_height;
        ////std::cout << "Dellocating " << toMb(m_numNodes) << "Mb\n";
        m_numNodes = newNodes;
        delete [] m_pool;
        delete m_storage;
        
        m_pool = newPool;
        m_storage = newStorage;
    }
    
    /**
     * @brief Prints the tree in Graphviz format
     * 
     * @param out The output stream
     * @param prefix An optional prefix to the node names in the Graphviz format  
     */
    void printGraphviz( std::ostream& out, const std::string& prefix = "")
    {
        if( m_height > 10)
        {
            SizeType size = (1 << m_height);
            std::cout << "The tree is too big to print (>" << size << ")\n";
            return;
        }
        
        out << "digraph BFS {\n\tedge [len=3]\n\tnode  [fontname=\"Arial\"]\n";

        Node m_auxNode( this, m_pool, 1);
                
        std::stack<Node> S;
        S.push(m_auxNode);
        while( !S.empty())
        {
            m_auxNode = S.top();
            S.pop();      	 
            out << prefix << m_auxNode.getPoolIndex();
            out << "[shape=record,label=\"{";
		    out << prefix << m_auxNode.getBfsIndex() << "|";
            out << prefix << m_auxNode.getPoolIndex();
	        out << "}\"]\n";
            out << "\t"; 
            if( !m_auxNode.isLeaf())
            {
                out << prefix << m_auxNode.getPoolIndex();    
                out << " -> ";  
                m_auxNode.goLeft();
                out << prefix << m_auxNode.getPoolIndex();    
                out << "\n"; 
                S.push(m_auxNode);
                m_auxNode.goUp();
                out << "\t";        
                out << prefix << m_auxNode.getPoolIndex(); 
                out << " -> ";     
                m_auxNode.goRight();
                out << prefix << m_auxNode.getPoolIndex();  
                S.push(m_auxNode);
            }
            out << "\n";
        }
        out << "}";
    }

protected:
    /**
     * @brief Array that represents the tree and stores the node data
     */
    DataType* m_pool;

    /**
     * @brief Array that represents the tree and stores the Node data
     */
    _QUEUESTATS(StatisticAnalyser<DataType> stats;)

    /**
     * @brief The tree height
     */
    SizeType m_height;
    
    /**
     * @brief The number of nodes in the tree
     */
    SizeType m_numNodes;
    
    /**
     * @brief The default data for nodes
     */
    DataType m_defaultValue;

    /**
     * @brief The storage scheme item
     */
    StorageScheme* m_storage;
};



/**
 * @class CompleteBinaryTree::Node 
 * 
 * @brief A node iterator of a CompleteBinaryTree
 *
 * @tparam DataType The data to associate with the nodes. It must be a class or struct
 * @tparam StorageStrategy The underlying storage implementation to use. Available implementations are 'HeapStorage', 'ExplicitHeapStorage', 'VebStorage', 'ExplicitVebStorage', 'ExplicitPowerVebStorage' and 'SplitStorage'
 * @author Panos Michail
 *
 */
template <typename DataType, template <typename datatype> class StorageStrategy>
class CompleteBinaryTree<DataType,StorageStrategy>::Node 
{
public:
    /**
     * @brief Default constructor
     */
    Node(): m_T(0),m_ptr(0),m_bfsIndex(0),m_depth(0)
    {
    }
    
    /**
     * @brief Default constructor
     *
     * @param T The tree the node lies upon
     * @param ptr A pointer to the data associated with this node
     * @param bfsIndex The index of the node in the Breadth First Search
     */
    Node( CompleteBinaryTree<DataType,StorageStrategy>* T, DataType* ptr, const SizeType bfsIndex): m_T(T), m_ptr(ptr), m_bfsIndex( bfsIndex), m_depth( floorLog2( bfsIndex))
    {   
    }
    
    /**
     * @brief Returns the address of the data of the node
     * @return A pointer to the data of the node
     */
    const DataType* getAddress() const
    {
        return m_ptr;
    }
    
    /**
     * @brief Returns the bfs index of a node ( root node has bfs index = 1)
     * @return The bfs index of the node
     */
    const SizeType& getBfsIndex() const
    {
        return m_bfsIndex;
    }
        
    /**
     * @brief Returns the depth of the node (its distance from root)
     * @return The depth of the node
     */
    const SizeType& getDepth() const
    {
        return m_depth;
    }

    /**
     * @brief Returns the height of the node (its distance from the leaves)
     * @return The height of the node
     */
    SizeType getHeight() const
    {
        return m_T->m_height - m_depth;
    }
    
    /**
     * @brief Returns the horizontal index of this node ( the index among the same level nodes)
     * @return The horizontal index of the node
     */
    SizeType getHorizontalIndex() const
    {
       return m_bfsIndex - pow2( getDepth());
    }
    
    /**
     * @brief Returns the number of bytes allocated to this node
     * @return The number of bytes allocated to this node
     */
    SizeType getMemoryUsage()
    {
        return 2*sizeof(SizeType) + sizeof(DataType*) + sizeof(CompleteBinaryTree<DataType,StorageStrategy>*);
    }
        
    /**
     * @brief Returns the pool index of the node ( the index of the array where it is stored)
     * @return The pool index of the node
     */
    SizeType getPoolIndex() const
    {
        return m_ptr - m_T->m_pool;
    }    
    
    /**
     * @brief Sets the node as its left child node
     * There is NO check if this node is already a leaf
     */
    void goLeft()
    {
        _QUEUESTATS( m_T->stats.recordJump( m_T->m_storage->getLeftChildAddress( m_ptr, m_bfsIndex, getHeight() ) - m_ptr);)
        m_ptr = m_T->m_storage->getLeftChildAddress( m_ptr, m_bfsIndex, getHeight() );
        m_bfsIndex = m_bfsIndex << 1;
        m_depth++;
    }
    
    /**
     * @brief Sets the node as its right child node
     * There is NO check if this node is already a leaf
     */
    void goRight()
    {
        _QUEUESTATS( m_T->stats.recordJump( m_T->m_storage->getRightChildAddress( m_ptr, m_bfsIndex, getHeight() ) - m_ptr);)
        m_ptr = m_T->m_storage->getRightChildAddress( m_ptr, m_bfsIndex, getHeight() );
        m_bfsIndex = (m_bfsIndex << 1) + 1;
        m_depth++;
    }
    
    /**
     * @brief Sets the node as its parent node
     * There is NO check if this node is already a leaf
     */
    void goUp()
    {
        _QUEUESTATS(m_T->stats.recordJump( m_ptr - m_T->m_storage->getParentAddress( m_ptr, m_bfsIndex, getHeight() ));)
        m_ptr = m_T->m_storage->getParentAddress( m_ptr, m_bfsIndex, getHeight() );
        m_bfsIndex = m_bfsIndex >> 1;
	    m_depth--;	
    }
        
    /**
     * @brief Checks if this Node is a leaf
     * @return True if this Node is a leaf, false otherwise
     */
    bool isLeaf() const
    {
        return m_depth == m_T->m_height;
    }

    /**
     * @brief Checks if this Node is a right child
     * @return True if this Node is a right child, false otherwise
     */
    bool isRightChild() const
    {
        return ( (m_bfsIndex & 1) == 1);
    }
    
    /**
     * @brief Checks if this leaf is to the left of another node 
     * @param other The node to compare to
	 * @return True the ancestor of this node on the level of the other node is to the left of the other node, false otherwise
     */
    bool isToTheLeftOf( const Node& other) const
    {
        assert( this->isLeaf());
        return ( (m_bfsIndex >> other.getHeight()) < other.m_bfsIndex);
    }
        
    /**
     * @brief Checks if this Node is root
     * @return True if this Node is root, false otherwise
     */
    bool isRoot() const
    {
        return !m_depth;
    }

    /**
     * @brief Assignment operator
     * @param The node to copy
	 * @return A reference to the newly assigned Node
     */
    Node& operator=(const Node& other)
    {
        if( this != &other)
        {       
            this->m_T = other.m_T;
            this->m_ptr = other.m_ptr;
            this->m_bfsIndex = other.m_bfsIndex;
            this->m_depth = other.m_depth;
        }
        return *this;
    }
    
    
    DataType& operator * ()
    {
        return *m_ptr;
    }

    DataType* operator -> ()
    {
        return m_ptr;
    }

    /**
     * @brief Equality operator
     * @param other The node to compare to
	 * @return True if the other node points to the same data as this node, false otherwise
     */
    bool operator == ( const Node& other) const
    {
        return (m_T == other.m_T) && (m_ptr == other.m_ptr);
    }
    
    /**
     * @brief Inequality operator
     * @param other The node to compare to
	 * @return True if the other node points to different data from this node, false otherwise
     */
    bool operator != ( const Node& other) const
    {
        return !(*this == other);
    }

    /**
     * @brief Sets the node at the given bfs index (root node has bfs index = 1)
     * @param bfsIndex The bfs index to set the node to
     */
    void setAtBfsIndex( const SizeType& bfsIndex)
    {
        m_ptr = m_T->m_storage->getBfsIndexAddress( bfsIndex);
        m_bfsIndex = bfsIndex;
        m_depth = floorLog2( bfsIndex);
    }

	/**
     * @brief Sets the Node at a given height and a given horizontal index
	 * @param height The new height of the node
	 * @param horizontalPosition The horizontal index of this node on this level
     */
	void setAtPos( const SizeType& height, const SizeType& horizontalPosition)
	{
        setAtBfsIndex( (1<<(m_T->m_height - height)) + horizontalPosition);
        return;
	    /*SizeType numLeaves = pow2( m_T->getHeight());
	    m_ptr = m_T->m_storage->getRootAddr();	
	    m_bfsIndex = 1;
	    m_depth = 0;
        unsigned int mask = numLeaves >> 1;
	    horizontalPosition <<= height;
	    while( getHeight() > height)
	    {
		    if( mask & horizontalPosition)
		    {
			    goRight();
		    }   
		    else
		    {
			    goLeft();
		    }
		    mask >>= 1;
	    }*/
    }

    /**
     * @brief Sets the node at the root node
     */
    void setAtRoot() 
    {
        m_bfsIndex = 1;
        m_depth = 0;
        m_ptr = m_T->m_storage->getRootAddr();
    }

protected:
    /**
     * @brief The binary tree this Node lies upon
     */
    CompleteBinaryTree<DataType,StorageStrategy>* m_T;  
    
    /**
     * @brief The address where the node's data lie upon
     */
    DataType* m_ptr;

    /**
     * @brief The index of the node in the node array using the Breadth First Search
     * layout indexing.
     */
    SizeType m_bfsIndex;

    /**
     * @brief The depth of the node (its distance from the root node)
     */
    SizeType m_depth;
};




#endif //COMPLETEBINARYTREE_H
