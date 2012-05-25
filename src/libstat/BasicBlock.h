//
// C++ Interface: BasicBlock
//
// Description: 
//
//
/// @author: hughes  <hughes@fraidy2-uf>, (C) 2006
///
/// @date:          07/01/06
/// Last Modified: 07/05/07
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef BASICBLOCK_H
#define BASICBLOCK_H

#include <iostream>
#include <math.h>
#include <list>
#include <map>

#include "stat-types.h"
#include "DInst.h"

#include "InstructionMix.h"
#include "InstructionContainer.h"

//history is based on 2^n-bits
#define MAX_HISTORY 4294967295

//need to limit the maximum dependency distance
#define MAX_INS_DISTANCE 128
#define BIN_SIZE 10

class BasicBlock
{
public:
   /* Constructor */
   BasicBlock();
   BasicBlock(const BasicBlock& objectIn);

   /* Variables */
   InstructionMix             instructionMix;
   std::vector < UINT_32 >    childThreads;

   #if defined(PROFILE)
   std::map <void*, UINT_32>  memoryMap;          //map - <Effective Address, count>
   #endif

   /* Functions */
   UINT_8            update_bbAddress(ADDRESS_INT bbAddress);
   UINT_8            update_nodeID(UINT_32 nodeID);
   UINT_8            update_bbCount(UINT_64 bbCount);
   UINT_8            update_bbThreadCount(UINT_64 bbCount, THREAD_ID threadID);

   UINT_8            update_numInstructions(UINT_64 numInstructions);
   UINT_8            update_isThreadFunc(BOOL isThreadFunc);
   UINT_8            update_isTrans(BOOL isTrans);
   UINT_8            update_isCritical(BOOL isCritical);
   UINT_8            update_isSpawn(BOOL isSpawn);
   UINT_8            update_isDestroy(BOOL isDestroy);
   UINT_8            update_isShared(BOOL isShared);
   UINT_8            update_isWait(BOOL isWait);
   UINT_8            update_isBarrier(BOOL isBarrier);
   UINT_8            update_isThreadEvent(BOOL isThreadEvent);
   UINT_8            update_numDependancies(UINT_32 numDependancies);
   UINT_8            update_avgDistance(float avgDistance);
   UINT_8            update_stdDistance(float stdDistance);

   UINT_8            update_sharedMemReads(ADDRESS_INT address);
   UINT_8            update_sharedMemWrites(ADDRESS_INT address);
   UINT_8            clearLists(void);

   UINT_8            update_branchHistory(BOOL branchTaken);
   UINT_64           return_branchHistory(void) const;

   ADDRESS_INT       return_bbAddress(void) const;
   UINT_32           return_nodeID(void) const;
   UINT_64           return_bbCount(void) const;
   UINT_64           return_bbThreadCount(THREAD_ID threadID) const;
   UINT_32           return_numDependancies(void);
   UINT_32           return_numDependancies(void) const;

   UINT_64           return_numInstructions(void);
   UINT_64           return_numInstructions(void) const;
   BOOL              return_isThreadFunc(void);
   BOOL              return_isThreadFunc(void) const;
   BOOL              return_isTrans(void);
   BOOL              return_isTrans(void) const;
   BOOL              return_isCritical(void);
   BOOL              return_isCritical(void) const;
   BOOL              return_isSpawn(void);
   BOOL              return_isSpawn(void) const;
   BOOL              return_isDestroy(void);
   BOOL              return_isDestroy(void) const;
   BOOL              return_isShared(void);
   BOOL              return_isShared(void) const;
   BOOL              return_isWait(void);
   BOOL              return_isWait(void) const;
   BOOL              return_isBarrier(void);
   BOOL              return_isBarrier(void) const;
   BOOL              return_isThreadEvent(void);
   BOOL              return_isThreadEvent(void) const;

   float             return_avgDistance(void) const;
   float             return_stdDistance(void) const;

   ADDRESS_INT       return_sharedMemRead(void);
   ADDRESS_INT       return_sharedMemWrite(void);
   UINT_32           return_size_of_sharedMemReads(void);
   UINT_32           return_size_of_nextSharedMemRead(float myUniformRV);
   UINT_32           return_size_of_sharedMemWrites(void);
   UINT_32           return_size_of_nextSharedMemWrite(float myUniformRV);
   float*            return_normalizedSharedWriteBins(void);
   float*            return_normalizedSharedReadBins(void);

   UINT_8            update_threadID(THREAD_ID threadID);
   THREAD_ID         return_threadID() const;
   UINT_8            update_targetThread(THREAD_ID targetThread);
   THREAD_ID         return_targetThread() const;

   UINT_8            update_lockID(IntRegValue lockID);
   IntRegValue       return_lockID() const;
   UINT_8            update_transID(ADDRESS_INT transID);
   ADDRESS_INT       return_transID() const;
   UINT_8            update_nodeDepth(UINT_32 nodeDepth);
   UINT_32           return_nodeDepth() const;
   UINT_8            update_accumulated(UINT_32 accumulated);
   UINT_32           return_accumulated(void) const;

   BOOL              operator==(BasicBlock &) const;
   void              printNormalizedBins(void);
   void              findMemoryStatistics(void);
   void              findBBStatistics(void);

   UINT_8                              clearMemoryMaps(void);
   UINT_8                              update_readConflictMap(const std::map< ADDRESS_INT, UINT_32 > &mapIn);
   std::map< ADDRESS_INT, UINT_32 >    return_readConflictMap(void);
   std::map< ADDRESS_INT, UINT_32 >    return_readConflictMap(void) const;
   std::map< ADDRESS_INT, UINT_32 > &  return_readConflictMapRef(void);
   UINT_32                             return_readConflictMapSize(void) const;
   UINT_8                              update_writeConflictMap(const std::map< ADDRESS_INT, UINT_32 > &mapIn);
   std::map< ADDRESS_INT, UINT_32 >    return_writeConflictMap(void);
   std::map< ADDRESS_INT, UINT_32 >    return_writeConflictMap(void) const;
   std::map< ADDRESS_INT, UINT_32 > &  return_writeConflictMapRef(void);
   UINT_32                             return_writeConflictMapSize(void) const;

protected:


private:
   /* Variables */
   THREAD_ID         threadID;
   THREAD_ID         targetThread;
   ADDRESS_INT       bbAddress;                      //starting address of basic block
   UINT_32           nodeID;                         //unique node ID
   IntRegValue       lockID;
   ADDRESS_INT       transID;
   UINT_32           nodeDepth;

   UINT_64           branchHistory;                  //bit vector of last 64 branches

   UINT_32           numDependancies;                //number of dependancies in a basic block
   UINT_64           numInstructions;                //number of instructions in a basic block
   UINT_64           bbCount;                        //number of times the basic block is called
   UINT_64           bbThreadCount[];                //number of times the basic block is called [thread]

   BOOL              isThreadFunc;
   BOOL              isTrans;
   BOOL              isCritical;                     //is the basic block part of a critical section
   BOOL              isSpawn;                        //is a new thread spawn during this basic block
   BOOL              isDestroy;                      //is a thread destroyed during this basic block
   BOOL              isShared;
   BOOL              isWait;
   BOOL              isBarrier;
   BOOL              isThreadEvent;                  //high level -- toggled if any of the other events are set

   float             avg_distance;                   //average distance between dependancies
   float             std_distance;                   //standard deviation of distance between dependancies

   UINT_32           accumulated;

   std::list <ADDRESS_INT> sharedMemReads;
   std::list <ADDRESS_INT> sharedMemWrites;
   UINT_64                 sharedWriteBins[BIN_SIZE];
   UINT_64                 sharedReadBins[BIN_SIZE];
   float                   normalizedSharedWriteBins[BIN_SIZE];
   float                   normalizedSharedReadBins[BIN_SIZE];

public:
   /* Functions */
   UINT_8 clear_instructionList();
   UINT_8 resize_instructionList(UINT_32 newSize);
   UINT_8 erase_instructionList(UINT_32 element_a);
   UINT_8 erase_instructionList(UINT_32 first, UINT_32 last);
   UINT_8 copy_instructionList(const std::list <InstructionContainer> &listIn);
   UINT_8 update_instructionList(DInst dynamic_instruction);

   std::list <InstructionContainer>    return_instructionList(void) const;
   std::list <InstructionContainer> &  return_instructionListRef(void);
   InstructionContainer                return_back_of_instructionList(void);
   InstructionContainer                return_front_of_instructionList(void);
   InstructionContainer                return_front_of_instructionList_pop(void);
   UINT_32                             return_instructionListSize(void);

   void print_instructionList(std::ostream &outputStream);

private:
   /* Variables */
   std::list < InstructionContainer >  instructionList;
   std::map  < ADDRESS_INT, UINT_32 >  readConflictMap;
   std::map  < ADDRESS_INT, UINT_32 >  writeConflictMap;

};

#endif
