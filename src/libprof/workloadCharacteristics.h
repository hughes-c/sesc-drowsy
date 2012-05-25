//
// C++ Interface: workloadCharacteristics
//
// Description: 
//
//
/// @author: hughes  <hughes@fraidy2-uf>, (C) 2008
///
/// @date:          04/01/08
/// Last Modified: 07/24/09
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef WORKLOADCHARACTERISTICS_H
#define WORKLOADCHARACTERISTICS_H

#include "OSSim.h"
#include "stat-types.h"
#include "stat-boost-types.h"

// #define DEP_BINS 33
#define DEP_BINS 6
#define STRIDE_BINS 10
#define BRANCH_BINS 11
#define TX_STRIDE_BINS 8

class BranchStatistics
{
public:
   BranchStatistics(): taken(0), notTaken(0), transition(0), lastBranch(0) {};

   UINT_32  taken;
   UINT_32  notTaken;
   UINT_32  transition;
   UINT_32  lastBranch;             //0 notTaken, 1 taken
};

typedef  std::map <ADDRESS_INT, BranchStatistics>  BranchMap;
typedef  std::map <ADDRESS_INT, UINT_64>           InstructionMap;
typedef  std::deque <UINT_32>                      BasicBlockDeque;

class WorkloadCharacteristics
{
public:
   /* Constructors */
   WorkloadCharacteristics();
   WorkloadCharacteristics(const WorkloadCharacteristics& objectIn);
   WorkloadCharacteristics(Time_t cycleIn, VAddr addressIn);
   ~WorkloadCharacteristics();

   UINT_8         update_loadMix(UINT_32 loadMix);
   UINT_8         update_storeMix(UINT_32 storeMix);
   UINT_8         update_intShortMix(UINT_32 intMix);
   UINT_8         update_intLongMix(UINT_32 intMix);
   UINT_8         update_fpMix(UINT_32 fpMix);
   UINT_8         update_branchMix(UINT_32 branchMix);
   UINT_8         update_dependencyDistance(UINT_32 depDist);
   UINT_8         update_dataFootprint(UINT_32 dataFootprint);
   UINT_8         update_dataStride(VAddr currentAddress);
   UINT_8         update_transactionStride(UINT_32 transactionDistance);
   UINT_8         update_memoryMap(VAddr memoryAddress);
   UINT_8         update_totalInstructionCount(UINT_64 totalInstructionCount);
   UINT_8         update_branchRate(UINT_32 branchRate, UINT_32 bin);
   UINT_8         update_averageBlockSize(double averageBlockSize);
   UINT_8         update_stdDevBlockSize(double stdDevBlockSize);
   UINT_8         update_basicBlock(WorkloadCharacteristics basicBlockIn);
   UINT_8         update_basicBlock(WorkloadCharacteristics basicBlockIn, TX_ID transactionID);
   UINT_8         update_cacheLineSize(INT_32 cacheLineSize);
   UINT_8         update_transactionID(TX_ID transactionID);
   UINT_8         update_branchMap(ADDRESS_INT instructionAddress, UINT_32 isTaken);
   UINT_8         update_instructionMap(ADDRESS_INT instructionAddress);
   UINT_8         writeBranches(void);
   UINT_8         add_cycleTime(Time_t cycleTime);
   UINT_8         update_cycleTime(Time_t cycleTime);
   UINT_8         update_NormalCycleTime(float normal_cycleTime);

   float          return_loadMix(void);
   float          return_storeMix(void);
   float          return_intShortMix(void);
   float          return_intLongMix(void);
   float          return_fpMix(void);
   float          return_branchMix(void);
   float          return_dependencyDistance(UINT_32 counter);
   float          return_dataFootprint(void);
   float          return_dataStride(UINT_32 counter);
   float          return_totalInstructionCount(void);
   float          return_branchRate(UINT_32 counter);
   TX_ID          return_transactionID(void);
   double         return_averageBlockSize(void);
   double         return_stdDevBlockSize(void);
   Time_t         return_cycleTime(void);
   float          return_normalCycleTime(void);
   Time_t         return_lastCycle();
   VAddr          return_lastAddress();

   UINT_8         reset(void);
   UINT_8         reset(Time_t cycleIn, VAddr addressIn);
   UINT_8         analyzeResults(void);
   UINT_8         normalizeResults(void);
   UINT_8         reduceResults(float reduction);
   UINT_8         print(INT_32 printType);
   UINT_8         print(INT_32 printType, string reportFileName);
   UINT_8         printTransactionStride(void);
   UINT_8         printTransactionStride(INT_32 printType, string reportFileName);

   WorkloadCharacteristics &operator=(const WorkloadCharacteristics &objectIn);
   WorkloadCharacteristics &operator+=(const WorkloadCharacteristics &objectIn);
   const WorkloadCharacteristics operator+(const WorkloadCharacteristics &objectIn) const;

protected:
   AddressMap           addressMap;
   BasicBlockDeque      basicBlockSize;
   BranchMap            branchMap;
   InstructionMap       instructionMap;

private:
   /* Variables */
   float          loadMix;
   float          storeMix;
   float          intShortMix;
   float          intLongMix;
   float          fpMix;
   float          branchMix;
   float          dataFootprint;
   float          instructionFootprint;
   float          totalInstructionCount;

   float          dataStride[STRIDE_BINS];
   float          branchRate[BRANCH_BINS];
   float          dependencyDistance[DEP_BINS];
   float          transactionStride[STRIDE_BINS];

   UINT_64        totalBasicBlockSize;
   UINT_64        totalBasicBlockCount;
   double         averageBlockSize;
   double         stdDevBlockSize;

   Time_t         cycleTime;
   Time_t         lastCycle;
   VAddr          lastAddress;
   INT_32         cacheLineSize;
   TX_ID          transactionID;

   //normalization
   float          normal_loadMix;
   float          normal_storeMix;
   float          normal_intShortMix;
   float          normal_intLongMix;
   float          normal_fpMix;
   float          normal_branchMix;
   float          normal_dataFootprint;
   float          normal_instructionFootprint;
   float          normal_totalInstructionCount;
   float          normal_cycleTime;

   float          normal_dataStride[STRIDE_BINS];
   float          normal_branchRate[BRANCH_BINS];
   float          normal_dependencyDistance[DEP_BINS];
   float          normal_transactionStride[STRIDE_BINS];
};

#endif
