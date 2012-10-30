#ifndef DYNAMICGRAPH_H
#define DYNAMICGRAPH_H

#include <Structs/Arrays/packedMemoryArray.h>
#include <Utilities/mersenneTwister.h>
#include <Utilities/graphIO.h>
#include <Utilities/graphGenerators.h>
#include <vector>
#include <algorithm>


class DefaultGraphItem
{
public:
    DefaultGraphItem()
    {
    }

    void print( std::ofstream& out)
    {
        return;
    }

    static unsigned int memUsage()   
    { 
        return sizeof( DefaultGraphItem);
    }

    void setProperty( const std::string& name, const std::string& value)
    {

    }

    void writeProperties( std::ofstream& out, const std::string& propertyDelimiter = "\n", const std::string& valueDelimiter = " ")
    {
    }
    
    void writeJSON( std::ofstream& out)
    {
    }    
};


class MemTransfersRecorder
{
public:

    MemTransfersRecorder( const std::string& filename):m_ptr(0)
    {
        m_out.open( filename.c_str(), std::fstream::out);
    }
    
    ~MemTransfersRecorder()
    {
        m_out.close();
    }
    
    void recordJumpAt( char* ptr)
    {
        if( m_ptr)
        {
            if( ptr > m_ptr)
            {
                m_out << ptr - m_ptr << "\n";
            }
            else
            {
                m_out << m_ptr - ptr << "\n";
            }
        }
        m_ptr = ptr;
    }
    
private:
    std::ofstream m_out;
    char* m_ptr;
};


/**
 * @class DynamicGraph
 *
 * @brief A dynamic graph type
 *
 * @tparam GraphImplementation The underlying graph implementation to use. Available implementations are 'PackedMemoryGraphImpl' and 'AdjacencyListImpl'
 * @tparam Vtype The data to associate with the nodes. It must be a class or struct
 * @tparam Etype The data to associate with the edges. It must be a class or struct
 * @author Panos Michail
 *
 */

template<template <typename vtype,typename etype> class GraphImplementation, typename Vtype = DefaultGraphItem, typename Etype = DefaultGraphItem >
class DynamicGraph
{
public:
    
    typedef typename GraphImplementation<Vtype,Etype>::SizeType         SizeType;
    typedef typename GraphImplementation<Vtype,Etype>::NodeDescriptor   NodeDescriptor;
    typedef std::pair<NodeDescriptor,NodeDescriptor>                    EdgeDescriptor;
    typedef typename GraphImplementation<Vtype,Etype>::NodeIterator     NodeIterator;
    typedef typename GraphImplementation<Vtype,Etype>::EdgeIterator     EdgeIterator;
    typedef typename GraphImplementation<Vtype,Etype>::InEdgeIterator   InEdgeIterator;
    typedef Vtype                                                       NodeData;
    typedef Etype                                                       EdgeData;
    typedef unsigned int                                                PropertyType;

    DynamicGraph()
    {
        impl = new GraphImplementation<Vtype,Etype>();
    }

    ~DynamicGraph() 
    { 
        delete impl;
    }
    

    /**
     * @brief Returns the first outgoing edge of a node
     *
     * @param The node
     *
     * @return An iterator to the first outgoing edge of a node
     */
    EdgeIterator beginEdges( const NodeIterator& u) const 
    { 
        return impl->beginEdges( u);
    }
    
    /**
     * @brief Returns the first incoming edge of a node
     *
     * @param The node
     *
     * @return An iterator to the first incoming edge of a node
     */
    InEdgeIterator beginInEdges( const NodeIterator& u) const 
    { 
        return impl->beginInEdges(u);
    }

    /**
     * @brief Returns the first node of the graph
     * 
     * @return An iterator to the first node of the graph
     */
    NodeIterator beginNodes() const 
    { 
        return impl->beginNodes();
    }

    /**
     * @brief Returns the random node
     * 
     * @return A random node
     */
	NodeIterator chooseNode() const 
	{ 
	    return impl->chooseNode();
	}

    /**
     * @brief Clears the graph, removes all nodes and edges
     */
    void clear()
    {
        impl->clear();
        m_numNodes = 0;
        m_numEdges = 0;
    }
    
    void compress()
    {
        impl->compress();
    }

    /**
     * @brief Returns the degree of a node (number of outgoing and incoming edges)
     * 
     * @param u The node
     * @return The degree of the node 
     */
    SizeType degree( NodeIterator& u) const 
    { 
        return indeg(u) + outdeg(u);
    }

    void expand()
    {
        impl->expand();
    }

    /**
     * @brief Checks if an edge exists
     * 
     * @param e The edge descriptor
     * @return True if e exists in the graph, false otherwise
     */
    bool hasEdge( const EdgeDescriptor& descriptor)
    {
        return hasEdge( descriptor.first, descriptor.second);
    }

    /**
     * @brief Returns the end to the container of the edges of a node
     *
     * @param u The node
     *
     * @return An iterator to the past-the-end edge of the node
     */
    EdgeIterator endEdges( const NodeIterator& u) const 
    { 
        return impl->endEdges( u);
    }

    /**
     * @brief Returns the end to the container of the incoming edge of a node
     *
     * @param u The node
     *
     * @return An iterator to the past-the-end incoming edge of the node
     */
    InEdgeIterator endInEdges( const NodeIterator& u) const 
    { 
        return impl->endInEdges(u);
    }

    /**
     * @brief Returns the end to the container of the nodes
     * 
     * @return An iterator to the past-the-end node of the graph
     */
    NodeIterator endNodes() const 
    { 
        return impl->endNodes();
    }

    /**
     * @brief Erases an edge from the graph
     *
     * @param descriptor The descriptor of the edge to be erased
     */
    void eraseEdge( EdgeDescriptor descriptor) 
    { 
        if( !hasEdge(descriptor)) return;
        impl->eraseEdge( descriptor.first, descriptor.second);
        --m_numEdges;
    }
    
    /**
     * @brief Erases a node from the graph
     *
     * @param descriptor The descriptor of the node to be erased
     */
    void eraseNode( NodeDescriptor descriptor) 
    { 
        if( !hasNode(descriptor)) return;
        NodeIterator v, u = getNodeIterator(descriptor);       
        EdgeIterator e;
        InEdgeIterator k;

        std::vector< EdgeDescriptor> edges;
        
        if( hasEdges(u))
        {
            for( EdgeIterator e = beginEdges(u), lastEdge = endEdges(u); e != lastEdge; ++e)
            {
                edges.push_back( getEdgeDescriptor(e));
            }
        }
        
        if( hasInEdges(u))
        {
            for( InEdgeIterator k = beginInEdges(u), lastInEdge = endInEdges(u); k != lastInEdge; ++k)
            {
                edges.push_back( getEdgeDescriptor(k));
            }
        }
        
        for( typename std::vector<EdgeDescriptor>::iterator it = edges.begin(), end = edges.end(); it != end; ++it)
        {
            EdgeDescriptor eD = *it;
            e = getEdgeIterator( eD);
            eraseEdge( getEdgeDescriptor(e));
            --m_numEdges;
        } 
        
         impl->eraseNode( descriptor);
         --m_numNodes;
         assert(hasValidInEdges());
    }

    /**
     * @brief Populates the graph from a generator
     *
     * @param generator A graph generator
     */
    void generateFrom( GraphGenerator< DynamicGraph>* generator)
    {
        this->clear();
        generator->generate(*this);
    }

    /**
     * @brief Returns the descriptor of an edge (u,v)
     * 
     * @param uD The descritpor of the source node of the edge
     * @param vD The descritpor of the target node of the edge
     * @return The descriptor of the edge between u and v 
     */
    EdgeDescriptor getEdgeDescriptor( const NodeDescriptor& uD, const NodeDescriptor& vD)
    {
        assert( hasEdge(uD,vD));
        return EdgeDescriptor(uD,vD);
    }

    /**
     * @brief Returns the descriptor of an edge (u,v)
     * 
     * @param u The source node of the edge
     * @param v The target node of the edge
     * @return The descriptor of the edge between u and v 
     */
    EdgeDescriptor getEdgeDescriptor( const NodeIterator& u, const NodeIterator& v)
    {
        assert( hasEdge(u,v));
        return EdgeDescriptor( getNodeDescriptor(u), getNodeDescriptor(v));
    }  
    
    /**
     * @brief Returns the descriptor of an outgoing edge
     * 
     * @param e The outgoing edge
     * @return The descriptor of the edge
     */
    EdgeDescriptor getEdgeDescriptor( EdgeIterator& e)
    {
        InEdgeIterator k = getInEdgeIterator(e);
        return EdgeDescriptor( getNodeDescriptor( source(k)), getNodeDescriptor(target(e)));
    }

    /**
     * @brief Returns the descriptor of an incoming edge
     * 
     * @param k The incoming edge
     * @return The descriptor of the edge
     *
     */
    EdgeDescriptor getEdgeDescriptor( InEdgeIterator& k)
    {
        EdgeIterator e = getEdgeIterator(k);
        return getEdgeDescriptor( e);
    }

    /**
     * @brief Returns an iterator to an outgoing edge
     * 
     * @param descriptor The descriptor of the edge
     * @return An iterator to the outgoing edge
     */
    EdgeIterator getEdgeIterator( const EdgeDescriptor& descriptor) 
    { 
        return impl->getEdgeIterator(descriptor.first, descriptor.second);
    }

	/**
     * @brief Returns an iterator to an outgoing edge
     * 
     * @param descriptor The descriptor of the edge
     * @return An iterator to the outgoing edge
     */
    EdgeIterator getEdgeIterator( const NodeDescriptor& uD, const NodeDescriptor& vD) 
    { 
        return impl->getEdgeIterator( uD, vD);
    }

    /**
     * @brief Returns an iterator to an outgoing edge
     * 
     * @param u The source node of the edge
     * @param v The target node of the edge
     * @return An iterator to the outgoing edge
     */
    EdgeIterator getEdgeIterator( const NodeIterator& u, const NodeIterator& v) const
    {
        EdgeIterator e, end;
        NodeIterator neigh;        
        for( e = beginEdges(u), end = endEdges(u); e != end; ++e)
        {
            neigh = target(e);
            if( neigh == v)
            {
                break;
            }
        }
        return e;
    }

     /**
     * @brief Returns an iterator to an outgoing edge
     * 
     * @param e An iterator to an incoming edge
     * @return An iterator to the corresponding outgoing edge
     */
    EdgeIterator getEdgeIterator( const InEdgeIterator& k) const
    {
        return impl->getEdgeIterator(k);
    }

    /**
     * @brief Returns an iterator to an incoming edge
     * 
     * @param e An iterator an outgoing edge
     * @return An iterator to the corresponding incoming edge
     */
    InEdgeIterator getInEdgeIterator( const EdgeIterator& e) const
    {
        return impl->getInEdgeIterator(e);
    }

    /**
     * @brief Returns the descriptor of a node
     * 
     * @param u The node
     * @return The descriptor of the node
     */
    NodeDescriptor getNodeDescriptor( const NodeIterator& u)
    {
        return impl->getDescriptor(u);
    }

    /**
     * @brief Returns iterator to a node
     * 
     * @param descriptor The descriptor of the node
     * @return An iterator to the node
     */
    NodeIterator getNodeIterator( const NodeDescriptor& descriptor) 
    { 
        return impl->getNodeIterator(descriptor);
    }

    /**
     * @brief Returns iterator to a node
     * 
     * @param descriptor The descriptor of the node as a memory address
     * @return An iterator to the node
     */
    NodeIterator getNodeIterator( const void* descriptor) 
    { 
        return impl->getNodeIterator( (NodeDescriptor)descriptor);
    }

    /**
     * @brief Returns the number of edges in the graph
     * 
     * @return The number of edges in the graph
     */
    SizeType getNumEdges() const 
    { 
        return m_numEdges;
    }

    /**
     * @brief Returns the number of nodes in the graph
     * 
     * @return The number of nodes in the graph
     */
    SizeType getNumNodes() const 
    { 
        return m_numNodes;
    }

    /**
     * @brief Returns the relative position of a node as an id in the range [0, numNodes-1]
     *
     * @return The relative position of a node
     */
    SizeType getRelativePosition( const NodeIterator& u) const
    {
        return impl->getId(u);
    }

    /**
     * @brief Checks if an edge exists in the graph
     * 
     * @param uD The source node descriptor
     * @param vD The target node descriptor
     * @return True if there exists and edge (uD,vD), false otherwise
     */
    bool hasEdge( const NodeDescriptor& uD, const NodeDescriptor& vD)
    {
        assert( hasNode( uD) && hasNode( vD));
        return hasEdge( getNodeIterator( uD), getNodeIterator( vD));
    }

    /**
     * @brief Checks if an edge exists in the graph
     * 
     * @param u The source node 
     * @param v The target node 
     * @return True if there exists and edge (u,v), false otherwise
     */
    bool hasEdge( const NodeIterator& u, const NodeIterator& v)
    {
        EdgeIterator e, end;
        NodeIterator neigh;        
        for( e = beginEdges(u), end = endEdges(u); e != end; ++e)
        {
            if( target(e) == v) return true;
        }
        return false;
    }
    
    /**
     * @brief Checks if a node has outgoing edges
     * 
     * @param u The node
     * @return True if u has outgoing edges, false otherwise
     */
    bool hasEdges( const NodeIterator& u) const
    {
        return impl->hasEdges(u);
    }

    /**
     * @brief Checks if a node has incoming edges
     * 
     * @param u The node
     * @return True if u has incoming edges, false otherwise
     */
    bool hasInEdges( const NodeIterator& u) const
    {
        return impl->hasInEdges(u);
    }

    /**
     * @brief Checks if a node exists in the graph
     * 
     * @param descriptor The node descriptor
     * @return True if u exists in the graph, false otherwise
     */
    bool hasNode( const NodeDescriptor& descriptor)
    {
        return impl->hasNode( descriptor);
    }

    /**
     * @brief Checks if all outgoing edges are paired to an incoming edge each
     * 
     * @return True if all outgoing edges are paired to an incoming edge each, false otherwise
     */
    bool hasValidInEdges()
    {
        NodeIterator u, end_nodes;
        EdgeIterator e, end_edges;
        InEdgeIterator k;
        for( u = beginNodes(), end_nodes = endNodes(); u != end_nodes; ++u)
        {
            for( e = beginEdges(u), end_edges = endEdges(u); e != end_edges; ++e)
            {
                k = getInEdgeIterator(e);
                if( getEdgeDescriptor(e) != getEdgeDescriptor(k))
                {
                    return false;
                }
                
                if( getEdgeIterator(getInEdgeIterator(e)) != e)
                {
                    return false;
                }                
            }
        }
        return true;
    }

    /**
     * @brief Returns the in degree of a node (number of incoming edges)
     * 
     * @param u The node
     * @return The in degree of the node 
     */
    SizeType indeg( NodeIterator& u) const 
    { 
        return impl->indeg(u);
    }
    
    /**
     * @brief Inserts an edge in the graph
     *
     * @param uD The descriptor of the source node of the edge
     * @param vD The descriptor of the target node of the edge
     * 
     * @return The descriptor of the new edge, zero if the operation failed 
     */
    EdgeDescriptor insertEdge( NodeDescriptor uD, NodeDescriptor vD) 
    { 
        //std::cout << "Inserting edge " << uD << "->" << vD << std::endl;
        if( uD == vD) return EdgeDescriptor(0,0);
        if( !hasNode(uD)) return EdgeDescriptor(0,0);
        if( !hasNode(vD)) return EdgeDescriptor(0,0);
        if( hasEdge( uD, vD)) return getEdgeDescriptor( uD, vD);
        ++m_numEdges;
        impl->insertEdge( uD, vD);
        return getEdgeDescriptor( uD, vD);
    }
    
    /**
     * @brief Inserts a node in the graph
     * 
     * @return The descriptor of the new node 
     */
    NodeDescriptor insertNode() 
    {
        ++m_numNodes;
        return impl->insertNode();
    }

    NodeDescriptor insertNodeBefore( NodeDescriptor descriptor) 
    {
        ++m_numNodes;
        return impl->insertNodeBefore( descriptor);
    }
    
        /**
     * @brief Returns the memory usage in bytes
     *
     * @return The memory usage in bytes
     */
    SizeType memUsage() const   
    { 
        return impl->memUsage() + 2 * sizeof(SizeType) + 2 * sizeof (std::vector< std::string>);
    }

    void move( NodeDescriptor sourceDescriptor, NodeDescriptor targetDescriptor)
    {
        //impl->move( uD, vD);
        assert(hasValidInEdges());
        typedef std::pair<EdgeDescriptor, NodeDescriptor> EdgeWrapper;
        
        std::vector< EdgeWrapper > outEdges;
        std::vector< EdgeWrapper > inEdges;
        
        // Create a new node
        NodeDescriptor newDescriptor = insertNodeBefore( targetDescriptor);
        
        // Get iterator to original node
        NodeIterator sourceNode = getNodeIterator(sourceDescriptor);
        
        // Gather out edges of source node
        for( EdgeIterator e = beginEdges( sourceNode), lastEdge = endEdges( sourceNode); e != lastEdge; ++e)
        {
            outEdges.push_back( EdgeWrapper( getEdgeDescriptor(e), getNodeDescriptor( target(e))));
        }
        
        // Gather in edges of source node
        for( InEdgeIterator k = beginInEdges( sourceNode), lastInEdge = endInEdges( sourceNode); k != lastInEdge; ++k)
        {
            inEdges.push_back( EdgeWrapper(getEdgeDescriptor(k), getNodeDescriptor( source(k))));
        }
        
        // Copy out edges of source node
        for( typename std::vector< EdgeWrapper >::iterator it = outEdges.begin(), end = outEdges.end(); it != end; ++it)
        {
            EdgeDescriptor eD = insertEdge( newDescriptor, it->second);
            EdgeIterator e = getEdgeIterator(eD);
            InEdgeIterator k = getInEdgeIterator(e);
            EdgeIterator oldE = getEdgeIterator( it->first);
            
            Etype* edge = &(*e);
            Etype* inedge = &(*k);
            (*edge) = (Etype)(*oldE);
            (*inedge) = (Etype)(*oldE);

            //oldE->setDescriptor( eD);
            //switchEdgeDescriptors( eD, it->first);
        }
        
        // Copy in edges of source node
        for( typename std::vector< EdgeWrapper >::iterator it = inEdges.begin(), end = inEdges.end(); it != end; ++it)
        {
            EdgeDescriptor eD = insertEdge( it->second, newDescriptor);
            EdgeIterator e = getEdgeIterator(eD);
            InEdgeIterator k = getInEdgeIterator(e);
            EdgeIterator oldE = getEdgeIterator( it->first);
            (Etype)(*e) = (Etype)(*oldE);
            (Etype)(*k) = (Etype)(*oldE);
            //oldE->setDescriptor( eD);
            //switchEdgeDescriptors( eD, it->first);
        }
        
        NodeIterator newNode = getNodeIterator(newDescriptor);
        NodeIterator oldNode = getNodeIterator( sourceDescriptor);

        Vtype* node = &(*newNode);
        (*node) = (Vtype)(*oldNode);

        switchNodeDescriptors( sourceDescriptor, newDescriptor);
        eraseNode( newDescriptor);
    }

    /**
     * @brief Returns an empty node descriptor
     *
     * @return An empty node descriptor
     */
    NodeDescriptor nilNodeDescriptor() const 
    { 
        return 0;
    } 

    /**
     * @brief Returns the degree of a node (number of outgoing edges)
     * 
     * @param u The node
     * @return The out degree of the node 
     */
    SizeType outdeg( NodeIterator& u) const 
    { 
        return impl->outdeg(u);
    }

    /**
     * @brief Prints a dot representation of the graph to a file
     * 
     * @param filename The output filename
     * @return The target node of the edge 
     */
    void printInternalDot(const std::string& filename)
    {
        std::ofstream out( filename.c_str());
        impl->printDot( out);
        out.close();      
    }

    EdgeDescriptor pushEdge( NodeDescriptor uD, NodeDescriptor vD) 
    { 
        if( uD == vD) return EdgeDescriptor(0,0);
        if( !hasNode(uD)) return EdgeDescriptor(0,0);
        if( !hasNode(vD)) return EdgeDescriptor(0,0);
        if( hasEdge( uD, vD)) return getEdgeDescriptor( uD, vD);
        ++m_numEdges;
        impl->pushEdge( uD, vD);
        return getEdgeDescriptor( uD, vD);
    }

    /**
     * @brief Populates the graph from a reader
     *
     * @param reader A graph reader
     */
    void read( GraphReader< DynamicGraph>* reader)
    {
        this->clear();
        reader->read(*this);
    }

    /**
     * @brief Reserves memory for the graph
     *
     * @param numNodes The expected number of nodes
     * @param numEdges The expected number of edges
     */
    void reserve( const SizeType& numNodes, const SizeType& numEdges)
    {
        impl->reserve( numNodes, numEdges);
    }

    /**
     * @brief Returns the source node of an outgoing edge
     * 
     * @param e The outgoing edge
     * @return The source node of the edge 
     *
    NodeIterator source( EdgeIterator& e) const 
    {
        return impl->getAdjacentNodeIterator( getInEdgeIterator(e));
    }*/

    /**
     * @brief Returns the source node of an incoming edge
     * 
     * @param k The incoming edge
     * @return The source node of the edge 
     */
    NodeIterator source( InEdgeIterator& k) const
    {
        return impl->getAdjacentNodeIterator( k);  
    }


    /*void switchEdgeDescriptors( EdgeDescriptor first, EdgeDescriptor second)
    {
        EdgeIterator e = getEdgeIterator(first);
        EdgeIterator k = getEdgeIterator(second);
        EdgeDescriptor eD = first;
        EdgeDescriptor kdesc = second;

        impl->setDescriptor( e, kdesc);
        impl->setDescriptor( k, eD);
    }*/

    void switchNodeDescriptors( NodeDescriptor first, NodeDescriptor second)
    {
        NodeIterator u = getNodeIterator(first);
        NodeIterator v = getNodeIterator(second);
        NodeDescriptor uD = first;
        NodeDescriptor vD = second;

        impl->setDescriptor( u, vD);
        impl->setDescriptor( v, uD);
    }

    /**
     * @brief Returns the target node of an outgoingedge
     * 
     * @param e The outgoing edge
     * @return The target node of the edge 
     */
    NodeIterator target( EdgeIterator& e) const 
    { 
        return impl->getAdjacentNodeIterator( e);
    }
     
    /**
     * @brief Returns the target node of an incoming edge
     * 
     * @param k The incoming edge
     * @return The target node of the edge 
     *
    NodeIterator target( InEdgeIterator& k) const 
    { 
        return impl->getAdjacentNodeIterator( getEdgeIterator(k) );
    }*/

    /**
     * @brief Outputs the graph to a writer
     *
     * @param writer A graph writer
     */
    void write( GraphWriter< DynamicGraph>* writer)
    {
        writer->write(*this);
    }

private:
    GraphImplementation<Vtype,Etype>*   impl;
    SizeType                            m_numNodes;
    SizeType                            m_numEdges;
};



template<typename DataType, typename DescriptorType, typename PropertyType = unsigned int>
class GraphElement : public DataType
{
public:  
    GraphElement():m_descriptor( 0)
    {
    }

    GraphElement( DescriptorType descriptor):m_descriptor( descriptor)
    {
    }

    DescriptorType getDescriptor() const
    {
        return m_descriptor;
    }

    bool operator ==( const GraphElement& other) const
	{
        return ( m_descriptor == other.m_descriptor );
	}
	
	bool operator !=( const GraphElement& other) const
	{
		return (m_descriptor != other.m_descriptor);
	}

    static unsigned int memUsage()   
    { 
        return sizeof( GraphElement);
    }
    
    void setDescriptor( const DescriptorType& descriptor)
    {
        m_descriptor = descriptor;
    }

protected:
    DescriptorType              m_descriptor; 
};


#endif // DYNAMICGRAPH_H
