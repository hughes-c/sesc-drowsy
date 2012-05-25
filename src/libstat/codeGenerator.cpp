//
// C++ Implementation: codeGenerator
//
// Description: 
//
//
/// @author: Clay Hughes <>, (C) 2008
/// @date:           01/15/07
/// Last Modified:   02/05/08
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "codeGenerator.h"

extern   UINT_32 totalNumThreads;
extern   std::deque< BBGraph * > myCFG;

extern   MutexMap globalMutexMap;

extern   std::vector < UINT_64 > numInstructions;
extern   std::vector < InstructionMix > programInstructionMix;

namespace Synthesis
{
   extern StatPaths statPaths;
}


namespace CodeGenerator
{
std::map< ADDRESS_INT, THREAD_ID > threadFuncMap;
UINT_32 lastReadStride;
UINT_32 lastWriteStride;

/**
 * @name anaylzeSynthetic
 * 
 * @param syntheticThreads[] 
 * @param arraySize 
 * @return 
 */
void anaylzeSynthetic(Synthetic *syntheticThreads[], UINT_32 arraySize)
{
   /* Variable Declaraion */


   /* Processes */
   for(UINT_32 threadCounter = 0; threadCounter < totalNumThreads; threadCounter++)
   {
      std::cout << "...T" << threadCounter << std::flush;

      ADDRESS_INT transID = 0;
      InstructionMix syntheticMix;

      UINT_32 bbCount = 0;

      std::list <BasicBlock> bbList = syntheticThreads[threadCounter]->return_coreList();
      std::list <BasicBlock>::iterator lastTrans, bbIterator;
      std::list <BasicBlock>::iterator start_of_trans = bbList.end();
      for(bbIterator = bbList.begin(); bbIterator != bbList.end(); bbIterator++)
      {
         if(bbIterator->return_isTrans() == 1)
         {
            bbCount = bbCount + 1;

            if(start_of_trans == bbList.end())
               start_of_trans = bbIterator;

            if(bbIterator->return_transID() != transID && transID != 0)
            {
               syntheticMix.normalize();
               std::cout << "\nTrans:  " << std::hex << transID << "  <>  " << bbIterator->return_transID() << std::dec << "\tBBCount:  " << bbCount << "\n";
               syntheticMix.print(std::cout);
               std::cout << "\n";

               lastTrans->instructionMix.normalize();
               lastTrans->instructionMix.print(std::cout);

               start_of_trans = bbIterator;
               syntheticMix.reset();
               bbCount = 0;
            }

            lastTrans = bbIterator;
            transID = bbIterator->return_transID();
            syntheticMix.update(bbIterator->return_instructionListRef(), 1);
         }
         else if(bbIterator->return_isTrans() == 0)
         {
            if(transID > 0)
            {
               syntheticMix.normalize();
               std::cout << "\n+Trans:  " << std::hex << transID << "  <>  " << bbIterator->return_transID() << std::dec << "\tBBCount:  " << bbCount << "\n";
               syntheticMix.print(std::cout);
               std::cout << "\n";

               lastTrans->instructionMix.normalize();
               lastTrans->instructionMix.print(std::cout);

               start_of_trans = bbList.end();
               syntheticMix.reset();

               bbCount = 0;
               transID = 0;
            }
         }
      }

      if(transID > 0)
      {
         syntheticMix.normalize();
         std::cout << "\n-Trans:  " << std::hex << transID << "  <>  " << lastTrans->return_transID() << std::dec << "\tBBCount:  " << bbCount << "\n";
         syntheticMix.print(std::cout);
         std::cout << "\n";

         lastTrans->instructionMix.normalize();
         lastTrans->instructionMix.print(std::cout);
      }


      std::cout << std::endl;
   }

   std::cout << "...Finished" << std::flush;
}

/**
 * 
 * @param syntheticThreads[] 
 * @param arraySize 
 * @return 
 */
void anaylzeTransactionMemoryReferences(Synthetic *syntheticThreads[], UINT_32 arraySize)
{
   /* Variables */
   UINT_32 instructionCount;
   MemoryPerformance memoryPerformance;
   float writes_reads;
   float loadCount, shared_loadCount, new_loadCount, old_loadCount, diff_loadCount;
   float storeCount, shared_storeCount, new_storeCount, old_storeCount, diff_storeCount;
   std::vector < THREAD_ID > uniqueThreads (1, 0);                   //this will hold the unique threads, but 0 will always be present so we include it here

   InstructionContainer loadInstruction;
   InstructionContainer storeInstruction;

   /* Processes */
   std::cout << "\nReorganizing Memory System..." << std::flush;
   std::cout << "\n\tFinding Target Threads"   << std::flush;

   //no need to do this for every thread -- it would throw off the count anyway
   for(std::map < ADDRESS_INT, THREAD_ID >::iterator threadIterator = CodeGenerator::threadFuncMap.begin(); threadIterator != CodeGenerator::threadFuncMap.end(); threadIterator++)
   {
      uniqueThreads.push_back(threadIterator->second);
   }

   //find a suitable load instruction
   for(std::vector < THREAD_ID >::iterator threadIterator = uniqueThreads.begin(); threadIterator != uniqueThreads.end(); threadIterator++)
   {
      for(std::list <BasicBlock>::iterator bbIterator = syntheticThreads[*threadIterator]->return_coreListRef().begin(); bbIterator != syntheticThreads[*threadIterator]->return_coreListRef().end(); bbIterator++)
      {
         for(std::list <InstructionContainer>::iterator insIterator = bbIterator->return_instructionListRef().begin(); insIterator != bbIterator->return_instructionListRef().end(); insIterator++)
         {
            if(insIterator->return_opCode() == iLoad)
            {
               loadInstruction = *insIterator;
               loadInstruction.strideAmount = 4;
               break;
            }
         }
      }
   }

   //find a suitable store instruction
   for(std::vector < THREAD_ID >::iterator threadIterator = uniqueThreads.begin(); threadIterator != uniqueThreads.end(); threadIterator++)
   {
      for(std::list <BasicBlock>::iterator bbIterator = syntheticThreads[*threadIterator]->return_coreListRef().begin(); bbIterator != syntheticThreads[*threadIterator]->return_coreListRef().end(); bbIterator++)
      {
         for(std::list <InstructionContainer>::iterator insIterator = bbIterator->return_instructionListRef().begin(); insIterator != bbIterator->return_instructionListRef().end(); insIterator++)
         {
            if(insIterator->return_opCode() == iStore)
            {
               storeInstruction = *insIterator;
               storeInstruction.strideAmount = 4;
               break;
            }
         }
      }
   }

   //go through the synthetic spine for each thread in uniqueThreads and count the mem mix
   for(std::vector < THREAD_ID >::iterator threadIterator = uniqueThreads.begin(); threadIterator != uniqueThreads.end(); threadIterator++)
   {
      //reset the L/S counts
      instructionCount = 0;
      loadCount = 0;
      storeCount = 0;

      float read_Acc = 0;
      float write_Acc = 0;
      ADDRESS_INT currentTrans = 0;
      std::list <BasicBlock>::iterator bbIterator, startTransaction, endTransaction;
      std::list <BasicBlock> &bbList = syntheticThreads[*threadIterator]->return_coreListRef();                                //Get the entire list of basic blocks -- reference
      for(bbIterator = bbList.begin(); bbIterator != bbList.end(); bbIterator++)
      {
         if(bbIterator->return_transID() > 0 && currentTrans == 0)
         {
            std::list <InstructionContainer> &instructionList = bbIterator->return_instructionListRef();
            for(std::list <InstructionContainer>::iterator insIterator = instructionList.begin(); insIterator != instructionList.end(); insIterator++)
            {
               instructionCount = instructionCount + 1;
               if(insIterator->return_opCode() == iLoad)
               {
                  loadCount = loadCount + 1;
                  insIterator->strideAmount = 4;
               }
               else if(insIterator->return_opCode() == iStore)
               {
                  storeCount = storeCount + 1;
                  insIterator->strideAmount = 4;
               }
            }

            startTransaction = bbIterator;
            currentTrans = bbIterator->return_transID();
         }
         else if(bbIterator->return_transID() > 0 && bbIterator->return_transID() == currentTrans)
         {
            std::list <InstructionContainer> &instructionList = bbIterator->return_instructionListRef();
            for(std::list <InstructionContainer>::iterator insIterator = instructionList.begin(); insIterator != instructionList.end(); insIterator++)
            {
               instructionCount = instructionCount + 1;
               if(insIterator->return_opCode() == iLoad)
               {
                  loadCount = loadCount + 1;
                  insIterator->strideAmount = 4;
               }
               else if(insIterator->return_opCode() == iStore)
               {
                  storeCount = storeCount + 1;
                  insIterator->strideAmount = 4;
               }
            }
         }
         else if(bbIterator->return_transID() > 0)
         {
            endTransaction = bbIterator;

std::cout << "\n***   " << *threadIterator << "   TRANS      " << std::hex << currentTrans << "\n" << std::dec;

            if(loadCount == 0)
            {
               for(std::list <BasicBlock>::iterator bbIterator_2 = startTransaction; bbIterator_2 != endTransaction; bbIterator_2++)
               {
                  std::list <InstructionContainer> &instructionList = bbIterator_2->return_instructionListRef();                //careful here since we are directly modifying the list
                  for(std::list <InstructionContainer>::iterator insIterator = instructionList.begin(); insIterator != instructionList.end(); insIterator++)
                  {
                     if(insIterator->return_opCode() != iLoad && insIterator->return_opCode() != iStore && loadCount <= 2)
                     {
                        *insIterator = loadInstruction;
                        loadCount = loadCount + 1;
                        break;
                     }
                  }
               }
            }

            if(storeCount == 0)
            {
               for(std::list <BasicBlock>::iterator bbIterator_2 = startTransaction; bbIterator_2 != endTransaction; bbIterator_2++)
               {
                  std::list <InstructionContainer> &instructionList = bbIterator_2->return_instructionListRef();                //careful here since we are directly modifying the list
                  for(std::list <InstructionContainer>::iterator insIterator = instructionList.begin(); insIterator != instructionList.end(); insIterator++)
                  {
                     if(insIterator->return_opCode() != iStore && insIterator->return_opCode() != iLoad && storeCount <= 2)
                     {
                        *insIterator = storeInstruction;
                        storeCount = storeCount + 1;
                        break;
                     }
                  }
               }
            }

            writes_reads = storeCount / loadCount;
            new_loadCount = old_loadCount = loadCount;
            new_storeCount = old_storeCount = storeCount;

            float targetReadSetSize = memoryPerformance.readSet_reads * new_loadCount;
            float targetWriteSetSize = memoryPerformance.writeSet_writes * new_storeCount;

std::cout << "\nIns Count:  " << instructionCount << "\tLoad Count:  " << loadCount << "\tStore Count:  " << storeCount << "\tW/R:  " << writes_reads << "\n";
std::cout << "writes_reads:  " << memoryPerformance.writes_reads << "\tHi:  " << memoryPerformance.writes_reads_HiLo().first << "\tLow:  " << memoryPerformance.writes_reads_HiLo().second<< "\n";
std::cout << "writeSet_writes:  " << memoryPerformance.writeSet_writes << "\tHi:  " << memoryPerformance.writeSet_writes_HiLo().first << "\tLow:  " << memoryPerformance.writeSet_writes_HiLo().second<< "\n";
std::cout << "readSet_reads:  " << memoryPerformance.readSet_reads << "\tHi:  " << memoryPerformance.readSet_reads_HiLo().first << "\tLow:  " << memoryPerformance.readSet_reads_HiLo().second<< "\n";
std::cout << "writeSet_readSet:  " << memoryPerformance.writeSet_readSet << "\tHi:  " << memoryPerformance.writeSet_readSet_HiLo().first << "\tLow:  " << memoryPerformance.writeSet_readSet_HiLo().second<< "\n";

            do
            {
               if(memoryPerformance.writes_reads_HiLo().first < writes_reads)
               {
                  float n = (new_storeCount - (memoryPerformance.writes_reads * new_loadCount)) / (memoryPerformance.writes_reads + 1);
                  new_loadCount = new_loadCount + n;
                  new_storeCount = new_storeCount - n;
               }
               else if(writes_reads < memoryPerformance.writes_reads_HiLo().second)
               {
                  float n = (new_storeCount - (memoryPerformance.writes_reads * new_loadCount)) / (-1 - memoryPerformance.writes_reads);
                  new_loadCount = new_loadCount - n;
                  new_storeCount = new_storeCount + n;
               }

               targetReadSetSize = memoryPerformance.readSet_reads * new_loadCount;
               targetWriteSetSize = memoryPerformance.writeSet_writes * new_storeCount;

               do
               {
                  if(targetReadSetSize < 1.0)
                  {
                     targetReadSetSize = targetReadSetSize + 1.0;
                     targetWriteSetSize = memoryPerformance.writeSet_readSet * targetReadSetSize;
                  }

                  if(targetWriteSetSize < 1.0)
                  {
                     targetWriteSetSize = targetWriteSetSize + 1.0;
                     targetReadSetSize = targetWriteSetSize / memoryPerformance.writeSet_readSet;
                  }
std::cout << "******targetWrite:  " << targetWriteSetSize << "\t\ttargetRead:  " << targetReadSetSize << "\n";
               }while(targetReadSetSize < 1.0 || targetWriteSetSize < 1.0);

               writes_reads = new_storeCount / new_loadCount;

            }while(memoryPerformance.writes_reads_HiLo().first < writes_reads && writes_reads < memoryPerformance.writes_reads_HiLo().second);

            new_loadCount = (int)(new_loadCount + 0.5f);
            new_storeCount = (int)(new_storeCount + 0.5f);

            read_Acc = read_Acc + targetReadSetSize - floor(targetReadSetSize);
            write_Acc = write_Acc + targetWriteSetSize - floor(targetWriteSetSize);

            targetReadSetSize = floor(targetReadSetSize);
            targetWriteSetSize = floor(targetWriteSetSize);

            if(read_Acc >= 1.0)
            {
               read_Acc = 0;
               targetReadSetSize = targetReadSetSize + 1;
            }

            if(write_Acc >= 1.0)
            {
               write_Acc = 0;
               targetWriteSetSize = targetWriteSetSize + 1;
            }

            diff_loadCount = new_loadCount - loadCount;
            diff_storeCount = new_storeCount - storeCount;

// std::cout << "\nNew Load Count:  " << new_loadCount << " (" << diff_loadCount << ")";
// std::cout << "\tNew Store Count:  " << new_storeCount << " (" << diff_storeCount << ")";
// std::cout << "\tW/R:  " << new_storeCount / new_loadCount;
// std::cout << "\nTarget Read Set:  " << targetReadSetSize << "\tTarget Write Set:  " << targetWriteSetSize << "\n";

            for(std::list <BasicBlock>::iterator bbIterator_2 = startTransaction; bbIterator_2 != endTransaction; bbIterator_2++)
            {
               std::list <InstructionContainer> &instructionList = bbIterator_2->return_instructionListRef();                //careful here since we are directly modifying the list
               for(std::list <InstructionContainer>::iterator insIterator = instructionList.begin(); insIterator != instructionList.end(); insIterator++)
               {
                  if(insIterator->return_opCode() == iLoad)
                  {
                     if(diff_storeCount >= 1)
                     {
//    std::cout << "Swapping LOAD for STORE" << "\n";
                        diff_storeCount = diff_storeCount - 1;
                        *insIterator = storeInstruction;

                        if(targetWriteSetSize >= 2)
                        {
                           if(targetWriteSetSize > (STREAM_SIZE * MEM_MULTIPLIER) / 32)
                           {
                              std::cerr << "\nYabba dabba doo!\n\n";
                              exit(0);
                           }
                           else if(bbIterator_2->return_accumulated() <= 1)
                           {
//       std::cout << "Ss-Target:  " << targetWriteSetSize;
                              insIterator->strideAmount = 32 * (UINT_32)targetWriteSetSize;
                              targetWriteSetSize = targetWriteSetSize - 1;
//       std::cout << "\tstride:  " << insIterator->strideAmount << "\tsharedMem:  " << insIterator->return_sharedMem() << "\n";
                           }
                        }

                     }
                     else if(targetReadSetSize >= 2)
                     {
                        if(targetWriteSetSize > (STREAM_SIZE * MEM_MULTIPLIER) / 32)
                        {
                           std::cerr << "\nYabba dabba doo!\n\n";
                           exit(0);
                        }
                        else if(bbIterator_2->return_accumulated() <= 1)
                        {
//    std::cout << "L-Target:  " << targetReadSetSize;
                           insIterator->strideAmount = 32 * (UINT_32)targetReadSetSize;
                           targetReadSetSize = targetReadSetSize - 1;
//    std::cout << "\tstride:  " << insIterator->strideAmount << "\tsharedMem:  " << insIterator->return_sharedMem() << "\n";
                        }
                     }

                     loadCount = loadCount + 1;
                  }
                  else if(insIterator->return_opCode() == iStore)
                  {
                     if(diff_loadCount >= 1)
                     {
//    std::cout << "Swapping STORE for LOAD" << "\n";
                        diff_loadCount = diff_loadCount - 1;
                        *insIterator = loadInstruction;

                        if(targetReadSetSize >= 2)
                        {
                           if(targetWriteSetSize > (STREAM_SIZE * MEM_MULTIPLIER) / 32)
                           {
                              std::cerr << "\nYabba dabba doo!\n\n";
                              exit(0);
                           }
                           else if(bbIterator_2->return_accumulated() <= 1)
                           {
//       std::cout << "Ls-Target:  " << targetReadSetSize;
                              insIterator->strideAmount = 32 * (UINT_32)targetReadSetSize;
                              targetReadSetSize = targetReadSetSize - 1;
//       std::cout << "\tstride:  " << insIterator->strideAmount << "\tsharedMem:  " << insIterator->return_sharedMem() << "\n";
                           }
                        }

                     }
                     else if(targetWriteSetSize >= 2)
                     {
                        if(targetWriteSetSize > (STREAM_SIZE * MEM_MULTIPLIER) / 32)
                        {
                           std::cerr << "\nYabba dabba doo!\n\n";
                           exit(0);
                        }
                        else if(bbIterator_2->return_accumulated() <= 1)
                        {
//    std::cout << "S-Target:  " << targetWriteSetSize;
                           insIterator->strideAmount = 32 * (UINT_32)targetWriteSetSize;
                           targetWriteSetSize = targetWriteSetSize - 1;
//    std::cout << "\tstride:  " << insIterator->strideAmount << "\tsharedMem:  " << insIterator->return_sharedMem() << "\n";
                        }
                     }

                     storeCount = storeCount + 1;
                  }
               }
            }  //end inner for



            startTransaction = bbIterator;
            currentTrans = bbIterator->return_transID();
            instructionCount = 0;
            loadCount = 0;
            storeCount = 0;

            std::list <InstructionContainer> &instructionList = bbIterator->return_instructionListRef();
            for(std::list <InstructionContainer>::iterator insIterator = instructionList.begin(); insIterator != instructionList.end(); insIterator++)
            {
               instructionCount = instructionCount + 1;
               if(insIterator->return_opCode() == iLoad)
               {
                  loadCount = loadCount + 1;
                  insIterator->strideAmount = 4;
               }
               else if(insIterator->return_opCode() == iStore)
               {
                  storeCount = storeCount + 1;
                  insIterator->strideAmount = 4;
               }
            }
         }
         else if(currentTrans > 0)
         {
            endTransaction = bbIterator;

// std::cout << "\n***   " << *threadIterator << "   TRANS      " << std::hex << currentTrans << "\n" << std::dec;

            if(loadCount == 0)
            {
               for(std::list <BasicBlock>::iterator bbIterator_2 = startTransaction; bbIterator_2 != endTransaction; bbIterator_2++)
               {
                  std::list <InstructionContainer> &instructionList = bbIterator_2->return_instructionListRef();                //careful here since we are directly modifying the list
                  for(std::list <InstructionContainer>::iterator insIterator = instructionList.begin(); insIterator != instructionList.end(); insIterator++)
                  {
                     if(insIterator->return_opCode() != iLoad && insIterator->return_opCode() != iStore && loadCount <= 2)
                     {
                        *insIterator = loadInstruction;
                        loadCount = loadCount + 1;
                        break;
                     }
                  }
               }
            }

            if(storeCount == 0)
            {
               for(std::list <BasicBlock>::iterator bbIterator_2 = startTransaction; bbIterator_2 != endTransaction; bbIterator_2++)
               {
                  std::list <InstructionContainer> &instructionList = bbIterator_2->return_instructionListRef();                //careful here since we are directly modifying the list
                  for(std::list <InstructionContainer>::iterator insIterator = instructionList.begin(); insIterator != instructionList.end(); insIterator++)
                  {
                     if(insIterator->return_opCode() != iStore && insIterator->return_opCode() != iLoad && storeCount <= 2)
                     {
                        *insIterator = storeInstruction;
                        storeCount = storeCount + 1;
                        break;
                     }
                  }
               }
            }

            writes_reads = storeCount / loadCount;
            new_loadCount = old_loadCount = loadCount;
            new_storeCount = old_storeCount = storeCount;

            float targetReadSetSize = memoryPerformance.readSet_reads * new_loadCount;
            float targetWriteSetSize = memoryPerformance.writeSet_writes * new_storeCount;

// std::cout << "\n--Ins Count:  " << instructionCount << "\tLoad Count:  " << loadCount << "\tStore Count:  " << storeCount << "\tW/R:  " << writes_reads << "\n";
// std::cout << "--writes_reads:  " << memoryPerformance.writes_reads << "\tHi:  " << memoryPerformance.writes_reads_HiLo().first << "\tLow:  " << memoryPerformance.writes_reads_HiLo().second<< "\n";
// std::cout << "--writeSet_writes:  " << memoryPerformance.writeSet_writes << "\tHi:  " << memoryPerformance.writeSet_writes_HiLo().first << "\tLow:  " << memoryPerformance.writeSet_writes_HiLo().second<< "\n";
// std::cout << "--readSet_reads:  " << memoryPerformance.readSet_reads << "\tHi:  " << memoryPerformance.readSet_reads_HiLo().first << "\tLow:  " << memoryPerformance.readSet_reads_HiLo().second<< "\n";
// std::cout << "--writeSet_readSet:  " << memoryPerformance.writeSet_readSet << "\tHi:  " << memoryPerformance.writeSet_readSet_HiLo().first << "\tLow:  " << memoryPerformance.writeSet_readSet_HiLo().second<< "\n";

            do
            {
               if(memoryPerformance.writes_reads_HiLo().first < writes_reads)
               {
                  float n = (new_storeCount - (memoryPerformance.writes_reads * new_loadCount)) / (memoryPerformance.writes_reads + 1);
                  new_loadCount = new_loadCount + n;
                  new_storeCount = new_storeCount - n;
               }
               else if(writes_reads < memoryPerformance.writes_reads_HiLo().second)
               {
                  float n = (new_storeCount - (memoryPerformance.writes_reads * new_loadCount)) / (-1 - memoryPerformance.writes_reads);
                  new_loadCount = new_loadCount - n;
                  new_storeCount = new_storeCount + n;
               }

               targetReadSetSize = memoryPerformance.readSet_reads * new_loadCount;
               targetWriteSetSize = memoryPerformance.writeSet_writes * new_storeCount;

               do
               {
                  if(targetReadSetSize < 1.0)
                  {
                     targetReadSetSize = targetReadSetSize + 1.0;
                     targetWriteSetSize = memoryPerformance.writeSet_readSet * targetReadSetSize;
                  }

                  if(targetWriteSetSize < 1.0)
                  {
                     targetWriteSetSize = targetWriteSetSize + 1.0;
                     targetReadSetSize = targetWriteSetSize / memoryPerformance.writeSet_readSet;
                  }
// std::cout << "******targetWrite:  " << targetWriteSetSize << "\t\ttargetRead:  " << targetReadSetSize << "\n";
               }while(targetReadSetSize < 1.0 || targetWriteSetSize < 1.0);

               writes_reads = new_storeCount / new_loadCount;

            }while(memoryPerformance.writes_reads_HiLo().first < writes_reads && writes_reads < memoryPerformance.writes_reads_HiLo().second);

            new_loadCount = (int)(new_loadCount + 0.5f);
            new_storeCount = (int)(new_storeCount + 0.5f);

            read_Acc = read_Acc + targetReadSetSize - floor(targetReadSetSize);
            write_Acc = write_Acc + targetWriteSetSize - floor(targetWriteSetSize);

            targetReadSetSize = floor(targetReadSetSize);
            targetWriteSetSize = floor(targetWriteSetSize);

            if(read_Acc >= 1.0)
            {
               read_Acc = 0;
               targetReadSetSize = targetReadSetSize + 1;
            }

            if(write_Acc >= 1.0)
            {
               write_Acc = 0;
               targetWriteSetSize = targetWriteSetSize + 1;
            }

            diff_loadCount = new_loadCount - loadCount;
            diff_storeCount = new_storeCount - storeCount;

// std::cout << "\n--New Load Count:  " << new_loadCount << " (" << diff_loadCount << ")";
// std::cout << "\tNew Store Count:  " << new_storeCount << " (" << diff_storeCount << ")";
// std::cout << "\tW/R:  " << new_storeCount / new_loadCount;
// std::cout << "\n--Target Read Set:  " << targetReadSetSize << "\tTarget Write Set:  " << targetWriteSetSize << "\n";


            for(std::list <BasicBlock>::iterator bbIterator_2 = startTransaction; bbIterator_2 != endTransaction; bbIterator_2++)
            {
               std::list <InstructionContainer> &instructionList = bbIterator_2->return_instructionListRef();                //careful here since we are directly modifying the list
               for(std::list <InstructionContainer>::iterator insIterator = instructionList.begin(); insIterator != instructionList.end(); insIterator++)
               {
                  if(insIterator->return_opCode() == iLoad)
                  {
                     if(diff_storeCount >= 1)
                     {
//    std::cout << "Swapping LOAD for STORE" << "\n";
                        diff_storeCount = diff_storeCount - 1;
                        *insIterator = storeInstruction;

                        if(targetWriteSetSize >= 2)
                        {
                           if(targetWriteSetSize > (STREAM_SIZE * MEM_MULTIPLIER) / 32)
                           {
                              std::cerr << "\nYabba dabba doo!\n\n";
                              exit(0);
                           }
                           else if(bbIterator_2->return_accumulated() <= 1)
                           {
//       std::cout << "Ss-Target:  " << targetWriteSetSize;
                              insIterator->strideAmount = 32 * (UINT_32)targetWriteSetSize;
                              targetWriteSetSize = targetWriteSetSize - 1;
//       std::cout << "\tstride:  " << insIterator->strideAmount << "\tsharedMem:  " << insIterator->return_sharedMem() << "\n";
                           }
                        }

                     }
                     else if(targetReadSetSize >= 2)
                     {
                        if(targetWriteSetSize > (STREAM_SIZE * MEM_MULTIPLIER) / 32)
                        {
                           std::cerr << "\nYabba dabba doo!\n\n";
                           exit(0);
                        }
                        else if(bbIterator_2->return_accumulated() <= 1)
                        {
//    std::cout << "L-Target:  " << targetReadSetSize;
                           insIterator->strideAmount = 32 * (UINT_32)targetReadSetSize;
                           targetReadSetSize = targetReadSetSize - 1;
//    std::cout << "\tstride:  " << insIterator->strideAmount << "\tsharedMem:  " << insIterator->return_sharedMem() << "\n";
                        }
                     }

                     loadCount = loadCount + 1;
                  }
                  else if(insIterator->return_opCode() == iStore)
                  {
                     if(diff_loadCount >= 1)
                     {
//    std::cout << "Swapping STORE for LOAD" << "\n";
                        diff_loadCount = diff_loadCount - 1;
                        *insIterator = loadInstruction;

                        if(targetReadSetSize >= 2)
                        {
                           if(targetWriteSetSize > (STREAM_SIZE * MEM_MULTIPLIER) / 32)
                           {
                              std::cerr << "\nYabba dabba doo!\n\n";
                              exit(0);
                           }
                           else if(bbIterator_2->return_accumulated() <= 1)
                           {
//       std::cout << "Ls-Target:  " << targetReadSetSize;
                              insIterator->strideAmount = 32 * (UINT_32)targetReadSetSize;
                              targetReadSetSize = targetReadSetSize - 1;
//       std::cout << "\tstride:  " << insIterator->strideAmount << "\tsharedMem:  " << insIterator->return_sharedMem() << "\n";
                           }
                        }

                     }
                     else if(targetWriteSetSize >= 2)
                     {
                        if(targetWriteSetSize > (STREAM_SIZE * MEM_MULTIPLIER) / 32)
                        {
                           std::cerr << "\nYabba dabba doo!\n\n";
                           exit(0);
                        }
                        else if(bbIterator_2->return_accumulated() <= 1)
                        {
//    std::cout << "S-Target:  " << targetWriteSetSize;
                           insIterator->strideAmount = 32 * (UINT_32)targetWriteSetSize;
                           targetWriteSetSize = targetWriteSetSize - 1;
//    std::cout << "\tstride:  " << insIterator->strideAmount << "\tsharedMem:  " << insIterator->return_sharedMem() << "\n";
                        }
                     }

                     storeCount = storeCount + 1;
                  }
               }
            }  //end inner for


            startTransaction = bbIterator;
            instructionCount = 0;
            currentTrans = 0;
            loadCount = 0;
            storeCount = 0;
         }
      }

      if(currentTrans > 0)
      {
// std::cout << "\n***   " << *threadIterator << "   TRANS      " << std::hex << currentTrans << "\n" << std::dec;

            if(loadCount == 0)
            {
               for(std::list <BasicBlock>::iterator bbIterator_2 = startTransaction; bbIterator_2 !=  bbList.end(); bbIterator_2++)
               {
                  std::list <InstructionContainer> &instructionList = bbIterator_2->return_instructionListRef();                //careful here since we are directly modifying the list
                  for(std::list <InstructionContainer>::iterator insIterator = instructionList.begin(); insIterator != instructionList.end(); insIterator++)
                  {
                     if(insIterator->return_opCode() != iLoad && insIterator->return_opCode() != iStore && loadCount <= 2)
                     {
                        *insIterator = loadInstruction;
                        loadCount = loadCount + 1;
                        break;
                     }
                  }
               }
            }

            if(storeCount == 0)
            {
               for(std::list <BasicBlock>::iterator bbIterator_2 = startTransaction; bbIterator_2 !=  bbList.end(); bbIterator_2++)
               {
                  std::list <InstructionContainer> &instructionList = bbIterator_2->return_instructionListRef();                //careful here since we are directly modifying the list
                  for(std::list <InstructionContainer>::iterator insIterator = instructionList.begin(); insIterator != instructionList.end(); insIterator++)
                  {
                     if(insIterator->return_opCode() != iStore && insIterator->return_opCode() != iLoad && storeCount <= 2)
                     {
                        *insIterator = storeInstruction;
                        storeCount = storeCount + 1;
                        break;
                     }
                  }
               }
            }

            writes_reads = storeCount / loadCount;
            new_loadCount = old_loadCount = loadCount;
            new_storeCount = old_storeCount = storeCount;

         float targetReadSetSize = memoryPerformance.readSet_reads * new_loadCount;
         float targetWriteSetSize = memoryPerformance.writeSet_writes * new_storeCount;

// std::cout << "\n++Ins Count:  " << instructionCount << "\tLoad Count:  " << loadCount << "\tStore Count:  " << storeCount << "\tW/R:  " << writes_reads << "\n";
// std::cout << "++writes_reads:  " << memoryPerformance.writes_reads << "\tHi:  " << memoryPerformance.writes_reads_HiLo().first << "\tLow:  " << memoryPerformance.writes_reads_HiLo().second<< "\n";
// std::cout << "++writeSet_writes:  " << memoryPerformance.writeSet_writes << "\tHi:  " << memoryPerformance.writeSet_writes_HiLo().first << "\tLow:  " << memoryPerformance.writeSet_writes_HiLo().second<< "\n";
// std::cout << "++readSet_reads:  " << memoryPerformance.readSet_reads << "\tHi:  " << memoryPerformance.readSet_reads_HiLo().first << "\tLow:  " << memoryPerformance.readSet_reads_HiLo().second<< "\n";
// std::cout << "++writeSet_readSet:  " << memoryPerformance.writeSet_readSet << "\tHi:  " << memoryPerformance.writeSet_readSet_HiLo().first << "\tLow:  " << memoryPerformance.writeSet_readSet_HiLo().second<< "\n";

            do
            {
               if(memoryPerformance.writes_reads_HiLo().first < writes_reads)
               {
                  float n = (new_storeCount - (memoryPerformance.writes_reads * new_loadCount)) / (memoryPerformance.writes_reads + 1);
                  new_loadCount = new_loadCount + n;
                  new_storeCount = new_storeCount - n;
               }
               else if(writes_reads < memoryPerformance.writes_reads_HiLo().second)
               {
                  float n = (new_storeCount - (memoryPerformance.writes_reads * new_loadCount)) / (-1 - memoryPerformance.writes_reads);
                  new_loadCount = new_loadCount - n;
                  new_storeCount = new_storeCount + n;
               }

               targetReadSetSize = memoryPerformance.readSet_reads * new_loadCount;
               targetWriteSetSize = memoryPerformance.writeSet_writes * new_storeCount;

               do
               {
                  if(targetReadSetSize < 1.0)
                  {
                     targetReadSetSize = targetReadSetSize + 1.0;
                     targetWriteSetSize = memoryPerformance.writeSet_readSet * targetReadSetSize;
                  }

                  if(targetWriteSetSize < 1.0)
                  {
                     targetWriteSetSize = targetWriteSetSize + 1.0;
                     targetReadSetSize = targetWriteSetSize / memoryPerformance.writeSet_readSet;
                  }
// std::cout << "******targetWrite:  " << targetWriteSetSize << "\t\ttargetRead:  " << targetReadSetSize << "\n";
               }while(targetReadSetSize < 1.0 || targetWriteSetSize < 1.0);

               writes_reads = new_storeCount / new_loadCount;

            }while(memoryPerformance.writes_reads_HiLo().first < writes_reads && writes_reads < memoryPerformance.writes_reads_HiLo().second);

            new_loadCount = (int)(new_loadCount + 0.5f);
            new_storeCount = (int)(new_storeCount + 0.5f);

            read_Acc = read_Acc + targetReadSetSize - floor(targetReadSetSize);
            write_Acc = write_Acc + targetWriteSetSize - floor(targetWriteSetSize);

            targetReadSetSize = floor(targetReadSetSize);
            targetWriteSetSize = floor(targetWriteSetSize);

            if(read_Acc >= 0.5)
            {
               read_Acc = 0;
               targetReadSetSize = targetReadSetSize + 1;
            }

            if(write_Acc >= 0.5)
            {
               write_Acc = 0;
               targetWriteSetSize = targetWriteSetSize + 1;
            }

            diff_loadCount = new_loadCount - loadCount;
            diff_storeCount = new_storeCount - storeCount;

// std::cout << "\n++New Load Count:  " << new_loadCount << " (" << diff_loadCount << ")";
// std::cout << "\tNew Store Count:  " << new_storeCount << " (" << diff_storeCount << ")";
// std::cout << "\tW/R:  " << new_storeCount / new_loadCount;
// std::cout << "\n++Target Read Set:  " << targetReadSetSize << "\tTarget Write Set:  " << targetWriteSetSize << "\n";

            for(std::list <BasicBlock>::iterator bbIterator_2 = startTransaction; bbIterator_2 != bbList.end(); bbIterator_2++)
            {
               std::list <InstructionContainer> &instructionList = bbIterator_2->return_instructionListRef();                //careful here since we are directly modifying the list
               for(std::list <InstructionContainer>::iterator insIterator = instructionList.begin(); insIterator != instructionList.end(); insIterator++)
               {
                  if(insIterator->return_opCode() == iLoad)
                  {
                     if(diff_storeCount >= 1)
                     {
//    std::cout << "Swapping LOAD for STORE" << "\n";
                        diff_storeCount = diff_storeCount - 1;
                        *insIterator = storeInstruction;

                        if(targetWriteSetSize >= 2)
                        {
                           if(targetWriteSetSize > (STREAM_SIZE * MEM_MULTIPLIER) / 32)
                           {
                              std::cerr << "\nYabba dabba doo!\n\n";
                              exit(0);
                           }
                           else if(bbIterator_2->return_accumulated() <= 1)
                           {
//       std::cout << "Ss-Target:  " << targetWriteSetSize;
                              insIterator->strideAmount = 32 * (UINT_32)targetWriteSetSize;
                              targetWriteSetSize = targetWriteSetSize - 1;
//       std::cout << "\tstride:  " << insIterator->strideAmount << "\tsharedMem:  " << insIterator->return_sharedMem() << "\n";
                           }
                        }

                     }
                     else if(targetReadSetSize >= 2)
                     {
                        if(targetWriteSetSize > (STREAM_SIZE * MEM_MULTIPLIER) / 32)
                        {
                           std::cerr << "\nYabba dabba doo!\n\n";
                           exit(0);
                        }
                        else if(bbIterator_2->return_accumulated() <= 1)
                        {
//    std::cout << "L-Target:  " << targetReadSetSize;
                           insIterator->strideAmount = 32 * (UINT_32)targetReadSetSize;
                           targetReadSetSize = targetReadSetSize - 1;
//    std::cout << "\tstride:  " << insIterator->strideAmount << "\tsharedMem:  " << insIterator->return_sharedMem() << "\n";
                        }
                     }

                     loadCount = loadCount + 1;
                  }
                  else if(insIterator->return_opCode() == iStore)
                  {
                     if(diff_loadCount >= 1)
                     {
//    std::cout << "Swapping STORE for LOAD" << "\n";
                        diff_loadCount = diff_loadCount - 1;
                        *insIterator = loadInstruction;

                        if(targetReadSetSize >= 2)
                        {
                           if(targetWriteSetSize > (STREAM_SIZE * MEM_MULTIPLIER) / 32)
                           {
                              std::cerr << "\nYabba dabba doo!\n\n";
                              exit(0);
                           }
                           else if(bbIterator_2->return_accumulated() <= 1)
                           {
//       std::cout << "Ls-Target:  " << targetReadSetSize;
                              insIterator->strideAmount = 32 * (UINT_32)targetReadSetSize;
                              targetReadSetSize = targetReadSetSize - 1;
//       std::cout << "\tstride:  " << insIterator->strideAmount << "\tsharedMem:  " << insIterator->return_sharedMem() << "\n";
                           }
                        }

                     }
                     else if(targetWriteSetSize >= 2)
                     {
                        if(targetWriteSetSize > (STREAM_SIZE * MEM_MULTIPLIER) / 32)
                        {
                           std::cerr << "\nYabba dabba doo!\n\n";
                           exit(0);
                        }
                        else if(bbIterator_2->return_accumulated() <= 1)
                        {
//    std::cout << "S-Target:  " << targetWriteSetSize;
                           insIterator->strideAmount = 32 * (UINT_32)targetWriteSetSize;
                           targetWriteSetSize = targetWriteSetSize - 1;
//    std::cout << "\tstride:  " << insIterator->strideAmount << "\tsharedMem:  " << insIterator->return_sharedMem() << "\n";
                        }
                     }

                     storeCount = storeCount + 1;
                  }
               }
            }  //end inner for


         instructionCount = 0;
         currentTrans = 0;
         loadCount = 0;
         storeCount = 0;
      }

   }


///------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
std::cout << "\n\n" << std::endl;
   loadCount = 0;
   storeCount = 0;
   UINT_32 s_loads = 0;
   UINT_32 s_stores = 0;
   UINT_32 u_stores = 0;
   UINT_32 u_loads = 0;
  //go through the synthetic spine for each thread in uniqueThreads and count the mem mix
   for(std::vector < THREAD_ID >::iterator threadIterator = uniqueThreads.begin(); threadIterator != uniqueThreads.end(); threadIterator++)
   {
      std::cout << "...NT" << *threadIterator << std::flush;

      std::list <BasicBlock> bbList = syntheticThreads[*threadIterator]->return_coreList();
      for(std::list <BasicBlock>::iterator bbIterator = bbList.begin(); bbIterator != bbList.end(); bbIterator++)
      {
         if(bbIterator->return_isTrans() == 1)
         {
            const std::list <InstructionContainer> &instructionList = bbIterator->return_instructionListRef();
            for(std::list <InstructionContainer>::const_iterator insIterator = instructionList.begin(); insIterator != instructionList.end(); insIterator++)
            {
               if(insIterator->return_opCode() == iLoad)
               {
                  loadCount = loadCount + 1;

                  if(insIterator->strideAmount != 4)
                     u_loads = u_loads + 1;
                  if(insIterator->return_sharedMem() == 1)
                     s_loads = s_loads + 1;
               }
               else if(insIterator->return_opCode() == iStore)
               {
                  storeCount = storeCount + 1;

                  if(insIterator->strideAmount != 4)
                     u_stores = u_stores + 1;
                  if(insIterator->return_sharedMem() == 1)
                     s_stores = s_stores + 1;
               }
            }
         }
      }  //end inner for
// std::cout << "\nLoad Count:  " << loadCount << "\tStore Count:  " << storeCount << "\tW/R:  " << writes_reads;
// std::cout << "\nUnique Loads:  " << u_loads << "\tUnique Stores:  " << u_stores;
// std::cout << "\nShared Loads:  " << s_loads << "\tShared Stores:  " << s_stores << "\n";
   }     //end outer for

   if(loadCount > 0)
      writes_reads = storeCount / loadCount;
   else
      writes_reads = -1;

// std::cout << "\n*Load Count:  " << loadCount << "\tStore Count:  " << storeCount << "\tW/R:  " << writes_reads;
// std::cout << "\n*Unique Loads:  " << u_loads << "\tUnique Stores:  " << u_stores;
// std::cout << "\n*Shared Loads:  " << s_loads << "\tShared Stores:  " << s_stores << "\n";

   std::cout << "...Finished" << std::flush;
}

/**
 * @name writeOutSynthetic
 * 
 * @param syntheticThreads[] 
 * @param arraySize 
 * @return 
 */
void writeOutSynthetic(Synthetic *syntheticThreads[], UINT_32 arraySize)
{
   /* Variable Declaraion */
   CodeLogic *syntheticCodeBlock = new CodeLogic(totalNumThreads);
   ConfObject *statConf = new ConfObject;
   UINT_32 numThreads = totalNumThreads;

   string fileName = Synthesis::statPaths.return_rootDirectory() + Synthesis::statPaths.return_synthDirectory() + Synthesis::statPaths.return_outputFileName();
   fileName = fileName + ".synthetic.c";
   ofstream outputFile(fileName.c_str(), ios::trunc);       //open a file for writing (truncate the current contents)
   if(!outputFile)                                          //check to be sure file is open
      std::cerr << "Error opening file.";

   /* Processes */
   std::cout << "\nBuilding Synthetic with a maximum of " << statConf->return_maxBasicBlocks() << " BBs" << std::flush;
   for(UINT_32 threadCounter = 0; threadCounter < numThreads; threadCounter++)
   {
      BOOL isUnique = 0;
      lastWriteStride = lastReadStride = 0;
      StatMemory::initMemory(threadCounter);

      //check to see if the thread shares code with another thread
      for(std::map< ADDRESS_INT, THREAD_ID >::iterator mapIter = CodeGenerator::threadFuncMap.begin(); mapIter != CodeGenerator::threadFuncMap.end(); mapIter++)
      {
         if(mapIter->second == threadCounter)
         {
            isUnique = 1;
            break;
         }
      }

      std::cout << "...T" << threadCounter << std::flush;

      if(isUnique == 1 || threadCounter == 0)
      {
         //set up the function
         if(threadCounter == 0)
            headerGen(outputFile);
         else
            funcHeaderGen(threadCounter, outputFile);

         //write contents of current basic block to file
         while(syntheticThreads[threadCounter]->return_size_of_coreList() > 0)
         {
            writeBasicBlock(syntheticCodeBlock, syntheticThreads[threadCounter]->return_front_of_coreList_pop(), threadCounter, syntheticThreads[threadCounter]->return_node_ID(), outputFile);
            syntheticThreads[threadCounter]->inc_node_ID();
         }
         writeLabel(threadCounter, syntheticThreads[threadCounter]->return_node_ID(), outputFile);

         //close out the function
         if(threadCounter == 0)
            trailerGen(syntheticCodeBlock, threadCounter, outputFile);
         else
            funcTrailerGen(syntheticCodeBlock, threadCounter, outputFile);
      }
   }

   delete statConf;
   delete syntheticCodeBlock;

   outputFile.close();
   std::cout << "...Finished" << std::flush;
}


/**
 * @name writeBasicBlock
 * 
 * @param localBasicBlcok 
 * @param fileName 
 * @return 
**/
void writeBasicBlock(CodeLogic *syntheticCodeBlock, const BasicBlock &localBasicBlock, THREAD_ID threadID, UINT_32 nodeID, std::ofstream &outputFile)
{
   /* Variable Declaration */

   /* Processes */
   writeLabel(threadID, nodeID, outputFile);

   if(localBasicBlock.return_isCritical() == 1)
   {
      //Lookup the synthtic ID associated with the lock address
      if(syntheticCodeBlock->lastLock[threadID] == 0)
      {
         startLockSection(globalMutexMap[localBasicBlock.return_lockID()], outputFile);
      }
      else if(syntheticCodeBlock->lastLock[threadID] != localBasicBlock.return_lockID())
      {
         endLockSection(globalMutexMap[syntheticCodeBlock->lastLock[threadID]], outputFile);
         startLockSection(globalMutexMap[localBasicBlock.return_lockID()], outputFile);
      }

      syntheticCodeBlock->lastLock[threadID] = localBasicBlock.return_lockID();
   }
   else if(syntheticCodeBlock->lastLock[threadID] > 0 && localBasicBlock.return_isCritical() == 0)
   {
      endLockSection(globalMutexMap[syntheticCodeBlock->lastLock[threadID]], outputFile);
      syntheticCodeBlock->lastLock[threadID] = 0;
   }

   if(localBasicBlock.return_isTrans() == 1)
   {
      //Lookup the synthtic ID associated with the trans address
      if(syntheticCodeBlock->lastTrans[threadID] == 0)
      {
         if(localBasicBlock.return_accumulated() > 0 && syntheticCodeBlock->accumulatedValue[threadID] == 0)
         {
            startAccumulationLoop(threadID, localBasicBlock.return_accumulated(), outputFile);
            syntheticCodeBlock->accumulatedValue[threadID] = localBasicBlock.return_accumulated();
         }

         startTransSection(localBasicBlock.return_transID(), outputFile);
      }
      else if(syntheticCodeBlock->lastTrans[threadID] != localBasicBlock.return_transID())
      {
         endTransSection(localBasicBlock.return_transID(), outputFile);

         if(localBasicBlock.return_accumulated() > 0 && syntheticCodeBlock->accumulatedValue[threadID] != localBasicBlock.return_accumulated() && syntheticCodeBlock->accumulatedValue[threadID] > 0)
         {
            endAccumulationLoop(outputFile);
            startAccumulationLoop(threadID, localBasicBlock.return_accumulated(), outputFile);
            syntheticCodeBlock->accumulatedValue[threadID] = localBasicBlock.return_accumulated();
         }
         else if(syntheticCodeBlock->accumulatedValue[threadID] > 0 && localBasicBlock.return_accumulated() == 0)
         {
            endAccumulationLoop(outputFile);
            syntheticCodeBlock->accumulatedValue[threadID] = 0;
         }

         startTransSection(localBasicBlock.return_transID(), outputFile);
      }

      syntheticCodeBlock->lastTrans[threadID] = localBasicBlock.return_transID();
   }
   else if(syntheticCodeBlock->lastTrans[threadID] > 0 && localBasicBlock.return_isTrans() == 0)
   {
      endTransSection(localBasicBlock.return_transID(), outputFile);

      if(localBasicBlock.return_accumulated() > 0)
      {
         if(syntheticCodeBlock->accumulatedValue[threadID] == 0)
         {
            startAccumulationLoop(threadID, localBasicBlock.return_accumulated(), outputFile);
         }
         else if(syntheticCodeBlock->accumulatedValue[threadID] != localBasicBlock.return_accumulated())
         {
            endAccumulationLoop(outputFile);
            startAccumulationLoop(threadID, localBasicBlock.return_accumulated(), outputFile);
         }

         syntheticCodeBlock->accumulatedValue[threadID] = localBasicBlock.return_accumulated();
      }
      else if(syntheticCodeBlock->accumulatedValue[threadID] > 0 && localBasicBlock.return_accumulated() == 0)
      {
         endAccumulationLoop(outputFile);
         syntheticCodeBlock->accumulatedValue[threadID] = 0;
      }

      syntheticCodeBlock->lastTrans[threadID] = 0;
   }

   if(localBasicBlock.return_isThreadEvent() == 1)
   {
      if(localBasicBlock.return_isSpawn() == 1)
      {
//          localBasicBlock.update_isSpawn(0);

         writeSpawn(localBasicBlock.childThreads, outputFile, 1);
      }
      else if(localBasicBlock.return_isDestroy() == 1)
      {

      }

      if(localBasicBlock.return_isWait() == 1)
      {
//          localBasicBlock.update_isWait(0);
         writeWait(outputFile);
      }

      if(localBasicBlock.return_isBarrier() == 1)
      {
//          localBasicBlock.update_isBarrier(0);
         writeBarrier("progBarr", 0, outputFile);
      }
   }
   else
   {
      std::list <InstructionContainer>::iterator instructionListIterator;
      std::list <InstructionContainer> tempInstructionList = localBasicBlock.return_instructionList();

//       writeLabel(threadID, nodeID, outputFile);

      for(instructionListIterator = tempInstructionList.begin(); instructionListIterator !=  tempInstructionList.end(); instructionListIterator++)
      {
         //NOTE uniform RV over [0,1)
         static boost::lagged_fibonacci607 rng(static_cast<unsigned> (std::time(0)));
         boost::uniform_real<double> norm_dist(0, 1);
         boost::variate_generator<boost::lagged_fibonacci607&, boost::uniform_real<double> >  uniformRandom(rng, norm_dist);

         UINT_32 numConflicts;
         float numOperations;
         float conflictProbability;
         double uniformRV =  uniformRandom();

         std::map< ADDRESS_INT, UINT_32 > conflictMap;
         instructionListIterator->update_transID(syntheticCodeBlock->lastTrans[threadID]);

         //need to get conflicts for the synthetic
         if(localBasicBlock.return_isTrans() == 1)
         {
//             #if defined(DEBUG)
//             std::cout << "CG-" << threadID << "\tTX " << localBasicBlock.return_transID() << "\tR:  " << localBasicBlock.return_readConflictMapSize() << "\tW:  " << localBasicBlock.return_writeConflictMapSize() << std::endl;
//             #endif

            if(instructionListIterator->return_opCode() == iLoad)
            {
               numConflicts = localBasicBlock.return_readConflictMapSize();

               if(numConflicts > 0)
               {
                  writeInstruction(syntheticCodeBlock, *instructionListIterator, threadID, nodeID, outputFile, localBasicBlock.return_readConflictMap());
// localBasicBlock.instructionMix.print_mixBins(std::cout);
//                   numOperations = (float)localBasicBlock.instructionMix.return_bin(7);
//                   conflictProbability = numConflicts / numOperations;
// std::cout << "L- numOps:  " << numOperations << "\tCProb:  " << conflictProbability << std::endl;
//                   if(conflictProbability > uniformRV)
//                   {
//                      writeInstruction(*instructionListIterator, threadID, nodeID, outputFile, localBasicBlock.return_readConflictMap());
//                   }
//                   else
//                   {
//                      writeInstruction(*instructionListIterator, threadID, nodeID, outputFile, conflictMap);
//                   }
               }
               else
               {
                  writeInstruction(syntheticCodeBlock, *instructionListIterator, threadID, nodeID, outputFile, conflictMap);
               }
            }
            else if(instructionListIterator->return_opCode() == iStore)
            {
               numConflicts = localBasicBlock.return_writeConflictMapSize();

               if(numConflicts > 0)
               {
                  writeInstruction(syntheticCodeBlock, *instructionListIterator, threadID, nodeID, outputFile, localBasicBlock.return_writeConflictMap());
//                   numOperations = (float)localBasicBlock.instructionMix.return_bin(8);
//                   conflictProbability = numConflicts / numOperations;
// 
//                   if(conflictProbability > uniformRV)
//                   {
//                      writeInstruction(*instructionListIterator, threadID, nodeID, outputFile, localBasicBlock.return_writeConflictMap());
//                   }
//                   else
//                   {
//                      writeInstruction(*instructionListIterator, threadID, nodeID, outputFile, conflictMap);
//                   }
               }
               else
               {
                  writeInstruction(syntheticCodeBlock, *instructionListIterator, threadID, nodeID, outputFile, conflictMap);
               }
            }
            else
            {
               writeInstruction(syntheticCodeBlock, *instructionListIterator, threadID, nodeID, outputFile, conflictMap);
            }
         }
         else
         {
            writeInstruction(syntheticCodeBlock, *instructionListIterator, threadID, nodeID, outputFile, conflictMap);
         }
      }

      outputFile << "\n" << std::flush;
   }
}


/**
 * @name writeInstruction
 * 
 * @param instructionIn 
 * @param threadID 
 * @param nodeID 
 * @param fileName 
 * @return 
__asm__ __volatile__ ("bgt %0, %1,END" : :"r"(counter), "r"(maxCounter) );
 */
void writeInstruction(CodeLogic *syntheticCodeBlock, const InstructionContainer &instructionIn, THREAD_ID threadID, UINT_32 nodeID, std::ofstream &outputFile, const std::map< ADDRESS_INT, UINT_32 > &conflictMap)
{
   OperandList operandList;
   string opCode = translateInstruction(syntheticCodeBlock, instructionIn, operandList, threadID, nodeID, conflictMap);

   outputFile << "   __asm__ __volatile__ ( \"";
   outputFile << opCode;

   if(operandList.rd != "")
      outputFile << " "  << operandList.rd;
   if(operandList.rs != "")
      outputFile << ", " << operandList.rs;
   if(operandList.rt != "")
      outputFile << ", " << operandList.rt;

   outputFile << setw(6) << "\"\t :";

   if(instructionIn.return_opCode() == iStore)
   {
      outputFile << " \"=" << operandList.rt_variable;

      outputFile << " :";
      outputFile << " \""  << operandList.rd_variable;
   }
   else
   {
      if(operandList.rd != "" && opCode != "b")
         outputFile << " \"=" << operandList.rd_variable;

      outputFile << " :";
      if(operandList.rs != "" && operandList.rs_variable != "")
         outputFile << " \""  << operandList.rs_variable;
      if(operandList.rt != "" && operandList.rs == "")
         outputFile << " \""  << operandList.rt_variable;
      else if(operandList.rt != "")
         outputFile << ", \"" << operandList.rt_variable;
   }

   if(operandList.clobberList.size() > 0)
      outputFile << " : " << operandList.clobberList;

   outputFile << " );";

   outputFile << std::endl;
}


/**
 * @name translateInstruction
 * 
 * @param instructionIn 
 * @param operandList 
 * @param threadID 
 * @param nodeID 
 * @return 
 */
string translateInstruction(CodeLogic *syntheticCodeBlock, InstructionContainer instructionIn, OperandList &operandList, THREAD_ID threadID, UINT_32 nodeID, std::map< ADDRESS_INT, UINT_32 > conflictMap)
{
   string opCodeOut;
   UINT_32 stride = 0;
   UINT_32 opNum = instructionIn.return_opNum();

   if(instructionIn.return_subCode() == iNop)                                                                                             //nops
   {
      opCodeOut = "nop";
   }
   else if(instructionIn.return_opCode() == iALU && instructionIn.return_subCode() == iSubInvalid)                                        //int alu
   {
      switch(opNum)
      {
         case move_opn:
         case addu_opn:
         case add_opn:
         case and_opn:
         case nor_opn:
         case or_opn:
         case sllv_opn:
         case slt_opn:
         case sltu_opn:
         case srlv_opn:
         case srav_opn:
         case sub_opn:
         case subu_opn:
         case xor_opn:
            opCodeOut = "add";
            operandList.rs = "%1";
            operandList.rt = "%2";
            operandList.rd = "%0";
            operandList.rs_variable = getIntVariable(instructionIn.return_src1());
            operandList.rt_variable = getIntVariable(instructionIn.return_src2());
            operandList.rd_variable = getIntVariable(instructionIn.return_dest());
            break;
         case li_opn:
            opCodeOut = "li";
            if(instructionIn.return_immediate() > (INT_32)MAX_VALUE)
               operandList.rs = IntToString(2);
            else
               operandList.rs = IntToString(instructionIn.return_immediate());
            operandList.rd = "%0";
            operandList.rs_variable = "";
            operandList.rd_variable = getIntVariable(instructionIn.return_dest());
            break;
         case addiu_opn:
         case addi_opn:
         case slti_opn:
         case sltiu_opn:
         case andi_opn:
         case ori_opn:
         case xori_opn:
            opCodeOut = "andi";
            if(instructionIn.return_immediate() > (INT_32)MAX_VALUE)
               operandList.rs = IntToString(2);
            else
               operandList.rs = IntToString(instructionIn.return_immediate());
            operandList.rd = "%0";
            operandList.rs_variable = "";
            operandList.rd_variable = getIntVariable(instructionIn.return_dest());
            break;
         case lui_opn:
            opCodeOut = "lui";
            if(instructionIn.return_immediate() > (INT_32)MAX_VALUE)
               operandList.rs = IntToString(2);
            else
               operandList.rs = IntToString(instructionIn.return_immediate());
            operandList.rd = "%0";
            operandList.rs_variable = "";
            operandList.rd_variable = getIntVariable(instructionIn.return_dest());
            break;
         case mfhi_opn:
         case mflo_opn:
            opCodeOut = "mfhi";
            operandList.rd = "%0";
            operandList.rs_variable = "";
            operandList.rd_variable = getIntVariable(instructionIn.return_dest());
            break;
         case mthi_opn:
         case mtlo_opn:
            opCodeOut = "mthi";
            break;
         case sll_opn:
         case sra_opn:
         case srl_opn:
            opCodeOut = "sll";
            operandList.rs = "%1";
            operandList.rd = "%0";
            operandList.rs_variable = getIntVariable(instructionIn.return_src1());
            operandList.rd_variable = getIntVariable(instructionIn.return_dest());
            break;
         default:
            break;
      }
   }
   else if(instructionIn.return_opCode() == iMult)                                                                                        //int mult
   {}
   else if(instructionIn.return_opCode() == iDiv)                                                                                         //int div
   {}
   else if(instructionIn.return_opCode() == iBJ)                                                                                          //control
   {
      opCodeOut = "b";
      operandList.rd = "I" + IntToString(threadID) + "_" + IntToString(nodeID + 1) + "_";

      if(instructionIn.return_subCode() == BJCond)
      {}
      else if(instructionIn.return_subCode() == BJUncond)
      {}
      else if(instructionIn.return_subCode() == BJCall)
      {}
      else if(instructionIn.return_subCode() == BJRet)
      {}
      else
      {}
   }
   else if(instructionIn.return_opCode() == fpALU)                                                                                         //fp alu
   {
      switch(opNum)
      {
         case add_s_opn:
         case sub_s_opn:
            opCodeOut = "add.s";
            operandList.rs = "%1";
            operandList.rt = "%2";
            operandList.rd = "%0";
            operandList.rs_variable = getFPVariable(instructionIn.return_src1());
            operandList.rt_variable = getFPVariable(instructionIn.return_src2());
            operandList.rd_variable = getFPVariable(instructionIn.return_dest());
            break;
         case abs_s_opn:
         case mov_s_opn:
         case neg_s_opn:
         case round_w_s_opn:
         case trunc_w_s_opn:
         case ceil_w_s_opn:
         case floor_w_s_opn:
         case cvt_w_s_opn:
            opCodeOut = "abs.s";
            operandList.rs = "%1";
            operandList.rd = "%0";
            operandList.rs_variable = getFPVariable(instructionIn.return_src1());
            operandList.rt_variable = "";
            operandList.rd_variable = getFPVariable(instructionIn.return_dest());
            break;
         case cvt_d_s_opn:
            opCodeOut = "cvt.d.s";
            operandList.rs = "%1";
            operandList.rd = "%0";
            operandList.rs_variable = getFPVariable(instructionIn.return_src1());
            operandList.rt_variable = "";
            operandList.rd_variable = getFPVariable(instructionIn.return_dest());
            break;
         case c_f_s_opn:
         case c_un_s_opn:
         case c_eq_s_opn:
         case c_ueq_s_opn:
         case c_olt_s_opn:
         case c_ult_s_opn:
         case c_ole_s_opn:
         case c_ule_s_opn:
         case c_sf_s_opn:
         case c_ngle_s_opn:
         case c_seq_s_opn:
         case c_ngl_s_opn:
         case c_lt_s_opn:
         case c_nge_s_opn:
         case c_le_s_opn:
         case c_ngt_s_opn:
            opCodeOut = "c.f.s";
            operandList.rs = "%1";
            operandList.rd = "%0";
            operandList.rs_variable = getFPVariable(instructionIn.return_src1());
            operandList.rt_variable = "";
            operandList.rd_variable = getFPVariable(instructionIn.return_dest());
            break;
         case add_d_opn:
         case sub_d_opn:
            opCodeOut = "add.d";
            operandList.rs = "%1";
            operandList.rt = "%2";
            operandList.rd = "%0";
            operandList.rs_variable = getFPVariable(instructionIn.return_src1());
            operandList.rt_variable = getFPVariable(instructionIn.return_src2());
            operandList.rd_variable = getFPVariable(instructionIn.return_dest());
            break;
         case abs_d_opn:
         case mov_d_opn:
         case neg_d_opn:
         case round_w_d_opn:
         case trunc_w_d_opn:
         case ceil_w_d_opn:
         case floor_w_d_opn:
            opCodeOut = "abs.d";
            operandList.rs = "%1";
            operandList.rd = "%0";
            operandList.rs_variable = getFPVariable(instructionIn.return_src1());
            operandList.rt_variable = "";
            operandList.rd_variable = getFPVariable(instructionIn.return_dest());
            break;
         case cvt_s_d_opn:
         case cvt_w_d_opn:
            opCodeOut = "cvt.s.d";
            operandList.rs = "%1";
            operandList.rd = "%0";
            operandList.rs_variable = getFPVariable(instructionIn.return_src1());
            operandList.rt_variable = "";
            operandList.rd_variable = getFPVariable(instructionIn.return_dest());
            break;
         case c_f_d_opn:
         case c_un_d_opn:
         case c_eq_d_opn:
         case c_ueq_d_opn:
         case c_olt_d_opn:
         case c_ult_d_opn:
         case c_ole_d_opn:
         case c_ule_d_opn:
         case c_sf_d_opn:
         case c_ngle_d_opn:
         case c_seq_d_opn:
         case c_ngl_d_opn:
         case c_lt_d_opn:
         case c_nge_d_opn:
         case c_le_d_opn:
         case c_ngt_d_opn:
            opCodeOut = "c.f.d";
            operandList.rs = "%1";
            operandList.rd = "%0";
            operandList.rs_variable = getFPVariable(instructionIn.return_src1());
            operandList.rt_variable = "";
            operandList.rd_variable = getFPVariable(instructionIn.return_dest());
            break;
         case cvt_s_w_opn:
            opCodeOut = "cvt.s.w";
            operandList.rs = "%1";
            operandList.rd = "%0";
            operandList.rs_variable = getFPVariable(instructionIn.return_src1());
            operandList.rt_variable = "";
            operandList.rd_variable = getFPVariable(instructionIn.return_dest());
            break;
         case cvt_d_w_opn:
            opCodeOut = "cvt.d.w";
            operandList.rs = "%1";
            operandList.rd = "%0";
            operandList.rs_variable = getFPVariable(instructionIn.return_src1());
            operandList.rt_variable = "";
            operandList.rd_variable = getFPVariable(instructionIn.return_dest());
            break;
         case mfc1_opn:
            opCodeOut = "mfc1";
            operandList.rs = "%1";
            operandList.rd = "%0";
            operandList.rs_variable = getFPVariable(instructionIn.return_src1());
            operandList.rt_variable = "";
            operandList.rd_variable = getFPVariable(instructionIn.return_dest());
            break;
         case mtc1_opn:
            opCodeOut = "mtc1";
            operandList.rs = "%1";
            operandList.rd = "%0";
            operandList.rs_variable = getFPVariable(instructionIn.return_src1());
            operandList.rt_variable = "";
            operandList.rd_variable = getIntVariable(instructionIn.return_dest());
            break;
         case cfc1_opn:
            opCodeOut = "cfc1";
            operandList.rs = "%1";
            operandList.rd = "%0";
            operandList.rs_variable = getFPVariable(instructionIn.return_src1());
            operandList.rt_variable = "";
            operandList.rd_variable = getIntVariable(instructionIn.return_dest());
            break;
         case ctc1_opn:
            opCodeOut = "ctc1";
            operandList.rs = "%1";
            operandList.rd = "%0";
            operandList.rs_variable = getFPVariable(instructionIn.return_src1());
            operandList.rt_variable = "";
            operandList.rd_variable = getIntVariable(instructionIn.return_dest());
            break;
         default:
            break;
      }
   }
   else if(instructionIn.return_opCode() == fpMult)                                       //fp mult
   {
   }
   else if(instructionIn.return_opCode() == fpDiv)                                        //fp div
   {
   }
   else if(instructionIn.return_opCode() == iLoad)                                        //loads
   {
      if(instructionIn.return_dest() >= 35 && instructionIn.return_dest() <= 63)
      {
         if(instructionIn.return_dataSize() == 4)
            opCodeOut = "lwc1 ";
         else
            opCodeOut = "ldc1 ";

         operandList.rd_variable = getFPVariable(instructionIn.return_dest());

         if(instructionIn.return_transID() > 0 && conflictMap.size() > 0)
            operandList.rs_variable =  "r\"(s_data_out_float)";                           //source
         else if(instructionIn.return_transID() > 0 && instructionIn.return_sharedMem() == 1)
            operandList.rs_variable =  "r\"(s_data_out_float)";                           //source
         else
            operandList.rs_variable =  "r\"(data_out_float)";                             //source
      }
      else
      {
         opCodeOut = "lw ";
         operandList.rd_variable = getIntVariable(instructionIn.return_dest());

         if(instructionIn.return_transID() > 0 && conflictMap.size() > 0)
            operandList.rs_variable =  "r\"(s_data_out_int)";                             //source
         else if(instructionIn.return_transID() > 0 && instructionIn.return_sharedMem() == 1)
            operandList.rs_variable =  "r\"(s_data_out_int)";                             //source
         else
            operandList.rs_variable =  "r\"(data_out_int)";                               //source
      }

      //get the offset from the base
      if(instructionIn.return_transID() > 0)
      {
         if(conflictMap.size() > 0)
         {
            ADDRESS_INT virtualAddress = 0;
            ADDRESS_INT physicalAddress = syntheticCodeBlock->virtual_to_physical[instructionIn.return_virtualAddress()];
            std::map< ADDRESS_INT, ADDRESS_INT >::iterator virtualMapIterator;

            if(physicalAddress == 0)
            {
               for(std::map< ADDRESS_INT, UINT_32 >::iterator conflictMapIterator = conflictMap.begin(); conflictMapIterator != conflictMap.end(); conflictMapIterator++)
               {
                  for(virtualMapIterator = syntheticCodeBlock->virtual_to_physical.begin(); virtualMapIterator != syntheticCodeBlock->virtual_to_physical.end(); virtualMapIterator++)
                  {
                     virtualAddress = virtualMapIterator->second;
                     if(virtualMapIterator->second == conflictMapIterator->first)
                     {
                        break;
                     }

                  }

                  if(virtualAddress == 0)
                  {
                     syntheticCodeBlock->virtual_to_physical[instructionIn.return_virtualAddress()] = conflictMapIterator->first;
                     break;
                  }
               }
            }

            stride = StatMemory::returnTransReadStride(syntheticCodeBlock->virtual_to_physical[instructionIn.return_virtualAddress()]);
            operandList.rs = IntToString(stride) + "(%1)";
         }
         else if(instructionIn.return_sharedMem() == 1)
         {
            stride = (UINT_32)ceil(instructionIn.strideAmount);
            operandList.rs = IntToString(stride) + "(%1)";
         }
         else if(instructionIn.strideAmount > 0)
         {
            stride = lastReadStride + instructionIn.strideAmount;
            operandList.rs = IntToString(stride) + "(%1)";
         }
         else
         {
            stride = StatMemory::returnReadStride();
            operandList.rs = IntToString(stride) + "(%1)";
         }
      }
      else
      {
         stride = StatMemory::returnReadStride();

         lastReadStride = lastReadStride + stride;
         if(lastReadStride > STREAM_SIZE * MEM_MULTIPLIER)
            lastReadStride = 0;

         operandList.rs = IntToString(lastReadStride) + "(%1)";
      }

      operandList.rt = "";
      operandList.rd = "%0";
   }
   else if(instructionIn.return_opCode() == iStore)                                       //stores
   {
      if(instructionIn.return_src2() >= 35 && instructionIn.return_src2() <= 63)
      {
         if(instructionIn.return_dataSize() == 4)
            opCodeOut = "swc1 ";
         else
            opCodeOut = "sdc1 ";

         operandList.rd_variable = getFPVariable(instructionIn.return_src2());            //source

         if(instructionIn.return_transID() > 0 && conflictMap.size() > 0)
            operandList.rt_variable =  "r\"(s_data_out_float)";                           //destination
         else if(instructionIn.return_transID() > 0 && instructionIn.return_sharedMem() == 1)
            operandList.rt_variable =  "r\"(s_data_out_float)";                           //destination
         else
            operandList.rt_variable =  "r\"(data_out_float)";                             //destination
      }
      else
      {
         opCodeOut = "sw ";
         operandList.rd_variable = getIntVariable(instructionIn.return_src2());           //source

         if(instructionIn.return_transID() > 0 && conflictMap.size() > 0)
            operandList.rt_variable =  "r\"(s_data_out_int)";                             //destination
         else if(instructionIn.return_transID() > 0 && instructionIn.return_sharedMem() == 1)
            operandList.rt_variable =  "r\"(s_data_out_int)";                             //destination
         else
            operandList.rt_variable =  "r\"(data_out_int)";                               //destination
      }

      //get the offset from the base
      if(instructionIn.return_transID() > 0)
      {
         if(conflictMap.size() > 0)
         {
            ADDRESS_INT virtualAddress = 0;
            ADDRESS_INT physicalAddress = syntheticCodeBlock->virtual_to_physical[instructionIn.return_virtualAddress()];
            std::map< ADDRESS_INT, ADDRESS_INT >::iterator virtualMapIterator;

            if(physicalAddress == 0)
            {
               for(std::map< ADDRESS_INT, UINT_32 >::iterator conflictMapIterator = conflictMap.begin(); conflictMapIterator != conflictMap.end(); conflictMapIterator++)
               {
                  for(virtualMapIterator = syntheticCodeBlock->virtual_to_physical.begin(); virtualMapIterator != syntheticCodeBlock->virtual_to_physical.end(); virtualMapIterator++)
                  {
                     virtualAddress = virtualMapIterator->second;
                     if(virtualMapIterator->second == conflictMapIterator->first)
                     {
                        break;
                     }

                  }

                  if(virtualAddress == 0)
                  {
                     syntheticCodeBlock->virtual_to_physical[instructionIn.return_virtualAddress()] = conflictMapIterator->first;
                     break;
                  }
               }
            }

            stride = StatMemory::returnTransWriteStride(syntheticCodeBlock->virtual_to_physical[instructionIn.return_virtualAddress()]);
            operandList.rt = IntToString(stride) + "(%0)";
         }
         else if(instructionIn.return_sharedMem() == 1)
         {
            stride = (UINT_32)ceil(instructionIn.return_sharedMem());
            operandList.rt = IntToString(stride) + "(%0)";
         }
         else if(instructionIn.strideAmount > 0)
         {
            stride = lastWriteStride + instructionIn.strideAmount;
            operandList.rt = IntToString(stride) + "(%0)";
         }
         else
         {
            stride = StatMemory::returnWriteStride();
            operandList.rt = IntToString(stride) + "(%0)";
         }
      }
      else
      {
         stride = StatMemory::returnWriteStride();

         lastWriteStride = lastWriteStride + stride;
         if(lastWriteStride > STREAM_SIZE * MEM_MULTIPLIER)
            lastWriteStride = 0;

         operandList.rt = IntToString(lastWriteStride) + "(%0)";
      }

      operandList.rs = "";
      operandList.rd = "%1";

      operandList.clobberList = operandList.clobberList + "\"memory\"";
   }
   else
   {
   }

   return opCodeOut;
}


/**
 * @name getVariable
 * 
 * @short This function decides which integer registers are used in the synthetic.
 * @param registerIn 
 * @return 
 */
string getIntVariable(RegType registerIn)
{
   string variableList;

   if(registerIn == 99)
   {
      variableList = "";
   }
   else
   {
      //saved temporary 20-23 used for memory accesses
      switch(registerIn)
      {
         case 0:
         case 1:
         case 2:
         case 3:
         case 4:
         case 5:
         case 6:
         case 7:
         case 8:
            variableList = "r\"(r_out_t0)";
            break;
         case 9:
            variableList = "r\"(r_out_t1)";
            break;
         case 10:
            variableList = "r\"(r_out_t2)";
            break;
         case 11:
            variableList = "r\"(r_out_t3)";
            break;
         case 12:
            variableList = "r\"(r_out_t4)";
            break;
         case 13:
         case 14:
         case 15:
            variableList = "r\"(r_out_t5)";
            break;
         case 16:
            variableList = "r\"(r_out_s0)";
            break;
         case 17:
            variableList = "r\"(r_out_s1)";
            break;
         case 18:
            variableList = "r\"(r_out_s2)";
            break;
         case 19:
            variableList = "r\"(r_out_s3)";
            break;
         case 20:
            variableList = "r\"(r_out_t4)";
            break;
         case 21:
         case 22:
         case 23:
            variableList = "r\"(r_out_t5)";
            break;
         case 24:
         case 25:
         case 26:
         case 27:
         case 28:
         case 29:
         case 30:
         case 31:
         default:
            variableList = "r\"(r_out_s0)";
            break;
      }
   }

   return variableList;
}


/**
 * @name getVariable
 * 
 * @short This function decides which floating point registers are used in the synthetic.
 * @param registerIn 
 * @return 
 */
string getFPVariable(RegType registerIn)
{
   string variableList;

   if(registerIn == 99)
   {
      variableList = "";
   }
   else
   {
      switch(registerIn)
      {
         case 35:
            variableList = "f\"(r_out_f2)";
            break;
         case 37:
            variableList = "f\"(r_out_f4)";
            break;
         case 39:
            variableList = "f\"(r_out_f6)";
            break;
         case 41:
            variableList = "f\"(r_out_f8)";
            break;
         case 43:
            variableList = "f\"(r_out_f10)";
            break;
         case 45:
         case 47:
         case 49:
         case 51:
         case 53:
         case 55:
         case 57:
         case 59:
         case 61:
         case 63:
         default:
            variableList = "f\"(r_out_f12)";
            break;
      }
   }

   return variableList;
}

/**
 * @name writeLabel
 * 
 * @short Writes assembly label.
 * @param threadID 
 * @param nodeID 
 * @param fileName 
 * @return 
 */
inline void writeLabel(THREAD_ID threadID, UINT_32 nodeID, std::ofstream &outputFile)
{
   outputFile << "   __asm__ __volatile__ (\"" << "I" << threadID << "_" << nodeID << "_" << ":\");";
   outputFile << std::endl;
}

/**
 * @name startLockSection
 * 
 * @param lockID 
 * @param fileName 
 * @return 
 */
inline void startLockSection(UINT_32 lockID, std::ofstream &outputFile)
{
   outputFile << "   sesc_lock(&progLock" << lockID << ");\n";
   outputFile << std::endl;
}

/**
 * @name endLockSection
 * 
 * @param lockID 
 * @param fileName 
 * @return 
 */
inline void endLockSection(UINT_32 lockID, std::ofstream &outputFile)
{
   outputFile << "   sesc_unlock(&progLock" << lockID << ");\n";
   outputFile << std::endl;
}

/**
 * @name startTransSection
 * 
 * @param transID 
 * @param fileName 
 * @return 
 */
inline void startTransSection(UINT_32 blue, std::ofstream &outputFile)
{
int transID = 0;
   outputFile << "   BEGIN_TRANSACTION(" << std:: hex << transID << ");\n" << std::dec;
   outputFile << std::endl;
}

/**
 * @name endTransSection
 * 
 * @param transID 
 * @param fileName 
 * @return 
 */
inline void endTransSection(UINT_32 blue, std::ofstream &outputFile)
{
int transID = 0;
   outputFile << "   COMMIT_TRANSACTION(" << std:: hex << transID << ");\n" << std::dec;
   outputFile << std::endl;
}

/**
 * @name writeBarrier
 * 
 * @param barrID 
 * @param numProcs 
 * @return 
 */
inline void writeBarrier(string barrID, UINT_32 numProcs, std::ofstream &outputFile)
{
   outputFile << "   sesc_barrier(&" << barrID << ", " << numProcs << ");\n";
   outputFile << std::endl;
}

/**
 * @name writeWait
 * 
 * @return 
 */
inline void writeWait(std::ofstream &outputFile)
{
   outputFile << "   sesc_wait();\n";
   outputFile << std::endl;
}

/**
 * @name writeSpawn
 * 
 * @param threadID 
 * @param fileName 
 * @return 
   sesc_spawn(FUNCTION, NULL, 0);
 */
inline void writeSpawn(THREAD_ID threadID, std::ofstream &outputFile)
{
   outputFile << "   if(i == 0)\n";
   outputFile << "      sesc_spawn(";
   outputFile << "threadFunc" << threadID;
   outputFile << ", NULL, 0);\n";
   outputFile << std::endl;
}

/**
 * @name writeSpawn
 */
inline void writeSpawn(std::vector < UINT_32 > childThreads, std::ofstream &outputFile)
{
   outputFile << "   if(i == 0)\n";

   if(childThreads.size() > 1)
   {
      outputFile << "   {\n";
      for(std::vector < UINT_32 >::iterator thisIter = childThreads.begin(); thisIter != childThreads.end(); thisIter++)
      {
         outputFile << "      sesc_spawn(";
         outputFile << "threadFunc" << *thisIter;
         outputFile << ", NULL, 0);\n";
      }
      outputFile << "   }\n";
   }
   else
   {
      outputFile << "      sesc_spawn(";
      outputFile << "threadFunc" << childThreads.front();
      outputFile << ", NULL, 0);\n";
   }

   outputFile << std::endl;
}

/**
 * @name writeSpawn
 */
inline void writeSpawn(std::vector < UINT_32 > childThreads, std::ofstream &outputFile, UINT_32 diffThreads)
{
   if(childThreads.size() > 1)
   {
      graph_traits <BBGraph>::vertex_iterator vertexIterator, vertexEnd;

      outputFile << "   if(i == 0)\n";
      outputFile << "   {\n";
      for(std::vector < UINT_32 >::iterator thisIter = childThreads.begin(); thisIter != childThreads.end(); thisIter++)
      {
         if(num_vertices(*myCFG[*thisIter]) > 0)
         {
            outputFile << "      sesc_spawn(";

            basicBlock_name_map_t basicBlockLocal = get(basicBlock_t(), *myCFG[*thisIter]);
            tie(vertexIterator, vertexEnd) = vertices(*myCFG[*thisIter]);

            if(CodeGenerator::threadFuncMap[basicBlockLocal[*vertexIterator].return_bbAddress()] == *thisIter)
               outputFile << "threadFunc" << *thisIter;
            else
               outputFile << "threadFunc" << CodeGenerator::threadFuncMap[basicBlockLocal[*vertexIterator].return_bbAddress()];

            outputFile << ", NULL, 0);\n";
         }
      }
      outputFile << "   }\n";
   }
   else
   {
      if(num_vertices(*myCFG[childThreads.front()]) > 0)
      {
         outputFile << "   if(i == 0)\n";
         outputFile << "      sesc_spawn(";
         outputFile << "threadFunc" << childThreads.front();
         outputFile << ", NULL, 0);\n";
      }
   }

   outputFile << std::endl;
}

/**
 * @name startAccumulationLoop
 * 
 * @param threadID 
 * @param accumulation 
 * @param fileName 
 * @return 
 */
inline void startAccumulationLoop(UINT_32 threadID, UINT_32 accumulation, std::ofstream &outputFile)
{
   UINT_32 modAmount = (UINT_32)ACC_MAX / accumulation;

   if(modAmount > 1)
   {
      outputFile << "   if(i % ";
      outputFile << modAmount;
      outputFile << " == 0)\n";
   }

   outputFile << "   {\n";
   outputFile << std::endl;
}

/**
 * @name endAccumulationLoop
 * 
 * @param fileName 
 * @return 
 */
inline void endAccumulationLoop(std::ofstream &outputFile)
{
   outputFile << "   }\t\t//End If-LOOP\n";
   outputFile << std::endl;
}

/**
 * @name initializeLoop
 * 
 * @param threadID 
 * @param fileName 
 * @return 
 */
void initializeLoop(THREAD_ID threadID, std::ofstream &outputFile)
{
   outputFile << "   size_t i;\n";
   outputFile << "   for(i = 0; i < LOOP_" << IntToString(threadID) << "; i++)\n";
   outputFile << "   {\n";
}

/**
 * @name headerGen
 * 
 * @param fileName 
**/
void headerGen(std::ofstream &outputFile)
{
   UINT_32 maxInstructions = 0;
   UINT_32 numThreads = totalNumThreads;
   UINT_32 memSize = (STREAM_SIZE * MEM_MULTIPLIER);

   outputFile << "//  This file was automatically generated.\n//  Do not edit this file.\n//\n//\n";
   outputFile << "/// @file synthetic.c\n";
   outputFile << "/// @author Clay Hughes\n";
   outputFile << "//\n// Copyright: See COPYING file that comes with this distribution\n//\n///////////////////////////////////////////////////////////////////////////////////\n";

   outputFile << "#include <stdint.h>\n";
   outputFile << "#include <stdlib.h>\n";
   outputFile << "#include \"sescapi.h\"\n\n";

   for(size_t counter = 0; counter < totalNumThreads; counter++)
   {
      UINT_64 tempVar = numInstructions[counter];
      maxInstructions = max((UINT_64)maxInstructions, tempVar);
   }

   for(UINT_32 threadCounter = 0; threadCounter < numThreads; threadCounter++)
   {
      outputFile << "#define LOOP_" << IntToString(threadCounter) << " ";
      if(numInstructions[threadCounter] == maxInstructions)
         outputFile << LOOP_SIZE;
      else
         outputFile << ceil(LOOP_SIZE * ((float)(numInstructions[threadCounter] / (float)maxInstructions)));
      outputFile << "\n";
   }
   outputFile << "\n";

//    for(map <ADDRESS_INT, string>::iterator globalMutexMapIterator = globalMutexMap.begin(); globalMutexMapIterator != globalMutexMap.end(); globalMutexMapIterator++)
//    {
//       outputFile << "pthread_mutex_t " << globalMutexMapIterator->second << " = PTHREAD_MUTEX_INITIALIZER;\n";
//    }	

   outputFile << "#define TOGGLE_ON(n) do {\t\t\t\\\n";
   outputFile << "        __asm__ __volatile__ (\".word 0x74000000+\" #n);\t\\\n";
   outputFile << "} while (0)\n\n";

   outputFile << "#define BEGIN_TRANSACTION(n) do {\t\t\t\\\n";
   outputFile << "        __asm__ __volatile__ (\".word 0x70000000+\" #n);\t\\\n";
   outputFile << "} while (0)\n\n";

   outputFile << "#define COMMIT_TRANSACTION(n) do {\t\t\t\\\n";
   outputFile << "        __asm__ __volatile__ (\".word 0x7C000000+\" #n);\t\\\n";
   outputFile << "} while (0)\n\n";

   //Declare the thread functions
   for(UINT_32 threadCounter = 1; threadCounter < numThreads; threadCounter++)
   {
      outputFile << "void threadFunc" << IntToString(threadCounter) << "(void *ptr);\n";
   }

   //Declare mutex & synch variables
   outputFile << "\n";
   outputFile << "sbarrier_t progBarr;\n";
   for(MutexMap::iterator globalMutexMapIterator = globalMutexMap.begin(); globalMutexMapIterator != globalMutexMap.end(); globalMutexMapIterator++)
   {
      outputFile << "slock_t " << "progLock" << globalMutexMapIterator->second << ";\n";
   }

   outputFile << "\n/* Initialize Shared Memory Region */\n";
   outputFile << "int* shared_memInt;\n";
   outputFile << "register int s_data_out_int asm(\"22\");\n";
   outputFile << "float* shared_memFloat;\n";
   outputFile << "register int s_data_out_float asm(\"23\");\n";

   outputFile << "int s_data_out_int_base;\n";
   outputFile << "int s_data_out_float_base;\n";

   outputFile << "\n\nint main()\n{\n";							// --- main

   outputFile << "   register int r_out_t0 asm(\"8\");\n";
   outputFile << "   register int r_out_t1 asm(\"9\");\n";
   outputFile << "   register int r_out_t2 asm(\"10\");\n";
   outputFile << "   register int r_out_t3 asm(\"11\");\n";
   outputFile << "   register int r_out_t4 asm(\"12\");\n";
   outputFile << "   register int r_out_t5 asm(\"13\");\n";

   outputFile << "   register int r_out_s0 asm(\"16\");\n";
   outputFile << "   register int r_out_s1 asm(\"17\");\n";
   outputFile << "   register int r_out_s2 asm(\"18\");\n";
   outputFile << "   register int r_out_s3 asm(\"19\");\n";
   outputFile << "   register int r_out_s4 asm(\"20\");\n";
   outputFile << "   register int r_out_s5 asm(\"21\");\n";

   outputFile << "   register int r_out_f2 asm(\"$f2\");\n";
   outputFile << "   register int r_out_f4 asm(\"$f4\");\n";
   outputFile << "   register int r_out_f6 asm(\"$f6\");\n";
   outputFile << "   register int r_out_f8 asm(\"$f8\");\n";
   outputFile << "   register int r_out_f10 asm(\"$f10\");\n";
   outputFile << "   register int r_out_f12 asm(\"$f12\");\n";

   outputFile << "\n   register int data_out_int asm(\"20\");\n";
   outputFile << "   register int data_out_float asm(\"21\");\n";

   outputFile << "\n   /* Initialize Private Memory Region */\n";
   outputFile << "   int* memInt;\n";
   outputFile << "   float* memFloat;\n";

   outputFile << "\n";
   outputFile << "   memInt           = (int*)malloc(sizeof(int) * " << memSize << ");\n";
   outputFile << "   data_out_int     = (int)&(memInt[0]);\n";
   outputFile << "   memFloat         = (float*)malloc(sizeof(float) * " << memSize << ");\n";
   outputFile << "   data_out_float   = (int)&(memFloat[0]);\n";

   outputFile << "\n";
   outputFile << "   shared_memInt    = (int*)malloc(sizeof(int) * " << memSize << ");\n";
   outputFile << "   s_data_out_int   = (int)&(shared_memInt[0]);\n";
   outputFile << "   shared_memFloat  = (float*)malloc(sizeof(float) * " << memSize << ");\n";
   outputFile << "   s_data_out_float = (int)&(shared_memFloat[0]);\n";

   outputFile << "\n";
   outputFile << "   s_data_out_int_base = s_data_out_int;\n";
   outputFile << "   s_data_out_float_base = s_data_out_float;\n";
   outputFile << "   int data_out_int_base = data_out_int;\n";
   outputFile << "   int data_out_float_base = data_out_float;\n";

   outputFile << "\n   TOGGLE_ON(1);\n";
   outputFile << std::endl;

   initializeLoop(0, outputFile);
}


/**
 * @name trailerGen
 * 
 * @param threadID 
 * @param fileName 
 * @return 
 */
void trailerGen(CodeLogic *syntheticCodeBlock, THREAD_ID threadID, std::ofstream &outputFile)
{
   //Accumulated basic blocks don't always terminate properly
   //so we need to check to see if there is an open Tx.
   if(syntheticCodeBlock->lastTrans[threadID] > 0)
   {
      endTransSection(0, outputFile);
      syntheticCodeBlock->lastTrans[threadID] = 0;
   }
   if(syntheticCodeBlock->accumulatedValue[threadID] > 0)
   {
      endAccumulationLoop(outputFile);
      syntheticCodeBlock->accumulatedValue[threadID] = 0;
   }

   outputFile << "\n";
//   outputFile << "}\n";

//    outputFile << "for(i=0; i < MAX_NUM_THREADS; i++)\n";
//    outputFile << "{\n";
//    outputFile << "   pthread_join(thread[i], NULL);\n";
//    outputFile << "}\n";
   if(syntheticCodeBlock->lastLock[threadID] > 0)
   {
      endLockSection(globalMutexMap[syntheticCodeBlock->lastLock[threadID]], outputFile);
      syntheticCodeBlock->lastLock[threadID] = 0;
   }

   outputFile << "\n   }\n";
   outputFile << "   TOGGLE_ON(1);\n";

   outputFile << "\n   return 0;\n}\n";						// --- end main
}

/**
 * @name funcHeaderGen
 * 
 * @param fileName 
 */
void funcHeaderGen(THREAD_ID threadID, std::ofstream &outputFile)
{
   UINT_32 memSize = (STREAM_SIZE * MEM_MULTIPLIER);
   UINT_32 numThreads = totalNumThreads;

   outputFile << "\n\nvoid threadFunc" << IntToString(threadID) << "(void *ptr)\n";
   outputFile << "{\n";

   outputFile << "   register int r_out_t0 asm(\"8\");\n";
   outputFile << "   register int r_out_t1 asm(\"9\");\n";
   outputFile << "   register int r_out_t2 asm(\"10\");\n";
   outputFile << "   register int r_out_t3 asm(\"11\");\n";
   outputFile << "   register int r_out_t4 asm(\"12\");\n";
   outputFile << "   register int r_out_t5 asm(\"13\");\n";

   outputFile << "   register int r_out_s0 asm(\"16\");\n";
   outputFile << "   register int r_out_s1 asm(\"17\");\n";
   outputFile << "   register int r_out_s2 asm(\"18\");\n";
   outputFile << "   register int r_out_s3 asm(\"19\");\n";
   outputFile << "   register int r_out_s4 asm(\"20\");\n";
   outputFile << "   register int r_out_s5 asm(\"21\");\n";

   outputFile << "   register int r_out_f2 asm(\"$f2\");\n";
   outputFile << "   register int r_out_f4 asm(\"$f4\");\n";
   outputFile << "   register int r_out_f6 asm(\"$f6\");\n";
   outputFile << "   register int r_out_f8 asm(\"$f8\");\n";
   outputFile << "   register int r_out_f10 asm(\"$f10\");\n";
   outputFile << "   register int r_out_f12 asm(\"$f12\");\n";

   outputFile << "\n   register int data_out_int asm(\"20\");\n";
   outputFile << "   register int data_out_float asm(\"21\");\n";

   outputFile << "\n   /* Initialize Private Memory Region */\n";
   outputFile << "   int* memInt;\n";
   outputFile << "   float* memFloat;\n";

   outputFile << "\n";
   outputFile << "   memInt         = (int*)malloc(sizeof(int) * " << memSize << ");\n";
   outputFile << "   data_out_int   = (int)&(memInt[0]);\n";
   outputFile << "   memFloat       = (float*)malloc(sizeof(float) * " << memSize << ");\n";
   outputFile << "   data_out_float = (int)&(memFloat[0]);\n";

   outputFile << "\n";
   outputFile << "   int data_out_int_base = data_out_int;\n";
   outputFile << "   int data_out_float_base = data_out_float;\n";

   outputFile << std::endl;

   initializeLoop(threadID, outputFile);
}

/**
 * @name funcTrailerGen
 * 
 * @param threadID 
 * @param fileName 
 * @return 
 */
void funcTrailerGen(CodeLogic *syntheticCodeBlock, THREAD_ID threadID, std::ofstream &outputFile)
{
   //Accumulated basic blocks don't always terminate properly
   //so we need to check to see if there is an open Tx.
   if(syntheticCodeBlock->lastTrans[threadID] > 0)
   {
      endTransSection(0, outputFile);
      syntheticCodeBlock->lastTrans[threadID] = 0;
   }
   if(syntheticCodeBlock->accumulatedValue[threadID] > 0)
   {
      endAccumulationLoop(outputFile);
      syntheticCodeBlock->accumulatedValue[threadID] = 0;
   }

   if(syntheticCodeBlock->lastLock[threadID] > 0)
   {
      endLockSection(globalMutexMap[syntheticCodeBlock->lastLock[threadID]], outputFile);
      syntheticCodeBlock->lastLock[threadID] = 0;
   }

   outputFile << "\n   }\n";
   outputFile << "}\n";
}

/**
 * @name instructionGenerator
 * 
 * @param numInstructions 
 * @param instructionMix 
 * @return 
 */
BasicBlock instructionGenerator(float numInstructions, InstructionMix instructionMix)
{
   BasicBlock tempBB;
   float totalInstructions = 0;
   UINT_32 robin = 0;
   UINT_32 insType;

//    instructionMix.print_mixBins(std::cout);

   tempBB.instructionMix = instructionMix;
   tempBB.instructionMix.normalize();

   return tempBB;
}

}  //end CodeGenerator
