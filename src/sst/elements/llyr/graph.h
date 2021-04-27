// Copyright 2009-2020 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2020, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.


#ifndef _LLYR_GRAPH_H
#define _LLYR_GRAPH_H

#include <sst/core/sst_config.h>
#include <sst/core/output.h>

#include <map>
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>

namespace SST {
namespace Llyr {

struct EdgeProperties
{
    float weight_;
};

struct VertexProperties
{

};

class Edge
{
private:
    EdgeProperties* properties_;
    uint32_t destinationVertex_;

protected:

public:
    explicit Edge( uint32_t vertexIn )
    {
        properties_ = NULL;
        destinationVertex_ = vertexIn;
    }
    explicit Edge( EdgeProperties* properties, uint32_t vertexIn )
    {
        properties_ = properties;
        destinationVertex_ = vertexIn;
    }
    ~Edge();

    bool setProperties( EdgeProperties* properties )
    {
        properties_ = properties;
        return true;
    }

    EdgeProperties* getProperties( void ) const
    {
        return properties_;
    }

    uint32_t getDestination( void ) const
    {
        return destinationVertex_;
    }

};


template<class T>
class Vertex
{
private:
    T    value_;
    bool visited_;
    uint32_t numInEdges_;
    uint32_t numOutEdges_;

    std::vector< Edge* >* adjacencyList_;

protected:

public:
    Vertex()
    {
        adjacencyList_ = new std::vector< Edge* >;
        visited_ = 0;
        numInEdges_ = 0;
        numOutEdges_ = 0;
    }

    Vertex( T typeIn ) : value_(typeIn)
    {
        adjacencyList_ = new std::vector< Edge* >;
        visited_ = 0;
        numInEdges_ = 0;
        numOutEdges_ = 0;
    }

    Vertex(const Vertex &valueIn)
    {
        value_ = valueIn.value_;
        visited_ = valueIn.visited_;
        numInEdges_ = valueIn.numInEdges_;
        numOutEdges_ = valueIn.numOutEdges_;
        adjacencyList_ = new std::vector< Edge* >(*(valueIn.adjacencyList_));
    }

    bool operator == (const Vertex &valueIn) const
    {
        return(this->value_ == valueIn.getValue());
    }

    void setValue( T typeIn )
    {
        value_ = typeIn;
    }

    T getValue( void ) const
    {
        return value_;
    }

    void setVisited( bool visitIn )
    {
        visited_ = visitIn;
    }

    bool getVisited( void ) const
    {
        return visited_;
    }

    std::vector< Edge* >* getAdjacencyList( void ) const
    {
        return adjacencyList_;
    }

    void addEdge( Edge* edgeIn )
    {
        adjacencyList_->push_back(edgeIn);
    }

    void addInDegree()
    {
        ++numInEdges_;
    }

    uint32_t getInDegree() const
    {
        return numInEdges_;
    }

    void addOutDegree()
    {
        ++numOutEdges_;
    }

    uint32_t getOutDegree() const
    {
        return numOutEdges_;
    }

};

template<class T>
class LlyrGraph
{

private:
    uint32_t vertices_;
    std::map< uint32_t, Vertex<T> >* vertex_map_;

protected:

public:
    LlyrGraph();
    ~LlyrGraph();

    void printGraph();
    void printDot( std::string fileName );

    uint32_t outEdges( uint32_t vertexId );
    uint32_t numVertices();

    void addEdge( uint32_t beginVertex, uint32_t endVertex );
    void addEdge( uint32_t beginVertex, uint32_t endVertex, EdgeProperties* properties );
    uint32_t addVertex( T type );
    uint32_t addVertex( uint32_t vertexNum, T type );

    Vertex<T>* getVertex( uint32_t vertexNum )
    {
        return &vertex_map_->at(vertexNum);
    }

    void setVertex( uint32_t vertexNum, const Vertex<T> &vertex )
    {
        vertex_map_->at(vertexNum) = vertex;
    }

    uint32_t operator []( const Vertex<T>& value )
    {
        for( auto it = vertex_map_->begin(); it != vertex_map_ ->end(); ++it ) {
            if( it->second == value ) {
                return it->first;
            }
        }

        return 0;
    }

    std::map< uint32_t, Vertex<T> >* getVertexMap( void ) const
    {
        return vertex_map_;
    }

};

template<class T>
LlyrGraph<T>::LlyrGraph()
{
    vertex_map_ = new std::map< uint32_t, Vertex<T> >;
    vertices_ = 0;
}

template<class T>
LlyrGraph<T>::~LlyrGraph()
{}

template<class T>
void LlyrGraph<T>::printGraph(void)
{
    typename std::map< uint32_t, Vertex<T> >::iterator vertexIterator;
    for(vertexIterator = vertex_map_->begin(); vertexIterator != vertex_map_->end(); ++vertexIterator)
    {
        std::cout << "\n Adjacency list of vertex " << vertexIterator->first << "\n head ";

        std::vector< Edge* >* adjacencyList = vertexIterator->second.getAdjacencyList();

        for( auto it = adjacencyList->begin(); it != adjacencyList->end(); ++it ) {
            std::cout << "-> " << (*it)->getDestination();
        }
        std::cout << std::endl;
    }
}

template<class T>
void LlyrGraph<T>::printDot( std::string fileName )
{
    std::ofstream outputFile(fileName.c_str(), std::ios::trunc);         //open a file for writing (truncate the current contents)
    if ( !outputFile )                                                   //check to be sure file is open
        std::cerr << "Error opening file.";

    outputFile << "digraph G {" << "\n";

    typename std::map< uint32_t, Vertex<T> >::iterator vertexIterator;
    for(vertexIterator = vertex_map_->begin(); vertexIterator != vertex_map_->end(); ++vertexIterator) {
        outputFile << vertexIterator->first << "[label=\"";
        outputFile << vertexIterator->second.getValue();
        outputFile << "\"];\n";
    }

    for(vertexIterator = vertex_map_->begin(); vertexIterator != vertex_map_->end(); ++vertexIterator) {
        std::vector< Edge* >* adjacencyList = vertexIterator->second.getAdjacencyList();

        for( auto it = adjacencyList->begin(); it != adjacencyList->end(); ++it ) {
            outputFile << vertexIterator->first;
            outputFile << "->";
            outputFile << (*it)->getDestination();
            outputFile << "\n";
        }
    }

    outputFile << "}";
    outputFile.close();
}

template<class T>
uint32_t LlyrGraph<T>::outEdges(uint32_t vertexId)
{
    return vertex_map_->at(vertexId).adjacencyList_->size();
}

template<class T>
uint32_t LlyrGraph<T>::numVertices(void)
{
    return vertices_;
}

template<class T>
void LlyrGraph<T>::addEdge( uint32_t beginVertex, uint32_t endVertex )
{
    std::cout << "add edge:  " << beginVertex << " --> " << endVertex << "\n" << std::endl;
    Edge* edge = new Edge( endVertex );

    vertex_map_->at(beginVertex).addEdge(edge);
    vertex_map_->at(beginVertex).addOutDegree();
    vertex_map_->at(endVertex).addInDegree();
}

template<class T>
void LlyrGraph<T>::addEdge( uint32_t beginVertex, uint32_t endVertex, EdgeProperties* properties )
{
    std::cout << "add edge:  " << beginVertex << " --> " << endVertex << "\n" << std::endl;

    Edge* edge = new Edge( properties, endVertex );

    vertex_map_->at(beginVertex).addEdge(edge);
    vertex_map_->at(beginVertex).addOutDegree();
    vertex_map_->at(endVertex).addInDegree();
}

template<class T>
uint32_t LlyrGraph<T>::addVertex(T type)
{
    Vertex<T> vertex;
    vertex.setValue(type);

    uint32_t vertexNum = vertices_;
    std::cout << "add vertex:  " << vertexNum << "\n" << std::endl;
    auto retVal = vertex_map_->emplace( vertexNum, vertex );
    if( retVal.second == false ) {
        ///TODO
    }

    vertices_ = vertices_ + 1;
    return vertexNum;
}

template<class T>
uint32_t LlyrGraph<T>::addVertex(uint32_t vertexNum, T type)
{
    Vertex<T> vertex;
    vertex.setValue(type);

    std::cout << "add vertex:  " << vertexNum << "\n" << std::endl;
    auto retVal = vertex_map_->emplace( vertexNum, vertex );
    if( retVal.second == false ) {
        ///TODO
    }

    vertices_ = vertices_ + 1;
    return vertexNum;
}


} // namespace LLyr
} // namespace SST

#endif



