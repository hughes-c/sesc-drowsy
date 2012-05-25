//
// C++ Interface: InstructionContainer
//
// Description: 
//
//
/// @author: Clay Hughes <>, (C) 2006
///
/// @date:           06/01/06
/// Last Modified:   01/14/07
//
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef INSTRUCTIONCONTAINER_H
#define INSTRUCTIONCONTAINER_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

#include "stat-types.h"
#include "Events.h"
#include "InstType.h"
#include "Instruction.h"

class InstructionContainer
{
public:
   /* Constructor */
   InstructionContainer();
   InstructionContainer(const InstructionContainer& objectIn);

   /* Variables */
   UINT_32 strideAmount;

   /* Functions */
   void                 reset(void);

   //update
   UINT_8               update_instructionID(ADDRESS_INT instructionID);

   UINT_8               update_opCode(InstType opCodeIn);
   UINT_8               update_opNum(UINT_32 opNum);
   UINT_8               update_src1(RegType src1);
   UINT_8               update_src2(RegType src2);
   UINT_8               update_dest(RegType dest);
   UINT_8               update_immediate(INT_64 immediate);
   UINT_8               update_physicalAddress(UINT_32 physicalAddress);
   UINT_8               update_virtualAddress(VAddr virtualAddress);

   UINT_8               update_uEvent(EventType uEvent);
   UINT_8               update_subCode(InstSubType subCode);
   UINT_8               update_dataSize(MemDataSize dataSize);

   UINT_8               update_guessTaken(BOOL condIn);
   UINT_8               update_condLikely(BOOL condIn);
   UINT_8               update_jumpLabel(BOOL condIn);

   UINT_8               update_transID(ADDRESS_INT transID);
   UINT_8               update_lockID(IntRegValue lockID);
   UINT_8               update_nodeDepth(UINT_32 nodeDepth);

   UINT_8               update_sharedMem(BOOL sharedMem);

   //return
   ADDRESS_INT          return_instructionID(void) const;

   InstType             return_opCode(void) const;
   UINT_32              return_opNum(void) const;
   RegType              return_src1(void) const;
   RegType              return_src2(void) const;
   RegType              return_dest(void) const;
   INT_32               return_immediate(void) const;
   INT_32               return_physicalAddress(void) const;
   VAddr                return_virtualAddress(void) const;

   EventType            return_uEvent(void) const;
   InstSubType          return_subCode(void) const;
   MemDataSize          return_dataSize(void) const;

   BOOL                 return_guessTaken(void) const;
   BOOL                 return_condLikely(void) const;
   BOOL                 return_jumpLabel(void) const;

   ADDRESS_INT          return_transID(void) const;
   IntRegValue          return_lockID(void) const;
   UINT_32              return_nodeDepth(void) const;

   BOOL                 return_sharedMem(void);
   BOOL                 return_sharedMem(void) const;

protected:


private:
   /* Variables */
   ADDRESS_INT          instructionID;

   InstType             opCode;
   UINT_32              opNum;
   RegType              src1;
   RegType              src2;
   RegType              dest;
   INT_64               immediate;
   INT_32               physicalAddress;
   VAddr                virtualAddress;

   EventType            uEvent;
   InstSubType          subCode;
   MemDataSize          dataSize;

   char                 src1Pool;   // src1 register is in the FP pool?
   char                 src2Pool;   // src2 register is in the FP pool?
   char                 dstPool;    // Destination register is in the FP pool?
   char                 skipDelay;  // 1 when the instruction has delay slot (iBJ ^ !BJCondLikely only)

   BOOL                 guessTaken;
   BOOL                 condLikely;
   BOOL                 jumpLabel;  // If iBJ jumps to offset (not register)

   ADDRESS_INT          transID;    //need to insert this so that it can be propegated through translate NOTE may move later
   IntRegValue          lockID;
   UINT_32              nodeDepth;

   UINT_32              sharedMem;
};

#endif
