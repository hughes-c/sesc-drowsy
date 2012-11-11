/**
 * @file
 * @author  chughes   <>, (C) 2012
 * @date    09/18/12
 * @brief
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Interface: transCoherence \n
 * This object provides the functional coherence.
 *
 * @note
 *
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEM_ACCESSES
#define MEM_ACCESSES

#define DR_SIZE 12

#define NUM_SEQ 2
#define NUM_TXM 3

#define BIN_0  4
#define BIN_1  5
#define BIN_2  6
#define BIN_3  7
#define BIN_4  8
#define BIN_5  9
#define BIN_6  10
#define BIN_7  11


extern std::map<RAddr, memInfo > memAccesses;
extern std::map<RAddr, memInfo >::const_iterator memIter;

size_t binNumber(size_t diff)
{
   if(diff >= 0 && diff <= 2)
      return BIN_0;
   else if(diff > 2 && diff <= 4)
      return BIN_1;
   else if(diff > 4 && diff <= 8)
      return BIN_2;
   else if(diff > 8 && diff <= 16)
      return BIN_3;
   else if(diff > 16 && diff <= 32)
      return BIN_4;
   else if(diff > 32 && diff <= 64)
      return BIN_5;
   else if(diff > 64 && diff <= 128)
      return BIN_6;
   else
      return BIN_7;
}

void memAccessFunc(RAddr raddrIn, bool sequentialIn)
{
   size_t bin;
   memIter =  memAccesses.find(raddrIn);

   if(memIter == memAccesses.end())                    //If at the end of the map, then it does not exist & need to create new
   {
      //create new vecotr of size DR_SIZE
      memAccesses[raddrIn].memBins.resize(DR_SIZE, 0);

      if(sequentialIn == true)
      {
         memAccesses[raddrIn].memBins[0] = 1;
         memAccesses[raddrIn].memBins[NUM_SEQ] = memAccesses[raddrIn].memBins[NUM_SEQ] + 1;
      }
      else
      {
         memAccesses[raddrIn].memBins[1] = 1;
         memAccesses[raddrIn].memBins[NUM_TXM] = memAccesses[raddrIn].memBins[NUM_TXM] + 1;
      }
   }
   else                                                //Otherwise it already exists somewhere
   {
      if(sequentialIn == true)
      {
         if(memAccesses[raddrIn].memBins[1] >= 1)      //If Prev==T and Curr==S
         {
            bin = binNumber(globalClock - memAccesses[raddrIn].lastClock);
            memAccesses[raddrIn].memBins[bin] = memAccesses[raddrIn].memBins[bin] + 1;

            memAccesses[raddrIn].memBins[0] = 1;       //update for next reference
            memAccesses[raddrIn].memBins[1] = 0;
         }

         memAccesses[raddrIn].memBins[NUM_SEQ] = memAccesses[raddrIn].memBins[NUM_SEQ] + 1;
      }
      else
      {
         if(memAccesses[raddrIn].memBins[0] >= 1)     //If Prev==S and Curr==T
         {
            bin = binNumber(globalClock - memAccesses[raddrIn].lastClock);
            memAccesses[raddrIn].memBins[bin] = memAccesses[raddrIn].memBins[bin] + 1;

            memAccesses[raddrIn].memBins[0] = 0;      //update for next reference
            memAccesses[raddrIn].memBins[1] = 1;
         }

         memAccesses[raddrIn].memBins[NUM_TXM] = memAccesses[raddrIn].memBins[NUM_TXM] + 1;
      }
   }

   memAccesses[raddrIn].lastClock = globalClock;

}

#endif