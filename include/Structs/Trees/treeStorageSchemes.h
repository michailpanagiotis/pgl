#ifndef TREESTORAGESCHEMES_H
#define TREESTORAGESCHEMES_H

#include <stack>
#include <assert.h>


template <typename DataType>
class HeapStorage
{
public:
    typedef PriorityQueueSizeType SizeType; 
    HeapStorage( DataType* pool, const SizeType& height):m_pool(pool)
    {
    }
    
    ~HeapStorage()
    {
    }
    
    DataType* getRootAddr() const
    {
        return m_pool;
    }
    
    DataType* getBfsIndexAddress( const SizeType& bfsIndex) 
    {
        return m_pool + bfsIndex - 1;
    }

    DataType* getLeftChildAddress( DataType* ptr, const SizeType& bfsIndex, const SizeType& height) 
    {
        return ptr + bfsIndex;
    }
    
    SizeType getMemoryUsage() const
    {
        return sizeof( DataType*);
    }
    
    DataType* getRightChildAddress( DataType* ptr, const SizeType& bfsIndex, const SizeType& height) 
    {
        return ptr + bfsIndex + 1;
    }
    
    DataType* getParentAddress( DataType* ptr, const SizeType& bfsIndex, const SizeType& height) 
    {
        return (bfsIndex & 1) ?  ptr - ((bfsIndex >> 1) + 1) : ptr - (bfsIndex >> 1); 
    }

private:
    DataType* m_pool;    
};


template <typename DataType>
class VebStorage
{
public:
    typedef PriorityQueueSizeType SizeType; 
    VebStorage( DataType* pool, const SizeType& height):m_pool(pool),m_height(height)
    {
        upperLevels = new SizeType[ height + 1];
        lowerLevels = new SizeType[ height + 1];
        treeSize = new SizeType[ height + 1];

        calculateTransitionValues( height);
    }
    
    ~VebStorage()
    {
        delete [] upperLevels;
        delete [] lowerLevels;
        delete [] treeSize;
    }
    
    DataType* getRootAddr() const
    {
        return m_pool;
    }
    
    DataType* getBfsIndexAddress( const SizeType& bfsIndex) 
    {
        return m_pool + getVebFromBfs( bfsIndex);
    }

    DataType* getLeftChildAddress( DataType* ptr, const SizeType& bfsIndex, const SizeType& height) 
    {
        if( upperLevels[height] == 1) return ptr + 1;  
        return ptr + getLeftStep( bfsIndex, height);
    }
    
    SizeType getMemoryUsage() const
    {
        return sizeof( DataType*);
    }
    
    DataType* getRightChildAddress( DataType* ptr, const SizeType& bfsIndex, const SizeType& height) 
    {
        if( upperLevels[height] == 1) return ptr + 2; 
        return ptr + getRightStep( bfsIndex, height);
    }
    
    DataType* getParentAddress( DataType* ptr, const SizeType& bfsIndex, const SizeType& height) 
    {
        if( upperLevels[height + 1] == 1)
        {
            return ptr - (1 + (bfsIndex & 1)); 
        }
        if( bfsIndex & 1)
        {
            return ptr - getRightStep( bfsIndex >> 1, height + 1);
        }
        else
        {
            return ptr - getLeftStep( bfsIndex >> 1, height + 1);
        }
    }
    
    const SizeType& getRightStep( const SizeType bfsIndex, const SizeType height) 
    {
        levels = upperLevels[height];
        childIndex = (bfsIndex << 1);
        siblings = (childIndex & treeSize[levels]) + 1;
        sum = (1 << levels) + ( (siblings) << lowerLevels[ height]) - (siblings << 1);
        
        while( levels > 1)
        {
            upper = (levels >> 1) + (levels & 1);
            levels = (levels >> 1);      
            fatherIndex = bfsIndex >> (levels - 1);
            sum += (fatherIndex & treeSize[upper]);
            sum -= treeSize[upper];
        }
        return sum;
    }

    const SizeType& getLeftStep(const SizeType& bfsIndex, const SizeType& height) 
    {
        levels = upperLevels[height];

        childIndex = (bfsIndex << 1);
        siblings = (childIndex & treeSize[levels]);
        sum = (1<<levels) - 1 + (siblings << lowerLevels[height]) - (siblings << 1);

        while( levels > 1)
        {
            upper = (levels >> 1) + (levels & 1);
            levels = (levels >> 1);      
            fatherIndex = bfsIndex >> (levels - 1);
            sum += (fatherIndex & treeSize[upper]);
            sum -= treeSize[upper];
        }   
        return sum;
    }
    
private:
    DataType* m_pool;
    
    SizeType* upperLevels;
    SizeType* lowerLevels;
    SizeType* treeSize;
    
    SizeType m_height;
    
    SizeType sum,upper,levels;
    SizeType siblings, fatherIndex, childIndex;

    
    void calculateTransitionValues( const SizeType& height)
    {              
        std::stack< std::pair<SizeType,SizeType> > S;

        S.push( std::pair<SizeType,SizeType>( 0, height + 1));
        SizeType subTreeLevels;
        SizeType middleLevel;
        std::pair<SizeType,SizeType> levelRange;

        //std::cout << "*" << std::endl;
        while( !S.empty())
        {
            levelRange = S.top();
            //std::cout << "Examining " << levelRange.first << "->" << levelRange.second ;
            S.pop();
            subTreeLevels = levelRange.second - levelRange.first;

            for( SizeType h = 0; h <= height; h++)
            {
                treeSize[ h] = (1<<h) - 1;  
            }

            if( subTreeLevels > 1)
            {
                //std::cout << "\tsetting";
                
                middleLevel = levelRange.first + (subTreeLevels >> 1);

                //middleLevel = levelRange.first + (1 << floorLog2(subTreeLevels - 1));

                //std::cout << "through " << middleLevel << "...";
                
                //std::cout << "\tLowerlevels" << middleLevel - levelRange.first;
                lowerLevels[ middleLevel] = middleLevel - levelRange.first;
                //std::cout << "\tUpperlevels" << levelRange.second - middleLevel;
                upperLevels[ middleLevel] = levelRange.second - middleLevel;
                
                S.push( std::pair<SizeType,SizeType>( middleLevel, levelRange.second));
                S.push( std::pair<SizeType,SizeType>( levelRange.first, middleLevel));   
            }
            //std::cout << std::endl;
        }
        
        upperLevels[height] = 1;
        //lowerLevels[height] = height;
        lowerLevels[0] = 0;
        upperLevels[0] = 1;

    }
    
    const SizeType& getVebFromBfs( const SizeType& bfsIndex)
    {
        sum = 0;
        if (bfsIndex == 1) return sum;
        
        childIndex = bfsIndex; 
        SizeType height = m_height - floorLog2( childIndex);
        
        while( childIndex != 1)
        {
            sum += ( childIndex & ((1 << upperLevels[height + 1]) - 1)) * treeSize[ lowerLevels[ height + 1]];
            sum += treeSize[ upperLevels[height + 1]];
            childIndex >>= upperLevels[height + 1];
            height += upperLevels[height + 1];
        }
        return sum;
    }
};




template <typename DataType>
class ExplicitNodePointers
{
public:
    DataType* leftChild;
    DataType* rightChild;
    DataType* parent;   
};


template <typename DataType>
class ExplicitHeapStorage
{
public:
    typedef PriorityQueueSizeType SizeType; 
    typedef ExplicitNodePointers<DataType> PointerType;
    
    ExplicitHeapStorage( DataType* pool, const SizeType& height):m_pool(pool)
    {
        m_explicit = new PointerType[ (1<<(height + 1))];
        calculateTransitionValues( height);
    }
    
    ~ExplicitHeapStorage()
    {
        delete [] m_explicit;
    }
    
    DataType* getRootAddr() const
    {
        return m_pool;
    }
    
    DataType* getBfsIndexAddress( const SizeType& bfsIndex) 
    {
        return m_pool + bfsIndex - 1;
    }

    DataType* getLeftChildAddress( DataType* ptr, const SizeType& bfsIndex, const SizeType& height) 
    {
        assert( m_explicit[ptr-m_pool].leftChild == ptr + bfsIndex);
        return m_explicit[ptr-m_pool].leftChild;
    }
    
    SizeType getMemoryUsage() const
    {
        return sizeof( DataType*);
    }
    
    DataType* getRightChildAddress( DataType* ptr, const SizeType& bfsIndex, const SizeType& height) 
    {
        //std::cout << "\n\t" << bfsIndex << " " << ptr->rightChild << " " << ptr - m_pool << "\n";
        assert( m_explicit[ptr-m_pool].rightChild == ptr + bfsIndex + 1);
        return m_explicit[ptr-m_pool].rightChild;
    }
    
    DataType* getParentAddress( DataType* ptr, const SizeType& bfsIndex, const SizeType& height) 
    {
        return m_explicit[ptr-m_pool].parent;
    }

private:
    DataType* m_pool;
    PointerType* m_explicit;
    
    void calculateTransitionValues( const SizeType& height)
    {              
        for( unsigned int i = 1; i < ( (unsigned int)1<< (height+1)); ++i)
        {
            if( i != 1)
            {
                m_explicit[i - 1].parent =  m_pool + (i>>1) - 1;
            }
            
            if( (unsigned int)floorLog2(i) != height)
            {
                m_explicit[i - 1].leftChild = m_pool + ( i<<1) - 1;
                
                m_explicit[i - 1].rightChild = m_pool + ((i<<1) + 1) - 1;
            }
        }
    }
    
};


template <typename DataType>
class ExplicitVebStorage
{
public:
    typedef PriorityQueueSizeType SizeType; 
    typedef ExplicitNodePointers<DataType> PointerType;
    
    ExplicitVebStorage( DataType* pool, const SizeType& height):m_pool(pool),m_height(height)
    {
        upperLevels = new SizeType[ height + 1];
        lowerLevels = new SizeType[ height + 1];
        treeSize = new SizeType[ height + 1];
        bfsToVeb = new SizeType[ (1<<(height + 1))];
        
        m_explicit = new PointerType[ (1<<(height + 1))];
        calculateTransitionValues( height);
    }
    
    ~ExplicitVebStorage()
    {
        delete [] upperLevels;
        delete [] lowerLevels;
        delete [] treeSize;
        delete [] bfsToVeb;
        delete [] m_explicit;
    }
    
    DataType* getRootAddr() const
    {
        return m_pool;
    }
    
    DataType* getBfsIndexAddress( const SizeType& bfsIndex) 
    {
        return m_pool + bfsToVeb[ bfsIndex];
    }

    DataType* getLeftChildAddress( DataType* ptr, const SizeType& bfsIndex, const SizeType& height) 
    {
        return m_explicit[ptr-m_pool].leftChild;
    }
    
    SizeType getMemoryUsage() const
    {
        return sizeof( DataType*);
    }
    
    DataType* getRightChildAddress( DataType* ptr, const SizeType& bfsIndex, const SizeType& height) 
    {
        return m_explicit[ptr-m_pool].rightChild;
    }
    
    DataType* getParentAddress( DataType* ptr, const SizeType& bfsIndex, const SizeType& height) 
    {
        return m_explicit[ptr-m_pool].parent;
    }
    
private:
    DataType* m_pool;
    
    SizeType m_height, sum, childIndex;
    
    SizeType* upperLevels;
    SizeType* lowerLevels;
    SizeType* treeSize;
    SizeType* bfsToVeb;
    PointerType* m_explicit;
    
    void calculateTransitionValues( const SizeType& height)
    {              
        std::stack< std::pair<SizeType,SizeType> > S;

        S.push( std::pair<SizeType,SizeType>( 0, height + 1));
        SizeType subTreeLevels;
        SizeType middleLevel;
        std::pair<SizeType,SizeType> levelRange;


        //std::cout << "*" << std::endl;
        while( !S.empty())
        {
            levelRange = S.top();
            //std::cout << "Examining " << levelRange.first << "->" << levelRange.second ;
            S.pop();
            subTreeLevels = levelRange.second - levelRange.first;

            for( SizeType h = 0; h <= height; h++)
            {
                treeSize[ h] = (1<<h) - 1;  
            }

            if( subTreeLevels > 1)
            {
                //std::cout << "\tsetting";
                
                middleLevel = levelRange.first + (subTreeLevels >> 1) + (subTreeLevels & 1);

                //middleLevel = levelRange.first + (1 << floorLog2(subTreeLevels - 1));

                //std::cout << "through " << middleLevel << "...";
                
                //std::cout << "\tLowerlevels" << middleLevel - levelRange.first;
                lowerLevels[ middleLevel] = middleLevel - levelRange.first;
                //std::cout << "\tUpperlevels" << levelRange.second - middleLevel;
                upperLevels[ middleLevel] = levelRange.second - middleLevel;
                
                S.push( std::pair<SizeType,SizeType>( middleLevel, levelRange.second));
                S.push( std::pair<SizeType,SizeType>( levelRange.first, middleLevel));   
            }
            //std::cout << std::endl;
        }
        
        upperLevels[height] = 1;
        //lowerLevels[height] = height;
        lowerLevels[0] = 0;
        upperLevels[0] = 1;

        for( unsigned int i = 1; i < ((unsigned int) 1<< (height+1)); ++i)
        {
            bfsToVeb[i] = getVebFromBfs(i);
            
            if( i != 1)
            {
                m_explicit[ getVebFromBfs(i)].parent =  m_pool + getVebFromBfs(i>>1);
            }
            
            if( (unsigned int)floorLog2(i) != height)
            {
                m_explicit[ getVebFromBfs(i)].leftChild = m_pool + getVebFromBfs( i<<1);
                m_explicit[ getVebFromBfs(i)].rightChild = m_pool + getVebFromBfs( (i<<1) + 1);
            }
        }
    }
    
    const SizeType& getVebFromBfs( const SizeType& bfsIndex)
    {
        sum = 0;
        if (bfsIndex == 1) return sum;
        
        childIndex = bfsIndex; 
        SizeType height = m_height - floorLog2( childIndex);
        
        while( childIndex != 1)
        {
            sum += ( childIndex & ((1 << upperLevels[height + 1]) - 1)) * treeSize[ lowerLevels[ height + 1]];
            sum += treeSize[ upperLevels[height + 1]];
            childIndex >>= upperLevels[height + 1];
            height += upperLevels[height + 1];
        }
        return sum;
    }
};


template <typename DataType>
class ExplicitPowerVebStorage
{
public:
    typedef PriorityQueueSizeType SizeType; 
    typedef ExplicitNodePointers<DataType> PointerType;
    
    ExplicitPowerVebStorage( DataType* pool, const SizeType& height):m_pool(pool),m_height(height)
    {
        upperLevels = new SizeType[ height + 1];
        lowerLevels = new SizeType[ height + 1];
        treeSize = new SizeType[ height + 1];
        bfsToVeb = new SizeType[ (1<<(height + 1))];
        
        m_explicit = new PointerType[ (1<<(height + 1))];
        calculateTransitionValues( height);
    }
    
    ~ExplicitPowerVebStorage()
    {
        delete [] upperLevels;
        delete [] lowerLevels;
        delete [] treeSize;
        delete [] bfsToVeb;
        delete [] m_explicit;
    }
    
    DataType* getRootAddr() const
    {
        return m_pool;
    }
    
    DataType* getBfsIndexAddress( const SizeType& bfsIndex) 
    {
        return m_pool + bfsToVeb[ bfsIndex];
    }

    DataType* getLeftChildAddress( DataType* ptr, const SizeType& bfsIndex, const SizeType& height) 
    {
        return m_explicit[ptr-m_pool].leftChild;
    }
    
    SizeType getMemoryUsage() const
    {
        return sizeof( DataType*);
    }
    
    DataType* getRightChildAddress( DataType* ptr, const SizeType& bfsIndex, const SizeType& height) 
    {
        return m_explicit[ptr-m_pool].rightChild;
    }
    
    DataType* getParentAddress( DataType* ptr, const SizeType& bfsIndex, const SizeType& height) 
    {
        return m_explicit[ptr-m_pool].parent;
    }
    
private:
    DataType* m_pool;
    
    SizeType m_height, sum, childIndex;
    
    SizeType* upperLevels;
    SizeType* lowerLevels;
    SizeType* treeSize;
    SizeType* bfsToVeb;
    PointerType* m_explicit;
    
    void calculateTransitionValues( const SizeType& height)
    {              
        std::stack< std::pair<SizeType,SizeType> > S;

        S.push( std::pair<SizeType,SizeType>( 0, height + 1));
        SizeType subTreeLevels;
        SizeType middleLevel;
        std::pair<SizeType,SizeType> levelRange;


        //std::cout << "*" << std::endl;
        while( !S.empty())
        {
            levelRange = S.top();
            //std::cout << "Examining " << levelRange.first << "->" << levelRange.second ;
            S.pop();
            subTreeLevels = levelRange.second - levelRange.first;

            for( SizeType h = 0; h <= height; h++)
            {
                treeSize[ h] = (1<<h) - 1;  
            }

            if( subTreeLevels > 1)
            {
                //std::cout << "\tsetting";
                
                if (!isPowerOf2(subTreeLevels))
                { 
                   subTreeLevels = nextPowerOf2( subTreeLevels) >> 1;
                }
                else
                {
                    subTreeLevels >>= 1;
                }
               
                
                middleLevel = levelRange.first + subTreeLevels;

                //middleLevel = levelRange.first + (1 << floorLog2(subTreeLevels - 1));

                //std::cout << "through " << middleLevel << "...";
                
                //std::cout << "\tLowerlevels" << middleLevel - levelRange.first;
                lowerLevels[ middleLevel] = middleLevel - levelRange.first;
                //std::cout << "\tUpperlevels" << levelRange.second - middleLevel;
                upperLevels[ middleLevel] = levelRange.second - middleLevel;
                
                S.push( std::pair<SizeType,SizeType>( middleLevel, levelRange.second));
                S.push( std::pair<SizeType,SizeType>( levelRange.first, middleLevel));   
            }
            //std::cout << std::endl;
        }
        
        upperLevels[height] = 1;
        //lowerLevels[height] = height;
        lowerLevels[0] = 0;
        upperLevels[0] = 1;

        for( unsigned int i = 1; i < ( (unsigned int)1<< (height+1)); ++i)
        {
            bfsToVeb[i] = getVebFromBfs(i);
            
            if( i != 1)
            {
                m_explicit[ getVebFromBfs(i)].parent =  m_pool + getVebFromBfs(i>>1);
            }
            
            if( (unsigned int)floorLog2(i) != height)
            {
                m_explicit[ getVebFromBfs(i)].leftChild = m_pool + getVebFromBfs( i<<1);
                m_explicit[ getVebFromBfs(i)].rightChild = m_pool + getVebFromBfs( (i<<1) + 1);
            }
        }
    }
    
    const SizeType& getVebFromBfs( const SizeType& bfsIndex)
    {
        sum = 0;
        if (bfsIndex == 1) return sum;
        
        childIndex = bfsIndex; 
        SizeType height = m_height - floorLog2( childIndex);
        
        while( childIndex != 1)
        {
            sum += ( childIndex & ((1 << upperLevels[height + 1]) - 1)) * treeSize[ lowerLevels[ height + 1]];
            sum += treeSize[ upperLevels[height + 1]];
            childIndex >>= upperLevels[height + 1];
            height += upperLevels[height + 1];
        }
        return sum;
    }
};


template <typename DataType>
class SplitStorage
{
public:
    typedef PriorityQueueSizeType SizeType; 
    SplitStorage( DataType* pool, const SizeType& height):m_pool(pool)
    {
        m_L2Ltransitions = new SizeType[ height + 1];
        m_L2Rtransitions = new SizeType[ height + 1];
        m_R2Ltransitions = new SizeType[ height + 1];
        m_R2Rtransitions = new SizeType[ height + 1];
        calculateTransitionValues( height);
    }
    
    ~SplitStorage()
    {
        delete[] m_L2Ltransitions;
        delete[] m_L2Rtransitions;
        delete[] m_R2Ltransitions;
        delete[] m_R2Rtransitions;
    }
    
    DataType* getRootAddr() const
    {
        return m_pool;
    }
    
    DataType* getLeftChildAddress( DataType* ptr, const SizeType& bfsIndex, const SizeType& height) 
    {
        return (bfsIndex & 1)?  ptr + m_R2Ltransitions[height] : ptr + m_L2Ltransitions[height];
    }
    
    DataType* getRightChildAddress( DataType* ptr, const SizeType& bfsIndex, const SizeType& height) 
    {
        return (bfsIndex & 1)?  ptr + m_R2Rtransitions[height] :  ptr + m_L2Rtransitions[height];
    }
    
    DataType* getParentAddress( DataType* ptr, const SizeType& bfsIndex, const SizeType& height) 
    {
        if(bfsIndex & 1)
        {
            return ( (bfsIndex >> 1) & 1 )?  ptr - m_R2Rtransitions[ height + 1]:  ptr - m_L2Rtransitions[ height + 1];
        }
        else
        {
            return ( (bfsIndex >> 1) & 1 )?  ptr - m_R2Ltransitions[ height + 1]:  ptr - m_L2Ltransitions[ height + 1];
        }
    }
    
private:
    DataType* m_pool;
    
    /**
     * @brief Lookup table for the transition step of a parent who is itself a left child towards its left child
     */
    SizeType* m_L2Ltransitions;

    /**
     * @brief Lookup table for the transition step of a parent who is itself a left child towards its right child
     */
    SizeType* m_L2Rtransitions;

    /**
     * @brief Lookup table for the transition step of a parent who is itself a right child towards its left child
     */
    SizeType* m_R2Ltransitions;

    /**
     * @brief Lookup table for the transition step of a parent who is itself a right child towards its right child
     */
    SizeType* m_R2Rtransitions; 
    
    void calculateTransitionValues( const SizeType& height)
    {      
        m_L2Ltransitions[height] = 1;
        m_R2Ltransitions[height] = 1;
        if( modulusPow2(height,2))
        {
            m_L2Rtransitions[height] = 2;
            m_R2Rtransitions[height] = 2; 
        }
        else
        {
            m_L2Rtransitions[height] = (1 << height);
            m_R2Rtransitions[height] = (1 << height); 
        }
        
        for( SizeType h = 1; h < height; h++)
        {        
            if( modulusPow2( h, 2))
            {
                m_L2Ltransitions[h] = 1;
                m_R2Ltransitions[h] = 1;
                m_L2Rtransitions[h] = 2;
                m_R2Rtransitions[h] = 2;
            }
            else
            {
                m_L2Ltransitions[h] = 2;
                m_R2Ltransitions[h] = pow2( h + 1) -1;
                m_L2Rtransitions[h] = pow2( h) + 1;
                m_R2Rtransitions[h] = pow2( h + 1) + pow2( h) - 2;
            }
        }
        
        m_L2Ltransitions[0] = 0;
        m_R2Ltransitions[0] = 0;
        m_L2Rtransitions[0] = 0;
        m_R2Rtransitions[0] = 0;
    }
};


#endif //TREESTORAGESCHEMES_H
