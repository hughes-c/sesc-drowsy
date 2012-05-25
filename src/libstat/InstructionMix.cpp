//
// C++ Implementation: InstructionMix
//
// Description: 
//
//
// Author: Clay Hughes <hughes@fraidy2-uf>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "InstructionMix.h"

InstructionMix::InstructionMix()
{
   for(UINT_32 counter = 0; counter < MIX_BINS; counter++)
   {
      mixBins[counter] = 0;
      normalized_mixBins[counter] = 0;
   }
}

InstructionMix::InstructionMix(const InstructionMix &objectIn)
{
   for(UINT_32 counter = 0; counter < MIX_BINS; counter++)
   {
      mixBins[counter] = objectIn.mixBins[counter];
      normalized_mixBins[counter] = objectIn.normalized_mixBins[counter];
   }
}

UINT_8 InstructionMix::reset(void)
{
   for(UINT_32 counter = 0; counter < MIX_BINS; counter++)
   {
      mixBins[counter] = 0;
      normalized_mixBins[counter] = 0;
   }

   return 1;
}

void InstructionMix::update(const std::list <InstructionContainer> &tempList)
{
   for(std::list <InstructionContainer>::const_iterator instructionListIterator = tempList.begin(); instructionListIterator != tempList.end(); instructionListIterator++)
   {
      if(instructionListIterator->return_subCode() == iNop)
      {
         mixBins[1] = mixBins[1] + 1;
      }
      else if(instructionListIterator->return_opCode() == iALU)
      {
         mixBins[0] = mixBins[0] + 1;
      }
      else if(instructionListIterator->return_subCode() == BJUncond || instructionListIterator->return_subCode() == BJCall || instructionListIterator->return_subCode() == BJRet)
      {
         mixBins[3] = mixBins[3] + 1;
      }
      else if(instructionListIterator->return_subCode() == BJCond)
      {
         mixBins[2] = mixBins[2] + 1;
      }
      else if(instructionListIterator->return_opCode() == fpALU)
      {
         mixBins[4] = mixBins[4] + 1;
      }
      else if(instructionListIterator->return_opCode() == iMult)
      {
         mixBins[5] = mixBins[5] + 1;
      }
      else if(instructionListIterator->return_opCode() == iDiv)
      {
         mixBins[6] = mixBins[6] + 1;
      }
      else if(instructionListIterator->return_opCode() == iStore)
      {
         mixBins[8] = mixBins[8] + 1;
      }
      else if(instructionListIterator->return_opCode() == iLoad)
      {
         mixBins[7] = mixBins[7] + 1;
      }
      else if(instructionListIterator->return_opCode() == fpMult)
      {
         mixBins[9] = mixBins[9] + 1;
      }
      else if(instructionListIterator->return_opCode() == fpDiv)
      {
         mixBins[10] = mixBins[10] + 1;
      }
   }
}

void InstructionMix::update(const std::list <InstructionContainer> &tempList, BOOL ref)
{
   for(std::list <InstructionContainer>::const_iterator instructionListIterator = tempList.begin(); instructionListIterator != tempList.end(); instructionListIterator++)
   {
      if(instructionListIterator->return_subCode() == iNop)
      {
         mixBins[1] = mixBins[1] + 1;
      }
      else if(instructionListIterator->return_opCode() == iALU)
      {
         mixBins[0] = mixBins[0] + 1;
      }
      else if(instructionListIterator->return_subCode() == BJUncond || instructionListIterator->return_subCode() == BJCall || instructionListIterator->return_subCode() == BJRet)
      {
         mixBins[3] = mixBins[3] + 1;
      }
      else if(instructionListIterator->return_subCode() == BJCond)
      {
         mixBins[2] = mixBins[2] + 1;
      }
      else if(instructionListIterator->return_opCode() == fpALU)
      {
         mixBins[4] = mixBins[4] + 1;
      }
      else if(instructionListIterator->return_opCode() == iMult)
      {
         mixBins[5] = mixBins[5] + 1;
      }
      else if(instructionListIterator->return_opCode() == iDiv)
      {
         mixBins[6] = mixBins[6] + 1;
      }
      else if(instructionListIterator->return_opCode() == iStore)
      {
         mixBins[8] = mixBins[8] + 1;
      }
      else if(instructionListIterator->return_opCode() == iLoad)
      {
         mixBins[7] = mixBins[7] + 1;
      }
      else if(instructionListIterator->return_opCode() == fpMult)
      {
         mixBins[9] = mixBins[9] + 1;
      }
      else if(instructionListIterator->return_opCode() == fpDiv)
      {
         mixBins[10] = mixBins[10] + 1;
      }
   }
}

void InstructionMix::normalize(void)
{
   float total = return_totalInstructions();

   for(UINT_32 counter = 0; counter < MIX_BINS; counter++)
   {
      normalized_mixBins[counter] = (float)mixBins[counter] / total;
   }
}

void InstructionMix::print(std::ostream &streamIn)
{
   print_mixBins(streamIn);
   print_norm_mixBins(streamIn);
}

void InstructionMix::print_mixBins(std::ostream &streamIn)
{
   for(UINT_32 counter = 0; counter < MIX_BINS; counter++)
      streamIn << setiosflags(ios::fixed) << setprecision(5) << mixBins[counter] << "\t";

   streamIn << std::endl;
}

void InstructionMix::print_norm_mixBins(std::ostream &streamIn)
{
   for(UINT_32 counter = 0; counter < MIX_BINS; counter++)
      streamIn << setiosflags(ios::fixed) << setprecision(5) << normalized_mixBins[counter] << "\t";

   streamIn << std::endl;
}

float InstructionMix::return_totalInstructions(void)
{
   float total = 0;

   for(UINT_32 counter = 0; counter < MIX_BINS; counter++)
   {
      total = total + mixBins[counter];
   }

   return total;
}

UINT_32 InstructionMix::return_bin(UINT_32 binNumber)
{
   return mixBins[binNumber];
}

float InstructionMix::return_normal_bin(UINT_32 binNumber)
{
   return normalized_mixBins[binNumber];
}

BOOL InstructionMix::compare(InstructionMix &mixIn)
{
   float minTolerance;
   float maxTolerance;
   float tolerance = 0.125;

   for(UINT_32 counter = 0; counter < MIX_BINS; counter++)
   {
      //skip branch instructions
      if(counter == 2 || counter == 3)
         continue;

      maxTolerance = normalized_mixBins[counter] + normalized_mixBins[counter] * tolerance;
      minTolerance = normalized_mixBins[counter] - normalized_mixBins[counter] * tolerance;

//       std::cout << "(" << counter << ")Min:  " << minTolerance << "\tMax:  " << maxTolerance << "\n";
// this->print(std::cout);
// mixIn.print(std::cout);
      if(maxTolerance < mixIn.normalized_mixBins[counter] || minTolerance > mixIn.normalized_mixBins[counter])
      {
//          std::cout << "----(" << counter << ")Min:  " << minTolerance << "\tMax:  " << maxTolerance << "\tThisBin:  " << mixIn.normalized_mixBins[counter] << "\n";
         return 0;
      }
   }

   return 1;
}

BOOL InstructionMix::compare(const std::list <InstructionContainer> &listIn)
{
   InstructionMix mixIn;

   mixIn.update(listIn, 1);
   mixIn.normalize();

   return this->compare(mixIn);
}

BOOL InstructionMix::compare(const std::list <InstructionContainer> &listIn, InstructionMix mixIn)
{
   mixIn.update(listIn, 1);
   mixIn.normalize();

   return this->compare(mixIn);
}
