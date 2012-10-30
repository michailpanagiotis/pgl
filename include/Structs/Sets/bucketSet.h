#ifndef BUCKETSET_H
#define BUCKETSET_H

#include <Utilities/mersenneTwister.h>
#include <assert.h>
#include <limits>
#include <vector>


template <typename dataType>
class BucketSet
{
public:

    typedef typename std::vector<dataType>::iterator Iterator;
    typedef unsigned int SizeType;

    class Bucket;

    BucketSet():m_pool(0)
    {
    }

    Iterator atIndex(const SizeType& pos)
    {
        Iterator it = m_pool.begin() + pos;
        return it;
    }

    void registerBucket(  Bucket* bucket, const SizeType& size = 1)
    {
        SizeType initPoolSize = m_pool.size();
        m_pool.resize( m_pool.size() + size);
        bucket->reset( this, initPoolSize, size);
    }

    void allocateMoreSpaceFor( Bucket* bucket)
    {        
        Bucket* newBucket = new Bucket();
        registerBucket( newBucket, 2*bucket->m_capacity);
        bucket->copyTo(*newBucket);
        (*bucket) = (*newBucket);        
        delete newBucket;
    }

private:
    std::vector<dataType> m_pool;
};



template <typename dataType>
class BucketSet<dataType>::Bucket
{
public:

    typedef typename BucketSet<dataType>::Iterator Iterator;

    Bucket( BucketSet* bucketSet = 0, const SizeType& begin = 0, const SizeType& size = 0): 
                                m_bucketSet(bucketSet), 
                                m_begin(begin), 
                                m_capacity(size),
                                m_numElements(0)
    {
    }

    Iterator begin()
    {
        return m_bucketSet->atIndex(m_begin);
    }

    void clear()
    {
        m_numElements = 0;
    }

    void copyTo( Bucket& other)
    {
        assert( m_capacity < other.m_capacity);
        Iterator it = begin();
        Iterator last = end();
        for( ; it != last; ++it)
        {
            other.push_back(*it);
        }
    }

    bool empty()
    {
        return m_numElements == 0;
    }

    Iterator end()
    {
        return m_bucketSet->atIndex(m_begin + m_numElements);
    }

    void erase( Iterator& it) 
    {
        Iterator last = end();
        Iterator next = it + 1;
        std::copy( next, last, it);
        --m_numElements;
        /*Iterator prev = it;
        ++it;
        for( Iterator last = end(); it != last; ++it, ++prev)
        {
            *prev = *it; 
        }
        --m_numElements;*/
    }

    bool hasRoom()
    {
        return m_numElements < m_capacity;
    }

    bool isRegistered()
    {
        return m_bucketSet != 0;
    }   

    Bucket& operator=(const Bucket& other)
    {
        if( this != &other)
        {       
            this->m_bucketSet = other.m_bucketSet;
            this->m_begin = other.m_begin;
            this->m_capacity = other.m_capacity;
            this->m_numElements = other.m_numElements;
        }
        return *this;
    }

    void push_back( const dataType& data) 
    {
        if( !hasRoom())
        {
            m_bucketSet->allocateMoreSpaceFor(this);
        }
        Iterator it = end();
        *it = data;
        ++m_numElements;
    }

    void reset( BucketSet* bucketSet = 0, const SizeType& begin = 0, const SizeType& size = 0) 
    {
        m_bucketSet = bucketSet;
        m_begin = begin;
        m_capacity = size;
        m_numElements = 0;
    }

    SizeType getMemoryUsage() const
    {
        return 3 * sizeof(SizeType) + sizeof( BucketSet*) + sizeof(Bucket*);
    }


    BucketSet*          m_bucketSet;
    unsigned int        m_begin;
    unsigned int        m_capacity;
    unsigned int        m_numElements;
};


#endif //BUCKETSET_H
