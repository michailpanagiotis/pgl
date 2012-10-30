#ifndef BUCKETLIST_H
#define BUCKETLIST_H

#include <Utilities/mersenneTwister.h>
#include <set>
#include <deque>
#include <assert.h>
#include <limits>
#include <memory>




template <typename dataType>
class BucketList<dataType>
{
public:
    BucketList()
    {
    }

    void insertAtBucket( const BucketDescriptor& bucketDesc, const dataType& data)
    {
        SizeType bucketId = *bucketDesc;
        if( !m_buckets[bucketId].hasRoom())
        {
            newBucketId = newBucket( m_buckets[bucketId].capacity << 1);
            copyBucket( bucketId, newBucketId);
            m_buckets[bucketId].clear()
            if( bucketId)
            {
                m_buckets[bucketId-1].doubleSize();
            }
        }
    }

    SizeType newBucket( SizeType bucketSize = 1)
    {
        m_pool.resize( m_pool.size() + bucketSize);
        m_buckets.push_back( Bucket( this, m_pool.size() - bucketSize -1 , bucketSize));
        return m_buckets.size() - 1;
    }
}







template <typename dataType>
class BucketList<dataType>::Bucket
{
public:
    Bucket():m_PMA(0),m_begin(0),m_end(0),m_head(0){}

    Bucket( BucketList* bucketList, Iterator begin): m_bucketList(bucketList), 
                                            m_begin(begin), 
                                            m_capacity(capacity),
                                            m_numElements(numElements)
    {
    }

    Iterator begin()
    {
        return m_bucketList.atIndex(m_begin);
    }

    void clear()
    {
        m_numElements = 0;
    }

    void copy( Bucket& other)
    {
        assert( m_capacity < other.m_capacity);
        for( Iterator it = begin(), end = end(); it != end; ++it)
        {
            other.insert(*it);
        }
    }

    Iterator end()
    {
        return m_bucketList.atIndex(m_begin + m_numElements);
    }

    void erase( const Iterator& it) 
    {
        Iterator prev = it;
        ++it;
        for( Iterator end = end(); it != end; ++it, ++prev)
        {
            *prev = *it; 
        }
        --m_numElements;
    }

    bool hasRoom()
    {
        return m_numElements < m_capacity;
    }

    Iterator push_back( const dataType& data) 
    {
        if( hasRoom())
        {
            Iterator it = end();
            *it = data;
            ++m_numElements;
        }
    }

    SizeType getMemoryUsage() const
    {
        return 3 * sizeof(dataType*) + sizeof( PackedMemoryArray*);
    }

private:
    unsigned int        m_begin;
    unsigned int        m_capacity;
    unsigned int        m_numElements;
};


#endif //BUCKETLIST_H
