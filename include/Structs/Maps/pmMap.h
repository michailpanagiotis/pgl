#ifndef PMMAP_H
#define PMMAP_H

#include <Structs/Arrays/packedMemoryArray.h>

template <typename KeyType, typename DataType>
class PMMapItem;

/**
 * @class PMDictionary
 *
 * @brief 
 * 
 * @author Panos Michail
 *
 */

template <typename KeyType, typename DataType>
class PMMap
{
public:
    typedef PMMapItem<KeyType,DataType> MapItem;
    typedef typename PackedMemoryArray<MapItem>::Iterator Iterator;
    typedef typename PackedMemoryArray<MapItem>::SizeType SizeType;
    
    PMMap()
    {
    }
    
    Iterator begin()
    {
        return m_pool.begin();
    }
    
    void clear()
    {
        m_pool.clear();
    }

    Iterator end()
    {
        return m_pool.end();
    }
    
    void erase( const Iterator& it)
    {
        m_pool.erase(it);
    }
    
    Iterator find( const KeyType& key)
    {
        MapItem item(key,DataType());
        return m_pool.find( item);
    }
    
    DataType& operator[]( const KeyType& key)
    {
        Iterator it = find(key);
        if( it == m_pool.end())
        {
            it = unmanagedInsert( MapItem(key, DataType()));
        }
        return it->m_data;
    }
    
    SizeType size()
    {
        return m_pool.size();
    }

private:
    PackedMemoryArray<MapItem> m_pool;
    
    Iterator unmanagedInsert( const MapItem& newItem)
    {
        Iterator it = m_pool.lower_bound(newItem);
        return m_pool.insert( it, newItem);
    }
};


template <typename KeyType, typename DataType>
class PMMapItem
{
public: 

    PMMapItem( unsigned int init = 0):m_key(),m_data(0)
    {
    }
    
    PMMapItem( const KeyType& key, const DataType& data):m_key(key),m_data(data)
    {
    }
    
    bool operator < ( const PMMapItem& other) const
    {
        return m_key < other.m_key;
    }
    
    bool operator > ( const PMMapItem& other) const 
    {
        return m_key > other.m_key;
    }
    
    bool operator == ( const PMMapItem& other) const
    {
        return (m_key == other.m_key);
    }
    
    bool operator != ( const PMMapItem& other) const
    {
        return (m_key != other.m_key);
    }
    
    KeyType m_key;
    DataType m_data;
};



#endif //PMMAP_H
