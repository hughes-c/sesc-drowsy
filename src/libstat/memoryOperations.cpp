//
// C++ Implementation: memoryOperations
//
// Description: 
//
//
// Author: Clay Hughes <hughes@fraidy2-uf>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include "memoryOperations.h"

//NOTE memory
extern AddressMap sharedAddressMap;
extern std::vector < std::vector < UINT_32 > > per_threadReadBins;
extern std::vector < std::vector < UINT_32 > > per_threadWriteBins;
extern std::vector < ADDRESS_INT > lastReadAddress;
extern std::vector < ADDRESS_INT > lastWriteAddress;

//NOTE graph
extern std::deque< BBGraph * > myCFG;
extern PCFG myPCFG;

float normalizedWriteBins[BIN_SIZE];
float normalizedReadBins[BIN_SIZE];

namespace StatMemory
{
std::map< ADDRESS_INT, BOOL >  foundTransactions;

/**
 * @name recordMemWrite
 * 
 * @param memoryAddress 
 * @param writeSize 
 * @param bbAddress 
 * @param threadID 
 * @return 
 */
void recordMemWrite(VAddr memoryAddress, INT_32 writeSize, ADDRESS_INT bbAddress, THREAD_ID threadID)
{
   /* Variable Declaration */
   size_t delta = 0;
   bool shared = 0;
   ADDRESS_INT lastAddress = lastWriteAddress[threadID];
   ADDRESS_INT currentAddress = memoryAddress;
   AddressMap::iterator memMapIterator;

   //graph
   graph_traits <BBGraph>::vertex_iterator vertexIterator, vertexEnd;
   basicBlock_name_map_t basicBlockLocal = get(basicBlock_t(), *myCFG[threadID]);

      /* Processes */
   //Check to see if the address is already known -- if it is then check the threadID against the stored
   //ID.  If they do not match then the address lies within the thread-shared region and should be recorded.
   //Find the distance, in bytes, from the last entry
   if(lastAddress == 0)
      delta = 0;
   else if(lastAddress > currentAddress)
      delta = (lastAddress - currentAddress);
   else
      delta = (currentAddress -lastAddress);	

// cout << "W <> Current:  " << hex << currentAddress << "  Previous:  " << lastAddress << dec << "  Delta:  " << delta << "  Size:  " << writeSize << endl;

   //based on the assumption that one cache line is 32B
   if(delta < 31)
      per_threadWriteBins[threadID][0] = per_threadWriteBins[threadID][0] + 1;
   else if(delta < 63)
      per_threadWriteBins[threadID][1] = per_threadWriteBins[threadID][1] + 1;
   else if(delta < 95)
      per_threadWriteBins[threadID][2] = per_threadWriteBins[threadID][2] + 1;
   else if(delta < 127)
      per_threadWriteBins[threadID][3] = per_threadWriteBins[threadID][3] + 1;
   else if(delta < 159)
      per_threadWriteBins[threadID][4] = per_threadWriteBins[threadID][4] + 1;
   else if(delta < 191)
      per_threadWriteBins[threadID][5] = per_threadWriteBins[threadID][5] + 1;
   else if(delta < 223)
      per_threadWriteBins[threadID][6] = per_threadWriteBins[threadID][6] + 1;
   else if(delta < 255)
      per_threadWriteBins[threadID][7] = per_threadWriteBins[threadID][7] + 1;
   else if(delta < 287)
      per_threadWriteBins[threadID][8] = per_threadWriteBins[threadID][8] + 1;
   else
      per_threadWriteBins[threadID][9] = per_threadWriteBins[threadID][9] + 1;

   //Need to ensure that the vector is large enough to hold the next thread
   if(threadID >= lastWriteAddress.size())
   {
      if(threadID == lastWriteAddress.size())
      {
         #if defined(DEBUG)
         std::cerr << "Synthesis::lastWriteAddress.push_back with " << threadID;
         #endif
         lastWriteAddress.push_back(currentAddress);
         #if defined(DEBUG)
         std::cerr << " and new size of " << lastWriteAddress.size() << "*" << std::endl;
         #endif
      }
      else
      {
         #if defined(DEBUG)
         std::cerr << "Synthesis::lastWriteAddress.resize with " << threadID;
         #endif
         lastWriteAddress.resize(threadID + 1);
         #if defined(DEBUG)
         std::cerr << " and new size of " << lastWriteAddress.size() << std::endl;
         #endif
      }
   }
   else
   {
      lastWriteAddress[threadID] = currentAddress;
   }
}

/**
 * @name recordMemRead
 * 
 * @param memoryAddress 
 * @param writeSize 
 * @param bbAddress 
 * @param threadID 
 * @return 
 */
void recordMemRead(VAddr memoryAddress, INT_32 writeSize, ADDRESS_INT bbAddress, THREAD_ID threadID)
{
   /* Variable Declaration */
   size_t delta = 0;
   bool shared = 0;
   ADDRESS_INT lastAddress = lastReadAddress[threadID];
   ADDRESS_INT currentAddress = memoryAddress;
   AddressMap::iterator memMapIterator;

   //graph
   graph_traits <BBGraph>::vertex_iterator vertexIterator, vertexEnd;
   basicBlock_name_map_t basicBlockLocal = get(basicBlock_t(), *myCFG[threadID]);

      /* Processes */
   //Check to see if the address is already known -- if it is then check the threadID against the stored
   //ID.  If they do not match then the address lies within the thread-shared region and should be recorded.

   //Find the distance, in bytes, from the last entry
   if(lastAddress == 0)
      delta = 0;
   else if(lastAddress > currentAddress)
      delta = (lastAddress - currentAddress);
   else
      delta = (currentAddress -lastAddress);	

// cout << "R <> Current:  " << hex << currentAddress << "  Previous:  " << lastAddress << dec << "  Delta:  " << delta << "  Size:  " << writeSize << endl;

   //based on the assumption that one cache line is 32B
   if(delta < 31)
      per_threadReadBins[threadID][0] = per_threadReadBins[threadID][0] + 1;
   else if(delta < 63)
      per_threadReadBins[threadID][1] = per_threadReadBins[threadID][1] + 1;
   else if(delta < 95)
      per_threadReadBins[threadID][2] = per_threadReadBins[threadID][2] + 1;
   else if(delta < 127)
      per_threadReadBins[threadID][3] = per_threadReadBins[threadID][3] + 1;
   else if(delta < 159)
      per_threadReadBins[threadID][4] = per_threadReadBins[threadID][4] + 1;
   else if(delta < 191)
      per_threadReadBins[threadID][5] = per_threadReadBins[threadID][5] + 1;
   else if(delta < 223)
      per_threadReadBins[threadID][6] = per_threadReadBins[threadID][6] + 1;
   else if(delta < 255)
      per_threadReadBins[threadID][7] = per_threadReadBins[threadID][7] + 1;
   else if(delta < 287)
      per_threadReadBins[threadID][8] = per_threadReadBins[threadID][8] + 1;
   else
      per_threadReadBins[threadID][9] = per_threadReadBins[threadID][9] + 1;

   //Need to ensure that the vector is large enough to hold the next thread
   if(threadID >= lastReadAddress.size())
   {
      if(threadID == lastReadAddress.size())
      {
         #if defined(DEBUG)
         std::cerr << "Synthesis::lastReadAddress.push_back with " << threadID;
         #endif
         lastReadAddress.push_back(currentAddress);
         #if defined(DEBUG)
         std::cerr << " and new size of " << lastReadAddress.size() << "*" << std::endl;
         #endif
      }
      else
      {
         #if defined(DEBUG)
         std::cerr << "Synthesis::lastReadAddress.resize with " << threadID;
         #endif
         lastReadAddress.resize(threadID + 1);
         #if defined(DEBUG)
         std::cerr << " and new size of " << lastReadAddress.size() << std::endl;
         #endif
      }
   }
   else
   {
      lastReadAddress[threadID] = currentAddress;
   }
}

/**
 * @name normalizeBins
 * 
 * @param rawBins[] 
 * @param normBins[] 
 * @param size 
 * @return 
 */
void normalizeBins(std::vector < UINT_32 > rawBins, float normBins[], UINT_32 size)
{
   UINT_32 counter = 0;
   float total = 0;

   for(counter = 0; counter < rawBins.size(); counter++)
   {
      total = total + rawBins[counter];
   }

   for(counter = 0; counter < rawBins.size(); counter++)
   {
      normBins[counter] = (float)rawBins[counter] / total;
   }

//    std::cout << "\n\nNormalized:\n" << flush;
//    for(counter = 0; counter < size; counter++)
//    {
//       std::cout << rawBins[counter] << " (" << (float)rawBins[counter] / (float)total << ")   ";
//       std::cout << normBins[counter] << "\t";
//    }
//    std::cout << "\n" << endl;
}

/**
 * @name initMemory
 * 
 * @param  threadID
 * @return 
 */
void initMemory(THREAD_ID threadID)
{
   normalizeBins(per_threadReadBins[threadID], normalizedReadBins, BIN_SIZE);
   normalizeBins(per_threadWriteBins[threadID], normalizedWriteBins, BIN_SIZE);
}

/**
 * @name returnReadStride
 * 
 * @return 
 */
UINT_32 returnReadStride()
{
   static boost::lagged_fibonacci1279 generator(static_cast<unsigned> (std::time(0)));
   boost::uniform_real<double> uniformDistribution(0, 1);
   boost::variate_generator<boost::lagged_fibonacci1279&, boost::uniform_real<double> >  uniformReal(generator, uniformDistribution);

   UINT_32 memRead = 0;
   float nextMemReference = uniformReal();

   UINT_32 total = 0;
   float normalizedRead[BIN_SIZE];
   for(UINT_32 i = 0; i < BIN_SIZE; i++)
   {
      normalizedRead[i] = normalizedReadBins[0] + total;
      total = total + (UINT_32)normalizedReadBins[0];
   }

   for(UINT_32 i = 0; i < BIN_SIZE; i++)
   {
      if(nextMemReference < normalizedRead[i])
      {
         memRead = i * 32;
//          cout << "Rand:  " << nextMemReference << "  Bin:  " << normalizedRead[i] << "  i:  " << i << "  Next Read:  " << memRead << endl;
         break;
      }
   }

   return memRead;
}

/**
 * @name returnWriteStride
 * 
 * @return 
 */
UINT_32 returnWriteStride()
{
   static boost::lagged_fibonacci1279 generator(static_cast<unsigned> (std::time(0)));
   boost::uniform_real<double> uniformDistribution(0, 1);
   boost::variate_generator<boost::lagged_fibonacci1279&, boost::uniform_real<double> >  uniformReal(generator, uniformDistribution);

   UINT_32 memWrite = 0;
   float nextMemReference = uniformReal();

   UINT_32 total = 0;
   float normalizedWrite[BIN_SIZE];
   for(UINT_32 i = 0; i < BIN_SIZE; i++)
   {
      normalizedWrite[i] = normalizedWriteBins[0] + total;
      total = total + (UINT_32)normalizedWriteBins[0];
   }

   for(UINT_32 i = 0; i < BIN_SIZE; i++)
   {
      if(nextMemReference < normalizedWrite[i])
      {
         memWrite = i * 32;
//          cout << "Rand:  " << nextMemReference << "  Bin:  " << normalizedWrite[i] << "  i:  " << i << "  Next Write:  " << memWrite << endl;
         break;
      }
   }

   return memWrite;
}

/**
 * @name returnTransReadStride
 * 
 * @param addressIn 
 * @return 
 */
UINT_32 returnTransReadStride(ADDRESS_INT addressIn)
{
   UINT_32 stride;

   stride = sharedAddressMap[addressIn];
   return stride;
}

/**
 * @name returnTransWriteStride
 * 
 * @param addressIn 
 * @return 
 */
UINT_32 returnTransWriteStride(ADDRESS_INT addressIn)
{
   UINT_32 stride;

   stride = sharedAddressMap[addressIn];
   return stride;
}

/**
 * @name buildGlobalMemoryMap
 * 
 * @param  
 * @return 
 */
void buildGlobalMemoryMap()
{
   UINT_32 offset;
   std::map< ADDRESS_INT, BOOL >::iterator foundIterator;

   //graph
   graph_traits<PCFG>::vertex_iterator vertexIterator, vertexEnd;
   graph_traits<PCFG>::vertex_iterator inner_vertexIterator, inner_vertexEnd;

   flowNode_name_map_t flowNode = get(flowNode_t(), myPCFG);
   flowName_name_map_t  nodeName  = get(vertex_name, myPCFG);
   flowIndex_name_map_t flowNodeIndex = get(vertex_index, myPCFG);
   flow_edgeWeight_name_map_t edgeWeight = get(edge_weight, myPCFG);

   //map
   BOOL unique;
   AddressMap::iterator addressIterator;
   std::map< ADDRESS_INT, UINT_32 > read_conflictMap;
   std::map< ADDRESS_INT, UINT_32 > write_conflictMap;

   std::cout << "\nBuilding Global Memory Map" << std::flush;

   //Go through the graph and build a map of all of the memory locations shared between transactions
   for(tie(vertexIterator, vertexEnd) = vertices(myPCFG); vertexIterator != vertexEnd; vertexIterator++)
   {
      if(flowNode[*vertexIterator].return_isTrans() == 1)
      {
         boost::tie(foundIterator, unique) = foundTransactions.insert(make_pair(flowNode[*vertexIterator].return_transID(), 1));

         if(unique == 1)
         {
            std::cout << "...Building Tx " << std::hex << flowNode[*vertexIterator].return_transID() << std::flush << std::dec;
            StatMemory::buildLocalMap(read_conflictMap, write_conflictMap, flowNode[*vertexIterator].return_transID());

            for(tie(inner_vertexIterator, inner_vertexEnd) = vertices(myPCFG); inner_vertexIterator != inner_vertexEnd; inner_vertexIterator++)
            {
               if(flowNode[*inner_vertexIterator].return_isTrans() == 1 && flowNode[*vertexIterator].return_threadID() != flowNode[*inner_vertexIterator].return_threadID())
               {
//                   StatMemory::getMemoryConflicts(read_conflictMap, write_conflictMap, flowNode[*inner_vertexIterator].return_transID());
               }
            }
         }
      }

      //Look through the list of potential read onflicts and add actual conflicts to the global memory map
      for(std::map< ADDRESS_INT, UINT_32 >::iterator localMapIterator = read_conflictMap.begin(); localMapIterator != read_conflictMap.end(); localMapIterator++)
      {
//          if(localMapIterator->second > 1)
         {
            boost::tie(addressIterator, unique) = sharedAddressMap.insert(make_pair(localMapIterator->first, 0));
         }
      }

      //Look through the list of potential write conflicts and add actual conflicts to the global memory map
      for(std::map< ADDRESS_INT, UINT_32 >::iterator localMapIterator = write_conflictMap.begin(); localMapIterator != write_conflictMap.end(); localMapIterator++)
      {
//          if(localMapIterator->second > 1)
         {
            boost::tie(addressIterator, unique) = sharedAddressMap.insert(make_pair(localMapIterator->first, 0));
         }
      }
   }

   //Now that we have the map, we need to determine offsets for the global map
   std::cout << "...Calculating Offsets" << std::flush;
   offset = 0;
   for(addressIterator = sharedAddressMap.begin(); addressIterator != sharedAddressMap.end(); addressIterator++)
   {
//       #if defined(DEBUG)
      std::cout << "ADDR:  " << std::hex << addressIterator->first << std::endl << std::dec;
//       #endif

      addressIterator->second = offset;
      offset = offset + 4;
   }

   std::cout << "...Finished" << std::flush;
}

void buildGlobalMemoryMap(const std::set<RAddr> &readSet, const std::set<RAddr> &writeSet)
{
   /* Variable Declaration */
   UINT_32 offset;
   BOOL unique;
   AddressMap::iterator addressIterator;

   /* Processes */
   std::cout << "\nBuilding Global Memory Map" << std::flush;

   for(std::set<RAddr>::const_iterator setIterator = readSet.begin(); setIterator != readSet.end(); setIterator++)
   {
      boost::tie(addressIterator, unique) = sharedAddressMap.insert(make_pair(*setIterator, 0));
   }

   for(std::set<RAddr>::const_iterator setIterator = writeSet.begin(); setIterator != writeSet.end(); setIterator++)
   {
      boost::tie(addressIterator, unique) = sharedAddressMap.insert(make_pair(*setIterator, 0));
   }

   //Now that we have the map, we need to determine offsets for the global map
   std::cout << "...Calculating Offsets" << std::flush;
   offset = 0;
   for(addressIterator = sharedAddressMap.begin(); addressIterator != sharedAddressMap.end(); addressIterator++)
   {
//       #if defined(DEBUG)
      std::cout << "ADDR:  " << std::hex << addressIterator->first << std::endl << std::dec;
//       #endif

      addressIterator->second = offset;
      offset = offset + 4;
   }

   std::cout << "...Finished" << std::flush;
}

/**
 * @name buildLocalMap
 * 
 * @param readConflictMap 
 * @param writeConflictMap 
 * @param transID_A 
 * @return 
 */
UINT_8 buildLocalMap(std::map< ADDRESS_INT, UINT_32 > &readConflictMap, std::map< ADDRESS_INT, UINT_32 > &writeConflictMap, ADDRESS_INT transID_A)
{
   /* Variable Declaration */
   BOOL unique;
   std::map< ADDRESS_INT, UINT_32 >::iterator localMapIterator;

   std::list < std::list < ADDRESS_INT > > *trans_A_loads  = tmReport->transMemRef_ref_getLoadLists(transID_A);
   std::list < std::list < ADDRESS_INT > > *trans_A_stores = tmReport->transMemRef_ref_getStoreLists(transID_A);

   //build read Map
   for(std::list < std::list < ADDRESS_INT > >::iterator outerIterator = trans_A_loads->begin(); outerIterator != trans_A_loads->end(); outerIterator++)
   {
      for(std::list < ADDRESS_INT >::iterator innerIterator = outerIterator->begin(); innerIterator != outerIterator->end(); innerIterator++)
      {
         boost::tie(localMapIterator, unique) = readConflictMap.insert(make_pair(*innerIterator, 0));
      }
   }

   //build write Map
   for(std::list < std::list < ADDRESS_INT > >::iterator outerIterator = trans_A_stores->begin(); outerIterator != trans_A_stores->end(); outerIterator++)
   {
      for(std::list < ADDRESS_INT >::iterator innerIterator = outerIterator->begin(); innerIterator != outerIterator->end(); innerIterator++)
      {
         boost::tie(localMapIterator, unique) = writeConflictMap.insert(make_pair(*innerIterator, 0));
      }
   }

   return 1;
}

/**
 * @name getMemoryConflicts
 * 
 * @param readConflictMap 
 * @param writeConflictMap 
 * @param transID_B 
 * @return 
 */
UINT_8 getMemoryConflicts(std::map< ADDRESS_INT, UINT_32 > &readConflictMap, std::map< ADDRESS_INT, UINT_32 > &writeConflictMap, ADDRESS_INT transID_B)
{
   BOOL unique;
   std::map< ADDRESS_INT, UINT_32 >::iterator localMapIterator;

   std::list < std::list < ADDRESS_INT > > *trans_B_loads  = tmReport->transMemRef_ref_getLoadLists(transID_B);
   std::list < std::list < ADDRESS_INT > > *trans_B_stores = tmReport->transMemRef_ref_getStoreLists(transID_B);

   //check against other read set
   for(std::list < std::list < ADDRESS_INT > >::iterator outerIterator = trans_B_loads->begin(); outerIterator != trans_B_loads->end(); outerIterator++)
   {
      for(std::list < ADDRESS_INT >::iterator innerIterator = outerIterator->begin(); innerIterator != outerIterator->end(); innerIterator++)
      {
         boost::tie(localMapIterator, unique) = readConflictMap.insert(make_pair(*innerIterator, 0));
         if(unique == 0)
            localMapIterator->second = localMapIterator->second + 1;

         boost::tie(localMapIterator, unique) = writeConflictMap.insert(make_pair(*innerIterator, 0));
         if(unique == 0)
            localMapIterator->second = localMapIterator->second + 1;
      }
   }

   //check against other write set
   for(std::list < std::list < ADDRESS_INT > >::iterator outerIterator = trans_B_stores->begin(); outerIterator != trans_B_stores->end(); outerIterator++)
   {
      for(std::list < ADDRESS_INT >::iterator innerIterator = outerIterator->begin(); innerIterator != outerIterator->end(); innerIterator++)
      {
         boost::tie(localMapIterator, unique) = readConflictMap.insert(make_pair(*innerIterator, 0));
         if(unique == 0)
            localMapIterator->second = localMapIterator->second + 1;

         boost::tie(localMapIterator, unique) = writeConflictMap.insert(make_pair(*innerIterator, 0));
         if(unique == 0)
            localMapIterator->second = localMapIterator->second + 1;
      }
   }

   return 1;
}

/**
 * @name getMemoryConflicts
 * 
 * @param readConflictMap 
 * @param writeConflictMap 
 * @param transID_A 
 * @param transID_B 
 * @return 
 */
UINT_8 getMemoryConflicts(std::map< ADDRESS_INT, UINT_32 > &readConflictMap, std::map< ADDRESS_INT, UINT_32 > &writeConflictMap, ADDRESS_INT transID_A, ADDRESS_INT transID_B)
{
   BOOL unique;
   std::map< ADDRESS_INT, UINT_32 >::iterator localMapIterator;

   std::list < std::list < ADDRESS_INT > > *trans_A_loads  = tmReport->transMemRef_ref_getLoadLists(transID_A);
   std::list < std::list < ADDRESS_INT > > *trans_A_stores = tmReport->transMemRef_ref_getStoreLists(transID_A);
   std::list < std::list < ADDRESS_INT > > *trans_B_loads  = tmReport->transMemRef_ref_getLoadLists(transID_B);
   std::list < std::list < ADDRESS_INT > > *trans_B_stores = tmReport->transMemRef_ref_getStoreLists(transID_B);

   //build read Map
   for(std::list < std::list < ADDRESS_INT > >::iterator outerIterator = trans_A_loads->begin(); outerIterator != trans_A_loads->end(); outerIterator++)
   {
      for(std::list < ADDRESS_INT >::iterator innerIterator = outerIterator->begin(); innerIterator != outerIterator->end(); innerIterator++)
      {
         boost::tie(localMapIterator, unique) = readConflictMap.insert(make_pair(*innerIterator, 0));
      }
   }

   //build write Map
   for(std::list < std::list < ADDRESS_INT > >::iterator outerIterator = trans_A_stores->begin(); outerIterator != trans_A_stores->end(); outerIterator++)
   {
      for(std::list < ADDRESS_INT >::iterator innerIterator = outerIterator->begin(); innerIterator != outerIterator->end(); innerIterator++)
      {
         boost::tie(localMapIterator, unique) = writeConflictMap.insert(make_pair(*innerIterator, 0));
      }
   }

   //check against other read set
   for(std::list < std::list < ADDRESS_INT > >::iterator outerIterator = trans_B_loads->begin(); outerIterator != trans_B_loads->end(); outerIterator++)
   {
      for(std::list < ADDRESS_INT >::iterator innerIterator = outerIterator->begin(); innerIterator != outerIterator->end(); innerIterator++)
      {
         boost::tie(localMapIterator, unique) = readConflictMap.insert(make_pair(*innerIterator, 0));
         if(unique == 0)
            localMapIterator->second = localMapIterator->second + 1;

         boost::tie(localMapIterator, unique) = writeConflictMap.insert(make_pair(*innerIterator, 0));
         if(unique == 0)
            localMapIterator->second = localMapIterator->second + 1;
      }
   }

   //check against other write set
   for(std::list < std::list < ADDRESS_INT > >::iterator outerIterator = trans_B_stores->begin(); outerIterator != trans_B_stores->end(); outerIterator++)
   {
      for(std::list < ADDRESS_INT >::iterator innerIterator = outerIterator->begin(); innerIterator != outerIterator->end(); innerIterator++)
      {
         boost::tie(localMapIterator, unique) = readConflictMap.insert(make_pair(*innerIterator, 0));
         if(unique == 0)
            localMapIterator->second = localMapIterator->second + 1;

         boost::tie(localMapIterator, unique) = writeConflictMap.insert(make_pair(*innerIterator, 0));
         if(unique == 0)
            localMapIterator->second = localMapIterator->second + 1;
      }
   }

   #if defined(DEBUG)
   for(localMapIterator = readConflictMap.begin(); localMapIterator != readConflictMap.end(); localMapIterator++)
      std::cout << "R-ADDR:  " << std::hex << localMapIterator->first <<std::dec << "\n";

   for(localMapIterator = writeConflictMap.begin(); localMapIterator != writeConflictMap.end(); localMapIterator++)
      std::cout << "W-ADDR:  " << std::hex << localMapIterator->first <<std::dec << "\n";
   #endif

   return 1;
}


}              //--StatMemory
