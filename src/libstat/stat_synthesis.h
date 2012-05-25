//
// C++ Interface: stat_synthesis
//
// Description: 
//
//
// Author: Clay Hughes <hughes@fraidy2-uf>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef STAT_SYNTH_H
#define STAT_SYNTH_H

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/pending/integer_range.hpp>

#include <iostream>
#include <deque>

#include "InstructionMix.h"
#include "BasicBlock.h"
#include "Synthetic.h"
#include "ConfObject.h"

#include "stat-boost-types.h"
#include "codeGenerator.h"
#include "graphManipulation.h"
#include "printers.h"
#include "statPaths.h"

namespace Synthesis
{
void init(void);
void cleanup(void);
void instructionCounts(void);
void checkContainerSizes(THREAD_ID threadID);
void analysis(tuple<DInst, Time_t>  tempTuple);
void analysisCleanup(THREAD_ID threadID);
void finished(void);
}  //NOTE end Synthesis

#endif
