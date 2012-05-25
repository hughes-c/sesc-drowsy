//
// C++ Implementation: InstructionContainer
//
// Description: 
//
//
/// @author: Clay Hughes <>, (C) 2006
/// @date:           06/01/06
/// Last Modified:   01/14/07
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "InstructionContainer.h"

InstructionContainer::InstructionContainer()
{
   instructionID = 0;

   opCode = (InstType)INVALID;
   opNum = 0;
   src1 = (RegType)99;
   src2 = (RegType)99;
   dest = (RegType)99;

   immediate = -1;
   virtualAddress = -1;
   physicalAddress = -1;

   guessTaken = 0;
   condLikely = 0;
   jumpLabel = 0;

   lockID = 0;
   transID = 0;
   nodeDepth = 0;

   strideAmount = 0;
   sharedMem = 0;
}

InstructionContainer::InstructionContainer(const InstructionContainer& objectIn)
{
   instructionID = objectIn.instructionID;

   opCode = objectIn.opCode;
   opNum = objectIn.opNum;
   src1 = objectIn.src1;
   src2 = objectIn.src2;
   dest = objectIn.dest;
   immediate = objectIn.immediate;
   physicalAddress = objectIn.physicalAddress;
   virtualAddress = objectIn.virtualAddress;

   uEvent = objectIn.uEvent;
   subCode = objectIn.subCode;
   dataSize = objectIn.dataSize;

   src1Pool = objectIn.src1Pool;
   src2Pool = objectIn.src2Pool;
   dstPool = objectIn.dstPool;
   skipDelay = objectIn.skipDelay;

   guessTaken = objectIn.guessTaken;
   condLikely = objectIn.condLikely;
   jumpLabel = objectIn.jumpLabel;

   transID = objectIn.transID;
   lockID = objectIn.lockID;
   nodeDepth = objectIn.nodeDepth;

   strideAmount = objectIn.strideAmount;
   sharedMem = objectIn.sharedMem;
}

void InstructionContainer::reset()
{
   instructionID = 0;

   opCode = (InstType)INVALID;
   opNum = 0;
   src1 = (RegType)99;
   src2 = (RegType)99;
   dest = (RegType)99;

   immediate = -1;
   virtualAddress = -1;
   physicalAddress = -1;

   guessTaken = 0;
   condLikely = 0;
   jumpLabel = 0;

   lockID = 0;
   transID = 0;
   nodeDepth = 0;

   strideAmount = 0;
   sharedMem = 0;
}

UINT_8 InstructionContainer::update_instructionID(ADDRESS_INT instructionID)
{
   this->instructionID = instructionID;
   return 1;
}

UINT_8 InstructionContainer::update_opCode(InstType opCodeIn)
{
   this->opCode = opCodeIn;
   return 1;
}

UINT_8 InstructionContainer::update_opNum(UINT_32 opNum)
{
   this->opNum = opNum;
   return 1;
}

UINT_8 InstructionContainer::update_src1(RegType src1)
{
   this->src1 = src1;
   return 1;
}

UINT_8 InstructionContainer::update_src2(RegType src2)
{
   this->src2 = src2;
   return 1;
}

UINT_8 InstructionContainer::update_dest(RegType dest)
{
   this->dest = dest;
   return 1;
}

UINT_8 InstructionContainer::update_immediate(INT_64 immediate)
{
   this->immediate = immediate;
   return 1;
}

UINT_8 InstructionContainer::update_physicalAddress(UINT_32 physicalAddress)
{
   this->physicalAddress = physicalAddress;
   return 1;
}

UINT_8 InstructionContainer::update_virtualAddress(VAddr virtualAddress)
{
   this->virtualAddress = virtualAddress;
   return 1;
}

UINT_8 InstructionContainer::update_uEvent(EventType uEvent)
{
   this->uEvent = uEvent;
   return 1;
}

UINT_8 InstructionContainer::update_subCode(InstSubType subCode)
{
   this->subCode = subCode;
   return 1;
}

UINT_8 InstructionContainer::update_dataSize(MemDataSize dataSize)
{
   this->dataSize = dataSize;
   return 1;
}

UINT_8 InstructionContainer::update_guessTaken(BOOL condIn)
{
   this->guessTaken = condIn;
   return 1;
}

UINT_8 InstructionContainer::update_condLikely(BOOL condIn)
{
   this->condLikely = condIn;
   return 1;
}

UINT_8 InstructionContainer::update_jumpLabel(BOOL condIn)
{
   this->jumpLabel = condIn;
   return 1;
}

ADDRESS_INT InstructionContainer::return_instructionID(void) const
{
   return this->instructionID;
}

InstType InstructionContainer::return_opCode(void) const
{
   return this->opCode;
}

UINT_32 InstructionContainer::return_opNum(void) const
{
   return this->opNum;
}

RegType  InstructionContainer::return_src1(void) const
{
   return this->src1;
}

RegType  InstructionContainer::return_src2(void) const
{
   return this->src2;
}

RegType  InstructionContainer::return_dest(void) const
{
   return this->dest;
}

INT_32 InstructionContainer::return_immediate(void) const
{
   return this->immediate;
}

INT_32 InstructionContainer::return_physicalAddress(void) const
{
   return this->physicalAddress;
}

VAddr InstructionContainer::return_virtualAddress(void) const
{
   return this->virtualAddress;
}

EventType InstructionContainer::return_uEvent(void) const
{
   return this->uEvent;
}

InstSubType InstructionContainer::return_subCode(void) const
{
   return this->subCode;
}

MemDataSize InstructionContainer::return_dataSize(void) const
{
   return this->dataSize;
}

BOOL InstructionContainer::return_guessTaken(void) const
{
   return this->guessTaken;
}

BOOL  InstructionContainer::return_condLikely(void) const
{
   return this->condLikely;
}

BOOL  InstructionContainer::return_jumpLabel(void) const
{
   return this->jumpLabel;
}

UINT_8  InstructionContainer::update_lockID(IntRegValue lockID)
{
   this->lockID = lockID;
   return 1;
}

IntRegValue InstructionContainer::return_lockID(void) const
{
   return this->lockID;
}

UINT_8  InstructionContainer::update_transID(ADDRESS_INT transID)
{
   this->transID = transID;
   return 1;
}

ADDRESS_INT  InstructionContainer::return_transID(void) const
{
   return this->transID;
}

UINT_8   InstructionContainer::update_nodeDepth(UINT_32 nodeDepth)
{
   this->nodeDepth = nodeDepth;
   return 1;
}

UINT_32 InstructionContainer::return_nodeDepth(void) const
{
   return this->nodeDepth;
}

UINT_8 InstructionContainer::update_sharedMem(BOOL sharedMem)
{
   this->sharedMem = sharedMem;
   return 1;
}

BOOL InstructionContainer::return_sharedMem(void)
{
   return this->sharedMem;
}

BOOL InstructionContainer::return_sharedMem(void) const
{
   return this->sharedMem;
}

