//
// C++ Implementation: BasicBlock
//
// Description: 
//
//
/// @author: Clay Hughes <>, (C) 2006
/// @date:           07/01/06
/// Last Modified:   01/14/07
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "BasicBlock.h"

BasicBlock::BasicBlock()
{
   bbAddress = 0;
   nodeID = 0;
   transID = 0;
   lockID = 0;
   bbCount = 0;
   numInstructions = 0;
   numDependancies = 0;
   avg_distance = 0;
   std_distance = 0;
   isThreadFunc = 0;
   isTrans = 0;
   isCritical = 0;
   isSpawn = 0;
   isShared = 0;
   isDestroy = 0;
   isWait = 0;
   isBarrier = 0;
   isThreadEvent = 0;
   branchHistory = 0;

   threadID = 0;
   targetThread = 0;

   accumulated = 0;

   for(UINT_64 counter = 0; counter <  BIN_SIZE; counter++)
   {
      sharedWriteBins[counter] = 0;
      sharedReadBins[counter] = 0;
      normalizedSharedWriteBins[counter] = 0;
      normalizedSharedReadBins[counter] = 0;
   }
}

BasicBlock::BasicBlock(const BasicBlock& objectIn)
{
   bbAddress = objectIn.bbAddress;
   nodeID = objectIn.nodeID;
   transID = objectIn.transID;
   lockID = objectIn.lockID;
   bbCount = objectIn.bbCount;
   numInstructions = objectIn.numInstructions;
   numDependancies = objectIn.numDependancies;
   avg_distance = objectIn.avg_distance;
   std_distance = objectIn.std_distance;
   isThreadFunc = objectIn.isThreadFunc;
   isTrans = objectIn.isTrans;
   isCritical = objectIn.isCritical;
   isSpawn = objectIn.isSpawn;
   isShared = objectIn.isShared;
   isDestroy = objectIn.isDestroy;
   isWait = objectIn.isWait;
   isBarrier = objectIn.isBarrier;
   isThreadEvent = objectIn.isThreadEvent;
   branchHistory = objectIn.branchHistory;

   threadID = objectIn.threadID;
   targetThread = objectIn.targetThread;

   instructionMix = objectIn.instructionMix;
   childThreads   = objectIn.childThreads;

   #if defined(PROFILE)
   memoryMap = objectIn.memoryMap;
   #endif

   instructionList = objectIn.instructionList;

   readConflictMap = objectIn.readConflictMap;
   writeConflictMap = objectIn.writeConflictMap;

   accumulated = objectIn.accumulated;

   for(UINT_64 counter = 0; counter <  BIN_SIZE; counter++)
   {
      sharedWriteBins[counter] = objectIn.sharedWriteBins[counter];
      sharedReadBins[counter] = objectIn.sharedReadBins[counter];
      normalizedSharedWriteBins[counter] = objectIn.normalizedSharedWriteBins[counter];
      normalizedSharedReadBins[counter] = objectIn.normalizedSharedReadBins[counter];
   }
}

UINT_8 BasicBlock::clearLists(void)
{
   this->sharedMemReads.clear();
   this->sharedMemWrites.clear();
   return 1;
}

void BasicBlock::printNormalizedBins()
{
   cout << "\nWrite Bins:\n";
   for(UINT_64 counter = 0; counter < BIN_SIZE; counter++)
      cout << this->normalizedSharedWriteBins[counter] << "\t";

   cout << "\nRead Bins:\n";
   for(UINT_64 counter = 0; counter < BIN_SIZE; counter++)
      cout << this->normalizedSharedReadBins[counter] << "\t";
}

UINT_8 BasicBlock::update_bbAddress(ADDRESS_INT bbAddress)
{
   this->bbAddress = bbAddress;
   return 1;
}

UINT_8 BasicBlock::update_nodeID(UINT_32 nodeID)
{
   this->nodeID = nodeID;
   return 1;
}

UINT_8 BasicBlock::update_bbCount(UINT_64 bbCount)
{
   this->bbCount = bbCount;
   return 1;
}

UINT_8 BasicBlock::update_bbThreadCount(UINT_64 bbCount, THREAD_ID threadID)
{
   this->bbThreadCount[threadID] = bbCount;
   return 1;
}

UINT_8 BasicBlock::update_numInstructions(UINT_64 numInstructions)
{
   this->numInstructions = numInstructions;
   return 1;
}

UINT_8 BasicBlock::update_isTrans(BOOL isTrans)
{
   this->isTrans = isTrans;
   return 1;
}

UINT_8 BasicBlock::update_isCritical(BOOL isCritical)
{
   this->isCritical = isCritical;
   return 1;
}

UINT_8 BasicBlock::update_isThreadFunc(BOOL isThreadFunc)
{
   this->isThreadFunc = isThreadFunc;
   return 1;
}

UINT_8 BasicBlock::update_isSpawn(BOOL isSpawn)
{
   this->isSpawn = isSpawn;
   this->isThreadEvent = 1;
   return 1;
}

UINT_8 BasicBlock::update_isDestroy(BOOL isDestroy)
{
   this->isDestroy = isDestroy;
   this->isThreadEvent = 1;
   return 1;
}

UINT_8 BasicBlock::update_isShared(BOOL isShared)
{
   this->isShared = isShared;
   return 1;
}

UINT_8 BasicBlock::update_isWait(BOOL isWait)
{
   this->isWait = isWait;
   this->isThreadEvent = 1;
   return 1;
}

UINT_8 BasicBlock::update_isBarrier(BOOL isBarrier)
{
   this->isBarrier = isBarrier;
   this->isThreadEvent = 1;
   return 1;
}

UINT_8 BasicBlock::update_isThreadEvent(BOOL isThreadEvent)
{
   this->isThreadEvent = isThreadEvent;
   return 1;
}

UINT_8 BasicBlock::update_numDependancies(UINT_32 numDependancies)
{
   this->numDependancies = numDependancies;
   return 1;
}

UINT_8 BasicBlock::update_avgDistance(float avgDistance)
{
   this->avg_distance = avgDistance;
   return 1;
}

UINT_8 BasicBlock::update_stdDistance(float stdDistance)
{
   this->std_distance = stdDistance;
   return 1;
}

UINT_8 BasicBlock::update_branchHistory(BOOL branchTaken)
{
   this->branchHistory = this->branchHistory << 1;                   //shift left 1

   if(branchTaken == 1)
      this->branchHistory = this->branchHistory | 1;                 //bitwise or 1

   return 1;	
}

UINT_64 BasicBlock::return_branchHistory(void) const
{
   return this->branchHistory;
}

ADDRESS_INT BasicBlock::return_bbAddress() const
{
   return this->bbAddress;
}

UINT_32 BasicBlock::return_nodeID() const
{
   return this->nodeID;
}

UINT_64 BasicBlock::return_bbCount() const
{
   return this->bbCount;
}

UINT_64 BasicBlock::return_bbThreadCount(THREAD_ID threadID) const
{
   return this->bbThreadCount[threadID];
}

UINT_64 BasicBlock::return_numInstructions()
{
   return this->numInstructions;
}

UINT_64 BasicBlock::return_numInstructions() const
{
   return this->numInstructions;
}

UINT_32 BasicBlock::return_numDependancies()
{
   return this->numDependancies;
}

UINT_32 BasicBlock::return_numDependancies() const
{
   return this->numDependancies;
}

BOOL BasicBlock::return_isTrans()
{
   return this->isTrans;
}

BOOL BasicBlock::return_isTrans() const
{
   return this->isTrans;
}

BOOL BasicBlock::return_isCritical()
{
   return this->isCritical;
}

BOOL BasicBlock::return_isCritical() const
{
   return this->isCritical;
}

BOOL BasicBlock::return_isThreadFunc()
{
   return this->isThreadFunc;
}

BOOL BasicBlock::return_isThreadFunc() const
{
   return this->isThreadFunc;
}

BOOL BasicBlock::return_isSpawn()
{
   return this->isSpawn;
}

BOOL BasicBlock::return_isSpawn() const
{
   return this->isSpawn;
}

BOOL BasicBlock::return_isDestroy()
{
   return this->isDestroy;
}

BOOL BasicBlock::return_isDestroy() const
{
   return this->isDestroy;
}

BOOL BasicBlock::return_isShared()
{
   return this->isShared;
}

BOOL BasicBlock::return_isShared() const
{
   return this->isShared;
}

BOOL BasicBlock::return_isWait()
{
   return this->isWait;
}

BOOL BasicBlock::return_isWait() const
{
   return this->isWait;
}

BOOL BasicBlock::return_isBarrier()
{
   return this->isBarrier;
}

BOOL BasicBlock::return_isBarrier() const
{
   return this->isBarrier;
}

BOOL BasicBlock::return_isThreadEvent()
{
   return this->isThreadEvent;
}

BOOL BasicBlock::return_isThreadEvent() const
{
   return this->isThreadEvent;
}

float BasicBlock::return_avgDistance() const
{
   return this->avg_distance;
}

float BasicBlock::return_stdDistance() const
{
   return this->std_distance;
}

BOOL BasicBlock::operator==(BasicBlock &objectIn) const
{
   if(objectIn.return_bbAddress() == bbAddress)
      return true;
   else
      return false;
}

UINT_8 BasicBlock::update_sharedMemReads(ADDRESS_INT address)
{
   this->sharedMemReads.push_back(address);
		
   return 1;
}

UINT_8 BasicBlock::update_sharedMemWrites(ADDRESS_INT address)
{
   this->sharedMemWrites.push_back(address);
		
   return 1;
}

ADDRESS_INT BasicBlock::return_sharedMemRead()
{
	ADDRESS_INT front;
	
	front = this->sharedMemReads.front();
   this->sharedMemReads.pop_front();
	
   return front;
}

ADDRESS_INT BasicBlock::return_sharedMemWrite()
{
	ADDRESS_INT front;
	
	front = this->sharedMemWrites.front();
   this->sharedMemWrites.pop_front();
	
   return front;
}

UINT_32 BasicBlock::return_size_of_sharedMemReads()
{
   return this->sharedMemReads.size();
}

UINT_32 BasicBlock::return_size_of_sharedMemWrites()
{
   return this->sharedMemWrites.size();
}

float* BasicBlock::return_normalizedSharedWriteBins()
{
   return this->normalizedSharedWriteBins;
}

float* BasicBlock::return_normalizedSharedReadBins()
{
   return this->normalizedSharedReadBins;
}

UINT_32 BasicBlock::return_size_of_nextSharedMemRead(float myUniformRV)
{
   UINT_64 memRead;
   float nextMemReference = myUniformRV;

   if(nextMemReference < this->normalizedSharedReadBins[0])
      memRead = 0;
   else if(nextMemReference < this->normalizedSharedReadBins[0] + this->normalizedSharedReadBins[1])
      memRead = 1;	
   else if(nextMemReference < this->normalizedSharedReadBins[0] + this->normalizedSharedReadBins[1] + this->normalizedSharedReadBins[2])
      memRead = 2;
   else if(nextMemReference < this->normalizedSharedReadBins[0] + this->normalizedSharedReadBins[1] + this->normalizedSharedReadBins[2] + this->normalizedSharedReadBins[3])
      memRead = 3;
   else if(nextMemReference < this->normalizedSharedReadBins[0] + this->normalizedSharedReadBins[1] + this->normalizedSharedReadBins[2] + this->normalizedSharedReadBins[3] + this->normalizedSharedReadBins[4])
      memRead = 4;
   else if(nextMemReference < this->normalizedSharedReadBins[0] + this->normalizedSharedReadBins[1] + this->normalizedSharedReadBins[2] + this->normalizedSharedReadBins[3] + this->normalizedSharedReadBins[4] + this->normalizedSharedReadBins[5])
      memRead = rand() % 25;
   else
      memRead = 6;

   return memRead;
}

UINT_32 BasicBlock::return_size_of_nextSharedMemWrite(float myUniformRV)
{
   UINT_64 memWrite;
   float nextMemReference = myUniformRV;
   
   if(nextMemReference < this->normalizedSharedWriteBins[0])
      memWrite = 0;
   else if(nextMemReference < this->normalizedSharedWriteBins[0] + this->normalizedSharedWriteBins[1])
      memWrite = 1;	
   else if(nextMemReference < this->normalizedSharedWriteBins[0] + this->normalizedSharedWriteBins[1] + this->normalizedSharedWriteBins[2])
      memWrite = 2;
   else if(nextMemReference < this->normalizedSharedWriteBins[0] + this->normalizedSharedWriteBins[1] + this->normalizedSharedWriteBins[2] + this->normalizedSharedWriteBins[3])
      memWrite = 3;
   else if(nextMemReference < this->normalizedSharedWriteBins[0] + this->normalizedSharedWriteBins[1] + this->normalizedSharedWriteBins[2] + this->normalizedSharedWriteBins[3] + this->normalizedSharedWriteBins[4])
      memWrite = 4;
   else if(nextMemReference < this->normalizedSharedWriteBins[0] + this->normalizedSharedWriteBins[1] + this->normalizedSharedWriteBins[2] + this->normalizedSharedWriteBins[3] + this->normalizedSharedWriteBins[4] + this->normalizedSharedWriteBins[5])
      memWrite = rand() % 25;
   else
      memWrite = 6;

   return memWrite;
}

void BasicBlock::findMemoryStatistics()
{
   /* Variable Declaration */
   ADDRESS_INT lastAddress = 0;
   ADDRESS_INT currentAddress = 0;
   UINT_64 delta = 0;
   std::list <ADDRESS_INT>::iterator addressListIterator;


   /* Processes */
   //Writes
   if(this->sharedMemWrites.size() > 0)
   {
      for(addressListIterator = this->sharedMemWrites.begin(); addressListIterator != this->sharedMemWrites.end(); addressListIterator++)
      {
         currentAddress = *addressListIterator;

         if(lastAddress == 0)
            delta = 0;
         else if(lastAddress > currentAddress)
            delta = (lastAddress - currentAddress);
         else
            delta = (currentAddress -lastAddress);	

         switch(delta)
         {
            case 0 :
               this->sharedWriteBins[0] = this->sharedWriteBins[0] + 1;
               break;
            case 1 :	
               this->sharedWriteBins[1] = this->sharedWriteBins[1] + 1;
               break;
            case 2 :
               this->sharedWriteBins[2] = this->sharedWriteBins[2] + 1;
               break;
            case 4 :
               this->sharedWriteBins[3] = this->sharedWriteBins[3] + 1;
               break;
            case 8 :
               this->sharedWriteBins[4] = this->sharedWriteBins[4] + 1;
               break;
            default :
               this->sharedWriteBins[5] = this->sharedWriteBins[5] + 1;
               break;
         }

         lastAddress = currentAddress;
      }


      float total = 0;
      for(UINT_64 counter = 0; counter < BIN_SIZE; counter++)
      {
         total = total + this->sharedWriteBins[counter];
      }
      for(UINT_64 counter = 0; counter < BIN_SIZE; counter++)
      {
         this->normalizedSharedWriteBins[counter] = (float)this->sharedWriteBins[counter] / (float)total;
         cout << "--" << this->normalizedSharedWriteBins[counter] << "   ";
      }
      cout << "\n" << flush;
   }	

   //Reads
   if(this->sharedMemReads.size() > 0)
   {
      for(addressListIterator = this->sharedMemReads.begin(); addressListIterator != this->sharedMemReads.end(); addressListIterator++)
      {
         currentAddress = *addressListIterator;

         if(lastAddress == 0)
            delta = 0;
         else if(lastAddress > currentAddress)
            delta = (lastAddress - currentAddress);
         else
            delta = (currentAddress -lastAddress);	

         switch(delta)
         {
            case 0 :
               this->sharedReadBins[0] = this->sharedReadBins[0] + 1;
               break;
            case 1 :	
               this->sharedReadBins[1] = this->sharedReadBins[1] + 1;
               break;
            case 2 :
               this->sharedReadBins[2] = this->sharedReadBins[2] + 1;
               break;
            case 4 :
               this->sharedReadBins[3] = this->sharedReadBins[3] + 1;
               break;
            case 8 :
               this->sharedReadBins[4] = this->sharedReadBins[4] + 1;
               break;
            default :
               this->sharedReadBins[5] = this->sharedReadBins[5] + 1;
               break;
         }

         lastAddress = currentAddress;
      }


      float total = 0;
      for(UINT_64 counter = 0; counter < BIN_SIZE; counter++)
      {
         total = total + this->sharedReadBins[counter];
      }
      for(UINT_64 counter = 0; counter < BIN_SIZE; counter++)
      {
         this->normalizedSharedReadBins[counter] = (float)this->sharedReadBins[counter] / (float)total;
         cout << "--" << this->normalizedSharedReadBins[counter] << "   ";
      }
      cout << "\n" << flush;
   }
}

UINT_8 BasicBlock::update_threadID(THREAD_ID threadID)
{
   this->threadID = threadID;
   return 1;
}

THREAD_ID BasicBlock::return_threadID() const
{
   return this->threadID;
}

UINT_8 BasicBlock::update_targetThread(THREAD_ID targetThread)
{
   this->targetThread = targetThread;
   return 1;
}

THREAD_ID BasicBlock::return_targetThread() const
{
   return this->targetThread;
}

UINT_8  BasicBlock::update_lockID(IntRegValue lockID)
{
   this->lockID = lockID;
   return 1;
}

IntRegValue BasicBlock::return_lockID() const
{
   return this->lockID;
}

UINT_8  BasicBlock::update_transID(ADDRESS_INT transID)
{
   this->transID = transID;
   return 1;
}

ADDRESS_INT  BasicBlock::return_transID() const
{
   return this->transID;
}

UINT_8   BasicBlock::update_nodeDepth(UINT_32 nodeDepth)
{
   this->nodeDepth = nodeDepth;
   return 1;
}

UINT_32 BasicBlock::return_nodeDepth(void) const
{
   return this->nodeDepth;
}

UINT_8 BasicBlock::update_accumulated(UINT_32 accumulated)
{
   this->accumulated = accumulated;
   return 1;
}

UINT_32 BasicBlock::return_accumulated(void) const
{
   return this->accumulated;
}

/**
 * 
 * @param listIn 
 * @return 
**/
UINT_8 BasicBlock::copy_instructionList(const std::list <InstructionContainer> &listIn)
{
   std::list <InstructionContainer>::const_iterator instructionListIterator;

   for(instructionListIterator = listIn.begin(); instructionListIterator != listIn.end(); instructionListIterator++)
   {
      instructionList.push_back(*instructionListIterator);
   }

   return 1;
}

/**
 * 
 * @return 
**/
UINT_8 BasicBlock::clear_instructionList()
{
   instructionList.clear();

   return 1;
}

/**
 * 
 * @param newSize 
 * @return 
 */
UINT_8 BasicBlock::resize_instructionList(UINT_32 newSize)
{
   instructionList.resize(newSize);

   return 1;
}

/**
 * 
 * @param element_a 
 * @return 
 */
UINT_8 BasicBlock::erase_instructionList(UINT_32 element_a)
{
   UINT_32 counter = 0;
   std::list <InstructionContainer>::iterator instructionListIterator;

   for(instructionListIterator = instructionList.begin(); instructionListIterator != instructionList.end(); instructionListIterator++)
   {
      if(counter == element_a)
      {
         break;
      }
      counter = counter + 1;
   }

   instructionList.erase(instructionListIterator);

   return 1;
}

/**
 * 
 * @param element_a 
 * @param element_b 
 * @return 
 * Iterators specifying a range within the list container to
 * be removed: [first,last). i.e., the range includes all the
 * elements between first and last, including the element
 * pointed by first but not the one pointed by last.
 */
UINT_8 BasicBlock::erase_instructionList(UINT_32 first, UINT_32 last)
{
   UINT_32 counter = 0;
   std::list <InstructionContainer>::iterator instructionListIterator_first;
   std::list <InstructionContainer>::iterator instructionListIterator_last;

   for(instructionListIterator_first = instructionList.begin(); instructionListIterator_first != instructionList.end(); instructionListIterator_first++)
   {
      if(counter == first)
      {
         break;
      }
      counter = counter + 1;
   }

   for(instructionListIterator_last = instructionListIterator_first; instructionListIterator_last != instructionList.end(); instructionListIterator_last++)
   {
      if(counter == last)
      {
         break;
      }
      counter = counter + 1;
   }

   instructionList.erase(instructionListIterator_first, instructionListIterator_last);

   return 1;
}

/**
 * 
 * @param dynamic_instruction 
 * @return 
**/
UINT_8 BasicBlock::update_instructionList(DInst dynamic_instruction)
{
   InstructionContainer tempInstruction;

   tempInstruction.update_instructionID((ADDRESS_INT)dynamic_instruction.get_instructionAddress());
   tempInstruction.update_opCode(dynamic_instruction.getOpcode());
   tempInstruction.update_opNum(dynamic_instruction.get_opNum());
   tempInstruction.update_src1(dynamic_instruction.get_src1());
   tempInstruction.update_src2(dynamic_instruction.get_src2());
   tempInstruction.update_dest(dynamic_instruction.get_dest());
   tempInstruction.update_immediate(dynamic_instruction.get_immediate());
   tempInstruction.update_virtualAddress(dynamic_instruction.getVaddr());
   tempInstruction.update_subCode(dynamic_instruction.get_subCode());
   tempInstruction.update_uEvent(dynamic_instruction.get_uEvent());
   tempInstruction.update_dataSize(dynamic_instruction.get_dataSize());
   tempInstruction.update_guessTaken(dynamic_instruction.get_guessTaken());
   tempInstruction.update_condLikely(dynamic_instruction.get_condLikely());
   tempInstruction.update_jumpLabel(dynamic_instruction.get_jumpLabel());

   instructionList.push_back(tempInstruction);

   if(dynamic_instruction.get_lockID() != 0)
      this->lockID = dynamic_instruction.get_lockID();

   return 1;
}

/**
 * 
 * @param  
 * @return 
**/
InstructionContainer BasicBlock::return_back_of_instructionList(void)
{
   InstructionContainer temp;

   if(instructionList.empty() == 1)
      return temp;
   else
      return instructionList.back();
}

/**
 * 
 * @param  
 * @return 
**/
InstructionContainer BasicBlock::return_front_of_instructionList(void)
{
   InstructionContainer temp;

   if(instructionList.empty() == 1)
      return temp;
   else
      return instructionList.front();
}

/**
 * 
 * @param  
 * @return 
 */
InstructionContainer BasicBlock::return_front_of_instructionList_pop(void)
{
   InstructionContainer temp;

   if(instructionList.empty() == 1)
   {
      return temp;
   }
   else
   {
      InstructionContainer front = instructionList.front();
      instructionList.pop_front();

      return front;
   }
}

/**
 * 
 * @param  
 * @return 
**/
std::list <InstructionContainer> BasicBlock::return_instructionList(void) const
{
   return instructionList;
}

/**
 * 
 * @param  
 * @return 
**/
std::list <InstructionContainer> &BasicBlock::return_instructionListRef(void)
{
   return instructionList;
}

/**
 * 
 * @param  
 * @return 
 */
UINT_32 BasicBlock::return_instructionListSize(void)
{
   return instructionList.size();
}

/**
 * 
 * @param streamIn 
 * @return 
 */
void BasicBlock::print_instructionList(std::ostream &outputStream)
{
   std::list <InstructionContainer>::iterator instructionListIterator;

   for(instructionListIterator = instructionList.begin(); instructionListIterator != instructionList.end(); instructionListIterator++)
   {
      outputStream << Instruction::opcode2Name(instructionListIterator->return_opCode()) << "\n";
   }
   outputStream << std::endl;
}

UINT_8 BasicBlock::clearMemoryMaps()
{
   readConflictMap.clear();
   writeConflictMap.clear();

   return 1;
}

UINT_8 BasicBlock::update_readConflictMap(const std::map< ADDRESS_INT, UINT_32 > &mapIn)
{
   this->readConflictMap = mapIn;
   return 1;
}

std::map< ADDRESS_INT, UINT_32 > BasicBlock::return_readConflictMap(void)
{
   return this->readConflictMap;
}

std::map< ADDRESS_INT, UINT_32 > BasicBlock::return_readConflictMap(void) const
{
   return this->readConflictMap;
}

std::map< ADDRESS_INT, UINT_32 > &BasicBlock::return_readConflictMapRef(void)
{
   return this->readConflictMap;
}

UINT_32 BasicBlock::return_readConflictMapSize(void) const
{
   return this->readConflictMap.size();
}

UINT_8 BasicBlock::update_writeConflictMap(const std::map< ADDRESS_INT, UINT_32 > &mapIn)
{
   this->writeConflictMap = mapIn;
   return 1;
}

std::map< ADDRESS_INT, UINT_32 > BasicBlock::return_writeConflictMap(void)
{
   return this->writeConflictMap;
}

std::map< ADDRESS_INT, UINT_32 > BasicBlock::return_writeConflictMap(void) const
{
   return this->writeConflictMap;
}

std::map< ADDRESS_INT, UINT_32 > &BasicBlock::return_writeConflictMapRef(void)
{
   return this->writeConflictMap;
}

UINT_32 BasicBlock::return_writeConflictMapSize(void) const
{
   return this->writeConflictMap.size();
}
