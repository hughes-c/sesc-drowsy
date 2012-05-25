//
// C++ Interface: InstructionMix
//
// Description: 
//
//
// Author: Clay Hughes <hughes@fraidy2-uf>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef INSTRUCTIONMIX_H
#define INSTRUCTIONMIX_H

#include <list>
#include <utility>
#include "stat-types.h"
#include "InstructionContainer.h"

#define MIX_BINS 11

/**
   alu 0, nop 1, cond 2, uncond/call/ret 3, fp 4, alu-mult 5, alu-div 6, ld 7, st 8, fp-mult 9, fp-div 10
 */

class InstructionMix
{
public:
   /* Constructor */
   InstructionMix();
   InstructionMix(const InstructionMix &objectIn);

   /* Variables */

   /* Functions */
   UINT_8 reset(void);
   void normalize(void);

   void update(const std::list <InstructionContainer> &tempList);
   void update(const std::list <InstructionContainer> &tempList, BOOL ref);

   void print(std::ostream &streamIn);
   void print_mixBins(std::ostream &streamIn);
   void print_norm_mixBins(std::ostream &streamIn);

   BOOL compare(InstructionMix &mixIn);
   BOOL compare(const std::list <InstructionContainer> &listIn);
   BOOL compare(const std::list <InstructionContainer> &listIn, InstructionMix mixIn);

   float          return_totalInstructions(void);
   UINT_32        return_bin(UINT_32 binNumber);
   float          return_normal_bin(UINT_32 binNumber);

protected:


private:
   UINT_32        mixBins[MIX_BINS];
   float          normalized_mixBins[MIX_BINS];

};

#endif
