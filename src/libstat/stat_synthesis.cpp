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

#include "stat_synthesis.h"

//NOTE woo
AddressMap                    uniqueBBMap;
UINT_32                       totalNumThreads = 0;
UINT_64                       numBasicBlocks = 0;                          //can't tally BB except at global
std::vector < UINT_64 >       numInstructions (MAX_NUM_THREADS, 0);        //can't tally INS except at global
std::vector < BasicBlock* >   currBB;
std::vector < BasicBlock* >   prevBB;
std::vector < InstructionMix >programInstructionMix;                       //

//NOTE memory
AddressMap sharedAddressMap;
UINT_32 baseOffset;
std::vector < std::vector < UINT_32 > > per_threadReadBins;                   //per-thread histogram of next memory acceess 0/8/16/32/64/n
std::vector < std::vector < UINT_32 > > per_threadWriteBins;                  //per-thread histogram of next memory acceess 0/8/16/32/64/n
std::vector < ADDRESS_INT > lastReadAddress (MAX_NUM_THREADS, 0);             //the last read effective address for each thread
std::vector < ADDRESS_INT > lastWriteAddress (MAX_NUM_THREADS, 0);            //the last write effective address for each thread

//NOTE SFG
std::deque  < BBGraph * > myCFG;                                  //SFG graph container, type BBGraph
std::vector < BBVertex >  myCFG_VertexA;                          //used to ID the vertices
std::vector < BBVertex >  myCFG_VertexB;                          //used to ID the vertices
std::vector < BBVertexMap > vertexMap;                            //map container, type BBVertexMap

//NOTE PCFG
PCFG myPCFG;                                                      //PCFG graph container, type PCFG
FlowVertex myPCFG_VertexA;                                        //used to ID the vertices
std::vector < FlowVertex > lastInsertedNode;                      //used to ID the vertices

//NOTE synch
MutexMap globalMutexMap;

//NOTE 
BOOL                          profilingEnabled = 0;
UINT_32                       numLocks = 0;
std::vector < BOOL >          updateGraph (MAX_NUM_THREADS, 0);
std::vector < BOOL >          ignoreInstructions (MAX_NUM_THREADS, 0);
std::vector < UINT_32 >       lockToggle (MAX_NUM_THREADS, 0);
std::vector < ADDRESS_INT >   transactionID (MAX_NUM_THREADS, 0);

extern std::vector< std::deque< tuple<DInst, Time_t> > * > instructionQueueVector;

namespace GraphManipulation
{
extern std::vector < FlowVertex > lastInsertedNode;
}

/**
 * @name IntToString 
 * 
 * @short This function converts an int to a string (dec)
 * @param input 
 * @return integer converted to string (dec)
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
 * @short This function converts an int to a string (hex)
 * @param input 
 * @return integer converted to string (hex)
 */
std::string HexToString(INT_64 input)
{
  std::ostringstream output;
  output << std::hex << input;

  return output.str();
}

namespace Synthesis
{
StatPaths statPaths;
std::vector < BOOL > isTransaction  (MAX_NUM_THREADS, 0);
std::vector < std::list < BasicBlock > * >  transBuffer;

std::vector < BOOL > wasCommitted (MAX_NUM_THREADS, 0);
std::vector < BOOL > wasSpawn (MAX_NUM_THREADS, 0);
std::vector < FlowNode * > currentFlowNode;

ADDRESS_INT AddressOfSpawn;

/**
 * @name init
 * 
 * @short This function is called before the simulation starts
 */
void init(void)
{
   UINT_32 startingGraphs = 2;
   Synthesis::statPaths.set_outputFileName(OSSim::getBenchName());

   for(UINT_32 counter = 0; counter < startingGraphs; counter++)
   {
      myCFG.push_back(new BBGraph());
   }

   baseOffset = (STREAM_SIZE * MEM_MULTIPLIER) >> 1;
}

/**
 * @name cleanup
 *
 * @short This function is called when the simulation ends
 */
void cleanup(void)
{
   std::cout << "\nCleaning" << flush;
   for(UINT_32 counter = 0; counter < myCFG.size(); counter++)
      delete myCFG[counter];

   std::cout << "...Finished" << flush;
}

/**
 * @name instructionCounts
 *
 * @short 
 */
void instructionCounts(void)
{
   UINT_32 totalDynamicInstructions = 0;

   for(size_t counter = 0; counter < totalNumThreads; counter++)
   {
      totalDynamicInstructions = totalDynamicInstructions + numInstructions[counter];
   }

   std::cout << "Number of instructions:  " << totalDynamicInstructions << "\n";
   //since we use the zero index for the total count, loop must be <=
   for(size_t counter = 0; counter < totalNumThreads; counter++)
   {
      std::cout << std::right << std::setw(10) << "Thread " << std::left << counter << ":  " << numInstructions[counter] << "\n";
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
   bool skip = 0;
   ConfObject *statConf = new ConfObject;
   DInst tempDinst = tempTuple.get<0>();
   THREAD_ID threadID = tempDinst.get_threadID();

   if(tempDinst.transType == transAbort)
   {
      skip = 1;                                   //we want to skip the very first instruction
      profilingEnabled = 1^profilingEnabled;      //xor toggles profiling on and off between 'abort' instructions
      ADDRESS_INT bbAddress = (ADDRESS_INT)tempDinst.get_instructionAddress();

      if(statConf->return_debugAll() == 1)
      {
         std::cout << "BOO  " << tempDinst.transTid << "   " << std::hex << bbAddress << "    " << profilingEnabled << std::endl;
      }
   }

   //Write instruction to temporary list or commit basic block
   if((skip == 0 && profilingEnabled == 1) || statConf->return_synthOverride() == 1)
   {
      currBB[threadID]->update_threadID(threadID);
      currBB[threadID]->update_instructionList(tempDinst);

      if(ignoreInstructions[threadID] == 1 && tempDinst.get_subCode() == iMemFence)
      {
         ignoreInstructions[threadID] = 0;

         delete currBB[threadID];
         currBB[threadID] = new BasicBlock();
      }

      //Check if this is within a lock section
      if(tempDinst.get_isCriticalStart() == 1)
      {
         lockToggle[threadID] = tempDinst.get_lockID();
         ignoreInstructions[threadID] = 1;
      }
      else if(tempDinst.get_isCriticalEnd() == 1)
      {
         lockToggle[threadID] = 0;
         currBB[threadID]->update_isCritical(0);
      }

      if(lockToggle[threadID] != 0)
      {
         currBB[threadID]->update_lockID(lockToggle[threadID]);
         currBB[threadID]->update_isCritical(1);

         if(tempDinst.get_lockID() != 0 && globalMutexMap.find(currBB[threadID]->return_lockID()) == globalMutexMap.end())
         {
            globalMutexMap[currBB[threadID]->return_lockID()] = numLocks;
            numLocks = numLocks + 1;
         }
      }

      if(wasCommitted[threadID] == 1)
      {
         wasCommitted[threadID] = 0;
         currentFlowNode[threadID]->update_startPC((ADDRESS_INT)tempDinst.get_instructionAddress());
      }

      //Check if this is a spawn point
      if(tempDinst.get_isSpawn() == 1)
      {
         currBB[threadID]->update_isSpawn(1);
         currBB[threadID]->update_targetThread(tempDinst.get_targetThread());
      }
      else if(tempDinst.get_isWait() == 1)                                                                                 //WAIT
      {
         currBB[threadID]->update_isWait(1);

         if(Synthesis::isTransaction[threadID] == 1)
         {
            std::list < BasicBlock >::iterator transBufferIterator;
            for(transBufferIterator = Synthesis::transBuffer[threadID]->begin(); transBufferIterator != Synthesis::transBuffer[threadID]->end(); transBufferIterator++)
            {
               if((ADDRESS_INT)(*transBufferIterator).return_front_of_instructionList().return_instructionID() != 0)
               {
                  numInstructions[threadID] = numInstructions[threadID] + (*transBufferIterator).return_instructionListSize();
                  currentFlowNode[threadID]->incrementNumInstructions((*transBufferIterator).return_instructionListSize());
                  GraphManipulation::updateGraph(&(*transBufferIterator), threadID);
               }
            }

            if((ADDRESS_INT)currBB[threadID]->return_front_of_instructionList().return_instructionID() != 0)
            {
               numInstructions[threadID] = numInstructions[threadID] + currBB[threadID]->return_instructionListSize();
               currentFlowNode[threadID]->incrementNumInstructions(currBB[threadID]->return_instructionListSize());
               GraphManipulation::updateGraph(currBB[threadID], threadID);
               programInstructionMix[threadID].update(currBB[threadID]->return_instructionListRef(), 1);
            }
         }
         else
         {
            if((ADDRESS_INT)prevBB[threadID]->return_front_of_instructionList().return_instructionID() != 0)
            {
               numInstructions[threadID] = numInstructions[threadID] + prevBB[threadID]->return_instructionListSize();
               currentFlowNode[threadID]->incrementNumInstructions(prevBB[threadID]->return_instructionListSize());
               GraphManipulation::updateGraph(prevBB[threadID], threadID);
               programInstructionMix[threadID].update(prevBB[threadID]->return_instructionListRef(), 1);
            }
         }

         //BEGIN PCFG COMMIT-----------------------------------------------------------------------------------------------
         if(currentFlowNode[threadID]->return_numInstructions() > 0)
         {
            wasCommitted[threadID] = 1;
            GraphManipulation::addPCFGNode(*currentFlowNode[threadID]);

            delete currentFlowNode[threadID];
            currentFlowNode[threadID] = new FlowNode(0, threadID, 0, currBB[threadID]->return_targetThread(), 0, transactionID[threadID], 0, 0, 0, 1, 0);
         }
         else if(currentFlowNode[threadID]->return_isTrans() == 1 || currentFlowNode[threadID]->return_isBarrier() == 1 || currentFlowNode[threadID]->return_isSpawn() == 1)
         {
            wasCommitted[threadID] = 1;
            GraphManipulation::addPCFGNode(*currentFlowNode[threadID]);

            delete currentFlowNode[threadID];
            currentFlowNode[threadID] = new FlowNode(0, threadID, 0, currBB[threadID]->return_targetThread(), 0, transactionID[threadID], 0, 0, 0, 1, 0);
         }
         else
         {
            currentFlowNode[threadID]->update_isWait(1);
         }
         //END   PCFG COMMIT-----------------------------------------------------------------------------------------------
      }
      else if(tempDinst.get_isBarrier() == 1)                                                                                 //BARRIER
      {
         currBB[threadID]->update_isBarrier(1);

         if(Synthesis::isTransaction[threadID] == 1)
         {
            std::list < BasicBlock >::iterator transBufferIterator;
            for(transBufferIterator = Synthesis::transBuffer[threadID]->begin(); transBufferIterator != Synthesis::transBuffer[threadID]->end(); transBufferIterator++)
            {
               if((ADDRESS_INT)(*transBufferIterator).return_front_of_instructionList().return_instructionID() != 0)
               {
                  numInstructions[threadID] = numInstructions[threadID] + (*transBufferIterator).return_instructionListSize();
                  currentFlowNode[threadID]->incrementNumInstructions((*transBufferIterator).return_instructionListSize());
                  GraphManipulation::updateGraph(&(*transBufferIterator), threadID);
               }
            }

            if((ADDRESS_INT)currBB[threadID]->return_front_of_instructionList().return_instructionID() != 0)
            {
               numInstructions[threadID] = numInstructions[threadID] + currBB[threadID]->return_instructionListSize();
               currentFlowNode[threadID]->incrementNumInstructions(currBB[threadID]->return_instructionListSize());
               GraphManipulation::updateGraph(currBB[threadID], threadID);
               programInstructionMix[threadID].update(currBB[threadID]->return_instructionListRef(), 1);
            }
         }
         else
         {
            if((ADDRESS_INT)prevBB[threadID]->return_front_of_instructionList().return_instructionID() != 0)
            {
               numInstructions[threadID] = numInstructions[threadID] + prevBB[threadID]->return_instructionListSize();
               currentFlowNode[threadID]->incrementNumInstructions(prevBB[threadID]->return_instructionListSize());
               GraphManipulation::updateGraph(prevBB[threadID], threadID);
               programInstructionMix[threadID].update(prevBB[threadID]->return_instructionListRef(), 1);
            }
         }

         //BEGIN PCFG COMMIT-----------------------------------------------------------------------------------------------
         if(currentFlowNode[threadID]->return_numInstructions() > 0)
         {
            wasCommitted[threadID] = 1;
            GraphManipulation::addPCFGNode(*currentFlowNode[threadID]);

            delete currentFlowNode[threadID];
            currentFlowNode[threadID] = new FlowNode(0, threadID, 0, currBB[threadID]->return_targetThread(), 0, transactionID[threadID], 0, 0, 0, 0, 1);
         }
         else if(currentFlowNode[threadID]->return_isTrans() == 1 || currentFlowNode[threadID]->return_isWait() == 1 || currentFlowNode[threadID]->return_isSpawn() == 1)
         {
            wasCommitted[threadID] = 1;
            GraphManipulation::addPCFGNode(*currentFlowNode[threadID]);

            delete currentFlowNode[threadID];
            currentFlowNode[threadID] = new FlowNode(0, threadID, 0, currBB[threadID]->return_targetThread(), 0, transactionID[threadID], 0, 0, 0, 0, 1);
         }
         else
         {
            currentFlowNode[threadID]->update_isBarrier(1);
         }
         //END   PCFG COMMIT-----------------------------------------------------------------------------------------------
      }

      //Memory operation?
      if(tempDinst.get_subCode() == iMemory)
      {
         if(statConf->return_debugAll() == 1)
            cout << "Memory Op-- " << std::hex << currBB[threadID]->return_back_of_instructionList().return_virtualAddress() << "\n";

         if(tempDinst.getOpcode() == iLoad)
            StatMemory::recordMemWrite(tempDinst.getVaddr(), tempDinst.get_dataSize(), (ADDRESS_INT)currBB[threadID]->return_front_of_instructionList().return_instructionID(), threadID);
         else if(tempDinst.getOpcode() == iStore)
            StatMemory::recordMemRead(tempDinst.getVaddr(), tempDinst.get_dataSize(), (ADDRESS_INT)currBB[threadID]->return_front_of_instructionList().return_instructionID(), threadID);
      }

      if(statConf->return_debugAll() == 1)
      {
         cout << std::hex << (ADDRESS_INT)tempDinst.get_instructionAddress() << "::" << Instruction::opcode2Name(tempDinst.getOpcode()) << "-" << Instruction::subCode_to_Name(tempDinst.get_subCode());
         cout << "\n";
      }

      ///Begin Transaction
      //If this is the beginning of a transaction, we want to start a new basic block
      if(tempDinst.getTmcode() == transBegin && Synthesis::isTransaction[threadID] == 0 && tempDinst.get_transBCFlag() != 2)
      {
         //add the previous basic block to the graph
         if((ADDRESS_INT)prevBB[threadID]->return_front_of_instructionList().return_instructionID() != 0)
         {
            numInstructions[threadID] = numInstructions[threadID] + prevBB[threadID]->return_instructionListSize();
            currentFlowNode[threadID]->incrementNumInstructions(prevBB[threadID]->return_instructionListSize());
            GraphManipulation::updateGraph(prevBB[threadID], threadID);
            programInstructionMix[threadID].update(prevBB[threadID]->return_instructionListRef(), 1);
         }

         if((ADDRESS_INT)currBB[threadID]->return_front_of_instructionList().return_instructionID() != 0)
         {
            numInstructions[threadID] = numInstructions[threadID] + currBB[threadID]->return_instructionListSize();
            currentFlowNode[threadID]->incrementNumInstructions(currBB[threadID]->return_instructionListSize());
         }

         transactionID[threadID] = (ADDRESS_INT)tempDinst.get_instructionAddress();

         //BEGIN PCFG COMMIT-----------------------------------------------------------------------------------------------
         if(currentFlowNode[threadID]->return_numInstructions() > 0)
         {
            wasCommitted[threadID] = 1;
            GraphManipulation::addPCFGNode(*currentFlowNode[threadID]);

            delete currentFlowNode[threadID];
            currentFlowNode[threadID] = new FlowNode(0, threadID, 0, currBB[threadID]->return_targetThread(), 0, transactionID[threadID], 0, 1, 0, 0, 0);
         }
         else
         {
            currentFlowNode[threadID]->update_isTrans(1);
            currentFlowNode[threadID]->update_transID(transactionID[threadID]);

            currentFlowNode[threadID]->instructionMix.reset();
         }
         //END   PCFG COMMIT-----------------------------------------------------------------------------------------------

         delete prevBB[threadID];
         prevBB[threadID] = new BasicBlock(*currBB[threadID]);

         delete currBB[threadID];
         currBB[threadID] = new BasicBlock();

         Synthesis::isTransaction[threadID] = 1;
         currBB[threadID]->update_isTrans(1);
      }

      ///Abort Transaction
      //Check to see if this transaction is the restart of an aborted transaction
      if(Synthesis::isTransaction[threadID] == 1 && tempDinst.getTmcode() == transBegin && tempDinst.get_transBCFlag() == 1)
      {
         //clear the previous transaction
         delete Synthesis::transBuffer[threadID];
         Synthesis::transBuffer[threadID] = new std::list < BasicBlock >;

         transactionID[threadID] = (ADDRESS_INT)tempDinst.get_instructionAddress();

         //BEGIN PCFG COMMIT-----------------------------------------------------------------------------------------------
         BOOL isFirstNode;
         if(currentFlowNode[threadID]->return_isFirst() == 1)
            isFirstNode = 1;
         else
            isFirstNode = 0;

         wasCommitted[threadID] = 1;

         delete currentFlowNode[threadID];
         currentFlowNode[threadID] = new FlowNode(isFirstNode, threadID, 0, currBB[threadID]->return_targetThread(), 0, transactionID[threadID], 0, 1, 0, 0, 0);
         currentFlowNode[threadID]->instructionMix.reset();
         //END   PCFG COMMIT-----------------------------------------------------------------------------------------------

         delete currBB[threadID];
         currBB[threadID] = new BasicBlock();

         Synthesis::isTransaction[threadID] = 1;
         currBB[threadID]->update_isTrans(1);
      }

      /// FIXME -- There is still a problem with removing unwated basic blocks, such as those on the create() boundry.
      if(tempDinst.getOpcode() == iBJ)
      {
         //Need to ensure that the vector is large enough to hold the next thread
         if(threadID >= updateGraph.size())
         {
            if(threadID == updateGraph.size())
            {
               std::cerr << "Synthesis::updateGraph.push_back with " << threadID;
               updateGraph.push_back(0);
               std::cerr << " and new size of " << updateGraph.size() << std::endl;
            }
            else
            {
               std::cerr << "Synthesis::updateGraph.resize with " << threadID;
               updateGraph.resize(threadID + 1, 0);
               std::cerr << " and new size of " << updateGraph.size() << std::endl;
            }
         }

         //Check to see if we're in a transaction
         if(Synthesis::isTransaction[threadID] == 1)
         {
            currBB[threadID]->update_isTrans(1);
            currBB[threadID]->update_transID(transactionID[threadID]);

            currentFlowNode[threadID]->instructionMix.update(currBB[threadID]->return_instructionList());
         }

         //Need to check to see if the program is inside of a critical section. If it is, we need to
         //remove some basic blocks and some extra instrucionts.
         if(updateGraph[threadID] == 1)
         {
            if(currBB[threadID]->return_isCritical() == 1 && prevBB[threadID]->return_isCritical() == 0)
            {
               std::list <InstructionContainer>	tempInstructionList = prevBB[threadID]->return_instructionList();
               std::list <InstructionContainer>::iterator instructionListIterator;
               for(instructionListIterator = tempInstructionList.begin(); instructionListIterator != tempInstructionList.end(); instructionListIterator++)
               {
                  if(instructionListIterator->return_subCode() == iFetchOp)
                  {
                     prevBB[threadID]->clear_instructionList();
                     break;
                  }
               }
            }
            else if(currBB[threadID]->return_isCritical() == 0 && prevBB[threadID]->return_isCritical() == 1)
            {
               //there are remnants of this block left in the previous block which need to be cleaned
               std::list <InstructionContainer>::iterator instructionListIterator;
               std::list <InstructionContainer>	tempInstructionList = prevBB[threadID]->return_instructionList();
               for(instructionListIterator = tempInstructionList.begin(); instructionListIterator != tempInstructionList.end(); instructionListIterator++)
               {
                  if(instructionListIterator->return_subCode() == BJCall)
                  {
                     prevBB[threadID]->erase_instructionList(tempInstructionList.size() - 3, tempInstructionList.size() - 1);
                  }
               }

               tempInstructionList = currBB[threadID]->return_instructionList();
               for(instructionListIterator = tempInstructionList.begin(); instructionListIterator != tempInstructionList.end(); instructionListIterator++)
               {
                  if(instructionListIterator->return_subCode() == iRelease)
                  {
                     currBB[threadID]->clear_instructionList();
                     break;
                  }
               }
            }

            //BEGIN PCFG COMMIT----------------------------------------------------------------------------------------------------------------------------
            //NOTE spawn
            //If this basic block is marked as a spawn, push the previous node to the PCFG.
            //If this is a loop spawns, just add the child thread the child list.
            //If we're finished, push the spawn node to the PCFG and begin a new one.
            if(currBB[threadID]->return_isSpawn() == 1)
            {
               wasSpawn[threadID] = 1;
               AddressOfSpawn = (ADDRESS_INT)currBB[threadID]->return_front_of_instructionList().return_instructionID();

               //If the current node is already marked as a spawn point, we don't want to start
               //a new node, instead we just want to update the contents of childThreads
               if(currentFlowNode[threadID]->return_isSpawn() == 1)
               {
                  currentFlowNode[threadID]->childThreads.push_back(currBB[threadID]->return_targetThread());
               }
               else
               {
                  if(currentFlowNode[threadID]->return_numInstructions() > 0)
                  {
                     wasCommitted[threadID] = 1;
                     GraphManipulation::addPCFGNode(*currentFlowNode[threadID]);

                     delete currentFlowNode[threadID];
                     currentFlowNode[threadID] = new FlowNode(0, threadID, 0, currBB[threadID]->return_targetThread(), 0, 0, 1, 0, 0, 0, 0);
                  }
                  else
                  {
                     currentFlowNode[threadID]->update_isSpawn(1);
                     currentFlowNode[threadID]->childThreads.push_back(currBB[threadID]->return_targetThread());
                  }
               }
            }
            else if(wasSpawn[threadID] == 1)                            //there are two BBs between spawn nodes that need to be skipped for the PCFG
            {
               if((ADDRESS_INT)prevBB[threadID]->return_front_of_instructionList().return_instructionID() != AddressOfSpawn)
                  wasSpawn[threadID] = 0;
            }
            else if(currentFlowNode[threadID]->return_isSpawn() == 1)         //this is the end of the spawning node so we need to add it to the PCFG
            {
               if(currentFlowNode[threadID]->return_numInstructions() > 0)
               {
                  wasCommitted[threadID] = 1;
                  GraphManipulation::addPCFGNode(*currentFlowNode[threadID]);

                  delete currentFlowNode[threadID];
                  currentFlowNode[threadID] = new FlowNode(0, threadID, 0, 0, 0, 0, 0, 0, 0, 0, 0);
               }
            }

            //NOTE critial section
            //If this is a cricital section and the flowNode is not marked then it is the first block of a critical section.
            //If we're already in a critical section, do nothing.
            //If this is the end of a critical section, push the node to the PCFG and begin a new one.
            if(lockToggle[threadID] != 0 && currentFlowNode[threadID]->return_isCritical() == 0)
            {
               if(currentFlowNode[threadID]->return_numInstructions() > 0)
               {
                  wasCommitted[threadID] = 1;
                  GraphManipulation::addPCFGNode(*currentFlowNode[threadID]);

                  delete currentFlowNode[threadID];
                  currentFlowNode[threadID] = new FlowNode(0, threadID, 0, currBB[threadID]->return_targetThread(), lockToggle[threadID], 0, 0, 0, 1, 0, 0);
               }
               else
               {
                     currentFlowNode[threadID]->update_isCritical(1);
                     currentFlowNode[threadID]->update_lockID(lockToggle[threadID]);
               }
            }
            else if(lockToggle[threadID] != 0 && currentFlowNode[threadID]->return_isCritical() == 1)
            {
            }
            else if(lockToggle[threadID] == 0 && currentFlowNode[threadID]->return_isCritical() == 1)
            {
               if(currentFlowNode[threadID]->return_numInstructions() > 0)
               {
                  wasCommitted[threadID] = 1;
                  GraphManipulation::addPCFGNode(*currentFlowNode[threadID]);

                  delete currentFlowNode[threadID];
                  currentFlowNode[threadID] = new FlowNode(0, threadID, 0, currBB[threadID]->return_targetThread(), 0, 0, 0, 0, 0, 0, 0);
               }
            }

            //END PCFG COMMIT----------------------------------------------------------------------------------------------------------------------------

            //If the program is inside of a transaction, buffer the BB otherwise add it to the graph
            if(Synthesis::isTransaction[threadID] == 1)
            {
               if((ADDRESS_INT)prevBB[threadID]->return_front_of_instructionList().return_instructionID() != 0)
                  Synthesis::transBuffer[threadID]->push_back(*prevBB[threadID]);
            }
            else
            {
               if((ADDRESS_INT)prevBB[threadID]->return_front_of_instructionList().return_instructionID() != 0)
               {
                  numInstructions[threadID] = numInstructions[threadID] + prevBB[threadID]->return_instructionListSize();
                  currentFlowNode[threadID]->incrementNumInstructions(prevBB[threadID]->return_instructionListSize());
                  GraphManipulation::updateGraph(prevBB[threadID], threadID);
                  programInstructionMix[threadID].update(prevBB[threadID]->return_instructionListRef(), 1);
               }
            }

            delete prevBB[threadID];
            prevBB[threadID] = new BasicBlock(*currBB[threadID]);

            delete currBB[threadID];
            currBB[threadID] = new BasicBlock();
         }
         else
         {
            updateGraph[threadID] = 1;

            delete prevBB[threadID];
            prevBB[threadID] = new BasicBlock(*currBB[threadID]);

            delete currBB[threadID];
            currBB[threadID] = new BasicBlock();
         }
      }

      //We force commit boundries to resemble (potential) control flow changes
      if(tempDinst.getTmcode() == transCommit && tempDinst.get_transBCFlag() != 2)
      {
         currBB[threadID]->update_isTrans(1);
         currBB[threadID]->update_transID(transactionID[threadID]);

         currentFlowNode[threadID]->instructionMix.update(currBB[threadID]->return_instructionList());

         std::list < BasicBlock >::iterator transBufferIterator;
         for(transBufferIterator = Synthesis::transBuffer[threadID]->begin(); transBufferIterator != Synthesis::transBuffer[threadID]->end(); transBufferIterator++)
         {
            if((ADDRESS_INT)(*transBufferIterator).return_front_of_instructionList().return_instructionID() != 0)
            {
               numInstructions[threadID] = numInstructions[threadID] + (*transBufferIterator).return_instructionListSize();
               currentFlowNode[threadID]->incrementNumInstructions((*transBufferIterator).return_instructionListSize());
               GraphManipulation::updateGraph(&(*transBufferIterator), threadID);
            }
         }

         //Need to add the current and previous basic blocks since we're starting fresh
         if((ADDRESS_INT)prevBB[threadID]->return_front_of_instructionList().return_instructionID() != 0)
         {
            GraphManipulation::updateGraph(prevBB[threadID], threadID);
            programInstructionMix[threadID].update(prevBB[threadID]->return_instructionListRef(), 1);
         }

         if((ADDRESS_INT)currBB[threadID]->return_front_of_instructionList().return_instructionID() != 0)
         {
            numInstructions[threadID] = numInstructions[threadID] + currBB[threadID]->return_instructionListSize();
            currentFlowNode[threadID]->incrementNumInstructions(currBB[threadID]->return_instructionListSize());
            GraphManipulation::updateGraph(currBB[threadID], threadID);
            programInstructionMix[threadID].update(currBB[threadID]->return_instructionListRef(), 1);
         }

         //BEGIN PCFG COMMIT-----------------------------------------------------------------------------------------------
         if(currentFlowNode[threadID]->return_numInstructions() > 0)
         {
            wasCommitted[threadID] = 1;
            GraphManipulation::addPCFGNode(*currentFlowNode[threadID]);

            delete currentFlowNode[threadID];
            currentFlowNode[threadID] = new FlowNode(0, threadID, 0, currBB[threadID]->return_targetThread(), 0, 0, 0, 0, 0, 0, 0);
         }
         //END   PCFG COMMIT-----------------------------------------------------------------------------------------------

         delete Synthesis::transBuffer[threadID];
         Synthesis::transBuffer[threadID] = new std::list< BasicBlock >;

         delete prevBB[threadID];
         prevBB[threadID] = new BasicBlock();
         delete currBB[threadID];
         currBB[threadID] = new BasicBlock();

         //exiting the transaction -- reset the flag
         Synthesis::isTransaction[threadID] = 0;
         currBB[threadID]->update_isTrans(0);
      }
   }

   delete statConf;

//FIXME The initial thread skips the last few instructions -- these should be flushed
   //If the last block does not end with a branch, we still need to flush to the graph
   if(tempDinst.getInst()->getICode()->func == mint_exit)
   {
      analysisCleanup(threadID);
   }
}

/**
 * @name analysisCleanup
 * 
 * @short This function is called to clean up all of the remaining data at the end of a thread/processor.
 * @param  
 * @return 
 */
void analysisCleanup(THREAD_ID threadID)
{
   /* Processes */
   numInstructions[threadID] = numInstructions[threadID] + prevBB[threadID]->return_instructionListSize() + currBB[threadID]->return_instructionListSize();
   currentFlowNode[threadID]->incrementNumInstructions(currBB[threadID]->return_instructionListSize() + prevBB[threadID]->return_instructionListSize());

   if(Synthesis::isTransaction[threadID] == 1)
   {
      std::list< BasicBlock >::iterator transBufferIterator;
      for(transBufferIterator = Synthesis::transBuffer[threadID]->begin(); transBufferIterator != Synthesis::transBuffer[threadID]->end(); transBufferIterator++)
      {
         if((ADDRESS_INT)(*transBufferIterator).return_front_of_instructionList().return_instructionID() != 0)
         {
            GraphManipulation::updateGraph(&(*transBufferIterator), threadID);
         }
      }

      if((ADDRESS_INT)currBB[threadID]->return_front_of_instructionList().return_instructionID() != 0)
      {
         GraphManipulation::updateGraph(currBB[threadID], threadID);
         programInstructionMix[threadID].update(currBB[threadID]->return_instructionListRef(), 1);
      }
   }
   else
   {
      if((ADDRESS_INT)prevBB[threadID]->return_front_of_instructionList().return_instructionID() != 0)
      {
         GraphManipulation::updateGraph(prevBB[threadID], threadID);
         programInstructionMix[threadID].update(prevBB[threadID]->return_instructionListRef(), 1);
      }

      if((ADDRESS_INT)currBB[threadID]->return_front_of_instructionList().return_instructionID() != 0)
      {
         GraphManipulation::updateGraph(currBB[threadID], threadID);
         programInstructionMix[threadID].update(currBB[threadID]->return_instructionListRef(), 1);
      }
   }

   //This is the last node in the thread so we need to find its parent, however
   //the main thread will not have a parent so there is a bounds check
   //FIXME This needs to be more intelligent -- moved to complete post-runtime analysis
   GraphManipulation::addPCFGNode(*currentFlowNode[threadID]);

   delete currentFlowNode[threadID];

   delete Synthesis::transBuffer[threadID];
   delete prevBB[threadID];
   delete currBB[threadID];
}


/**
 * @name finished
 *
 * @param  
 * @return 
 */
void finished(void)
{
   /* Variables */
   UINT_32 numBasicBlocks[totalNumThreads];
   ConfObject *statConf = new ConfObject;
   string reduced = "reduced";

   /* Processes */
   if(statConf->return_enableSynth() == 1)
   {
      analysisCleanup(0);
      GraphManipulation::finalizePCFG();

      Synthetic *syntheticThreads[totalNumThreads];

      if(statConf->return_debugAll() == 1 || statConf->return_printContents() == 1)
      {
         statConf->print();
      }

      if(statConf->return_debugAll() == 1 || statConf->return_debugPrintGraph() == 1)
      {
         SynthPrinters::SFGprinter(totalNumThreads);
      }

      //generate internal IDs for and dotty
      GraphManipulation::generatePCFGNodeIDs();
      GraphManipulation::generateSFGNodeIDs();

      //go through the PCFG and find threads with the same starting address
      GraphManipulation::identifyDuplicateThreads();

      if(statConf->return_debugAll() == 1 || statConf->return_debugPrintGraphStructure() == 1)
      {
         SynthPrinters::printPCFGStructure();
         SynthPrinters::printSFGStructure();
      }

      if(statConf->return_reduceGraph() == 1)
      {
         GraphManipulation::reducePCFG(numInstructions);
         GraphManipulation::reduceSFG();
      }

      if(statConf->return_debugAll() == 1 || statConf->return_debugPrintGraph() == 1)
      {
         SynthPrinters::PCFGprinter(reduced);
         SynthPrinters::SFGprinter(totalNumThreads, reduced);
      }

      //need to regenerate the IDs if we want to see the reduced graph
      GraphManipulation::generatePCFGNodeIDs();
      GraphManipulation::generateSFGNodeIDs();

      if(statConf->return_debugAll() == 1 || statConf->return_debugPrintDOTs() == 1)
      {
         GraphManipulation::writePCFGDots(reduced);
         GraphManipulation::writeSFGDots(reduced);
      }

      //ouch
//       StatMemory::buildGlobalMemoryMap(tmReport->return_globalReadSet(), tmReport->return_globalReadSet());
//       StatMemory::buildGlobalMemoryMap();

      for(UINT_8 threadID = 0; threadID < totalNumThreads; threadID++)
      {
         GraphManipulation::walkPCFG(threadID, syntheticThreads, totalNumThreads);
      }

      //Analyze current spine
      CodeGenerator::anaylzeSynthetic(syntheticThreads, totalNumThreads);
//       CodeGenerator::anaylzeTransactionMemoryReferences(syntheticThreads, totalNumThreads);

      //write the new C-program to file
      CodeGenerator::writeOutSynthetic(syntheticThreads, totalNumThreads);

      std::cout << "\n\nNumber of threads:  " << totalNumThreads << "\n";
      std::cout << "Number of unique basic blocks:  " << uniqueBBMap.size() << "\n";
      instructionCounts();
   }

   if(statConf->return_debugAll() == 1)
   {
      for(MutexMap::iterator globalMutexMapIterator = globalMutexMap.begin(); globalMutexMapIterator != globalMutexMap.end(); globalMutexMapIterator++)
      {
         std::cout << "pthread_mutex_t " << std::hex << globalMutexMapIterator->first << std::dec << " = PTHREAD_MUTEX_INITIALIZER" << globalMutexMapIterator->second << ";\n";
      }

      SynthPrinters::printBins();
   }

   cleanup();
   delete statConf;
}


/**
 * @name checkContainerSizes
 * 
 * @param threadID 
 * @return 
 * This function makes sure that all of the data structures are large enough
 * to handle any references -- external or internal.
 */
void checkContainerSizes(THREAD_ID threadID)
{
   //Need to ensure that the vector is large enough to hold the next thread
   if(threadID >= currBB.size())
   {
      if(threadID == currBB.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::currBB.push_back with " << threadID;
         #endif
         currBB.push_back(new BasicBlock());
         #ifdef DEBUG
         std::cerr << " and new size of " << currBB.size() << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::currBB.resize with " << threadID;
         #endif
         currBB.resize(threadID + 1, new BasicBlock());
         #ifdef DEBUG
         std::cerr << " and new size of " << currBB.size() << std::endl;
         #endif
      }
   }
   if(threadID >= prevBB.size())
   {
      if(threadID == prevBB.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::prevBB.push_back with " << threadID;
         #endif
         prevBB.push_back(new BasicBlock());
         #ifdef DEBUG
         std::cerr << " and new size of " << prevBB.size() << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::prevBB.resize with " << threadID;
         #endif
         prevBB.resize(threadID + 1, new BasicBlock());
         #ifdef DEBUG
         std::cerr << " and new size of " << prevBB.size() << std::endl;
         #endif
      }
   }
   if(threadID >= currentFlowNode.size())
   {
      if(threadID == currentFlowNode.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::currentFlowNode.push_back with " << threadID;
         #endif
         currentFlowNode.push_back(new FlowNode(1, threadID, 0, 0, 0, 0, 0, 0, 0, 0, 0));
         #ifdef DEBUG
         std::cerr << " and new size of " << currentFlowNode.size() << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::currentFlowNode.resize with " << threadID;
         #endif
         for(UINT_32 counter = currentFlowNode.size(); counter <= (threadID + 1); counter++)
            currentFlowNode.push_back(new FlowNode(1, counter, 0, 0, 0, 0, 0, 0, 0, 0, 0));
         #ifdef DEBUG
         std::cerr << " and new size of " << currentFlowNode.size() << std::endl;
         #endif
      }
   }
   if(threadID >= Synthesis::transBuffer.size())
   {
      if(threadID == Synthesis::transBuffer.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::Synthesis::transBuffer.push_back with " << threadID;
         #endif
         Synthesis::transBuffer.push_back(new std::list < BasicBlock >);
         #ifdef DEBUG
         std::cerr << " and new size of " << Synthesis::transBuffer.size() << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::Synthesis::transBuffer.resize with " << threadID;
         #endif
         Synthesis::transBuffer.resize(threadID + 1, new std::list < BasicBlock >);
         #ifdef DEBUG
         std::cerr << " and new size of " << Synthesis::transBuffer.size() << std::endl;
         #endif
      }
   }
   if(threadID >= numInstructions.size())
   {
      if(threadID == numInstructions.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::numInstructions.push_back with " << threadID;
         #endif
         numInstructions.push_back(0);
         #ifdef DEBUG
         std::cerr << " and new size of " << numInstructions.size() << "*" << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::numInstructions.resize with " << threadID;
         #endif
         numInstructions.resize(threadID + 1, 0);
         #ifdef DEBUG
         std::cerr << " and new size of " << numInstructions.size() << std::endl;
         #endif
      }
   }
   if(threadID >= Synthesis::isTransaction.size())
   {
      if(threadID == Synthesis::isTransaction.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::Synthesis::isTransaction.push_back with " << threadID;
         #endif
         Synthesis::isTransaction.push_back(0);
         #ifdef DEBUG
         std::cerr << " and new size of " << Synthesis::isTransaction.size() << "*" << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::Synthesis::isTransaction.resize with " << threadID;
         #endif
         Synthesis::isTransaction.resize(threadID + 1, 0);
         #ifdef DEBUG
         std::cerr << " and new size of " << Synthesis::isTransaction.size() << std::endl;
         #endif
      }
   }
   if(threadID >= ignoreInstructions.size())
   {
      if(threadID == ignoreInstructions.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::ignoreInstructions.push_back with " << threadID;
         #endif
         ignoreInstructions.push_back(0);
         #ifdef DEBUG
         std::cerr << " and new size of " << ignoreInstructions.size() << "*" << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::ignoreInstructions.resize with " << threadID;
         #endif
         ignoreInstructions.resize(threadID + 1, 0);
         #ifdef DEBUG
         std::cerr << " and new size of " << ignoreInstructions.size() << std::endl;
         #endif
      }
   }
   if(threadID >= lockToggle.size())
   {
      if(threadID == lockToggle.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::lockToggle.push_back with " << threadID;
         #endif
         lockToggle.push_back(0);
         #ifdef DEBUG
         std::cerr << " and new size of " << lockToggle.size() << "*" << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::lockToggle.resize with " << threadID;
         #endif
         lockToggle.resize(threadID + 1, 0);
         #ifdef DEBUG
         std::cerr << " and new size of " << lockToggle.size() << std::endl;
         #endif
      }
   }
   if(threadID >= transactionID.size())
   {
      if(threadID == transactionID.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::transactionID.push_back with " << threadID;
         #endif
         transactionID.push_back(0);
         #ifdef DEBUG
         std::cerr << " and new size of " << transactionID.size() << "*" << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::transactionID.resize with " << threadID;
         #endif
         transactionID.resize(threadID + 1, 0);
         #ifdef DEBUG
         std::cerr << " and new size of " << transactionID.size() << std::endl;
         #endif
      }
   }
   if(threadID >= per_threadWriteBins.size())
   {
      std::vector < UINT_32 > temp_1 (BIN_SIZE, 0);
      if(threadID == per_threadWriteBins.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::per_threadWriteBins.push_back with " << threadID;
         #endif
         per_threadWriteBins.push_back(temp_1);
         #ifdef DEBUG
         std::cerr << " and new size of " << per_threadWriteBins.size() << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::per_threadWriteBins.resize with " << threadID;
         #endif
         per_threadWriteBins.resize(threadID + 1, temp_1);
         #ifdef DEBUG
         std::cerr << " and new size of " << per_threadWriteBins.size() << std::endl;
         #endif
      }
   }
   if(threadID >= per_threadReadBins.size())
   {
      std::vector < UINT_32 > temp_1 (BIN_SIZE, 0);
      if(threadID == per_threadReadBins.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::per_threadReadBins.push_back with " << threadID;
         #endif
         per_threadReadBins.push_back(temp_1);
         #ifdef DEBUG
         std::cerr << " and new size of " << per_threadReadBins.size() << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::per_threadReadBins.resize with " << threadID;
         #endif
         per_threadReadBins.resize(threadID + 1, temp_1);
         #ifdef DEBUG
         std::cerr << " and new size of " << per_threadReadBins.size() << std::endl;
         #endif
      }
   }
   if(threadID >= myCFG_VertexA.size())
   {
      BBVertex temp_1;
      if(threadID == myCFG_VertexA.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::myCFG_VertexA.push_back with " << threadID;
         #endif
         myCFG_VertexA.push_back(temp_1);
         #ifdef DEBUG
         std::cerr << " and new size of " << myCFG_VertexA.size() << "*" << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::myCFG_VertexA.resize with " << threadID;
         #endif
         myCFG_VertexA.resize(threadID + 1, temp_1);
         #ifdef DEBUG
         std::cerr << " and new size of " << myCFG_VertexA.size() << std::endl;
         #endif
      }
   }
   if(threadID >= myCFG_VertexB.size())
   {
      BBVertex temp_1;
      if(threadID == myCFG_VertexB.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::myCFG_VertexB.push_back with " << threadID;
         #endif
         myCFG_VertexB.push_back(temp_1);
         #ifdef DEBUG
         std::cerr << " and new size of " << myCFG_VertexB.size() << "*" << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::myCFG_VertexB.resize with " << threadID;
         #endif
         myCFG_VertexB.resize(threadID + 1, temp_1);
         #ifdef DEBUG
         std::cerr << " and new size of " << myCFG_VertexB.size() << std::endl;
         #endif
      }
   }
   if(threadID >= vertexMap.size())
   {
      BBVertexMap temp_1;
      if(threadID == vertexMap.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::vertexMap.push_back with " << threadID;
         #endif
         vertexMap.push_back(temp_1);
         #ifdef DEBUG
         std::cerr << " and new size of " << vertexMap.size() << "*" << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::vertexMap.resize with " << threadID;
         #endif
         vertexMap.resize(threadID + 1, temp_1);
         #ifdef DEBUG
         std::cerr << " and new size of " << vertexMap.size() << std::endl;
         #endif
      }
   }
   if(threadID >= myCFG.size())
   {
      if(threadID == myCFG.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::myCFG.push_back with " << threadID;
         #endif
         myCFG.push_back(new BBGraph());
         #ifdef DEBUG
         std::cerr << " and new size of " << myCFG.size() << "*" << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::myCFG.resize with " << threadID;
         #endif
         myCFG.resize(threadID + 1, new BBGraph());
         #ifdef DEBUG
         std::cerr << " and new size of " << myCFG.size() << std::endl;
         #endif
      }
   }
   if(threadID >= Synthesis::wasSpawn.size())
   {
      if(threadID == Synthesis::wasSpawn.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::Synthesis::wasSpawn.push_back with " << threadID;
         #endif
         Synthesis::wasSpawn.push_back(0);
         #ifdef DEBUG
         std::cerr << " and new size of " << Synthesis::wasSpawn.size() << "*" << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::Synthesis::wasSpawn.resize with " << threadID;
         #endif
         Synthesis::wasSpawn.resize(threadID + 1, 0);
         #ifdef DEBUG
         std::cerr << " and new size of " << Synthesis::wasSpawn.size() << std::endl;
         #endif
      }
   }
   if(threadID >= lastInsertedNode.size())
   {
      if(threadID == lastInsertedNode.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::lastInsertedNode.push_back with " << threadID;
         #endif
         lastInsertedNode.push_back(graph_traits<PCFG>::null_vertex());
         #ifdef DEBUG
         std::cerr << " and new size of " << lastInsertedNode.size() << "*" << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::lastInsertedNode.resize with " << threadID;
         #endif
         lastInsertedNode.resize(threadID + 1, graph_traits<PCFG>::null_vertex());
         #ifdef DEBUG
         std::cerr << " and new size of " << lastInsertedNode.size() << std::endl;
         #endif
      }
   }
   if(threadID >= Synthesis::wasCommitted.size())
   {
      if(threadID == Synthesis::wasCommitted.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::Synthesis::wasCommitted.push_back with " << threadID;
         #endif
         Synthesis::wasCommitted.push_back(0);
         #ifdef DEBUG
         std::cerr << " and new size of " << Synthesis::wasCommitted.size() << "*" << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::Synthesis::wasCommitted.resize with " << threadID;
         #endif
         Synthesis::wasCommitted.resize(threadID + 1, 0);
         #ifdef DEBUG
         std::cerr << " and new size of " << Synthesis::wasCommitted.size() << std::endl;
         #endif
      }
   }
   if(threadID >= instructionQueueVector.size())
   {
      if(threadID == instructionQueueVector.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::Push back to instructionQueueVector with " << threadID;
         #endif
         instructionQueueVector.push_back(new std::deque< tuple<DInst, Time_t> >);
         #ifdef DEBUG
         std::cerr << " and new size of " << instructionQueueVector.size() << " and capacity of " << instructionQueueVector.capacity() << "*" << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::Resizing instructionQueueVector with " << threadID;
         #endif
         instructionQueueVector.resize(threadID + 1, new std::deque< tuple<DInst, Time_t> >);
         #ifdef DEBUG
         std::cerr << " and new size of " << instructionQueueVector.size() << " and capacity of " << instructionQueueVector.capacity() << "*" << std::endl;
         #endif
      }
   }
   if(threadID >= programInstructionMix.size())
   {
      InstructionMix temp_1;
      if(threadID == programInstructionMix.size())
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::programInstructionMix.push_back with " << threadID;
         #endif
         programInstructionMix.push_back(temp_1);
         #ifdef DEBUG
         std::cerr << " and new size of " << programInstructionMix.size() << "*" << std::endl;
         #endif
      }
      else
      {
         #ifdef DEBUG
         std::cerr << "Synthesis::programInstructionMix.resize with " << threadID;
         #endif
         programInstructionMix.resize(threadID + 1, temp_1);
         #ifdef DEBUG
         std::cerr << " and new size of " << programInstructionMix.size() << std::endl;
         #endif
      }
   }
}
//END checkContainerSizes

}
//END Synthesis --------------------------------------------------------------------------------------------------------------------------------------------------
