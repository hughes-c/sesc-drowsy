//
// C++ Interface: graphManipulation
//
// Description: 
//
//
/// @author: hughes  <hughes@fraidy2-uf>, (C) 2008
///
/// @date:          01/11/08
/// Last Modified:  07/03/08
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef GRAPH_MANIPULATION_H
#define GRAPH_MANIPULATION_H

#include <algorithm>

#include <boost/config.hpp>
#include <boost/random.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_iterator.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graphviz.hpp>

#include "ProcessId.h"
#include "Synthetic.h"
#include "OSSim.h"
#include "ConfObject.h"
#include "stat-types.h"
#include "stat-boost-types.h"
#include "statPaths.h"
#include "memoryOperations.h"

//set the maximum number of instructions in each thread
#define MAX_INSTRUCTIONS 5000

extern std::string IntToString(INT_64 number);
extern std::string HexToString(INT_64 number);

class bfs_thread_visitor:public default_bfs_visitor
{

public:
   bfs_thread_visitor(THREAD_ID &threadIDIn, std::vector<FlowVertex> &listIn):threadID(threadIDIn), vertexList(listIn) { }

   template < typename Vertex, typename Graph >
   void discover_vertex(Vertex u, const Graph& g) const
   {
      flowNode_name_map_t        flowNode   = get(flowNode_t(), const_cast<Graph&>(g));
      flowIndex_name_map_t       nodeIndex  = get(vertex_index, const_cast<Graph&>(g));
      flowName_name_map_t        nodeName   = get(vertex_name,  const_cast<Graph&>(g));
      flow_edgeWeight_name_map_t edgeWeight = get(edge_weight,  const_cast<Graph&>(g));

      if(flowNode[u].return_threadID() == threadID)
      {
         vertexList.push_back(u);
      }
   }

   THREAD_ID   &threadID;
   std::vector<FlowVertex> &vertexList;
};

class bfs_depth_finder:public default_bfs_visitor
{

public:
   bfs_depth_finder(THREAD_ID &threadIDIn, std::vector< std::list < FlowVertex > > &listIn):threadID(threadIDIn), vertexList(listIn) { }

   template < typename Vertex, typename Graph >
   void discover_vertex(Vertex u, const Graph& g) const
   {
      flowNode_name_map_t        flowNode   = get(flowNode_t(), const_cast<Graph&>(g));
      flowIndex_name_map_t       nodeIndex  = get(vertex_index, const_cast<Graph&>(g));
      flowName_name_map_t        nodeName   = get(vertex_name,  const_cast<Graph&>(g));
      flow_edgeWeight_name_map_t edgeWeight = get(edge_weight,  const_cast<Graph&>(g));

      if(flowNode[u].return_threadID() != threadID)
      {
         vertexList[flowNode[u].return_threadID()].push_back(u);
      }
   }

   THREAD_ID   &threadID;
   std::vector< std::list < FlowVertex > > &vertexList;
};

class bfs_thread_locater:public default_bfs_visitor
{

public:
   bfs_thread_locater(THREAD_ID &threadIDIn, std::list < FlowVertex > &listIn):threadID(threadIDIn), vertexList(listIn) { }

   template < typename Vertex, typename Graph >
   void discover_vertex(Vertex u, const Graph& g) const
   {
      flowNode_name_map_t        flowNode   = get(flowNode_t(), const_cast<Graph&>(g));
      flowIndex_name_map_t       nodeIndex  = get(vertex_index, const_cast<Graph&>(g));
      flowName_name_map_t        nodeName   = get(vertex_name,  const_cast<Graph&>(g));
      flow_edgeWeight_name_map_t edgeWeight = get(edge_weight,  const_cast<Graph&>(g));

      if(flowNode[u].return_threadID() != threadID && flowNode[u].return_threadID() > 0)
      {
         BOOL found = 0;
         for(std::list < FlowVertex >::iterator listIterator = vertexList.begin(); listIterator != vertexList.end(); listIterator++)
         {
            if(flowNode[*listIterator].return_threadID() == flowNode[u].return_threadID())
            {
               found =1;
               break;
            }
         }

         if(found == 0)
            vertexList.push_back(u);
      }
   }

   THREAD_ID   &threadID;
   std::list < FlowVertex > &vertexList;
};

namespace GraphManipulation
{

void        generateSFGNodeIDs(void);
void        writeSFGDots(string name);
void        updateGraph(BasicBlock *basicBlockIn, THREAD_ID threadID);
UINT_32     getBasicBlockSize(THREAD_ID threadID, UINT_32 totalInstructions);
void        reduceSFG(void);
void        walkSFG(THREAD_ID threadID, Synthetic *syntheticThreads[], UINT_32 arraySize);
float       walkSFG(THREAD_ID threadID, Synthetic *tempSynth, float numInstructions);
float       walkSFG(THREAD_ID threadID, ADDRESS_INT startPC, Synthetic *tempSynth, float numInstructions);
float       walkSFG(THREAD_ID threadID, Synthetic *tempSynth, float numInstructions, FlowNode flowNodeIn, std::vector< FlowVertex > foundNodes);
float       accumulatorDump(THREAD_ID threadID, ADDRESS_INT address, Synthetic *tempSynth, float numInstructions);
void        cleanNode(ADDRESS_INT bbAddress, THREAD_ID threadID);
void        identifyDuplicateThreads(void);

void        generatePCFGNodeIDs(void);
void        writePCFGDots(string name);
void        reducePCFG(const std::vector < UINT_64 > &numInstructions);
void        walkPCFG(THREAD_ID threadID, Synthetic *syntheticThreads[], const UINT_32 &arraySize);

FlowVertex  addPCFGNode(const FlowNode &flowNodeIn);
void        finalizePCFG(void);
}

#endif
