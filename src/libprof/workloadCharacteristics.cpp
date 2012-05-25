//
// C++ Implementation: sequentialcharacteristics
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

#include "workloadCharacteristics.h"

WorkloadCharacteristics::WorkloadCharacteristics()
{
   normal_loadMix = loadMix = 0;
   normal_storeMix = storeMix = 0;
   normal_intShortMix = intShortMix = 0;
   normal_intLongMix = intLongMix = 0;
   normal_fpMix = fpMix = 0;
   normal_branchMix = branchMix = 0;
   normal_dataFootprint = dataFootprint = 0;
   normal_instructionFootprint = instructionFootprint = 0;
   normal_totalInstructionCount = totalInstructionCount = 0;

   averageBlockSize = 0;
   stdDevBlockSize = 0;

   normal_cycleTime = cycleTime = 0;
   lastCycle = 0;
   lastAddress = 0;
   cacheLineSize = 0;
   transactionID = 0;

   for(UINT_32 counter = 0; counter < DEP_BINS; counter++)
   {
      normal_dependencyDistance[counter] = dependencyDistance[counter] = 0;
   }
   for(UINT_32 counter = 0; counter < STRIDE_BINS; counter++)
   {
      normal_dataStride[counter] = dataStride[counter] = 0;
   }
   for(UINT_32 counter = 0; counter < TX_STRIDE_BINS; counter++)
   {
      normal_transactionStride[counter] = transactionStride[counter] = 0;
   }
   for(UINT_32 counter = 0; counter < BRANCH_BINS; counter++)
   {
      normal_branchRate[counter] = branchRate[counter] = 0;
   }
}

WorkloadCharacteristics::WorkloadCharacteristics(Time_t cycleIn, VAddr addressIn)
{
   normal_loadMix = loadMix = 0;
   normal_storeMix = storeMix = 0;
   normal_intShortMix = intShortMix = 0;
   normal_intLongMix = intLongMix = 0;
   normal_fpMix = fpMix = 0;
   normal_branchMix = branchMix = 0;
   normal_dataFootprint = dataFootprint = 0;
   normal_instructionFootprint = instructionFootprint = 0;
   normal_totalInstructionCount = totalInstructionCount = 0;

   averageBlockSize = 0;
   stdDevBlockSize = 0;

   normal_cycleTime = cycleTime = 0;
   lastCycle = cycleIn;
   lastAddress = addressIn;

   for(UINT_32 counter = 0; counter < DEP_BINS; counter++)
   {
      normal_dependencyDistance[counter] = dependencyDistance[counter] = 0;
   }
   for(UINT_32 counter = 0; counter < STRIDE_BINS; counter++)
   {
      normal_dataStride[counter] = dataStride[counter] = 0;
   }
   for(UINT_32 counter = 0; counter < TX_STRIDE_BINS; counter++)
   {
      normal_transactionStride[counter] = transactionStride[counter] = 0;
   }
   for(UINT_32 counter = 0; counter < BRANCH_BINS; counter++)
   {
      normal_branchRate[counter] = branchRate[counter] = 0;
   }
}

WorkloadCharacteristics::WorkloadCharacteristics(const WorkloadCharacteristics& objectIn)
{
   addressMap = objectIn.addressMap;
   basicBlockSize = objectIn.basicBlockSize;
   branchMap = objectIn.branchMap;
   instructionMap = objectIn.instructionMap;

   loadMix = objectIn.loadMix;
   storeMix = objectIn.storeMix;
   intShortMix = objectIn.intShortMix;
   intLongMix = objectIn.intLongMix;
   fpMix = objectIn.fpMix;
   branchMix = objectIn.branchMix;
   dataFootprint = objectIn.dataFootprint;
   instructionFootprint = objectIn.instructionFootprint;
   totalInstructionCount = objectIn.totalInstructionCount;

   averageBlockSize = objectIn.averageBlockSize;
   stdDevBlockSize = objectIn.stdDevBlockSize;

   cycleTime = objectIn.cycleTime;
   lastCycle = objectIn.lastCycle;
   lastAddress = objectIn.lastAddress;
   cacheLineSize = objectIn.cacheLineSize;
   transactionID = objectIn.transactionID;

   normal_loadMix = objectIn.normal_loadMix;
   normal_storeMix = objectIn.normal_storeMix;
   normal_intShortMix = objectIn.normal_intShortMix;
   normal_intLongMix = objectIn.normal_intLongMix;
   normal_fpMix = objectIn.normal_fpMix;
   normal_branchMix = objectIn.normal_branchMix;
   normal_dataFootprint = objectIn.normal_dataFootprint;
   normal_instructionFootprint = objectIn.normal_instructionFootprint;
   normal_totalInstructionCount = objectIn.normal_totalInstructionCount;
   normal_cycleTime = objectIn.normal_cycleTime;

   for(UINT_32 counter = 0; counter < DEP_BINS; counter++)
   {
      dependencyDistance[counter] = objectIn.dependencyDistance[counter];
      normal_dependencyDistance[counter] = objectIn.normal_dependencyDistance[counter];
   }

   for(UINT_32 counter = 0; counter < STRIDE_BINS; counter++)
   {
      dataStride[counter] = objectIn.dataStride[counter];
      normal_dataStride[counter] = objectIn.normal_dataStride[counter];
   }

   for(UINT_32 counter = 0; counter < TX_STRIDE_BINS; counter++)
   {
      transactionStride[counter] = objectIn.transactionStride[counter];
      normal_transactionStride[counter] = objectIn.normal_transactionStride[counter];
   }

   for(UINT_32 counter = 0; counter < BRANCH_BINS; counter++)
   {
      branchRate[counter] = objectIn.branchRate[counter];
      normal_branchRate[counter] = objectIn.normal_branchRate[counter];
   }

}

WorkloadCharacteristics::~WorkloadCharacteristics()
{
}

UINT_8 WorkloadCharacteristics::update_loadMix(UINT_32 loadMix)
{
   this->loadMix = this->loadMix + loadMix;
   return 1;
}

UINT_8 WorkloadCharacteristics::update_storeMix(UINT_32 storeMix)
{
   this->storeMix = this->storeMix + storeMix;
   return 1;
}

UINT_8 WorkloadCharacteristics::update_intShortMix(UINT_32 intShortMix)
{
   this->intShortMix = this->intShortMix + intShortMix;
   return 1;
}

UINT_8 WorkloadCharacteristics::update_intLongMix(UINT_32 intLongMix)
{
   this->intLongMix = this->intLongMix + intLongMix;
   return 1;
}

UINT_8 WorkloadCharacteristics::update_fpMix(UINT_32 fpMix)
{
   this->fpMix = this->fpMix + fpMix;
   return 1;
}

UINT_8 WorkloadCharacteristics::update_branchMix(UINT_32 branchMix)
{
   this->branchMix = this->branchMix + branchMix;
   return 1;
}

UINT_8 WorkloadCharacteristics::update_dependencyDistance(UINT_32 depDist)
{
   if(depDist == 1)
      dependencyDistance[0] = dependencyDistance[0] + 1;
   else if(depDist <= 2)
      dependencyDistance[1] = dependencyDistance[1] + 1;
   else if(depDist <= 4)
      dependencyDistance[2] = dependencyDistance[2] + 1;
   else if(depDist <= 8)
      dependencyDistance[3] = dependencyDistance[3] + 1;
   else if(depDist <= 16)
      dependencyDistance[4] = dependencyDistance[4] + 1;
   else
      dependencyDistance[5] = dependencyDistance[5] + 1;

//    if(depDist == 1)
//       dependencyDistance[0] = dependencyDistance[0] + 1;
//    else if(depDist == 2)
//       dependencyDistance[1] = dependencyDistance[1] + 1;
//    else if(depDist == 3)
//       dependencyDistance[2] = dependencyDistance[2] + 1;
//    else if(depDist == 4)
//       dependencyDistance[3] = dependencyDistance[3] + 1;
//    else if(depDist == 5)
//       dependencyDistance[4] = dependencyDistance[4] + 1;
//    else if(depDist == 6)
//       dependencyDistance[5] = dependencyDistance[5] + 1;
//    else if(depDist == 7)
//       dependencyDistance[6] = dependencyDistance[6] + 1;
//    else if(depDist == 8)
//       dependencyDistance[7] = dependencyDistance[7] + 1;
//    else if(depDist == 9)
//       dependencyDistance[8] = dependencyDistance[8] + 1;
//    else if(depDist == 10)
//       dependencyDistance[9] = dependencyDistance[9] + 1;
//    else if(depDist == 11)
//       dependencyDistance[10] = dependencyDistance[10] + 1;
//    else if(depDist == 12)
//       dependencyDistance[11] = dependencyDistance[11] + 1;
//    else if(depDist == 13)
//       dependencyDistance[12] = dependencyDistance[12] + 1;
//    else if(depDist == 14)
//       dependencyDistance[13] = dependencyDistance[13] + 1;
//    else if(depDist == 15)
//       dependencyDistance[14] = dependencyDistance[14] + 1;
//    else if(depDist == 16)
//       dependencyDistance[15] = dependencyDistance[15] + 1;
//    else if(depDist == 17)
//       dependencyDistance[16] = dependencyDistance[16] + 1;
//    else if(depDist == 18)
//       dependencyDistance[17] = dependencyDistance[17] + 1;
//    else if(depDist == 19)
//       dependencyDistance[18] = dependencyDistance[18] + 1;
//    else if(depDist == 20)
//       dependencyDistance[19] = dependencyDistance[19] + 1;
//    else if(depDist == 21)
//       dependencyDistance[20] = dependencyDistance[20] + 1;
//    else if(depDist == 22)
//       dependencyDistance[21] = dependencyDistance[21] + 1;
//    else if(depDist == 23)
//       dependencyDistance[22] = dependencyDistance[22] + 1;
//    else if(depDist == 24)
//       dependencyDistance[23] = dependencyDistance[23] + 1;
//    else if(depDist == 25)
//       dependencyDistance[24] = dependencyDistance[24] + 1;
//    else if(depDist == 26)
//       dependencyDistance[25] = dependencyDistance[25] + 1;
//    else if(depDist == 27)
//       dependencyDistance[26] = dependencyDistance[26] + 1;
//    else if(depDist == 28)
//       dependencyDistance[27] = dependencyDistance[27] + 1;
//    else if(depDist == 29)
//       dependencyDistance[28] = dependencyDistance[28] + 1;
//    else if(depDist == 30)
//       dependencyDistance[29] = dependencyDistance[29] + 1;
//    else if(depDist == 31)
//       dependencyDistance[30] = dependencyDistance[30] + 1;
//    else if(depDist == 32)
//       dependencyDistance[31] = dependencyDistance[31] + 1;
//    else
//       dependencyDistance[32] = dependencyDistance[32] + 1;

   return 1;
}

UINT_8 WorkloadCharacteristics::update_dataFootprint(UINT_32 dataFootprint)
{
   this->dataFootprint = this->dataFootprint + dataFootprint;
   return 1;
}

UINT_8 WorkloadCharacteristics::update_dataStride(VAddr currentAddress)
{
   UINT_32 delta = 0;

   if(lastAddress == 0)
      delta = 0;
   else if(lastAddress > currentAddress)
      delta = (lastAddress - currentAddress);
   else
      delta = (currentAddress -lastAddress);

   //based on the assumption that one cache line is 32B
   if(delta < 31)
      dataStride[0] = dataStride[0] + 1;
   else if(delta < 63)
      dataStride[1] = dataStride[1] + 1;
   else if(delta < 95)
      dataStride[2] = dataStride[2] + 1;
   else if(delta < 127)
      dataStride[3] = dataStride[3] + 1;
   else if(delta < 159)
      dataStride[4] = dataStride[4] + 1;
   else if(delta < 191)
      dataStride[5] = dataStride[5] + 1;
   else if(delta < 223)
      dataStride[6] = dataStride[6] + 1;
   else if(delta < 255)
      dataStride[7] = dataStride[7] + 1;
   else if(delta < 287)
      dataStride[8] = dataStride[8] + 1;
   else
      dataStride[9] = dataStride[9] + 1;

   lastAddress = currentAddress;

   return 1;
}

UINT_8 WorkloadCharacteristics::update_transactionStride(UINT_32 transactionDistance)
{
   if(transactionDistance == 0)
      transactionStride[0] = transactionStride[0] + 1;
   else if(transactionDistance < 10)
      transactionStride[1] = transactionStride[1] + 1;
   else if(transactionDistance < 100)
      transactionStride[2] = transactionStride[2] + 1;
   else if(transactionDistance < 1000)
      transactionStride[3] = transactionStride[3] + 1;
   else if(transactionDistance < 10000)
      transactionStride[4] = transactionStride[4] + 1;
   else if(transactionDistance < 100000)
      transactionStride[5] = transactionStride[5] + 1;
   else if(transactionDistance < 1000000)
      transactionStride[6] = transactionStride[6] + 1;
   else
      transactionStride[7] = transactionStride[7] + 1;

   return 1;
}

UINT_8 WorkloadCharacteristics::update_memoryMap(VAddr memoryAddress)
{
   BOOL unique;
   AddressMap::iterator addressIterator;
   boost::tie(addressIterator, unique) = this->addressMap.insert(make_pair(memoryAddress, 1));

   return 1;
}

UINT_8 WorkloadCharacteristics::update_totalInstructionCount(UINT_64 totalInstructionCount)
{
   this->totalInstructionCount = this->totalInstructionCount + totalInstructionCount;
   return 1;
}

UINT_8 WorkloadCharacteristics::update_branchRate(UINT_32 branchRate, UINT_32 bin)
{
   this->branchRate[bin] = branchRate;
   return 1;
}

UINT_8 WorkloadCharacteristics::update_averageBlockSize(double averageBlockSize)
{
   this->averageBlockSize = averageBlockSize;
   return 1;
}

UINT_8 WorkloadCharacteristics::update_stdDevBlockSize(double stdDevBlockSize)
{
   this->stdDevBlockSize = stdDevBlockSize;
   return 1;
}

UINT_8 WorkloadCharacteristics::update_basicBlock(WorkloadCharacteristics basicBlockIn)
{
   this->loadMix = this->loadMix + basicBlockIn.return_loadMix();
   this->storeMix = this->storeMix + basicBlockIn.return_storeMix();
   this->intShortMix = this->intShortMix + basicBlockIn.return_intShortMix();
   this->intLongMix = this->intLongMix + basicBlockIn.return_intLongMix();
   this->fpMix = this->fpMix + basicBlockIn.return_fpMix();
   this->branchMix = this->branchMix + basicBlockIn.return_branchMix();

   this->totalInstructionCount = this->totalInstructionCount + basicBlockIn.return_totalInstructionCount();
   this->basicBlockSize.push_back((UINT_32)basicBlockIn.return_totalInstructionCount());

   this->cycleTime = this->cycleTime + basicBlockIn.return_cycleTime();

   pair<AddressMap::iterator, BOOL> temp;
   for(AddressMap::iterator addressIterator = basicBlockIn.addressMap.begin(); addressIterator != basicBlockIn.addressMap.end(); addressIterator++)
   {
       temp = this->addressMap.insert(make_pair(addressIterator->first, 1));
   }

   BOOL unique;
   InstructionMap::iterator instructionIterator_u;
   for(InstructionMap::iterator instructionIterator = basicBlockIn.instructionMap.begin(); instructionIterator != basicBlockIn.instructionMap.end(); instructionIterator++)
   {
      boost::tie(instructionIterator_u, unique) = this->instructionMap.insert(make_pair(instructionIterator->first, 1));
      if(unique != 1)
      {
         instructionIterator_u->second = instructionIterator_u->second + instructionIterator->second;
      }
   }

   UINT_32 counter;
   for(counter = 0; counter < DEP_BINS; counter++)
      dependencyDistance[counter] = dependencyDistance[counter] + basicBlockIn.return_dependencyDistance(counter);
   for(counter = 0; counter < STRIDE_BINS; counter++)
      dataStride[counter] = dataStride[counter] + basicBlockIn.return_dataStride(counter);
   for(counter = 0; counter < BRANCH_BINS; counter++)
      branchRate[counter] = branchRate[counter] + basicBlockIn.return_branchRate(counter);

   return 1;
}

UINT_8 WorkloadCharacteristics::update_basicBlock(WorkloadCharacteristics basicBlockIn, TX_ID transactionID)
{
   this->loadMix = this->loadMix + basicBlockIn.return_loadMix();
   this->storeMix = this->storeMix + basicBlockIn.return_storeMix();
   this->intShortMix = this->intShortMix + basicBlockIn.return_intShortMix();
   this->intLongMix = this->intLongMix + basicBlockIn.return_intLongMix();
   this->fpMix = this->fpMix + basicBlockIn.return_fpMix();
   this->branchMix = this->branchMix + basicBlockIn.return_branchMix();

   this->totalInstructionCount = this->totalInstructionCount + basicBlockIn.return_totalInstructionCount();
   this->basicBlockSize.push_back((UINT_32)basicBlockIn.return_totalInstructionCount());

   this->cycleTime = this->cycleTime + basicBlockIn.return_cycleTime();
   update_transactionID(transactionID);

   pair<AddressMap::iterator, BOOL> temp;
   for(AddressMap::iterator addressIterator = basicBlockIn.addressMap.begin(); addressIterator != basicBlockIn.addressMap.end(); addressIterator++)
   {
       temp = addressMap.insert(make_pair(addressIterator->first, 1));
   }

   BOOL unique;
   InstructionMap::iterator instructionIterator_u;
   for(InstructionMap::iterator instructionIterator = basicBlockIn.instructionMap.begin(); instructionIterator != basicBlockIn.instructionMap.end(); instructionIterator++)
   {
      boost::tie(instructionIterator_u, unique) = this->instructionMap.insert(make_pair(instructionIterator->first, 1));
      if(unique != 1)
      {
         instructionIterator_u->second = instructionIterator_u->second + instructionIterator->second;
      }
   }

   UINT_32 counter;
   for(counter = 0; counter < DEP_BINS; counter++)
      dependencyDistance[counter] = dependencyDistance[counter] + basicBlockIn.return_dependencyDistance(counter);
   for(counter = 0; counter < STRIDE_BINS; counter++)
      dataStride[counter] = dataStride[counter] + basicBlockIn.return_dataStride(counter);
   for(counter = 0; counter < BRANCH_BINS; counter++)
      branchRate[counter] = branchRate[counter] + basicBlockIn.return_branchRate(counter);

   return 1;
}

UINT_8 WorkloadCharacteristics::update_cacheLineSize(INT_32 cacheLineSize)
{
   this->cacheLineSize = cacheLineSize;
   return 1;
}

UINT_8 WorkloadCharacteristics::update_transactionID(TX_ID transactionID)
{
   this->transactionID = transactionID;
   return 1;
}

UINT_8 WorkloadCharacteristics::update_branchMap(ADDRESS_INT instructionAddress, UINT_32 isTaken)
{
   BOOL unique;
   BranchStatistics branchStats;
   BranchMap::iterator branchIterator;

   boost::tie(branchIterator, unique) = this->branchMap.insert(make_pair(instructionAddress, branchStats));
   if(unique != 1)
   {
      if(branchIterator->second.lastBranch != isTaken)
      {
         branchIterator->second.transition = branchIterator->second.transition + 1;
      }
   }

   if(isTaken == 1)
   {
      branchIterator->second.lastBranch = 1;
      branchIterator->second.taken =  branchIterator->second.taken + 1;
   }
   else
   {
      branchIterator->second.lastBranch = 0;
      branchIterator->second.notTaken =  branchIterator->second.notTaken + 1;
   }

   return 1;
}

UINT_8 WorkloadCharacteristics::update_instructionMap(ADDRESS_INT instructionAddress)
{
   BOOL unique;
   InstructionMap::iterator instructionIterator;

   boost::tie(instructionIterator, unique) = this->instructionMap.insert(make_pair(instructionAddress, 1));
   if(unique != 1)
   {
      instructionIterator->second = instructionIterator->second + 1;
   }

   return 1;
}

UINT_8 WorkloadCharacteristics::writeBranches()
{
   for(BranchMap::iterator branchIterator = branchMap.begin(); branchIterator != branchMap.end(); branchIterator++ )
   {
      float transitionCount = (branchIterator->second.taken + branchIterator->second.notTaken) - 1;
      float transitionRate = branchIterator->second.transition / transitionCount;

      if(transitionRate == 0)
         branchRate[0] = branchRate[0] + 1;
      else if(transitionRate < 0.10)
         branchRate[1] = branchRate[1] + 1;
      else if(transitionRate < 0.20)
         branchRate[2] = branchRate[2] + 1;
      else if(transitionRate < 0.30)
         branchRate[3] = branchRate[3] + 1;
      else if(transitionRate < 0.40)
         branchRate[4] = branchRate[4] + 1;
      else if(transitionRate < 0.50)
         branchRate[5] = branchRate[5] + 1;
      else if(transitionRate < 0.60)
         branchRate[6] = branchRate[6] + 1;
      else if(transitionRate < 0.70)
         branchRate[7] = branchRate[7] + 1;
      else if(transitionRate < 0.80)
         branchRate[8] = branchRate[8] + 1;
      else if(transitionRate < 0.90)
         branchRate[9] = branchRate[9] + 1;
      else
         branchRate[10] = branchRate[10] + 1;
   }

   return 1;
}

UINT_8 WorkloadCharacteristics::update_cycleTime(Time_t cycleTime)
{
   this->cycleTime = cycleTime;
   return 1;
}

UINT_8 WorkloadCharacteristics::add_cycleTime(Time_t cycleTime)
{
   if(this->lastCycle != 0)
   {
      this->cycleTime = this->cycleTime + (cycleTime - lastCycle);
   }

   lastCycle = cycleTime;

   return 1;
}

float WorkloadCharacteristics::return_loadMix()
{
   return this->loadMix;
}

float WorkloadCharacteristics::return_storeMix()
{
   return this->storeMix;
}

float WorkloadCharacteristics::return_intShortMix()
{
   return this->intShortMix;
}

float WorkloadCharacteristics::return_intLongMix()
{
   return this->intLongMix;
}

float WorkloadCharacteristics::return_fpMix()
{
   return this->fpMix;
}

float WorkloadCharacteristics::return_branchMix()
{
   return this->branchMix;
}

float WorkloadCharacteristics::return_dependencyDistance(UINT_32 bin)
{
   return this->dependencyDistance[bin];
}

float WorkloadCharacteristics::return_dataFootprint()
{
   return this->dataFootprint;
}

float WorkloadCharacteristics::return_dataStride(UINT_32 counter)
{
   return this->dataStride[counter];
}

float WorkloadCharacteristics::return_totalInstructionCount()
{
   return this->totalInstructionCount;
}

float WorkloadCharacteristics::return_branchRate(UINT_32 counter)
{
   return this->branchRate[counter];
}

TX_ID WorkloadCharacteristics::return_transactionID()
{
   return this->transactionID;
}

double WorkloadCharacteristics::return_averageBlockSize()
{
   return this->averageBlockSize;
}

double WorkloadCharacteristics::return_stdDevBlockSize()
{
   return this->stdDevBlockSize;
}

Time_t WorkloadCharacteristics::return_cycleTime()
{
   return this->cycleTime;
}

Time_t WorkloadCharacteristics::return_lastCycle()
{
   return this->lastCycle;
}

VAddr WorkloadCharacteristics::return_lastAddress()
{
   return this->lastAddress;
}

UINT_8 WorkloadCharacteristics::reset()
{
   normal_loadMix = loadMix = 0;
   normal_storeMix = storeMix = 0;
   normal_intShortMix = intShortMix = 0;
   normal_intLongMix = intLongMix = 0;
   normal_fpMix = fpMix = 0;
   normal_branchMix = branchMix = 0;
   normal_dataFootprint = dataFootprint = 0;
   normal_instructionFootprint = instructionFootprint = 0;
   normal_totalInstructionCount = totalInstructionCount = 0;

   averageBlockSize = 0;
   stdDevBlockSize = 0;
   normal_cycleTime = cycleTime = 0;

   UINT_32 counter;
   for(UINT_32 counter = 0; counter < DEP_BINS; counter++)
   {
      normal_dependencyDistance[counter] = dependencyDistance[counter] = 0;
   }
   for(UINT_32 counter = 0; counter < STRIDE_BINS; counter++)
   {
      normal_dataStride[counter] = dataStride[counter] = 0;
   }
   for(UINT_32 counter = 0; counter < TX_STRIDE_BINS; counter++)
   {
      normal_transactionStride[counter] = transactionStride[counter] = 0;
   }
   for(UINT_32 counter = 0; counter < BRANCH_BINS; counter++)
   {
      normal_branchRate[counter] = branchRate[counter] = 0;
   }

   basicBlockSize.clear();
   addressMap.clear();
   branchMap.clear();

   return 1;
}

UINT_8 WorkloadCharacteristics::reset(Time_t cycleIn, VAddr addressIn)
{
   normal_loadMix = loadMix = 0;
   normal_storeMix = storeMix = 0;
   normal_intShortMix = intShortMix = 0;
   normal_intLongMix = intLongMix = 0;
   normal_fpMix = fpMix = 0;
   normal_branchMix = branchMix = 0;
   normal_dataFootprint = dataFootprint = 0;
   normal_instructionFootprint = instructionFootprint = 0;
   normal_totalInstructionCount = totalInstructionCount = 0;

   averageBlockSize = 0;
   stdDevBlockSize = 0;

   normal_cycleTime = cycleTime = 0;
   lastCycle = cycleIn;
   lastAddress = addressIn;

   basicBlockSize.clear();
   addressMap.clear();
   branchMap.clear();

   for(UINT_32 counter = 0; counter < DEP_BINS; counter++)
   {
      normal_dependencyDistance[counter] = dependencyDistance[counter] = 0;
   }
   for(UINT_32 counter = 0; counter < STRIDE_BINS; counter++)
   {
      normal_dataStride[counter] = dataStride[counter] = 0;
   }
   for(UINT_32 counter = 0; counter < TX_STRIDE_BINS; counter++)
   {
      normal_transactionStride[counter] = transactionStride[counter] = 0;
   }
   for(UINT_32 counter = 0; counter < BRANCH_BINS; counter++)
   {
      normal_branchRate[counter] = branchRate[counter] = 0;
   }

   return 1;
}

UINT_8 WorkloadCharacteristics::analyzeResults()
{
   float averageSize = 0;
   float varianceSize = 0;
   float stdDevSize = 0;
   BasicBlockDeque::iterator basicBlockIterator;
   for(basicBlockIterator = this->basicBlockSize.begin(); basicBlockIterator != this->basicBlockSize.end(); basicBlockIterator++)
   {
      averageSize = averageSize + (*basicBlockIterator * (1.0 / this->basicBlockSize.size()));
   }
   this->averageBlockSize = averageSize;

   for(basicBlockIterator = this->basicBlockSize.begin(); basicBlockIterator != this->basicBlockSize.end(); basicBlockIterator++)
   {
      varianceSize = varianceSize + (pow(*basicBlockIterator - averageSize, 2) * (1.0 / this->basicBlockSize.size()));
   }
   stdDevSize = sqrt(varianceSize);
   this->stdDevBlockSize = stdDevSize;

   writeBranches();
   this->dataFootprint = addressMap.size();
   this->instructionFootprint = instructionMap.size();

   //we don't track fence instructions so sometimes the instruction footprint != the sum of
   //the instruction type
   float tempSum =  + loadMix + storeMix + intShortMix + intLongMix + fpMix + branchMix;
   if(totalInstructionCount > tempSum)
      totalInstructionCount = tempSum;

   return 1;
}

UINT_8 WorkloadCharacteristics::normalizeResults()
{
   float total;

   normal_loadMix = (float)loadMix / totalInstructionCount;
   normal_storeMix = (float)storeMix / totalInstructionCount;
   normal_intShortMix = (float)intShortMix / totalInstructionCount;
   normal_intLongMix = (float)intLongMix / totalInstructionCount;;
   normal_fpMix = (float)fpMix / totalInstructionCount;
   normal_branchMix = (float)branchMix / totalInstructionCount;

   //get normalized dependency distance
   total = 0;
   for(UINT_32 counter = 0; counter < DEP_BINS; counter++)
   {
      total = total + dependencyDistance[counter];
   }
   if(total > 0)
   {
      for(UINT_32 counter = 0; counter < DEP_BINS; counter++)
      {
         normal_dependencyDistance[counter] = dependencyDistance[counter] / total;
      }
   }

   //get normalized data stride
   total = 0;
   for(UINT_32 counter = 0; counter < STRIDE_BINS; counter++)
   {
      total = total + dataStride[counter];
   }
   if(total > 0)
   {
      for(UINT_32 counter = 0; counter < STRIDE_BINS; counter++)
      {
         normal_dataStride[counter] = dataStride[counter] / total;
      }
   }

   //get normalized branch rate
   total = 0;
   for(UINT_32 counter = 0; counter < BRANCH_BINS; counter++)
   {
      total = total + branchRate[counter];
   }
   if(total > 0)
   {
      for(UINT_32 counter = 0; counter < BRANCH_BINS; counter++)
      {
         normal_branchRate[counter] = branchRate[counter] / total;
      }
   }

   //get normalized transaction stride
   total = 0;
   for(UINT_32 counter = 0; counter < TX_STRIDE_BINS; counter++)
   {
      total = total + transactionStride[counter];
   }
   if(total > 0)
   {
      for(UINT_32 counter = 0; counter < TX_STRIDE_BINS; counter++)
      {
         normal_transactionStride[counter] = transactionStride[counter] / total;
      }
   }

   return 1;
}

UINT_8 WorkloadCharacteristics::reduceResults(float reduction)
{
   loadMix = (float)loadMix * reduction;
   storeMix = (float)storeMix * reduction;
   intShortMix = (float)intShortMix * reduction;
   intLongMix = (float)intLongMix * reduction;;
   fpMix = (float)fpMix * reduction;
   branchMix = (float)branchMix * reduction;
   dataFootprint = (float)dataFootprint * reduction;
   instructionFootprint = (float)instructionFootprint * reduction;
   totalInstructionCount = (float)totalInstructionCount * reduction;

// cout << endl;
// cout << "Loads:  " << loadMix << "  " << reduction << "  " << loadMix << endl;
// cout << "Storess:  " << storeMix << "  " << reduction << "  " << storeMix << endl;
// cout << "shorts:  " << intShortMix << "  " << reduction << "  " << intShortMix << endl;
// cout << "Longs:  " << intLongMix << "  " << reduction << "  " << intLongMix << endl;
// cout << "FPs:  " << fpMix << "  " << reduction << "  " << fpMix << endl;
// cout << "Branches:  " << branchMix << "  " << reduction << "  " << branchMix << endl;
// cout << "Data:  " << dataFootprint << "  " << reduction << "  " << dataFootprint << endl;
// cout << "Ins:  " << totalInstructionCount << "  " << reduction << "  " << totalInstructionCount << endl;
// cout << "\n" << endl;


   for(UINT_32 counter = 0; counter < DEP_BINS; counter++)
   {
      dependencyDistance[counter] = dependencyDistance[counter] * reduction;
   }

   for(UINT_32 counter = 0; counter < STRIDE_BINS; counter++)
   {
      dataStride[counter] = dataStride[counter] * reduction;
   }

   for(UINT_32 counter = 0; counter < BRANCH_BINS; counter++)
   {
      branchRate[counter] = branchRate[counter] * reduction;
   }

   return 1;
}

UINT_8 WorkloadCharacteristics::print(INT_32 printType)
{
   if(printType == 0)
   {
      if(this->transactionID > 0)
         std::cout << std::left << std::setw(15) <<  "TransID:" << this->transactionID << "\n";

      std::cout << std::left << std::setw(15) <<  "Cycles:  " << this->cycleTime << "\n";
      std::cout << std::left << std::setw(15) <<  "Loads:" << this->loadMix << "\n";
      std::cout << std::left << std::setw(15) <<  "Stores:" << this->storeMix << "\n";
      std::cout << std::left << std::setw(15) <<  "intShort:" << this->intShortMix << "\n";
      std::cout << std::left << std::setw(15) <<  "intLong:" << this->intLongMix << "\n";
      std::cout << std::left << std::setw(15) <<  "FP:" << this->fpMix << "\n";
      std::cout << std::left << std::setw(15) <<  "Branches:" << this->branchMix << "\n";
      std::cout << std::left << std::setw(15) <<  "Data Size:" << this->dataFootprint << "\n";
      std::cout << std::left << std::setw(15) <<  "Ins. Size:" << this->instructionFootprint << "\n";
      std::cout << std::left << std::setw(15) <<  "Total Ins.:" << this->totalInstructionCount << "\n";
      std::cout << std::left << std::setw(15) <<  "Avg. BB Size:" << this->averageBlockSize << "\n";
      std::cout << std::left << std::setw(15) <<  "Dev. BB Size:" << this->stdDevBlockSize;

      std::cout << std::setw(23) <<  "\nDependancy Distance:";
      for(UINT_32 counter = 0; counter < DEP_BINS; counter++)
      {
         std::cout << this->dependencyDistance[counter] << " ";
      }

      std::cout << std::setw(23) <<  "\nData Stride:";
      for(UINT_32 counter = 0; counter < STRIDE_BINS; counter++)
      {
         std::cout << this->dataStride[counter] << " ";
      }

      std::cout << std::setw(23) <<  "\nBranch Rate:";
      for(UINT_32 counter = 0; counter < BRANCH_BINS; counter++)
      {
         std::cout << this->branchRate[counter] << " ";
      }
   }
   else if(printType == 1)
   {
      if(this->transactionID > 0)
         std::cout << std::left << std::setw(15) <<  "TransID:" << this->transactionID << "\n";

      std::cout << std::left << std::setw(15) <<  "Cycles:  " << this->normal_cycleTime << "\n";
      std::cout << std::left << std::setw(15) <<  "Loads:" << this->normal_loadMix << "\n";
      std::cout << std::left << std::setw(15) <<  "Stores:" << this->normal_storeMix << "\n";
      std::cout << std::left << std::setw(15) <<  "intShort:" << this->normal_intShortMix << "\n";
      std::cout << std::left << std::setw(15) <<  "intLong:" << this->normal_intLongMix << "\n";
      std::cout << std::left << std::setw(15) <<  "FP:" << this->normal_fpMix << "\n";
      std::cout << std::left << std::setw(15) <<  "Branches:" << this->normal_branchMix << "\n";
      std::cout << std::left << std::setw(15) <<  "Data Size:" << this->dataFootprint << "\n";
      std::cout << std::left << std::setw(15) <<  "Ins. Size:" << this->instructionFootprint << "\n";
      std::cout << std::left << std::setw(15) <<  "Total Ins.:" << this->totalInstructionCount << "\n";
      std::cout << std::left << std::setw(15) <<  "Avg. BB Size:" << this->averageBlockSize << "\n";
      std::cout << std::left << std::setw(15) <<  "Dev. BB Size:" << this->stdDevBlockSize;

      std::cout << std::setw(23) <<  "\nDependancy Distance:";
      for(UINT_32 counter = 0; counter < DEP_BINS; counter++)
      {
         std::cout << this->normal_dependencyDistance[counter] << " ";
      }

      std::cout << std::setw(23) <<  "\nData Stride:";
      for(UINT_32 counter = 0; counter < STRIDE_BINS; counter++)
      {
         std::cout << this->normal_dataStride[counter] << " ";
      }

      std::cout << std::setw(23) <<  "\nBranch Rate:";
      for(UINT_32 counter = 0; counter < BRANCH_BINS; counter++)
      {
         std::cout << this->normal_branchRate[counter] << " ";
      }
   }
   else if(printType == 2)
   {
      std::cout << this->normal_cycleTime << ",";
      std::cout << this->normal_loadMix << ",";
      std::cout << this->normal_storeMix << ",";
      std::cout << this->normal_intShortMix << ",";
      std::cout << this->normal_intLongMix << ",";
      std::cout << this->normal_fpMix << ",";
      std::cout << this->normal_branchMix << ",";
      std::cout << this->dataFootprint << ",";
      std::cout << this->instructionFootprint << ",";
      std::cout << this->averageBlockSize << ",";
      std::cout << this->stdDevBlockSize << ",";

      for(UINT_32 counter = 0; counter < DEP_BINS; counter++)
      {
         std::cout << this->normal_dependencyDistance[counter] << ",";
      }

      for(UINT_32 counter = 0; counter < STRIDE_BINS; counter++)
      {
         std::cout << this->normal_dataStride[counter] << ",";
      }

      for(UINT_32 counter = 0; counter < BRANCH_BINS; counter++)
      {
         std::cout << this->normal_branchRate[counter] << ",";
      }
   }

   return 1;
}

UINT_8 WorkloadCharacteristics::print(INT_32 printType, string reportFileName)
{
   if(reportFileName == "")
   {
      print(printType);
   }
   else if(printType == 0)
   {
      print(printType);
   }
   else if(printType == 1)
   {
      string fileName = reportFileName;
      ofstream outputFile(fileName.c_str(), ios::app);      //open a file for writing (appends the current contents -- should be unique from timestamp)
      if(!outputFile)                                       //check to be sure file is open
         std::cerr << "Error opening file.";

      if(this->transactionID > 0)
         outputFile << std::left << std::setw(15) <<  "TransID:" << this->transactionID << "\n";

      outputFile << std::left << std::setw(15) <<  "Loads:" << this->loadMix << "\n";
      outputFile << std::left << std::setw(15) <<  "Stores:" << this->storeMix << "\n";
      outputFile << std::left << std::setw(15) <<  "intShort:" << this->intShortMix << "\n";
      outputFile << std::left << std::setw(15) <<  "intLong:" << this->intLongMix << "\n";
      outputFile << std::left << std::setw(15) <<  "FP:" << this->fpMix << "\n";
      outputFile << std::left << std::setw(15) <<  "Branches:" << this->branchMix << "\n";
      outputFile << std::left << std::setw(15) <<  "Data Size:" << this->dataFootprint << "\n";
      outputFile << std::left << std::setw(15) <<  "Ins. Size:" << this->instructionFootprint << "\n";
      outputFile << std::left << std::setw(15) <<  "Total Ins.:" << this->totalInstructionCount << "\n";
      outputFile << std::left << std::setw(15) <<  "Avg. BB Size:" << this->averageBlockSize << "\n";
      outputFile << std::left << std::setw(15) <<  "Dev. BB Size:" << this->stdDevBlockSize;

      outputFile << std::setw(23) <<  "\nDependancy Distance:";
      for(UINT_32 counter = 0; counter < DEP_BINS; counter++)
      {
         outputFile << this->dependencyDistance[counter] << " ";
      }

      outputFile << std::setw(23) <<  "\nData Stride:";
      for(UINT_32 counter = 0; counter < STRIDE_BINS; counter++)
      {
         outputFile << this->dataStride[counter] << " ";
      }

      outputFile << std::setw(23) <<  "\nBranch Rate:";
      for(UINT_32 counter = 0; counter < BRANCH_BINS; counter++)
      {
         outputFile << this->branchRate[counter] << " ";
      }

      outputFile.close();
   }
   else if(printType == 2)
   {
      string fileName = reportFileName;
      ofstream outputFile(fileName.c_str(), ios::app);      //open a file for writing (appends the current contents -- should be unique from timestamp)
      if(!outputFile)                                       //check to be sure file is open
         std::cerr << "Error opening file.";

      if(this->transactionID > 0)
         std::cout << std::left << std::setw(15) <<  "TransID:" << this->transactionID << "\n";

      outputFile << std::left << std::setw(15) <<  "Loads:" << this->normal_loadMix << "\n";
      outputFile << std::left << std::setw(15) <<  "Stores:" << this->normal_storeMix << "\n";
      outputFile << std::left << std::setw(15) <<  "intShort:" << this->normal_intShortMix << "\n";
      outputFile << std::left << std::setw(15) <<  "intLong:" << this->normal_intLongMix << "\n";
      outputFile << std::left << std::setw(15) <<  "FP:" << this->normal_fpMix << "\n";
      outputFile << std::left << std::setw(15) <<  "Branches:" << this->normal_branchMix << "\n";
      outputFile << std::left << std::setw(15) <<  "Data Size:" << this->dataFootprint << "\n";
      outputFile << std::left << std::setw(15) <<  "Ins. Size:" << this->instructionFootprint << "\n";
      outputFile << std::left << std::setw(15) <<  "Avg. BB Size:" << this->averageBlockSize << "\n";
      outputFile << std::left << std::setw(15) <<  "Dev. BB Size:" << this->stdDevBlockSize;

      outputFile << std::setw(23) <<  "\nDependancy Distance:";
      for(UINT_32 counter = 0; counter < DEP_BINS; counter++)
      {
         outputFile << this->normal_dependencyDistance[counter] << " ";
      }

      outputFile << std::setw(23) <<  "\nData Stride:";
      for(UINT_32 counter = 0; counter < STRIDE_BINS; counter++)
      {
         outputFile << this->normal_dataStride[counter] << " ";
      }

      outputFile << std::setw(23) <<  "\nBranch Rate:";
      for(UINT_32 counter = 0; counter < BRANCH_BINS; counter++)
      {
         outputFile << this->normal_branchRate[counter] << " ";
      }

      outputFile << "\n";
      outputFile.close();
   }
   else if(printType == 3)
   {
      string fileName = reportFileName;
      ofstream outputFile(fileName.c_str(), ios::app);      //open a file for writing (appends the current contents -- should be unique from timestamp)
      if(!outputFile)                                       //check to be sure file is open
         std::cerr << "Error opening file.";

      outputFile << "\n";
      outputFile << this->normal_loadMix << ",";
      outputFile << this->normal_storeMix << ",";
      outputFile << this->normal_intShortMix << ",";
      outputFile << this->normal_intLongMix << ",";
      outputFile << this->normal_fpMix << ",";
      outputFile << this->normal_branchMix << ",";
      outputFile << this->dataFootprint << ",";
      outputFile << this->instructionFootprint << ",";
      outputFile << this->averageBlockSize << ",";
      outputFile << this->stdDevBlockSize << ",";

      for(UINT_32 counter = 0; counter < DEP_BINS; counter++)
      {
         outputFile << this->normal_dependencyDistance[counter] << ",";
      }

      for(UINT_32 counter = 0; counter < STRIDE_BINS; counter++)
      {
         outputFile << this->normal_dataStride[counter] << ",";
      }

      for(UINT_32 counter = 0; counter < BRANCH_BINS; counter++)
      {
         outputFile << this->normal_branchRate[counter] << ",";
      }

      outputFile << "\n";
      outputFile.close();
   }

   return 1;
}

UINT_8 WorkloadCharacteristics::printTransactionStride()
{
   std::cout << std::setw(23) <<  "\nTransaction Stride:  ";
   for(UINT_32 counter = 0; counter < TX_STRIDE_BINS; counter++)
   {
      std::cout << this->transactionStride[counter] << " ";
   }

   return 1;
}

UINT_8 WorkloadCharacteristics::printTransactionStride(INT_32 printType, string reportFileName)
{
   if(printType == 0)
   {
      printTransactionStride();
   }
   else if(printType == 1)
   {
      std::cout << std::setw(23) <<  "\nTransaction Stride:  ";
      for(UINT_32 counter = 0; counter < TX_STRIDE_BINS; counter++)
      {
         std::cout << this->normal_transactionStride[counter] << " ";
      }
   }
   else if(printType == 2)
   {
      string fileName = reportFileName + "-txStride";
      ofstream outputFile(fileName.c_str(), ios::app);      //open a file for writing (appends the current contents -- should be unique from timestamp)
      if(!outputFile)                                       //check to be sure file is open
         std::cerr << "\nError opening file.\n";

      outputFile << std::setw(23) <<  "Transaction Stride:  ";
      for(UINT_32 counter = 0; counter < TX_STRIDE_BINS; counter++)
      {
         outputFile << this->transactionStride[counter] << " ";
      }

      outputFile.close();
   }
   else if(printType == 3)
   {
      string fileName = reportFileName + "-txStride";
      ofstream outputFile(fileName.c_str(), ios::app);      //open a file for writing (appends the current contents -- should be unique from timestamp)
      if(!outputFile)                                       //check to be sure file is open
         std::cerr << "\nError opening file.\n";

      for(UINT_32 counter = 0; counter < TX_STRIDE_BINS; counter++)
      {
         outputFile << this->transactionStride[counter] << ",";
      }

      for(UINT_32 counter = 0; counter < TX_STRIDE_BINS; counter++)
      {
         outputFile << this->normal_transactionStride[counter] << ",";
      }

      outputFile.close();
   }

   return 1;
}

WorkloadCharacteristics &WorkloadCharacteristics::operator=(const WorkloadCharacteristics &objectIn)
{
   addressMap = objectIn.addressMap;
   basicBlockSize = objectIn.basicBlockSize;
   branchMap = objectIn.branchMap;
   instructionMap = objectIn.instructionMap;

   loadMix = objectIn.loadMix;
   storeMix = objectIn.storeMix;
   intShortMix = objectIn.intShortMix;
   intLongMix = objectIn.intLongMix;
   fpMix = objectIn.fpMix;
   branchMix = objectIn.branchMix;
   dataFootprint = objectIn.dataFootprint;
   instructionFootprint = objectIn.instructionFootprint;
   totalInstructionCount = objectIn.totalInstructionCount;

   averageBlockSize = objectIn.averageBlockSize;
   stdDevBlockSize = objectIn.stdDevBlockSize;

   cycleTime = objectIn.cycleTime;
   lastCycle = objectIn.lastCycle;
   lastAddress = objectIn.lastAddress;
   cacheLineSize = objectIn.cacheLineSize;
   transactionID = objectIn.transactionID;

   normal_loadMix = objectIn.normal_loadMix;
   normal_storeMix = objectIn.normal_storeMix;
   normal_intShortMix = objectIn.normal_intShortMix;
   normal_intLongMix = objectIn.normal_intLongMix;
   normal_fpMix = objectIn.normal_fpMix;
   normal_branchMix = objectIn.normal_branchMix;
   normal_dataFootprint = objectIn.normal_dataFootprint;
   normal_instructionFootprint = objectIn.normal_instructionFootprint;
   normal_totalInstructionCount = objectIn.normal_totalInstructionCount;
   normal_cycleTime = objectIn.normal_cycleTime;

   for(UINT_32 counter = 0; counter < DEP_BINS; counter++)
   {
      dependencyDistance[counter] = objectIn.dependencyDistance[counter];
      normal_dependencyDistance[counter] = objectIn.normal_dependencyDistance[counter];
   }

   for(UINT_32 counter = 0; counter < STRIDE_BINS; counter++)
   {
      dataStride[counter] = objectIn.dataStride[counter];
      normal_dataStride[counter] = objectIn.normal_dataStride[counter];
   }

   for(UINT_32 counter = 0; counter < BRANCH_BINS; counter++)
   {
      branchRate[counter] = objectIn.branchRate[counter];
      normal_branchRate[counter] = objectIn.normal_branchRate[counter];
   }

   return *this;
}

WorkloadCharacteristics &WorkloadCharacteristics::operator+=(const WorkloadCharacteristics &objectIn)
{
   loadMix = loadMix + objectIn.loadMix;
   storeMix = storeMix + objectIn.storeMix;
   intShortMix = intShortMix + objectIn.intShortMix;
   intLongMix = intLongMix + objectIn.intLongMix;
   fpMix = fpMix + objectIn.fpMix;
   branchMix = branchMix + objectIn.branchMix;
   dataFootprint = dataFootprint + objectIn.dataFootprint;
   instructionFootprint = instructionFootprint + objectIn.instructionFootprint;
   totalInstructionCount = totalInstructionCount + objectIn.totalInstructionCount;

   cycleTime = cycleTime + objectIn.cycleTime;

   for(UINT_64 counter = 0; counter <  DEP_BINS; counter++)
   {
      dependencyDistance[counter] = dependencyDistance[counter] + objectIn.dependencyDistance[counter];
   }
   for(UINT_64 counter = 0; counter <  STRIDE_BINS; counter++)
   {
      dataStride[counter] = dataStride[counter] + objectIn.dataStride[counter];
   }
   for(UINT_64 counter = 0; counter <  BRANCH_BINS; counter++)
   {
      branchRate[counter] = branchRate[counter] + objectIn.branchRate[counter];
   }

   BasicBlockDeque::const_iterator basicBlockIterator;
   for(basicBlockIterator = objectIn.basicBlockSize.begin(); basicBlockIterator != objectIn.basicBlockSize.end(); basicBlockIterator++)
   {
      basicBlockSize.push_back(*basicBlockIterator);
   }

   return *this;
}

const WorkloadCharacteristics WorkloadCharacteristics::operator+(const WorkloadCharacteristics &objectIn) const
{
    WorkloadCharacteristics result = *this;
    result += objectIn;
    return result;
}
