#ifndef GRAPHGENERATORS_H
#define GRAPHGENERATORS_H

#include <Utilities/mersenneTwister.h>



template<typename GraphType>
class GraphGenerator
{
public:
    GraphGenerator()
    {
    }
    
    virtual void generate( GraphType& G)
    {
    }
protected:
};


template<typename GraphType>
class RandomGenerator : public GraphGenerator<GraphType>
{
public:
    typedef typename GraphType::SizeType        SizeType;
    typedef typename GraphType::NodeIterator    NodeIterator;
    typedef typename GraphType::NodeDescriptor  NodeDescriptor;
    typedef typename GraphType::EdgeIterator    EdgeIterator;
    typedef typename GraphType::EdgeDescriptor  EdgeDescriptor;

    RandomGenerator( const SizeType& numNodes, const SizeType& numEdges):m_numNodes(numNodes),m_numEdges(numEdges)
    {
    }

    void generate( GraphType& G)
    {
        NodeIterator u,v;
        NodeDescriptor uD;
        EdgeIterator e;
        EdgeDescriptor eD;
    
        std::stringstream nodestream;
        nodestream << "Generating " << m_numNodes << " random nodes";
        ProgressBar node_progress( m_numNodes,nodestream.str());
        for( SizeType i = 0; i < m_numNodes; ++i)
        {
            uD = G.insertNode();
            u = G.getNodeIterator(uD);
            ++node_progress;
        }
    
        std::stringstream edgestream;
        edgestream << "Generating " << m_numEdges << " random edges";
        ProgressBar edge_progress( m_numEdges,edgestream.str());
        for( SizeType i = 0; i < m_numEdges; ++i)
        {
            u = G.chooseNode();
            v = G.chooseNode();
            while( G.hasEdge( u->getDescriptor(),v->getDescriptor()) || u == v)
            {
                u = G.chooseNode();
                v = G.chooseNode();
            }
            eD = G.insertEdge( u->getDescriptor(),v->getDescriptor());
            e = G.getEdgeIterator(eD);
            ++edge_progress;
        }
    }
    
private:
    const typename GraphType::SizeType& m_numNodes;
    const typename GraphType::SizeType& m_numEdges;
};


template<typename GraphType>
class RandomWeightedGenerator : public GraphGenerator<GraphType>
{
public:
    typedef typename GraphType::SizeType        SizeType;
    typedef typename GraphType::NodeIterator    NodeIterator;
    typedef typename GraphType::NodeDescriptor  NodeDescriptor;
    typedef typename GraphType::EdgeIterator    EdgeIterator;
    typedef typename GraphType::EdgeDescriptor  EdgeDescriptor;

    RandomWeightedGenerator( const SizeType& numNodes, const SizeType& numEdges, unsigned int maxWeight):m_numNodes(numNodes),m_numEdges(numEdges),m_maxWeight(maxWeight)
    {
    }

    void generate( GraphType& G)
    {
        NodeIterator u,v;
        NodeDescriptor uD;
        EdgeIterator e;
        EdgeDescriptor eD;
    
        std::stringstream nodestream;
        nodestream << "Generating " << m_numNodes << " random nodes";
        ProgressBar node_progress( m_numNodes,nodestream.str());
        for( SizeType i = 0; i < m_numNodes; ++i)
        {
            uD = G.insertNode();
            u = G.getNodeIterator(uD);
            ++node_progress;
        }
    
        std::stringstream edgestream;
        edgestream << "Generating " << m_numEdges << " random edges";
        ProgressBar edge_progress( m_numEdges,edgestream.str());
        for( SizeType i = 0; i < m_numEdges; ++i)
        {
            u = G.chooseNode();
            v = G.chooseNode();
            while( G.hasEdge( u->getDescriptor(),v->getDescriptor()) || u == v)
            {
                u = G.chooseNode();
                v = G.chooseNode();
            }
            eD = G.insertEdge( u->getDescriptor(),v->getDescriptor());
            e = G.getEdgeIterator(eD);

            double random = m_random.getRandomNormalizedDouble();
            e->weight = m_maxWeight * random + 1;
            ++edge_progress;
        }
    }
    
private:
    const typename GraphType::SizeType& m_numNodes;
    const typename GraphType::SizeType& m_numEdges;
    unsigned int m_maxWeight;
    MersenneTwister                     m_random;
};

#endif //GRAPHGENERATORS_H
