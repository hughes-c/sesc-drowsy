//
// C++ Implementation: FlowNode
//
// Description:
//
//
/// @author: hughes  <hughes@fraidy2-uf>, (C) 2006
///
/// @date:          06/21/08
/// Last Modified: 06/23/08
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "FlowNode.h"

FlowNode::FlowNode()
{
   threadID.first = 0;
   threadID.second = 0;
   numInstructions = 0;
   weighted_numInstructions = 0;
   parentThread = 0;
   lockID = 0;
   transID = 0;
   startPC = 0;
   isSpawn = 0;
   isTrans = 0;
   isCritical = 0;
   isWait = 0;
   isBarrier = 0;

}

FlowNode::FlowNode(const FlowNode& objectIn)
{
   threadID.first = objectIn.threadID.first;
   threadID.second = objectIn.threadID.second;
   numInstructions = objectIn.numInstructions;
   weighted_numInstructions = objectIn.weighted_numInstructions;
   parentThread = objectIn.parentThread;
   lockID = objectIn.lockID;
   transID = objectIn.transID;
   startPC = objectIn.startPC;
   isSpawn = objectIn.isSpawn;
   isTrans = objectIn.isTrans;
   isCritical = objectIn.isCritical;
   isWait = objectIn.isWait;
   isBarrier = objectIn.isBarrier;

   instructionMix = objectIn.instructionMix;
   childThreads = objectIn.childThreads;
}

FlowNode::FlowNode(BOOL isFirst, THREAD_ID threadID, UINT_64 numInstructions, THREAD_ID targetThread, IntRegValue lockID, ADDRESS_INT transID, BOOL isSpawn, BOOL isTrans, BOOL isCritical, BOOL isWait, BOOL isBarrier)
{
   this->threadID.first = threadID;
   this->threadID.second = isFirst;
   this->numInstructions = numInstructions;
   this->lockID = lockID;
   this->transID = transID;
   this->isSpawn = isSpawn;
   this->isTrans = isTrans;
   this->isCritical = isCritical;
   this->isWait = isWait;
   this->isBarrier = isBarrier;

   this->weighted_numInstructions = 0;
   this->startPC = 0;
   this->parentThread = 0;

   if(targetThread > 0)
      childThreads.push_back(targetThread);
}

UINT_8 FlowNode::update_threadID(THREAD_ID threadID)
{
   this->threadID.first = threadID;
   return 1;
}

UINT_8 FlowNode::update_numInstructions(UINT_64 numInstructions)
{
   this->numInstructions = numInstructions;
   return 1;
}

UINT_8 FlowNode::incrementNumInstructions(void)
{
   this->numInstructions = this->numInstructions + 1;
   return 1;
}

UINT_8 FlowNode::incrementNumInstructions(UINT_64 numInstructions)
{
   this->numInstructions = this->numInstructions + numInstructions;
   return 1;
}

UINT_8 FlowNode::update_weighted_numInstructions(float weighted_numInstructions)
{
   this->weighted_numInstructions = weighted_numInstructions;
   return 1;
}

UINT_8 FlowNode::update_parentThread(THREAD_ID parentThread)
{
   this->parentThread = parentThread;
   return 1;
}

UINT_8 FlowNode::update_lockID(IntRegValue lockID)
{
   this->lockID = lockID;
   return 1;
}

UINT_8 FlowNode::update_transID(ADDRESS_INT transID)
{
   this->transID = transID;
   return 1;
}

UINT_8 FlowNode::update_startPC(ADDRESS_INT startPC)
{
   this->startPC = startPC;
   return 1;
}

UINT_8 FlowNode::update_isSpawn(BOOL isSpawn)
{
   this->isSpawn = isSpawn;
   return 1;
}

UINT_8 FlowNode::update_isTrans(BOOL isTrans)
{
   this->isTrans = isTrans;
   return 1;
}

UINT_8 FlowNode::update_isCritical(BOOL isCritical)
{
   this->isCritical = isCritical;
   return 1;
}

UINT_8 FlowNode::update_isWait(BOOL isWait)
{
   this->isWait = isWait;
   return 1;
}

UINT_8 FlowNode::update_isBarrier(BOOL isBarrier)
{
   this->isBarrier = isBarrier;
   return 1;
}

THREAD_ID FlowNode::return_threadID()
{
   return this->threadID.first;
}

BOOL FlowNode::return_isFirst()
{
   return this->threadID.second;
}

UINT_64 FlowNode::return_numInstructions(void)
{
   return this->numInstructions;
}

float FlowNode::return_weighted_numInstructions(void)
{
   return this->weighted_numInstructions;
}

THREAD_ID FlowNode::return_parentThread(void)
{
   return this->parentThread;
}

IntRegValue FlowNode::return_lockID()
{
   return this->lockID;
}

ADDRESS_INT FlowNode::return_transID()
{
   return this->transID;
}

ADDRESS_INT FlowNode::return_startPC()
{
   return this->startPC;
}

BOOL FlowNode::return_isSpawn(void)
{
   return this->isSpawn;
}

BOOL FlowNode::return_isTrans(void)
{
   return this->isTrans;
}

BOOL FlowNode::return_isCritical(void)
{
   return this->isCritical;
}

BOOL FlowNode::return_isWait(void)
{
   return this->isWait;
}

BOOL FlowNode::return_isBarrier(void)
{
   return this->isBarrier;
}

THREAD_ID FlowNode::return_threadID() const
{
   return this->threadID.first;
}

BOOL FlowNode::return_isFirst() const
{
   return this->threadID.second;
}

UINT_64 FlowNode::return_numInstructions(void) const
{
   return this->numInstructions;
}

float FlowNode::return_weighted_numInstructions(void) const
{
   return this->weighted_numInstructions;
}

THREAD_ID FlowNode::return_parentThread(void) const
{
   return this->parentThread;
}

IntRegValue FlowNode::return_lockID() const
{
   return this->lockID;
}

ADDRESS_INT FlowNode::return_transID() const
{
   return this->transID;
}

ADDRESS_INT FlowNode::return_startPC() const
{
   return this->startPC;
}

BOOL FlowNode::return_isSpawn(void) const
{
   return this->isSpawn;
}

BOOL FlowNode::return_isTrans(void) const
{
   return this->isTrans;
}

BOOL FlowNode::return_isCritical(void) const
{
   return this->isCritical;
}

BOOL FlowNode::return_isWait(void) const
{
   return this->isWait;
}

BOOL FlowNode::return_isBarrier(void) const
{
   return this->isBarrier;
}
