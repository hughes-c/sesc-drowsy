//
// C++ Interface: stat-boost-types
//
// Description: 
//
//
// Author: hughes,,, <hughes@fraidy2-uf>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef STAT_BOOST_TYPES_H
#define STAT_BOOST_TYPES_H

#include <boost/config.hpp>
#include <boost/random.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_iterator.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/depth_first_search.hpp>

#include "BasicBlock.h"
#include "FlowNode.h"

#if defined(PROFILE)
#define MAX_NUM_THREADS 4096
#elif defined(STAT)
#define MAX_NUM_THREADS 16
#endif

using namespace boost;

struct basicBlock_t
{
	typedef vertex_property_tag kind;
};
struct flowNode_t
{
	typedef vertex_property_tag kind;
};
struct parentThread_t
{
	typedef vertex_property_tag kind;
};

//SFG
typedef property <basicBlock_t, BasicBlock, property<vertex_index_t, int, property<vertex_color_t, default_color_type, property<vertex_name_t, string> > > > NodeProperties;
typedef property <edge_weight_t, float> EdgeWeightProperty;
typedef property <parentThread_t, UINT_32> EdgeParentProperty;

typedef adjacency_list <listS, listS, bidirectionalS, NodeProperties, EdgeWeightProperty> BBGraph;          //in_edges & out_edges

typedef property_map <BBGraph, basicBlock_t  >::type  basicBlock_name_map_t;
typedef property_map <BBGraph, edge_weight_t >::type  edgeWeight_name_map_t;
typedef property_map <BBGraph, vertex_color_t>::type  nodeColor_name_map_t;
typedef property_map <BBGraph, vertex_index_t>::type  nodeIndex_name_map_t;
typedef property_map <BBGraph, vertex_name_t >::type  nodeName_name_map_t;

typedef graph_traits <BBGraph>::vertex_descriptor BBVertex;

typedef map <ADDRESS_INT, BBVertex > BBVertexMap;                                                           //map -- BB Address -> Vertex
typedef map <ADDRESS_INT, UINT_32  > MutexMap;                                                              //map -- BB Address -> Vertex
typedef map <ADDRESS_INT, UINT_32  > AddressMap;                                                            //map -- Memory Address -> base address offset for synthetic

typedef minstd_rand base_generator_type;

//PCFG
typedef property <flowNode_t, FlowNode, property<vertex_index_t, int, property<vertex_color_t, default_color_type, property<vertex_name_t, string> > > > FlowNodeProperties;
typedef property <edge_weight_t, UINT_32> FlowEdgeProperty;

typedef adjacency_list <listS, listS, bidirectionalS, FlowNodeProperties, FlowEdgeProperty> PCFG;

typedef property_map <PCFG, flowNode_t    >::type  flowNode_name_map_t;
typedef property_map <PCFG, edge_weight_t >::type  flow_edgeWeight_name_map_t;
typedef property_map <PCFG, vertex_color_t>::type  flowColor_name_map_t;
typedef property_map <PCFG, vertex_index_t>::type  flowIndex_name_map_t;
typedef property_map <PCFG, vertex_name_t >::type  flowName_name_map_t;

typedef graph_traits <PCFG>::vertex_descriptor FlowVertex;

#endif
