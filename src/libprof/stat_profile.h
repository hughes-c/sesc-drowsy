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

#ifndef STAT_PROFILE_H
#define STAT_PROFILE_H

#include <iostream>
#include <sstream>

#include "ConfObject.h"
#include "stat-types.h"

#include "programStatistics.h"
#include "workloadCharacteristics.h"

namespace Profiling
{
void init(void);
void aggregateCharacteristics(INT_32 printType, BOOL threadProfiling);
inline BOOL regCheck(RegType destinationReg, DInst instructionIn);
void dependencyCheck(DInst tempDinst);
void analysis(tuple<DInst, Time_t>  tempTuple);
void finished(void);
}  //NOTE end Profiling

#endif
