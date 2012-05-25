//
// C++ Interface: FlowNode
//
// Description: 
//
//
/// @author: hughes  <hughes@fraidy2-uf>, (C) 2006, 2007, 2008
///
/// @date:          06/21/08
/// Last Modified: 07/11/08
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FLOWNODE_H
#define FLOWNODE_H

#include <list>
#include <vector>
#include <utility>
#include "stat-types.h"
#include "InstructionMix.h"

class FlowNode
{
public:
   /* Constructor */
   FlowNode();
   FlowNode(const FlowNode& objectIn);
   FlowNode(BOOL isFirst, THREAD_ID threadID, UINT_64 numInstructions, THREAD_ID targetThread, IntRegValue lockID, ADDRESS_INT transID, BOOL isSpawn, BOOL isTrans, BOOL isCritical, BOOL isWait, BOOL isBarrier);

   /* Variables */
   InstructionMix                instructionMix;
   std::vector< UINT_32 >        childThreads;

   /* Functions */
   UINT_8            update_threadID(THREAD_ID threadID);
   UINT_8            update_numInstructions(UINT_64 numInstructions);
   UINT_8            incrementNumInstructions(void);
   UINT_8            incrementNumInstructions(UINT_64 numInstructions);
   UINT_8            update_weighted_numInstructions(float weighted_numInstructions);
   UINT_8            update_parentThread(THREAD_ID threadID);
   UINT_8            update_lockID(IntRegValue lockID);
   UINT_8            update_transID(ADDRESS_INT transID);
   UINT_8            update_startPC(ADDRESS_INT startPC);
   UINT_8            update_isSpawn(BOOL isSpawn);
   UINT_8            update_isTrans(BOOL isTrans);
   UINT_8            update_isCritical(BOOL isCritical);
   UINT_8            update_isWait(BOOL isWait);
   UINT_8            update_isBarrier(BOOL isBarrier);

   THREAD_ID         return_threadID(void);
   THREAD_ID         return_threadID(void) const;
   BOOL              return_isFirst(void);
   BOOL              return_isFirst(void) const;
   UINT_64           return_numInstructions(void);
   UINT_64           return_numInstructions(void) const;
   float             return_weighted_numInstructions(void);
   float             return_weighted_numInstructions(void) const;

   THREAD_ID         return_parentThread(void);
   THREAD_ID         return_parentThread(void) const;
   BOOL              return_isSpawn(void);
   BOOL              return_isSpawn(void) const;
   IntRegValue       return_lockID(void);
   IntRegValue       return_lockID(void) const;
   ADDRESS_INT       return_transID(void);
   ADDRESS_INT       return_transID(void) const;
   ADDRESS_INT       return_startPC(void);
   ADDRESS_INT       return_startPC(void) const;

   BOOL              return_isTrans(void);
   BOOL              return_isTrans(void) const;
   BOOL              return_isCritical(void);
   BOOL              return_isCritical(void) const;
   BOOL              return_isWait(void);
   BOOL              return_isWait(void) const;
   BOOL              return_isBarrier(void);
   BOOL              return_isBarrier(void) const;

protected:


private:

   std::pair< THREAD_ID, BOOL >  threadID;                       //threadID and whether this is the first node or not -- threadID/isFirst
   UINT_64                       numInstructions;
   float                         weighted_numInstructions;

   THREAD_ID                     parentThread;
   IntRegValue                   lockID;
   ADDRESS_INT                   transID;
   ADDRESS_INT                   startPC;

   BOOL                          isSpawn;
   BOOL                          isTrans;
   BOOL                          isCritical;
   BOOL                          isWait;
   BOOL                          isBarrier;
};

#endif
