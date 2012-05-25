//
// C++ Interface: memoryOperations
//
// Description: 
//
//
// Author: Clay Hughes <hughes@fraidy2-uf>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <iostream>

#include "BasicBlock.h"
#include "Synthetic.h"
#include "ConfObject.h"

#include "stat-boost-types.h"
#include "codeGenerator.h"
#include "graphManipulation.h"
#include "printers.h"

#include "transReport.h"

namespace StatMemory
{
   void recordMemWrite(VAddr memoryAddress, INT_32 writeSize, ADDRESS_INT bbAddress, THREAD_ID threadID);
   void recordMemRead(VAddr memoryAddress, INT_32 writeSize, ADDRESS_INT bbAddress, THREAD_ID threadID);
   void normalizeBins(std::vector < UINT_32 > rawBins, float normBins[], UINT_32 size);

   void     initMemory(THREAD_ID threadID);
   UINT_32  returnReadStride(void);
   UINT_32  returnTransReadStride(ADDRESS_INT addressIn);
   UINT_32  returnWriteStride(void);
   UINT_32  returnTransWriteStride(ADDRESS_INT addressIn);
   void     buildGlobalMemoryMap(void);
   void     buildGlobalMemoryMap(const std::set<RAddr> &readSet, const std::set<RAddr> &writeSet);
   UINT_8   buildLocalMap(std::map< ADDRESS_INT, UINT_32 > &readConflictMap, std::map< ADDRESS_INT, UINT_32 > &writeConflictMap, ADDRESS_INT transID_A);
   UINT_8   getMemoryConflicts(std::map< ADDRESS_INT, UINT_32 > &readConflictMap, std::map< ADDRESS_INT, UINT_32 > &writeConflictMap, ADDRESS_INT transID_B);
   UINT_8   getMemoryConflicts(std::map < ADDRESS_INT, UINT_32 > &readConflictMap, std::map < ADDRESS_INT, UINT_32 > &writeConflictMap, ADDRESS_INT transID_A, ADDRESS_INT transID_B);
}
