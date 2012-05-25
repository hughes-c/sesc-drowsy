//
// C++ Implementation: stat_synthesis
//
// Description: 
//
//
/// @author: hughes  <hughes@fraidy2-uf>, (C) 2008
///
/// @date:          01/21/08
/// Last Modified:  03/04/08
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "stat_profile.h"

//NOTE output directories
string rootDirectory    = "/home/hughes/Benchies/MIPS/asmTesting/";

//NOTE woo
UINT_32 totalNumThreads = 0;

extern std::vector< std::deque< tuple<DInst, Time_t> > * > instructionQueueVector;

/**
 * @name IntToString 
 * 
 * @param input 
 * @return 
 */
std::string IntToString(INT_64 input)
{
  std::ostringstream output;
  output << input;

  return output.str();
}

/**
 * @name HexToString 
 * 
 * @param input 
 * @return 
 */
std::string HexToString(INT_64 input)
{
  std::ostringstream output;
  output << std::hex << input;

  return output.str();
}

namespace Profiling
{
std::deque< UINT_32 > transactionDistance;
std::vector< BOOL > firstTransaction;
std::vector< WorkloadCharacteristics * > currBBStats;
ProgramStatistics       globalStatistics;
std::vector< BOOL >     isTransaction;


void init(void)
{
}

void cleanup(void)
{
}

void aggregateCharacteristics(INT_32 printType, BOOL threadProfiling)
{
   if(printType == 3)
   {
      globalStatistics.programCharacteristics.analyzeResults();
      globalStatistics.programCharacteristics.normalizeResults();

      globalStatistics.transactionCharacteristics.analyzeResults();
      globalStatistics.transactionCharacteristics.normalizeResults();
   }

   std::cout << "Total Threads:  " << totalNumThreads << "\n";
   std::cout << "Total Instructions:  " << globalStatistics.programCharacteristics.return_totalInstructionCount() << "\n";
   std::cout << "Total Committed Tx Instructions:  " << globalStatistics.transactionCharacteristics.return_totalInstructionCount() << "\n";

   if(printType != 3)
   {
      std::cout << "\n\n----Aggregate Statistics----";
      std::cout << "\nProcess:\n";
   }
   globalStatistics.programCharacteristics.print(printType, globalStatistics.return_reportFileName());

   if(threadProfiling == 1)
   {
      WorkloadCharacteristics aggregateThreadCharacteristics;

      if(printType != 3)
      {
         std::cout << "\n\nThread:\n";
      }

      for(UINT_32 counter = 0; counter < totalNumThreads; counter++)
      {
         aggregateThreadCharacteristics = aggregateThreadCharacteristics + *(globalStatistics.threadCharacteristics[counter]);
      }

      aggregateThreadCharacteristics.analyzeResults();
      aggregateThreadCharacteristics.normalizeResults();
      aggregateThreadCharacteristics.print(printType, globalStatistics.return_reportFileName());
   }

   if(printType != 3)
   {
      std::cout << "\n\nTransaction:\n";
   }
   globalStatistics.transactionCharacteristics.print(printType, globalStatistics.return_reportFileName());
   globalStatistics.programCharacteristics.printTransactionStride(printType, globalStatistics.return_reportFileName());
}

/**
 * @name regCheck
 * 
 * @param destinationReg 
 * @param instructionIn 
 * @return 
 */
inline BOOL regCheck(RegType destinationReg, DInst instructionIn)
{
   if(instructionIn.getOpcode() == iLoad)
   {
      //loads only have a source and destination -- we don't care if we overwrite a previous load
      if(destinationReg == instructionIn.get_src1())
         return 1;
      else
         return 0;
   }
   else if(instructionIn.getOpcode() == iStore)
   {
      //stored need to monitor both source and destination registers
      if(destinationReg == instructionIn.get_src1() || destinationReg == instructionIn.get_dest())
         return 1;
      else
         return 0;
   }
   else if(instructionIn.getOpcode() == iALU || instructionIn.getOpcode() == iMult || instructionIn.getOpcode() == iDiv)
   {
      //ALU/FP operations need to check against both source registers
      if(destinationReg == instructionIn.get_src1() || destinationReg == instructionIn.get_src2())
         return 1;
      else
         return 0;
   }
   else if(instructionIn.getOpcode() == fpALU || instructionIn.getOpcode() == fpMult || instructionIn.getOpcode() == fpDiv)
   {
      //ALU/FP operations need to check against both source registers
      if(destinationReg == instructionIn.get_src1())
         return 1;
      else
         return 0;
   }
   else if(instructionIn.getOpcode() == iBJ)
   {
      //branches need to check...can't check branches because the destination reg is mapped to ReturnReg
      return 0;
   }
   else
      return 0;
}

/**
 * @name dependencyCheck
 * 
 * @param tempDinst 
 * @return 
 */
void dependencyCheck(DInst tempDinst)
{
   UINT_32 distance = 1;
   RegType destinationReg = tempDinst.get_dest();
   THREAD_ID threadID = tempDinst.get_threadID();

   if(destinationReg != InvalidOutput && destinationReg != CoprocStatReg && destinationReg != ReturnReg)
   {
      //seperated in case we want to add other dependencies later
      if(tempDinst.getOpcode() == iLoad)
      {
         for(std::deque< tuple<DInst, Time_t> >::iterator instructionIterator = instructionQueueVector[threadID]->begin(); instructionIterator != instructionQueueVector[threadID]->end(); instructionIterator++ )
         {
            if(regCheck(destinationReg, instructionIterator->get<0>()) == 1)
            {
               Profiling::currBBStats[threadID]->update_dependencyDistance(distance);
            }

            distance = distance + 1;
         }
      }
      else if(tempDinst.getOpcode() == iStore)
      {
         for(std::deque< tuple<DInst, Time_t> >::iterator instructionIterator = instructionQueueVector[threadID]->begin(); instructionIterator != instructionQueueVector[threadID]->end(); instructionIterator++ )
         {
            if(regCheck(destinationReg, instructionIterator->get<0>()) == 1)
            {
               Profiling::currBBStats[threadID]->update_dependencyDistance(distance);
            }

            distance = distance + 1;
         }
      }
      else if(tempDinst.getOpcode() == iALU || tempDinst.getOpcode() == iMult || tempDinst.getOpcode() == iDiv)
      {
         for(std::deque< tuple<DInst, Time_t> >::iterator instructionIterator = instructionQueueVector[threadID]->begin(); instructionIterator != instructionQueueVector[threadID]->end(); instructionIterator++ )
         {
            if(regCheck(destinationReg, instructionIterator->get<0>()) == 1)
            {
               Profiling::currBBStats[threadID]->update_dependencyDistance(distance);
            }

            distance = distance + 1;
         }
      }
      else if(tempDinst.getOpcode() == fpALU || tempDinst.getOpcode() == fpMult || tempDinst.getOpcode() == fpDiv)
      {
         for(std::deque< tuple<DInst, Time_t> >::iterator instructionIterator = instructionQueueVector[threadID]->begin(); instructionIterator != instructionQueueVector[threadID]->end(); instructionIterator++ )
         {
            if(regCheck(destinationReg, instructionIterator->get<0>()) == 1)
            {
               Profiling::currBBStats[threadID]->update_dependencyDistance(distance);
            }

            distance = distance + 1;
         }
      }
   }
}

/**
 * @name analysis
 * 
 * @param tempDinst 
 * @return 
 */
void analysis(tuple<DInst, Time_t>  tempTuple)
{
   ConfObject *statConf = new ConfObject;
   BOOL threadProfiling = statConf->return_enablePerThreadProfiling();
   DInst tempDinst = tempTuple.get<0>();
   THREAD_ID threadID = tempDinst.get_threadID();

   //If this is the beginning of a transaction, we want to start a new basic block
   if(tempDinst.getTmcode() == transBegin && Profiling::isTransaction[threadID] == 0 && tempDinst.get_transBCFlag() != 2)
   {
      Profiling::globalStatistics.programCharacteristics.update_basicBlock(*Profiling::currBBStats[threadID]);
      if(threadProfiling == 1)
         Profiling::globalStatistics.threadCharacteristics[threadID]->update_basicBlock(*Profiling::currBBStats[threadID]);

      if(Profiling::firstTransaction[threadID] == 1)
         Profiling::firstTransaction[threadID] = 0;
      else
         Profiling::globalStatistics.programCharacteristics.update_transactionStride(transactionDistance[threadID]);

      Time_t cycle = Profiling::currBBStats[threadID]->return_lastCycle();
      VAddr  addr = Profiling::currBBStats[threadID]->return_lastAddress();;

      delete Profiling::currBBStats[threadID];
      Profiling::currBBStats[threadID] = new WorkloadCharacteristics(cycle, addr);

      Profiling::isTransaction[threadID] = 1;
   }

   //If this is an abort, restart
   if(Profiling::isTransaction[threadID] == 1 && tempDinst.getTmcode() == transBegin && tempDinst.get_transBCFlag() == 1)
   {
      Time_t cycle = Profiling::currBBStats[threadID]->return_lastCycle();
      VAddr  addr = Profiling::currBBStats[threadID]->return_lastAddress();;

      delete Profiling::currBBStats[threadID];
      Profiling::currBBStats[threadID] = new WorkloadCharacteristics(cycle, addr);
   }

   Profiling::dependencyCheck(tempDinst);
   Profiling::currBBStats[threadID]->add_cycleTime(globalClock);
   Profiling::currBBStats[threadID]->update_totalInstructionCount(1);
   Profiling::currBBStats[threadID]->update_instructionMap((ADDRESS_INT)tempDinst.get_instructionAddress());
   transactionDistance[threadID] = transactionDistance[threadID] + 1;

   //Get opcode and update
   if(tempDinst.getOpcode() == iLoad)
   {
      Profiling::currBBStats[threadID]->update_loadMix(1);
      Profiling::currBBStats[threadID]->update_dataStride(tempDinst.getVaddr());
      Profiling::currBBStats[threadID]->update_memoryMap(tempDinst.getVaddr());
   }
   else if(tempDinst.getOpcode() == iStore)
   {
      Profiling::currBBStats[threadID]->update_storeMix(1);
      Profiling::currBBStats[threadID]->update_dataStride(tempDinst.getVaddr());
      Profiling::currBBStats[threadID]->update_memoryMap(tempDinst.getVaddr());
   }
   else if(tempDinst.getOpcode() == iALU)
      Profiling::currBBStats[threadID]->update_intShortMix(1);
   else if(tempDinst.getOpcode() == iMult || tempDinst.getOpcode() == iDiv)
      Profiling::currBBStats[threadID]->update_intLongMix(1);
   else if(tempDinst.getOpcode() == fpALU || tempDinst.getOpcode() == fpMult || tempDinst.getOpcode() == fpDiv)
      Profiling::currBBStats[threadID]->update_fpMix(1);
   else if(tempDinst.getOpcode() == iBJ)
      Profiling::currBBStats[threadID]->update_branchMix(1);

   //We want to push back on a control flow operation
   if(tempDinst.getOpcode() == iBJ)
   {
      //record basic block profile
      Profiling::globalStatistics.programCharacteristics.update_basicBlock(*Profiling::currBBStats[threadID]);
      if(threadProfiling == 1)
         Profiling::globalStatistics.threadCharacteristics[threadID]->update_basicBlock(*Profiling::currBBStats[threadID]);

      //record branch profile
      Profiling::globalStatistics.programCharacteristics.update_branchMap((ADDRESS_INT)tempDinst.get_instructionAddress(), tempDinst.get_isTaken());
      if(threadProfiling == 1)
         Profiling::globalStatistics.threadCharacteristics[threadID]->update_branchMap((ADDRESS_INT)tempDinst.get_instructionAddress(), tempDinst.get_isTaken());

      if(Profiling::isTransaction[threadID] == 1)
      {
         Profiling::globalStatistics.transactionCharacteristics.update_basicBlock(*Profiling::currBBStats[threadID]);
         Profiling::globalStatistics.transactionCharacteristics.update_branchMap((ADDRESS_INT)tempDinst.get_instructionAddress(), tempDinst.get_isTaken());
      }

      Time_t cycle = Profiling::currBBStats[threadID]->return_lastCycle();
      VAddr  addr = Profiling::currBBStats[threadID]->return_lastAddress();;

      delete Profiling::currBBStats[threadID];
      Profiling::currBBStats[threadID] = new WorkloadCharacteristics(cycle, addr);
   }

   //We force commit boundries to resemble (potential) control flow changes
   if(tempDinst.getTmcode() == transCommit && tempDinst.get_transBCFlag() != 2)
   {
      //record basic block profile
      Profiling::globalStatistics.programCharacteristics.update_basicBlock(*Profiling::currBBStats[threadID]);
      Profiling::globalStatistics.transactionCharacteristics.update_basicBlock(*Profiling::currBBStats[threadID],42);
      if(threadProfiling == 1)
         Profiling::globalStatistics.threadCharacteristics[threadID]->update_basicBlock(*Profiling::currBBStats[threadID]);

      Time_t cycle = Profiling::currBBStats[threadID]->return_lastCycle();
      VAddr  addr = Profiling::currBBStats[threadID]->return_lastAddress();;

      delete Profiling::currBBStats[threadID];
      Profiling::currBBStats[threadID] = new WorkloadCharacteristics(cycle, addr);

      //exiting the transaction -- reset the flag and set the distance counter to zero
      Profiling::isTransaction[threadID] = 0;
      transactionDistance[threadID] = 0;
   }

   delete statConf;
}

void finished(void)
{
   ConfObject *statConf = new ConfObject;
   INT_32 printType = statConf->return_dumpType();
   BOOL threadProfiling = statConf->return_enablePerThreadProfiling();

   if(printType != 3)
   {
      std::cout << "\n\nProgram Statistics:\n";
      globalStatistics.programCharacteristics.analyzeResults();
      globalStatistics.programCharacteristics.normalizeResults();
      globalStatistics.programCharacteristics.print(printType, globalStatistics.return_reportFileName());

      if(threadProfiling == 1)
      {
         std::cout << "\n\nPer-Thread Statistics:\n";
         for(UINT_32 counter = 0; counter < totalNumThreads; counter++)
         {
            std::cout << "Thread " << counter << ":\n";
            globalStatistics.threadCharacteristics[counter]->analyzeResults();
            globalStatistics.threadCharacteristics[counter]->normalizeResults();
            globalStatistics.threadCharacteristics[counter]->print(printType, globalStatistics.return_reportFileName());
            std::cout << "\n\n" << std::flush;
         }
      }

      std::cout << "\n\nTransaction Statistics:\n";
      globalStatistics.transactionCharacteristics.analyzeResults();
      globalStatistics.transactionCharacteristics.normalizeResults();
      globalStatistics.transactionCharacteristics.print(printType, globalStatistics.return_reportFileName());
   }

   aggregateCharacteristics(printType, threadProfiling);

   Profiling::cleanup();
   delete statConf;
}

}  //NOTE end Profiling
