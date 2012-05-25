//
// C++ Implementation: graphManipulation
//
// Description: 
//
//
/// @author: hughes  <hughes@fraidy2-uf>, (C) 2008
///
/// @date:          01/11/08
/// Last Modified:  02/05/08
//
// Copyright: See COPYING file that comes with this distribution
// Pourquoi tu manges mes canards?
//
//

#include "graphManipulation.h"

//Hax for scaling problems enable-1 disable-0 (suggested by Fat Albert, woof-woof)
#define JUNKYARD

#if defined(JUNKYARD)
#define ACC_MAX 70.0
#define SEQUENTIAL 0
#endif

#define REP_COUNT 25

extern AddressMap uniqueBBMap;
extern std::vector < BasicBlock* > currBB;

extern UINT_32 totalNumThreads;

// SFG
extern std::deque  < BBGraph * > myCFG;                                  //SFG graph container, type BBGraph
extern std::vector < BBVertex > myCFG_VertexA;                           //used to ID the vertices
extern std::vector < BBVertex > myCFG_VertexB;                           //used to ID the vertices
extern std::vector < BBVertexMap > vertexMap;                            //map container, type BBVertexMap

// PCFG
extern PCFG myPCFG;                                                      //PCFG graph container, type PCFG
extern FlowVertex myPCFG_VertexA;                                        //used to ID the vertices
extern std::vector < FlowVertex > lastInsertedNode;                      //used to ID the vertices

namespace Synthesis
{
   extern StatPaths statPaths;
}

namespace CodeGenerator
{
   extern std::map< ADDRESS_INT, THREAD_ID > threadFuncMap;
}

namespace GraphManipulation
{
ofstream uniqueBBOutputFile("/home/hughes/Benchies/MIPS/asmTesting/raw/unique.out", ios::trunc);         //open a file for writing (truncate the current contents)


/**
 * @name generateSFGNodeIDs
 * 
**/
void generateSFGNodeIDs()
{
   UINT_32 numThreads = totalNumThreads;

   std::cout << "\nGenerating SFG Node IDs" << std::flush;
   for(UINT_32 threadCounter = 0; threadCounter < numThreads; threadCounter++)
   {
      std::cout << "..." << threadCounter << std::flush;

      graph_traits <BBGraph>::vertex_iterator vertexIterator, vertexEnd;
      nodeIndex_name_map_t nodeIndex = get(vertex_index, *myCFG[threadCounter]);
      basicBlock_name_map_t basicBlockLocal = get(basicBlock_t(), *myCFG[threadCounter]);
      nodeName_name_map_t nodeName = get(vertex_name, *myCFG[threadCounter]);

      UINT_32 c = 0;
      for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadCounter]);vertexIterator != vertexEnd; ++vertexIterator, ++c)
      {
         nodeIndex[*vertexIterator] = c;        //numbering for internal ID
         nodeName[*vertexIterator] = "0x" + HexToString(basicBlockLocal[*vertexIterator].return_bbAddress());       //external ID
      }
   }

   std::cout << "...Finished" << std::flush;
}

/**
 * @name writeSFGDots
 * 
 * @param name 
 * @return 
**/
void writeSFGDots(string name)
{
   ConfObject *statConf = new ConfObject;
   UINT_32 numThreads = totalNumThreads;
   INT_32 rSize = statConf->return_reductionFactor();
   UINT_32 threadCounter = 0;

   nodeName_name_map_t nodeName = get(vertex_name, *myCFG[threadCounter]);

   std::cout << "\nWriting SFG Dot File(s)" << std::flush;
   for(threadCounter = 0; threadCounter < numThreads; threadCounter++)
   {
      std::cout << "..." << threadCounter << std::flush;

      string fileName = Synthesis::statPaths.return_rootDirectory() + Synthesis::statPaths.return_dataDirectory() + Synthesis::statPaths.return_outputFileName() + ".";
      fileName = fileName + name + ".SFG-" + IntToString(threadCounter) + ".graphViz";
      ofstream outputFile(fileName.c_str(), ios::trunc);      //open a file for writing (truncate the current contents)
      if ( !outputFile )  //check to be sure file is open
         std::cerr << "Error opening file.";

      write_graphviz(outputFile, *myCFG[threadCounter], make_label_writer(nodeName));
      outputFile.close();
   }

   delete statConf;

   std::cout << "...Finished" << std::flush;
}

/**
 * @name updateGraph 
 *
 * @param basicBlockIn 
 * @param threadID 
 * @return 
   This function is used to update the graph.  It checks to see if a node is present, using the map.
   If not, it inserts it into the graph and inserts an edge between it and its parent.  If the node
   was already present, it simply increments the basic block and edge counts (these are used later).
**/
void updateGraph(BasicBlock *basicBlockIn, THREAD_ID threadID)
{
   /* Variable Declaration */
   BOOL found;
   BOOL inserted;
   BOOL unique;
   ConfObject *statConf = new ConfObject;
   ADDRESS_INT basicBlockAddress;
   BasicBlock localBBObject;
   BBVertexMap::iterator masterMapIterator;
   AddressMap::iterator bbMapIterator;

   graph_traits <BBGraph>::edge_descriptor edgeDesc;
   basicBlock_name_map_t basicBlock = get(basicBlock_t(), *myCFG[threadID]);
   edgeWeight_name_map_t edgeWeight = get(edge_weight, *myCFG[threadID]);

   /* Processes */
   basicBlockAddress = (ADDRESS_INT)basicBlockIn->return_front_of_instructionList().return_instructionID();
   localBBObject = *basicBlockIn;

   boost::tie(bbMapIterator, unique) = uniqueBBMap.insert(make_pair(basicBlockAddress, 1));
   if(unique == 1)
   {
      if(statConf->return_debugUniqueBB() == 1 || statConf->return_debugAll() == 1)
      {
         uniqueBBOutputFile << std::hex << basicBlockAddress << std::endl;
         while(currBB[threadID]->return_instructionListSize() > 0)
         {
            InstructionContainer temp_instruction = currBB[threadID]->return_front_of_instructionList_pop();

            uniqueBBOutputFile << temp_instruction.return_instructionID() << "::";
            uniqueBBOutputFile << Instruction::opcode2Name(temp_instruction.return_opCode()) << "-";
            uniqueBBOutputFile << Instruction::subCode_to_Name(temp_instruction.return_subCode());
            uniqueBBOutputFile << "\n";
         }
         uniqueBBOutputFile << endl;
      }
   }

   if(unique != 1 && bbMapIterator->second != threadID)
   {
      localBBObject.update_isShared(1);
      //cout << "Address-  " << bbAddress << "   Current-  " << threadID << "   Owner-  " << bbMapIterator->second << endl;
   }

   tie(masterMapIterator, inserted) = vertexMap[threadID].insert(make_pair(basicBlockAddress, BBVertex()));
   //If the address was not in the map, that means there was no vertex associated with it
   if(inserted == 1)
   {
      localBBObject.update_bbAddress(basicBlockAddress);
      localBBObject.update_bbCount(1);                            //set count to one, only stored if first instance

      myCFG_VertexA[threadID] = add_vertex(*myCFG[threadID]);     //place the node
      basicBlock[myCFG_VertexA[threadID]] = localBBObject;        //assign the value

      //need to check corner case (is there another node to link it to?)
      if(num_vertices(*myCFG[threadID]) > 1)
      {
         //create an edge for the new node if it isn't the first node of the graph			
         tie(edgeDesc, inserted) = add_edge(myCFG_VertexB[threadID], myCFG_VertexA[threadID], *myCFG[threadID]);
         if(inserted)
         {
            edgeWeight[edgeDesc] = 1;
         }
      }

      myCFG_VertexB[threadID] = myCFG_VertexA[threadID];          //set up for next iteration

      masterMapIterator->second = myCFG_VertexA[threadID];        //assign the correct vertex to the map

   }
   //NOTE Even if the node exists, we need to keep track of all synch events and need to insert another node
   //  ------REASSIGNING THE NODE IN THE MAP MAY PRODUCE ERRONEOUS RESULTS------
   else if(localBBObject.return_isThreadEvent())
   {
      localBBObject.update_bbAddress(basicBlockAddress);
      localBBObject.update_bbCount(1);                                 //set count to one, only stored if first instance

      myCFG_VertexA[threadID] = add_vertex(*myCFG[threadID]);                //place the node

      basicBlock[myCFG_VertexA[threadID]] = localBBObject;                  //assign the value

      //TODO Need to check corner case (is there another node to link it to?)
      if(num_vertices(*myCFG[threadID]) > 1)
      {
         //create an edge for the new node if it isn't the first node of the graph			
         tie(edgeDesc, inserted) = add_edge(myCFG_VertexB[threadID], myCFG_VertexA[threadID], *myCFG[threadID]);
         if(inserted)
         {
            edgeWeight[edgeDesc] = 1;
         }
      }

      myCFG_VertexB[threadID] = myCFG_VertexA[threadID];									//set up for next iteration

      masterMapIterator->second = myCFG_VertexA[threadID];				//assign the correct vertex to the map

   }
   else
   {
      myCFG_VertexA[threadID] = masterMapIterator->second;                                                        //the node exists, so we just need to point to it to set up edge

      basicBlock[myCFG_VertexA[threadID]].update_bbCount(basicBlock[myCFG_VertexA[threadID]].return_bbCount() + 1);     //increment the count

      if(localBBObject.return_isWait() == 1)
      {
         basicBlock[myCFG_VertexA[threadID]].update_isWait(1);
      }

      tie(edgeDesc, found) = edge(myCFG_VertexB[threadID], myCFG_VertexA[threadID], *myCFG[threadID]);                   //now check to see if an edge already exists
      if(found)
      {
         edgeWeight[edgeDesc] = edgeWeight[edgeDesc] + 1;
      }
      else
      {
         tie(edgeDesc, inserted) = add_edge(myCFG_VertexB[threadID], myCFG_VertexA[threadID], *myCFG[threadID]);
         if(inserted)
         {
            edgeWeight[edgeDesc] = 1;
         }
      }

      myCFG_VertexB[threadID] = myCFG_VertexA[threadID];
   }

   localBBObject.update_isSpawn(0);                                 //only set at thread generation
   localBBObject.update_isDestroy(0);                               //only set at thread generation

   delete statConf;
}//---------------------------------------------------------------------	// End updateGraph //

/**
 * @name getBasicBlockSize
 * 
 * @param threadID 
 * @param totalInstructions 
 * @return 
 */
UINT_32 getBasicBlockSize(THREAD_ID threadID, UINT_32 totalInstructions)
{
   /* Variables */
   UINT_32 BBCount = 0;
   graph_traits <BBGraph>::vertex_iterator vertexIterator, vertexEnd;
   basicBlock_name_map_t basicBlockLocal = get(basicBlock_t(), *myCFG[threadID]);

   /* Processes */
   for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]); vertexIterator != vertexEnd; vertexIterator++)
   {
      BBCount = BBCount + basicBlockLocal[*vertexIterator].return_bbCount();
   }

   return totalInstructions / BBCount;
}

/**
 * @name reduceSFG 
 * 
 * @short This function performs a reduction based on the k-value set at initialization.
 * @return 
 * @note  Transactions and critical sections are kept intact.
**/
void reduceSFG()
{
   /* Variable Declaration */
   ConfObject *statConf = new ConfObject;
   float BBCount;
   UINT_32 numThreads = totalNumThreads;
   UINT_64 reductionFactor = (UINT_32)statConf->return_reductionFactor();
   graph_traits <BBGraph>::vertex_iterator vertexIterator, vertexStart, vertexEnd, nextVertex;

   /* Processes */
   cout << "\nReducing SFG with R = " << reductionFactor << flush;
   for(UINT_32 counter = 0; counter < numThreads; counter++)
   {
      cout << "..." << counter << flush;
      basicBlock_name_map_t basicBlockLocal = get(basicBlock_t(), *myCFG[counter]);

      tie(vertexIterator, vertexEnd) = vertices(*myCFG[counter]);
      ++vertexIterator;
      for(nextVertex = vertexIterator; vertexIterator != vertexEnd; vertexIterator = nextVertex)
      {
         ++nextVertex;

         BBCount = (float)basicBlockLocal[*vertexIterator].return_bbCount() / (float)reductionFactor;

         //If the basic block is not a critical section or only contains a branch instruction, remove it
         if(basicBlockLocal[*vertexIterator].return_isTrans() != 1 && basicBlockLocal[*vertexIterator].return_isCritical() != 1 && basicBlockLocal[*vertexIterator].return_isSpawn() != 1)
         {
            if(BBCount < 1.0)
            {
               clear_vertex(*vertexIterator, *myCFG[counter]);                      //clear all edges
               remove_vertex(*vertexIterator, *myCFG[counter]);                     //plop
            }
            else if(basicBlockLocal[*vertexIterator].return_instructionListSize() <= 1)
            {
               clear_vertex(*vertexIterator, *myCFG[counter]);                      //clear all edges
               remove_vertex(*vertexIterator, *myCFG[counter]);                     //plop
            }
            else
            {
               basicBlockLocal[*vertexIterator].update_bbCount((UINT_32)BBCount);   //set new count
            }
         }
         else
         {
            //Even though we didn't remove the node, it still needs a number <= 1
            if(BBCount < 1.0)
            {
               basicBlockLocal[*vertexIterator].update_bbCount(1);                  //set new count
            }
            else
            {
               basicBlockLocal[*vertexIterator].update_bbCount((UINT_32)BBCount);   //set new count
            }
         }
      }//end for

      //Now we want to see if the thread is comprised *solely* of critical sections
      BOOL clearGraph  = 0;
      BOOL isCritical  = 0;
      BOOL wasCritical = 1;
      for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[counter]); vertexIterator != vertexEnd; vertexIterator++)
      {
         if(basicBlockLocal[*vertexIterator].return_isTrans() != 1 && basicBlockLocal[*vertexIterator].return_isCritical() != 1)
         {
            isCritical = 0;
         }
         else
         {
            isCritical = 1;
         }

         if(wasCritical == 1 && isCritical == 0)
         {
            clearGraph = 0;
            break;
         }
         else if(basicBlockLocal[*vertexIterator].return_bbCount() >= 25)
         {
            clearGraph = 0;
            break;
         }
         else
         {
            clearGraph = 1;
         }
      }//end for

      if(clearGraph == 1 || num_vertices(*myCFG[counter]) < 2)
      {
         delete myCFG[counter];
         myCFG[counter] = new BBGraph();
      }
   }

   delete statConf;

   std::cout << "...Finished" << std::flush;
}//---------------------------------------------------------------------	// End reduceSFG //

/**
 * @name walkSFG 
 * 
 * @param threadID 
 * @param syntheticThreads[] 
 * @param arraySize 
 * @return 
 */
void walkSFG(THREAD_ID threadID, Synthetic *syntheticThreads[], UINT_32 arraySize)
{
   /* Variable Declaraion */
   ConfObject *statConf = new ConfObject;
   float edgeTransit = 0;
   UINT_32 bbcount_out = 0;

   UINT_32 maxBB = statConf->return_maxBasicBlocks();

   //graph
   graph_traits <BBGraph>::vertex_iterator tempVertexIterator;
   graph_traits <BBGraph>::vertex_iterator vertexIterator, vertexEnd;
   graph_traits <BBGraph>::out_edge_iterator outEdgeIterator, outEdgeEnd;
   basicBlock_name_map_t basicBlockLocal = get(basicBlock_t(), *myCFG[threadID]);
   edgeWeight_name_map_t edgeWeight = get(edge_weight, *myCFG[threadID]);
   nodeIndex_name_map_t nodeIndex = get(vertex_index, *myCFG[threadID]);

   Synthetic *tempSynth = new Synthetic;

   //initialize uniform RV over [0,1)
   static boost::lagged_fibonacci1279 generator(static_cast<unsigned> (std::time(0)));
   boost::uniform_real<double> uniformDistribution(0, 1);
   boost::variate_generator<boost::lagged_fibonacci1279&, boost::uniform_real<double> >  uniformReal(generator, uniformDistribution);

   /* Processes */
   do
   {
      for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]); vertexIterator != vertexEnd; )
      {
         edgeTransit = uniformReal();

         //If the node count reaches zero, it is ignored.
         if(basicBlockLocal[*vertexIterator].return_bbCount() < 1)
         {
            tempVertexIterator = vertexIterator;
            ++vertexIterator;

            #ifdef DEBUG_ME
            //cout << "\nPopping  " << *tempVertexIterator << "    " << basicBlockLocal[*tempVertexIterator].return_bbAddress() << flush;
            #endif

//             clear_vertex(*tempVertexIterator, *myCFG[threadID]);                 //clear all edges
//             remove_vertex(*tempVertexIterator, *myCFG[threadID]);                //plop
         }
         else
         {
            ++bbcount_out;
            tempSynth->update_coreList(basicBlockLocal[*vertexIterator]);

//             if(num_vertices(*myCFG[threadID]) > 1)
//                basicBlockLocal[*vertexIterator].update_bbCount(basicBlockLocal[*vertexIterator].return_bbCount() - 1);

            UINT_32 repCount = 0;
            for(tie(outEdgeIterator, outEdgeEnd) = out_edges(*vertexIterator, *myCFG[threadID]); outEdgeIterator != outEdgeEnd; ++outEdgeIterator)
            {
               if(basicBlockLocal[target(*outEdgeIterator, *myCFG[threadID])] == basicBlockLocal[*vertexIterator] && repCount < REP_COUNT)
               {
                  repCount = repCount + 1;
               }
               else if((edgeWeight[*outEdgeIterator] >= edgeTransit && basicBlockLocal[target(*outEdgeIterator, *myCFG[threadID])].return_bbCount() > 0) || basicBlockLocal[target(*outEdgeIterator, *myCFG[threadID])].return_isSpawn() == 1)
               {
                  //need to point the current iterator to the next one based on the target node
                  BBVertex nextVertex = target(*outEdgeIterator, *myCFG[threadID]);
                  for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]); *vertexIterator != nextVertex; ++vertexIterator)
                  {
                  }

                  break;
               }
            }
         }/// --End Else

         //We want to stop writing out to the synthetic once we reach the
         //maximum number of basic blocks defined by the user.
         if(bbcount_out > maxBB)
            break;

      }/// --End Inner-For

   }while(bbcount_out < maxBB && num_vertices(*myCFG[threadID]) > 0);

   syntheticThreads[threadID] = tempSynth;
   delete statConf;
}//---------------------------------------------------------------------	// End walkSFG //

/**
 * @name walkSFG 
 * 
 * @param threadID 
 * @param tempSynth 
 * @param numInstructions 
 * @return 
 */
float walkSFG(THREAD_ID threadID, Synthetic *tempSynth, float numInstructions)
{
   /* Variable Declaraion */
   ConfObject *statConf = new ConfObject;
   UINT_32 iterations = 0;
   float edgeTransit = 0;
   UINT_32 bbcount_out = 0;
   float instructions_out = 0;
   float maxInstructions = ceil(std::max((float)ACC_MAX, numInstructions));

   UINT_32 maxBB = statConf->return_maxBasicBlocks();

   //graph
   graph_traits <BBGraph>::vertex_iterator tempVertexIterator;
   graph_traits <BBGraph>::vertex_iterator vertexIterator, vertexEnd;
   graph_traits <BBGraph>::out_edge_iterator outEdgeIterator, outEdgeEnd;
   basicBlock_name_map_t basicBlockLocal = get(basicBlock_t(), *myCFG[threadID]);
   edgeWeight_name_map_t edgeWeight = get(edge_weight, *myCFG[threadID]);
   nodeIndex_name_map_t nodeIndex = get(vertex_index, *myCFG[threadID]);

   //initialize uniform RV over [0,1)
   static boost::lagged_fibonacci1279 generator(static_cast<unsigned> (std::time(0)));
   boost::uniform_real<double> uniformDistribution(0, 1);
   boost::variate_generator<boost::lagged_fibonacci1279&, boost::uniform_real<double> >  uniformReal(generator, uniformDistribution);

   /* Processes */
   do
   {
      for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]); vertexIterator != vertexEnd; )
      {
         edgeTransit = uniformReal();

         //If the node count reaches zero, it is removed from consideration
         //If the basic block contains a single instruction (must be a branch), it is removed from consideration
         if(basicBlockLocal[*vertexIterator].return_bbCount() < 1)
         {
            tempVertexIterator = vertexIterator;
            ++vertexIterator;

            #ifdef DEBUG_ME
            //cout << "\nPopping  " << *tempVertexIterator << "    " << basicBlockLocal[*tempVertexIterator].return_bbAddress() << flush;
            #endif

//             clear_vertex(*tempVertexIterator, *myCFG[threadID]);                 //clear all edges
//             remove_vertex(*tempVertexIterator, *myCFG[threadID]);                //plop
         }
         else if(basicBlockLocal[*vertexIterator].return_instructionListSize() <= 1)
         {
            tempVertexIterator = vertexIterator;
            ++vertexIterator;

            #ifdef DEBUG_ME
            //cout << "\nPopping  " << *tempVertexIterator << "    " << basicBlockLocal[*tempVertexIterator].return_bbAddress() << flush;
            #endif

//             clear_vertex(*tempVertexIterator, *myCFG[threadID]);                 //clear all edges
//             remove_vertex(*tempVertexIterator, *myCFG[threadID]);                //plop
         }
         else if(basicBlockLocal[*vertexIterator].return_isSpawn() == 1)
         {
            //do nothing
            ++vertexIterator;
         }
         else
         {
//             std::cout << "+(" << iterations << ")MAX:  " << maxInstructions << "\tout:  " << instructions_out << "\tsize:  " << basicBlockLocal[*vertexIterator].return_instructionListSize() << std::endl;
            if((float)basicBlockLocal[*vertexIterator].return_instructionListSize() <= (maxInstructions - instructions_out))
            {
               iterations = 0;
               bbcount_out = bbcount_out + 1;
               instructions_out = instructions_out + (float)basicBlockLocal[*vertexIterator].return_instructionListSize();

               //If this basic block has special flags set, they need to be reset before being added to the synthetic stream
               if(basicBlockLocal[*vertexIterator].return_isCritical() == 1)
               {
                  BasicBlock temp_1 = basicBlockLocal[*vertexIterator];
                  temp_1.update_isCritical(0);
                  temp_1.update_isThreadEvent(0);
                  tempSynth->update_coreList(temp_1);
               }
               else if(basicBlockLocal[*vertexIterator].return_isTrans() == 1)
               {
                  BasicBlock temp_1 = basicBlockLocal[*vertexIterator];
                  temp_1.update_isTrans(0);
                  temp_1.update_isThreadEvent(0);
                  tempSynth->update_coreList(temp_1);
               }
               else if(basicBlockLocal[*vertexIterator].return_isWait() == 1)
               {
                  BasicBlock temp_1 = basicBlockLocal[*vertexIterator];
                  temp_1.update_isWait(0);
                  temp_1.update_isThreadEvent(0);
                  tempSynth->update_coreList(temp_1);
               }
               else if(basicBlockLocal[*vertexIterator].return_isBarrier() == 1)
               {
                  BasicBlock temp_1 = basicBlockLocal[*vertexIterator];
                  temp_1.update_isBarrier(0);
                  temp_1.update_isThreadEvent(0);
                  tempSynth->update_coreList(temp_1);
               }
               else
               {
                  tempSynth->update_coreList(basicBlockLocal[*vertexIterator]);
               }

//                if(num_vertices(*myCFG[threadID]) > 1)
//                   basicBlockLocal[*vertexIterator].update_bbCount(basicBlockLocal[*vertexIterator].return_bbCount() - 1);
            }
            else if(iterations >= 25 && (float)basicBlockLocal[*vertexIterator].return_instructionListSize() <= (maxInstructions - instructions_out + 4.0))
            {
               iterations = 0;

               BasicBlock temp_1 = basicBlockLocal[*vertexIterator];
               temp_1.update_isBarrier(0);
               temp_1.update_isWait(0);
               temp_1.update_isTrans(0);
               temp_1.update_isCritical(0);
               temp_1.update_isThreadEvent(0);

               temp_1.resize_instructionList((UINT_32)maxInstructions - (UINT_32)instructions_out);
               tempSynth->update_coreList(temp_1);

               bbcount_out = bbcount_out + 1;
               instructions_out = instructions_out + (float)temp_1.return_instructionListSize();
            }
            else if(iterations >= 50)
            {
               iterations = 0;

               BasicBlock temp_1 = basicBlockLocal[*vertexIterator];
               temp_1.update_isBarrier(0);
               temp_1.update_isWait(0);
               temp_1.update_isTrans(0);
               temp_1.update_isCritical(0);
               temp_1.update_isThreadEvent(0);

               if((UINT_32)maxInstructions - (UINT_32)instructions_out <= 1.0)
                  temp_1.resize_instructionList(2);
               else
                  temp_1.resize_instructionList((UINT_32)maxInstructions - (UINT_32)instructions_out);

               tempSynth->update_coreList(temp_1);

               bbcount_out = bbcount_out + 1;
               instructions_out = instructions_out + (float)temp_1.return_instructionListSize();
            }
            else
            {
               iterations = iterations + 1;
            }

            UINT_32 repCount = 0;
            for(tie(outEdgeIterator, outEdgeEnd) = out_edges(*vertexIterator, *myCFG[threadID]); outEdgeIterator != outEdgeEnd; ++outEdgeIterator)
            {
               if(basicBlockLocal[target(*outEdgeIterator, *myCFG[threadID])] == basicBlockLocal[*vertexIterator] && repCount < REP_COUNT)
               {
                  repCount = repCount + 1;
               }
               else if(edgeWeight[*outEdgeIterator] >= edgeTransit && basicBlockLocal[target(*outEdgeIterator, *myCFG[threadID])].return_bbCount() > 0)
               {
                  //need to point the current iterator to the next one based on the target node
                  BBVertex nextVertex = target(*outEdgeIterator, *myCFG[threadID]);
                  for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]); *vertexIterator != nextVertex; ++vertexIterator)
                  {
                  }

                  break;
               }
            }
         }/// --End Else

         //We want to stop writing out to the synthetic once we reach the
         //maximum number of basic blocks defined by the user.
         if(bbcount_out > maxBB)
            break;

         if(instructions_out > numInstructions)
            break;

      }/// --End Inner-For

   }while(num_vertices(*myCFG[threadID]) > 0 && instructions_out < numInstructions && bbcount_out < maxBB);

   #if defined(DEBUG)
   std::cout << "+Added " << instructions_out << " to T" << threadID << "  with weight of " << numInstructions << std::endl;
   #endif

   delete statConf;

   return instructions_out;
}//---------------------------------------------------------------------	// End walkSFG //

/**
 * @name walkSFG 
 * 
 * @param threadID 
 * @param startPC 
 * @param tempSynth 
 * @param numInstructions 
 * @return 
 */
float walkSFG(THREAD_ID threadID, ADDRESS_INT startPC, Synthetic *tempSynth, float numInstructions)
{
   /* Variable Declaraion */
   ConfObject *statConf = new ConfObject;
   float edgeTransit = 0;
   UINT_32 bbcount_out = 0;
   float instructions_out = 0;
   UINT_32 iterations = 0;
   float maxInstructions = ceil(std::max((float)ACC_MAX, numInstructions));

   UINT_32 maxBB = statConf->return_maxBasicBlocks();

   //graph
   graph_traits <BBGraph>::vertex_iterator tempVertexIterator;
   graph_traits <BBGraph>::vertex_iterator vertexIterator, vertexEnd;
   graph_traits <BBGraph>::out_edge_iterator outEdgeIterator, outEdgeEnd;
   basicBlock_name_map_t basicBlockLocal = get(basicBlock_t(), *myCFG[threadID]);
   edgeWeight_name_map_t edgeWeight = get(edge_weight, *myCFG[threadID]);
   nodeIndex_name_map_t nodeIndex = get(vertex_index, *myCFG[threadID]);

   //initialize uniform RV over [0,1)
   static boost::lagged_fibonacci1279 generator(static_cast<unsigned> (std::time(0)));
   boost::uniform_real<double> uniformDistribution(0, 1);
   boost::variate_generator<boost::lagged_fibonacci1279&, boost::uniform_real<double> >  uniformReal(generator, uniformDistribution);

   /* Processes */
   for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]); vertexIterator != vertexEnd; vertexIterator++)
   {
      if(basicBlockLocal[*vertexIterator].return_bbAddress() == startPC)
         break;
   }

   do
   {
      if(vertexIterator == vertexEnd)
         tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]);

      while(vertexIterator != vertexEnd)
      {
         edgeTransit = uniformReal();

         //If the node count reaches zero, it is removed from consideration
         //If the basic block contains a single instruction (must be a branch), it is removed from consideration
         if(basicBlockLocal[*vertexIterator].return_bbCount() < 1)
         {
            tempVertexIterator = vertexIterator;
            ++vertexIterator;

            #ifdef DEBUG_ME
            //cout << "\nPopping  " << *tempVertexIterator << "    " << basicBlockLocal[*tempVertexIterator].return_bbAddress() << flush;
            #endif

//             clear_vertex(*tempVertexIterator, *myCFG[threadID]);                 //clear all edges
//             remove_vertex(*tempVertexIterator, *myCFG[threadID]);                //plop
         }
         else if(basicBlockLocal[*vertexIterator].return_instructionListSize() <= 1)
         {
            tempVertexIterator = vertexIterator;
            ++vertexIterator;

            #ifdef DEBUG_ME
            //cout << "\nPopping  " << *tempVertexIterator << "    " << basicBlockLocal[*tempVertexIterator].return_bbAddress() << flush;
            #endif

//             clear_vertex(*tempVertexIterator, *myCFG[threadID]);                 //clear all edges
//             remove_vertex(*tempVertexIterator, *myCFG[threadID]);                //plop
         }
         else if(basicBlockLocal[*vertexIterator].return_isSpawn() == 1)
         {
            //do nothing
            ++vertexIterator;
         }
         else
         {
//             std::cout << "*(" << iterations << ")MAX:  " << maxInstructions << "\tout:  " << instructions_out << "\tsize:  " << basicBlockLocal[*vertexIterator].return_instructionListSize() << std::endl;
            if((float)basicBlockLocal[*vertexIterator].return_instructionListSize() <= (maxInstructions - instructions_out))
            {
               iterations = 0;
               bbcount_out = bbcount_out + 1;
               instructions_out = instructions_out + (float)basicBlockLocal[*vertexIterator].return_instructionListSize();

               //If this basic block has special flags set, they need to be reset before being added to the synthetic stream
               if(basicBlockLocal[*vertexIterator].return_isCritical() == 1)
               {
                  BasicBlock temp_1 = basicBlockLocal[*vertexIterator];
                  temp_1.update_isCritical(0);
                  temp_1.update_isThreadEvent(0);
                  tempSynth->update_coreList(temp_1);
               }
               else if(basicBlockLocal[*vertexIterator].return_isTrans() == 1)
               {
                  BasicBlock temp_1 = basicBlockLocal[*vertexIterator];
                  temp_1.update_isTrans(0);
                  temp_1.update_isThreadEvent(0);
                  tempSynth->update_coreList(temp_1);
               }
               else if(basicBlockLocal[*vertexIterator].return_isWait() == 1)
               {
                  BasicBlock temp_1 = basicBlockLocal[*vertexIterator];
                  temp_1.update_isWait(0);
                  temp_1.update_isThreadEvent(0);
                  tempSynth->update_coreList(temp_1);
               }
               else if(basicBlockLocal[*vertexIterator].return_isBarrier() == 1)
               {
                  BasicBlock temp_1 = basicBlockLocal[*vertexIterator];
                  temp_1.update_isBarrier(0);
                  temp_1.update_isThreadEvent(0);
                  tempSynth->update_coreList(temp_1);
               }
               else
               {
                  tempSynth->update_coreList(basicBlockLocal[*vertexIterator]);
               }

//                if(num_vertices(*myCFG[threadID]) > 1)
//                   basicBlockLocal[*vertexIterator].update_bbCount(basicBlockLocal[*vertexIterator].return_bbCount() - 1);
            }
            else if(iterations >= 25 && (float)basicBlockLocal[*vertexIterator].return_instructionListSize() <= (maxInstructions - instructions_out + 4.0 ))
            {
               iterations = 0;

               BasicBlock temp_1 = basicBlockLocal[*vertexIterator];
               temp_1.update_isBarrier(0);
               temp_1.update_isWait(0);
               temp_1.update_isTrans(0);
               temp_1.update_isCritical(0);
               temp_1.update_isThreadEvent(0);

               if((UINT_32)maxInstructions - (UINT_32)instructions_out <= 1.0)
                  temp_1.resize_instructionList(2);
               else
                  temp_1.resize_instructionList((UINT_32)maxInstructions - (UINT_32)instructions_out);

               tempSynth->update_coreList(temp_1);

               bbcount_out = bbcount_out + 1;
               instructions_out = instructions_out + (float)temp_1.return_instructionListSize();
            }
            else if(iterations >= 50)
            {
               iterations = 0;

               BasicBlock temp_1 = basicBlockLocal[*vertexIterator];
               temp_1.update_isBarrier(0);
               temp_1.update_isWait(0);
               temp_1.update_isTrans(0);
               temp_1.update_isCritical(0);
               temp_1.update_isThreadEvent(0);

               if((UINT_32)maxInstructions - (UINT_32)instructions_out <= 1.0)
                  temp_1.resize_instructionList(2);
               else
                  temp_1.resize_instructionList((UINT_32)maxInstructions - (UINT_32)instructions_out);

               tempSynth->update_coreList(temp_1);

               bbcount_out = bbcount_out + 1;
               instructions_out = instructions_out + (float)temp_1.return_instructionListSize();
            }
            else
            {
               iterations = iterations + 1;
            }

            UINT_32 repCount = 0;
            for(tie(outEdgeIterator, outEdgeEnd) = out_edges(*vertexIterator, *myCFG[threadID]); outEdgeIterator != outEdgeEnd; ++outEdgeIterator)
            {
               if(basicBlockLocal[target(*outEdgeIterator, *myCFG[threadID])] == basicBlockLocal[*vertexIterator] && repCount < REP_COUNT)
               {
                  repCount = repCount + 1;
               }
               else if(edgeWeight[*outEdgeIterator] >= edgeTransit && basicBlockLocal[target(*outEdgeIterator, *myCFG[threadID])].return_bbCount() > 0)
               {
                  //need to point the current iterator to the next one based on the target node
                  BBVertex nextVertex = target(*outEdgeIterator, *myCFG[threadID]);
                  for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]); *vertexIterator != nextVertex; ++vertexIterator)
                  {
                  }

                  break;
               }
            }
         }/// --End Else

         //We want to stop writing out to the synthetic once we reach the
         //maximum number of basic blocks defined by the user.
         if(bbcount_out > maxBB)
            break;

         if(instructions_out > numInstructions)
            break;

         if((numInstructions - instructions_out) < 1.0)
            break;

      }/// --End Inner-For

   }while(num_vertices(*myCFG[threadID]) > 0 && instructions_out < numInstructions && (numInstructions - instructions_out) >= 1.0 && bbcount_out < maxBB);

   #if defined(DEBUG)
   std::cout << "*Added " << instructions_out << " to T" << threadID << "  with weight of " << numInstructions << std::endl;
   #endif

   delete statConf;

   return instructions_out;
}//---------------------------------------------------------------------	// End walkSFG //

/**
 * @name walkSFG 
 * 
 * @short This function is only called for lock and trans sections.
 * @param threadID 
 * @param tempSynth 
 * @param numInstructions 
 * @param flowNodeIn 
 * @return 
 */
float walkSFG(THREAD_ID threadID, Synthetic *tempSynth, float numInstructions, FlowNode flowNodeIn, std::vector< FlowVertex > foundNodes)
{
   /* Variable Declaraion */
   ConfObject *statConf = new ConfObject;
   float edgeTransit = 0;
   UINT_32 bbcount_out = 0;
   float instructions_out = 0;
   UINT_32 iterations = 0;
   float maxInstructions = ceil(std::max((float)ACC_MAX, numInstructions));

   UINT_32 maxBB = statConf->return_maxBasicBlocks();
   InstructionMix tempMix = flowNodeIn.instructionMix;

   //graph
   graph_traits <BBGraph>::vertex_iterator tempVertexIterator;
   graph_traits <BBGraph>::vertex_iterator vertexIterator, vertexEnd;
   graph_traits <BBGraph>::out_edge_iterator outEdgeIterator, outEdgeEnd;
   basicBlock_name_map_t basicBlockLocal = get(basicBlock_t(), *myCFG[threadID]);
   edgeWeight_name_map_t edgeWeight = get(edge_weight, *myCFG[threadID]);
   nodeIndex_name_map_t nodeIndex = get(vertex_index, *myCFG[threadID]);

   flowNode_name_map_t  flowNode  = get(flowNode_t(), myPCFG);
   flowIndex_name_map_t flowNodeIndex = get(vertex_index, myPCFG);
   flowName_name_map_t  flowNodeName  = get(vertex_name, myPCFG);

   //initialize uniform RV over [0,1)
   static boost::lagged_fibonacci1279 generator(static_cast<unsigned> (std::time(0)));
   boost::uniform_real<double> uniformDistribution(0, 1);
   boost::variate_generator<boost::lagged_fibonacci1279&, boost::uniform_real<double> >  uniformReal(generator, uniformDistribution);

   /* Processes */
   if(flowNodeIn.return_isCritical() == 1)
   {
      IntRegValue lockID = flowNodeIn.return_lockID();
      for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]); vertexIterator != vertexEnd; )
      {
         if(basicBlockLocal[*vertexIterator].return_lockID() == lockID)
         {
            edgeTransit = uniformReal();

            bbcount_out = bbcount_out + 1;
            instructions_out = instructions_out + (float)basicBlockLocal[*vertexIterator].return_instructionListSize();

            tempSynth->update_coreList(basicBlockLocal[*vertexIterator]);

            UINT_32 repCount = 0;
            for(tie(outEdgeIterator, outEdgeEnd) = out_edges(*vertexIterator, *myCFG[threadID]); outEdgeIterator != outEdgeEnd; ++outEdgeIterator)
            {
               if(basicBlockLocal[target(*outEdgeIterator, *myCFG[threadID])] == basicBlockLocal[*vertexIterator] && repCount < REP_COUNT)
               {
                  repCount = repCount + 1;
               }
               else if(edgeWeight[*outEdgeIterator] >= edgeTransit && basicBlockLocal[target(*outEdgeIterator, *myCFG[threadID])].return_lockID() == lockID)
               {
                  //need to point the current iterator to the next one based on the target node
                  BBVertex nextVertex = target(*outEdgeIterator, *myCFG[threadID]);
                  for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]); *vertexIterator != nextVertex; ++vertexIterator)
                  {
                  }

                  break;
               }
            }
         }
         else
         {
            tempVertexIterator = vertexIterator;
            ++vertexIterator;
         }

         //We want to stop writing out to the synthetic once we reach the
         //maximum number of basic blocks defined by the user.
         if(bbcount_out > maxBB)
            break;

         if(instructions_out > numInstructions)
            break;
      }
   }
   else if(flowNodeIn.return_isTrans() == 1)
   {
      std::map< ADDRESS_INT, UINT_32 > read_conflictMap;
      std::map< ADDRESS_INT, UINT_32 > write_conflictMap;

      //Use the list of interfering nodes to build a list of potential memory conflicts ignoring any empty nodes
      while(foundNodes.empty() != 1)
      {
         if(flowNode[foundNodes.back()].return_isTrans() == 1 && flowNode[foundNodes.back()].return_weighted_numInstructions() >= 1.0)
         {
//             StatMemory::getMemoryConflicts(read_conflictMap, write_conflictMap, flowNodeIn.return_transID(), flowNode[foundNodes.back()].return_transID());

            #if defined(DEBUG)
            std::cout << "\nTID:  " << threadID << "  TxID:  " << hex << flowNodeIn.return_transID() << dec << "------------FNode " << flowNodeName[foundNodes.back()] << "(" << flowNode[foundNodes.back()].return_threadID() << ")" << std::endl;
            #endif
         }

         foundNodes.pop_back();
      }

      //NOTE Should use (a) or (b)
      //(a) Generate a list of instructions for the transaction
      BasicBlock transBasicBlock;
      transBasicBlock = CodeGenerator::instructionGenerator(flowNodeIn.return_weighted_numInstructions(), flowNodeIn.instructionMix);

      //(b) Iterate through the SFG looking for a transaction that matches this address
      ADDRESS_INT transactionID = flowNodeIn.return_transID();
      InstructionMix transTargetMix = flowNodeIn.instructionMix;
      transTargetMix.normalize();
      InstructionMix synthMix;
// std::cout << "Target (" << std::hex << transactionID << std::dec << "):  \n";
// transTargetMix.print(std::cout);
      for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]); vertexIterator != vertexEnd; )
      {
         if(basicBlockLocal[*vertexIterator].return_transID() == transactionID)
         {
            edgeTransit = uniformReal();
// std::cout << "Synth (" << std::hex << basicBlockLocal[*vertexIterator].return_bbAddress() << std::dec << "):  \n";
// synthMix.print(std::cout);
//             std::cout << "(" << iterations << ")MAX:  " << maxInstructions << "\tout:  " << instructions_out << "\tsize:  " << basicBlockLocal[*vertexIterator].return_instructionListSize() << std::endl;
            if(basicBlockLocal[*vertexIterator].return_instructionListSize() <= 1 && iterations < 50)
            {
               tempVertexIterator = vertexIterator;
               ++vertexIterator;

               iterations = iterations + 1;
            }
            else if((float)basicBlockLocal[*vertexIterator].return_instructionListSize() <= (maxInstructions - instructions_out))
            {
               if(transTargetMix.compare(basicBlockLocal[*vertexIterator].return_instructionListRef(), synthMix) && iterations < 35)
               {
                  basicBlockLocal[*vertexIterator].update_readConflictMap(read_conflictMap);
                  basicBlockLocal[*vertexIterator].update_writeConflictMap(write_conflictMap);

                  basicBlockLocal[*vertexIterator].instructionMix = flowNodeIn.instructionMix;

                  bbcount_out = bbcount_out + 1;
                  instructions_out = instructions_out + (float)basicBlockLocal[*vertexIterator].return_instructionListSize();
                  tempSynth->update_coreList(basicBlockLocal[*vertexIterator]);

                  synthMix.update(basicBlockLocal[*vertexIterator].return_instructionListRef());

                  iterations = 0;
               }
               else if(iterations >= 35)
               {
                  basicBlockLocal[*vertexIterator].update_readConflictMap(read_conflictMap);
                  basicBlockLocal[*vertexIterator].update_writeConflictMap(write_conflictMap);

                  basicBlockLocal[*vertexIterator].instructionMix = flowNodeIn.instructionMix;

                  bbcount_out = bbcount_out + 1;
                  instructions_out = instructions_out + (float)basicBlockLocal[*vertexIterator].return_instructionListSize();
                  tempSynth->update_coreList(basicBlockLocal[*vertexIterator]);

                  synthMix.update(basicBlockLocal[*vertexIterator].return_instructionListRef());

                  iterations = 0;
               }
               else
               {
                  iterations = iterations + 1;
               }
            }
            else if(iterations >= 35)
            {
               iterations = 0;
               BasicBlock temp_1 = basicBlockLocal[*vertexIterator];

               temp_1.update_readConflictMap(read_conflictMap);
               temp_1.update_writeConflictMap(write_conflictMap);

               temp_1.resize_instructionList((UINT_32)maxInstructions - (UINT_32)instructions_out);
               tempSynth->update_coreList(temp_1);

               synthMix.update(basicBlockLocal[*vertexIterator].return_instructionListRef());

               bbcount_out = bbcount_out + 1;
               instructions_out = instructions_out + (float)temp_1.return_instructionListSize();
            }
            else
            {
               iterations = iterations + 1;
            }

            UINT_32 repCount = 0;
            for(tie(outEdgeIterator, outEdgeEnd) = out_edges(*vertexIterator, *myCFG[threadID]); outEdgeIterator != outEdgeEnd; ++outEdgeIterator)
            {
               if(basicBlockLocal[target(*outEdgeIterator, *myCFG[threadID])] == basicBlockLocal[*vertexIterator] && repCount < REP_COUNT)
               {
                  repCount = repCount + 1;
               }
               else if(edgeWeight[*outEdgeIterator] >= edgeTransit && basicBlockLocal[target(*outEdgeIterator, *myCFG[threadID])].return_transID() == transactionID)
               {
                  //need to point the current iterator to the next one based on the target node
                  BBVertex nextVertex = target(*outEdgeIterator, *myCFG[threadID]);
                  for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]); *vertexIterator != nextVertex; ++vertexIterator)
                  {
                  }

                  break;
               }
            }
         }
         else
         {
            tempVertexIterator = vertexIterator;
            ++vertexIterator;
         }

         //We want to stop writing out to the synthetic once we reach the
         //maximum number of basic blocks defined by the user.
         if(bbcount_out > maxBB)
            break;

         if(instructions_out > numInstructions)
            break;
      }
   }
   else if(flowNodeIn.return_isWait() == 1)
   {
        instructions_out = instructions_out + walkSFG(threadID, tempSynth, numInstructions);

//       for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]); vertexIterator != vertexEnd; )
//       {
//          if(basicBlockLocal[*vertexIterator].return_isWait() == 1)
//          {
//             bbcount_out = bbcount_out + 1;
//             instructions_out = instructions_out + (float)basicBlockLocal[*vertexIterator].return_instructionListSize();
// 
//             tempSynth->update_coreList(basicBlockLocal[*vertexIterator]);
// 
//             break;
//          }
//          else
//          {
//             tempVertexIterator = vertexIterator;
//             ++vertexIterator;
//          }
//       }
   }
   else if(flowNodeIn.return_isBarrier() == 1)
   {
        instructions_out = instructions_out + walkSFG(threadID, tempSynth, numInstructions);

//       for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]); vertexIterator != vertexEnd; )
//       {
//          if(basicBlockLocal[*vertexIterator].return_isBarrier() == 1)
//          {
//             bbcount_out = bbcount_out + 1;
//             instructions_out = instructions_out + (float)basicBlockLocal[*vertexIterator].return_instructionListSize();
// 
//             tempSynth->update_coreList(basicBlockLocal[*vertexIterator]);
// 
//             break;
//          }
//          else
//          {
//             tempVertexIterator = vertexIterator;
//             ++vertexIterator;
//          }
//       }
   }

   #if defined(DEBUG)
   std::cout << "Added " << instructions_out << " to T" << threadID << "  with weight of " << numInstructions << std::endl;
   #endif

   delete statConf;

   return instructions_out;
}//---------------------------------------------------------------------	// End walkSFG //

/**
 * @name accumulatorDump
 * 
 * @param threadID 
 * @param address 
 * @param tempSynth 
 * @param numInstructions 
 * @return 
 */
float accumulatorDump(THREAD_ID threadID, ADDRESS_INT address, Synthetic *tempSynth, float numInstructions)
{
   /* Variable Declaraion */
   float edgeTransit = 0;
   UINT_32 bbcount_out = 0;
   float instructions_out = 0;
   float maxInstructions = ceil(std::max((float)ACC_MAX, numInstructions));

   //graph
   graph_traits <BBGraph>::vertex_iterator tempVertexIterator;
   graph_traits <BBGraph>::vertex_iterator vertexIterator, vertexEnd;
   graph_traits <BBGraph>::out_edge_iterator outEdgeIterator, outEdgeEnd;
   basicBlock_name_map_t basicBlockLocal = get(basicBlock_t(), *myCFG[threadID]);
   edgeWeight_name_map_t edgeWeight = get(edge_weight, *myCFG[threadID]);
   nodeIndex_name_map_t nodeIndex = get(vertex_index, *myCFG[threadID]);

   flowNode_name_map_t  flowNode  = get(flowNode_t(), myPCFG);
   flowIndex_name_map_t flowNodeIndex = get(vertex_index, myPCFG);
   flowName_name_map_t  flowNodeName  = get(vertex_name, myPCFG);

   //initialize uniform RV over [0,1)
   static boost::lagged_fibonacci1279 generator(static_cast<unsigned> (std::time(0)));
   boost::uniform_real<double> uniformDistribution(0, 1);
   boost::variate_generator<boost::lagged_fibonacci1279&, boost::uniform_real<double> >  uniformReal(generator, uniformDistribution);

   /* Processes */
//    std::cout << "ACC(" << std::hex << address << "):  " << numInstructions << "\tMax Ins Out:  " << maxInstructions << "\n" << std::dec;

   ADDRESS_INT transactionID = address;
   for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]); vertexIterator != vertexEnd; )
   {
      if(basicBlockLocal[*vertexIterator].return_transID() == transactionID && basicBlockLocal[*vertexIterator].return_instructionListSize() < (maxInstructions - instructions_out))
      {
// std::cout << "ID:  " << basicBlockLocal[*vertexIterator].return_transID() << "\tCount:  " << basicBlockLocal[*vertexIterator].return_instructionListSize() << "\n";

         BasicBlock tempBB(basicBlockLocal[*vertexIterator]);
         tempBB.update_accumulated((UINT_32)ceil(numInstructions));
         edgeTransit = uniformReal();
// std::cout << "Is Trans:  " << tempBB.return_isTrans() << "\tAccumulated:  " << tempBB.return_accumulated() << "\n";
         bbcount_out = bbcount_out + 1;
         instructions_out = instructions_out + (float)basicBlockLocal[*vertexIterator].return_instructionListSize();
         tempBB.instructionMix.update(basicBlockLocal[*vertexIterator].return_instructionListRef(), 1);
         tempSynth->update_coreList(tempBB);

         UINT_32 repCount = 0;
         for(tie(outEdgeIterator, outEdgeEnd) = out_edges(*vertexIterator, *myCFG[threadID]); outEdgeIterator != outEdgeEnd; ++outEdgeIterator)
         {
            if(basicBlockLocal[target(*outEdgeIterator, *myCFG[threadID])] == basicBlockLocal[*vertexIterator] && repCount < REP_COUNT)
            {
               repCount = repCount + 1;
            }
            else if(edgeWeight[*outEdgeIterator] >= edgeTransit && basicBlockLocal[target(*outEdgeIterator, *myCFG[threadID])].return_transID() == transactionID)
            {
               //need to point the current iterator to the next one based on the target node
               BBVertex nextVertex = target(*outEdgeIterator, *myCFG[threadID]);
               for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]); *vertexIterator != nextVertex; ++vertexIterator)
               {
               }

               break;
            }
         }
      }
      else if(basicBlockLocal[*vertexIterator].return_transID() == transactionID)
      {
// std::cout << "^ID:  " << basicBlockLocal[*vertexIterator].return_transID() << "\tCount:  " << basicBlockLocal[*vertexIterator].return_instructionListSize() << "\t" << maxInstructions - instructions_out<<"\n";
         BasicBlock tempBB(basicBlockLocal[*vertexIterator]);
         tempBB.update_accumulated((UINT_32)ceil(numInstructions));
         edgeTransit = uniformReal();

         tempBB.resize_instructionList((UINT_32)maxInstructions - (UINT_32)instructions_out);

// std::cout << "^Is Trans:  " << tempBB.return_isTrans() << "\tSize:  " << basicBlockLocal[*vertexIterator].return_instructionListSize() <<"\tNew Size:  " << tempBB.return_instructionListSize() << "\n";
         bbcount_out = bbcount_out + 1;
         tempBB.instructionMix.update(basicBlockLocal[*vertexIterator].return_instructionListRef(), 1);
         instructions_out = instructions_out + (float)tempBB.return_instructionListSize();
         tempSynth->update_coreList(tempBB);

         UINT_32 repCount = 0;
         for(tie(outEdgeIterator, outEdgeEnd) = out_edges(*vertexIterator, *myCFG[threadID]); outEdgeIterator != outEdgeEnd; ++outEdgeIterator)
         {
            if(basicBlockLocal[target(*outEdgeIterator, *myCFG[threadID])] == basicBlockLocal[*vertexIterator] && repCount < REP_COUNT)
            {
               repCount = repCount + 1;
            }
            else if(edgeWeight[*outEdgeIterator] >= edgeTransit && basicBlockLocal[target(*outEdgeIterator, *myCFG[threadID])].return_transID() == transactionID)
            {
               //need to point the current iterator to the next one based on the target node
               BBVertex nextVertex = target(*outEdgeIterator, *myCFG[threadID]);
               for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]); *vertexIterator != nextVertex; ++vertexIterator)
               {
               }

               break;
            }
         }

         break;
      }
      else
      {
         tempVertexIterator = vertexIterator;
         ++vertexIterator;
      }

      if(instructions_out > maxInstructions)
         break;
   }

   #if defined(DEBUG)
   std::cout << "-Added " << instructions_out << " to T" << threadID << "  with weight of " << numInstructions << std::endl;
   #endif

   return instructions_out;
}//---------------------------------------------------------------------	// End accumulatorDump //

/**
 * @name cleanNode
 * 
 * @param bbAddress 
 * @param threadID 
 * @return 
 */
void cleanNode(ADDRESS_INT bbAddress, THREAD_ID threadID)
{
   graph_traits <BBGraph>::vertex_iterator vertexIterator, vertexEnd;
   basicBlock_name_map_t basicBlockLocal = get(basicBlock_t(), *myCFG[threadID]);

   for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]);vertexIterator != vertexEnd; ++vertexIterator)
   {
      if(basicBlockLocal[*vertexIterator].return_bbAddress() == bbAddress)
         basicBlockLocal[*vertexIterator].update_isCritical(0);
   }
}//---------------------------------------------------------------------	// End cleanNode //

/**
 * @name identifyDuplicateThreads
 *
 * @short This function performs a BFS of the PCFG and identifies the starting address for each thread. 
 * @return 
 */
void identifyDuplicateThreads()
{
   graph_traits <BBGraph>::vertex_iterator vertexIterator, vertexEnd;

   flowNode_name_map_t  flowNode  = get(flowNode_t(), myPCFG);
   flowIndex_name_map_t flowNodeIndex = get(vertex_index, myPCFG);
   flowName_name_map_t  flowNodeName  = get(vertex_name, myPCFG);

   std::list < FlowVertex > threadStartList;

   /* Processes */
   for(UINT_32 threadID = 0; threadID < totalNumThreads; threadID++)
   {
      bfs_thread_locater threadStartLocater(threadID, threadStartList);
      breadth_first_search(myPCFG, vertex(0, myPCFG), visitor(threadStartLocater));
   }

   for(std::list < FlowVertex >::iterator listIterator = threadStartList.begin(); listIterator != threadStartList.end(); listIterator++)
   {
      THREAD_ID threadID = flowNode[*listIterator].return_threadID();

      basicBlock_name_map_t basicBlockLocal = get(basicBlock_t(), *myCFG[threadID]);
      tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadID]);

      flowNode[*listIterator].update_startPC(basicBlockLocal[*vertexIterator].return_bbAddress());

      if(CodeGenerator::threadFuncMap[basicBlockLocal[*vertexIterator].return_bbAddress()] == 0)
         CodeGenerator::threadFuncMap[basicBlockLocal[*vertexIterator].return_bbAddress()] = flowNode[*listIterator].return_threadID();
      else if(CodeGenerator::threadFuncMap[basicBlockLocal[*vertexIterator].return_bbAddress()] > flowNode[*listIterator].return_threadID())
         CodeGenerator::threadFuncMap[basicBlockLocal[*vertexIterator].return_bbAddress()] = flowNode[*listIterator].return_threadID();
   }

}

//BEGIN PCFG--------------------------------------------------------------------------------------------------
/**
 * @name generatePCFGNodeIDs
 * 
**/
void generatePCFGNodeIDs()
{
   std::cout << "\nGenerating PCFG Node IDs" << std::flush;

   graph_traits <PCFG>::vertex_iterator vertexIterator, vertexEnd;
   flowNode_name_map_t  flowNode  = get(flowNode_t(), myPCFG);
   flowIndex_name_map_t nodeIndex = get(vertex_index, myPCFG);
   flowName_name_map_t  nodeName  = get(vertex_name, myPCFG);

   UINT_32 c = 0;
   for(tie(vertexIterator, vertexEnd) = vertices(myPCFG);vertexIterator != vertexEnd; ++vertexIterator, ++c)
   {
      nodeIndex[*vertexIterator] = c;      //numbering for internal ID
      nodeName[*vertexIterator] = IntToString(c);       //external ID
   }

   std::cout << "...Finished" << std::flush;
}

/**
 * @name writePCFGDots
 * 
 * @param name 
 * @return 
**/
void writePCFGDots(string name)
{
   ConfObject *statConf = new ConfObject;
   UINT_32 numThreads = totalNumThreads;
   INT_32 rSize = statConf->return_reductionFactor();
   UINT_32 threadCounter = 0;

   flowName_name_map_t  nodeName  = get(vertex_name, myPCFG);
   flow_edgeWeight_name_map_t edgeWeight = get(edge_weight, myPCFG);

   std::cout << "\nWriting PCFG Dot File" << std::flush;

   string fileName = Synthesis::statPaths.return_rootDirectory() + Synthesis::statPaths.return_dataDirectory() + Synthesis::statPaths.return_outputFileName() + ".";
   fileName = fileName + name + ".PCFG-" + IntToString(threadCounter) + ".graphViz";
   ofstream outputFile(fileName.c_str(), ios::trunc);      //open a file for writing (truncate the current contents)
   if ( !outputFile )  //check to be sure file is open
      std::cerr << "Error opening file.";

   write_graphviz(outputFile, myPCFG, make_label_writer(nodeName), make_label_writer(edgeWeight));
   outputFile.close();

   delete statConf;

   std::cout << "...Finished" << std::flush;
}

/**
 * @name reducePCFG 
 * 
 * @short This function performs a reduction based on the k-value set at initialization.
 * @return 
**/
void reducePCFG(const std::vector < UINT_64 > &numInstructions)
{
   /* Variable Declaraion */
   ConfObject *statConf = new ConfObject;
   float minInstructionCount = MAX_INSTRUCTIONS;
   std::vector< UINT_64 > newInstructionCount (totalNumThreads,0);

   //graph
   graph_traits <PCFG>::vertex_iterator vertexIterator, vertexEnd;
   flowNode_name_map_t flowNode = get(flowNode_t(), myPCFG);

   Synthetic *tempSynth = new Synthetic;

   /* Processes */
   std::cout << "\nReducing PCFG with MAX of ";
   for(UINT_32 threadID = 0; threadID < totalNumThreads; threadID++)
   {
      if(minInstructionCount == MAX_INSTRUCTIONS)
      {
         if(numInstructions[threadID] < MAX_INSTRUCTIONS)
         {
            minInstructionCount = numInstructions[threadID];
         }
      }
      else if(numInstructions[threadID] < MAX_INSTRUCTIONS)
      {
         minInstructionCount = numInstructions[threadID];
      }
   }

   for(UINT_32 threadID = 0; threadID < totalNumThreads; threadID++)
   {
      //BFS on PCFG
      std::vector< FlowVertex > nameList;

      bfs_thread_visitor nodeVisitor(threadID, nameList);
      breadth_first_search(myPCFG, vertex(0, myPCFG), visitor(nodeVisitor));

      for(std::vector< FlowVertex>::iterator itBegin = nameList.begin(); itBegin != nameList.end(); itBegin++)
      {
         float weight = minInstructionCount * ((float)flowNode[*itBegin].return_numInstructions() / (float)numInstructions[threadID]);
         flowNode[*itBegin].update_weighted_numInstructions(weight);
         newInstructionCount[flowNode[*itBegin].return_threadID()] = newInstructionCount[flowNode[*itBegin].return_threadID()] + (UINT_64)weight;
      }
   }

   std::cout << minInstructionCount << flush;

   delete statConf;

   std::cout << "...Finished" << std::flush;
}//---------------------------------------------------------------------	// End reducePCFG //

/**
 * @name walkPCFG 
 * 
 * @short 
 * @return 
 * @note This takes care of zero-nodes by passing the weighted instruction count to the SFG or
 *       by using the JUNKYARD define.
**/
void walkPCFG(THREAD_ID threadID, Synthetic *syntheticThreads[], const UINT_32 &arraySize)
{
   /* Variable Declaraion */
   ConfObject *statConf = new ConfObject;
   UINT_32 maxBB = statConf->return_maxBasicBlocks();
   UINT_32 bbcount_out = 0;
   float   totalInstructions = 0;
   UINT_32 inscount_low[totalNumThreads];
   UINT_32 inscount_high[totalNumThreads];

   #if defined(JUNKYARD)
   std::map< ADDRESS_INT, float > transactionAccumulator;
   #endif

   //graph
   flowNode_name_map_t flowNode = get(flowNode_t(), myPCFG);
   flowName_name_map_t  flowNodeName  = get(vertex_name, myPCFG);
   flow_edgeWeight_name_map_t edgeWeight = get(edge_weight, myPCFG);

   //synthetic
   Synthetic *tempSynth = new Synthetic;

   /* Processes */
   if(threadID == 0)
      std::cout << "\nPopulating Synthetic Backbone..." << std::flush;

   std::cout << "\n\t-T" << threadID << ":  " << std::flush;

   if(num_vertices(*myCFG[threadID]) > 0)
   {
      //BFS on PCFG -- build a vector of the nodes as they appear in the graph
      std::vector< FlowVertex > nameList;

      std::cout << "Building node list..." << std::flush;

      bfs_thread_visitor nodeVisitor(threadID, nameList);
      breadth_first_search(myPCFG, vertex(0, myPCFG), visitor(nodeVisitor));

      for(UINT_32 counter = 0; counter < totalNumThreads; counter++)
      {
         inscount_low[counter] = inscount_high[counter] = 0;
      }

      std::cout << "Iterating through PCFG..." << std::flush;

      //Iterate through the vector of vertices, for each node add the appropriate
      //number of instructions to the synthetic buffer
      for(std::vector< FlowVertex>::iterator itBegin = nameList.begin(); itBegin != nameList.end(); itBegin++)
      {
         THREAD_ID nodeThreadID = flowNode[*itBegin].return_threadID();
         std::vector< FlowVertex > foundNodes;
         inscount_high[nodeThreadID] = inscount_low[nodeThreadID] + (UINT_32)round(flowNode[*itBegin].return_weighted_numInstructions());

         //For thread event nodes, do not perform a normal population of the synthetic.
         //Instead, insert a special basic block with the required information.
         if(flowNode[*itBegin].return_isSpawn() == 1)
         {
            BasicBlock temp_basicBlock;
            temp_basicBlock.update_isSpawn(1);
            temp_basicBlock.childThreads = flowNode[*itBegin].childThreads;

            tempSynth->update_coreList(temp_basicBlock);
         }
//          else if(flowNode[*itBegin].return_isWait() == 1)
//          {
//             totalInstructions = totalInstructions + walkSFG(threadID, tempSynth, flowNode[*itBegin].return_weighted_numInstructions(), flowNode[*itBegin], foundNodes);
//          }
//          else if(flowNode[*itBegin].return_isBarrier() == 1)
//          {
//             totalInstructions = totalInstructions + walkSFG(threadID, tempSynth, flowNode[*itBegin].return_weighted_numInstructions(), flowNode[*itBegin], foundNodes);
//          }
         else if(flowNode[*itBegin].return_isCritical() == 1)
         {
            if(flowNode[*itBegin].return_weighted_numInstructions() >= 1.0)
               totalInstructions = totalInstructions + walkSFG(threadID, tempSynth, flowNode[*itBegin].return_weighted_numInstructions(), flowNode[*itBegin], foundNodes);
         }
         else if(flowNode[*itBegin].return_isTrans() == 1)
         {
            //Need to check if there is another node at the same depth.
            //If there is a node, push it to the foundNode list which can be passed to the SFG population function.
            float instructionCount[totalNumThreads];
            std::vector< std::list < FlowVertex > > depthList (totalNumThreads);
            bfs_depth_finder depthVisitor(threadID, depthList);
            breadth_first_search(myPCFG, vertex(0, myPCFG), visitor(depthVisitor));

            for(UINT_32 counter = 0; counter < totalNumThreads; counter++)
            {
               instructionCount[counter] = 0;
               for(std::list < FlowVertex >::iterator vertexIterator = depthList[counter].begin(); vertexIterator != depthList[counter].end(); vertexIterator++)
               {
                  instructionCount[counter] = instructionCount[counter] + flowNode[*vertexIterator].return_weighted_numInstructions();
                  if(instructionCount[counter] >= inscount_low[nodeThreadID] && instructionCount[counter] <= (inscount_high[nodeThreadID] + 10))
                  {
                     foundNodes.push_back(*vertexIterator);
                  }
               }
            }

            #if defined(JUNKYARD)
               #if defined(DEBUG)
               std::cout << "T" << threadID << " ";
               std::cout << "Map Size (" << flowNode[*itBegin].return_transID() << ") is " << transactionAccumulator[flowNode[*itBegin].return_transID()] << " adding " << flowNode[*itBegin].return_weighted_numInstructions();
               #endif

               transactionAccumulator[flowNode[*itBegin].return_transID()] = transactionAccumulator[flowNode[*itBegin].return_transID()] + flowNode[*itBegin].return_weighted_numInstructions();

               #if defined(DEBUG)
               std::cout << " New Map Size (" << flowNode[*itBegin].return_transID() << ") is " << transactionAccumulator[flowNode[*itBegin].return_transID()] << std::endl;
               #endif

               if(flowNode[*itBegin].return_weighted_numInstructions() >= ACC_MAX)
               {
                  totalInstructions = totalInstructions + walkSFG(threadID, tempSynth, transactionAccumulator[flowNode[*itBegin].return_transID()], flowNode[*itBegin], foundNodes);
                  transactionAccumulator[flowNode[*itBegin].return_transID()] = 0.0;
               }
               else if(transactionAccumulator[flowNode[*itBegin].return_transID()] >= ACC_MAX)
               {
                  totalInstructions = totalInstructions + walkSFG(threadID, tempSynth, transactionAccumulator[flowNode[*itBegin].return_transID()], flowNode[*itBegin], foundNodes);
                  transactionAccumulator[flowNode[*itBegin].return_transID()] = 0.0;
               }
            #else
               if(flowNode[*itBegin].return_weighted_numInstructions() >= 1.0)
                  totalInstructions = totalInstructions + walkSFG(threadID, tempSynth, flowNode[*itBegin].return_weighted_numInstructions(), flowNode[*itBegin], foundNodes);
            #endif
         }
         else
         {
            #if defined(JUNKYARD)
               #if defined(DEBUG)
               std::cout << "*T" << threadID << " ";
               std::cout << "Map Size (" << flowNode[*itBegin].return_transID() << ") is " << transactionAccumulator[SEQUENTIAL] << " adding " << flowNode[*itBegin].return_weighted_numInstructions();
               #endif

               transactionAccumulator[SEQUENTIAL] = transactionAccumulator[SEQUENTIAL] + flowNode[*itBegin].return_weighted_numInstructions();

               #if defined(DEBUG)
               std::cout << " New Map Size (" << flowNode[*itBegin].return_transID() << ") is " << transactionAccumulator[SEQUENTIAL] << std::endl;
               #endif

               if(flowNode[*itBegin].return_weighted_numInstructions() >= ACC_MAX)
               {
                  totalInstructions = totalInstructions + walkSFG(threadID, flowNode[*itBegin].return_startPC(), tempSynth, transactionAccumulator[SEQUENTIAL]);
                  transactionAccumulator[SEQUENTIAL] = 0.0;
               }
               else if(transactionAccumulator[SEQUENTIAL] >= ACC_MAX)
               {
                  totalInstructions = totalInstructions + walkSFG(threadID, flowNode[*itBegin].return_startPC(), tempSynth, transactionAccumulator[SEQUENTIAL]);
                  transactionAccumulator[SEQUENTIAL] = 0.0;
               }
            #else
               if(flowNode[*itBegin].return_weighted_numInstructions() >= 1.0)
                  totalInstructions = totalInstructions + walkSFG(threadID, flowNode[*itBegin].return_startPC(), tempSynth, flowNode[*itBegin].return_weighted_numInstructions());
            #endif
         }

         inscount_low[nodeThreadID] = inscount_high[nodeThreadID];
      }

      #if defined(JUNKYARD)
      for(std::map< ADDRESS_INT, float >::iterator boo = transactionAccumulator.begin(); boo != transactionAccumulator.end(); boo++)
      {
         //skip the sequential part and anything with nothing accumulated
         if(boo->first > 0 && boo->second > 0.0)
            totalInstructions = totalInstructions + accumulatorDump(threadID, boo->first, tempSynth, boo->second);
      }
      #endif
   }
   else
   {
      std::cout << "Empty...";
   }

   std::cout << "Total Instructions:  " << totalInstructions << "...";

   if(threadID == totalNumThreads - 1)
      std::cout << "Finished" << std::flush;

   syntheticThreads[threadID] = tempSynth;
   delete statConf;
}//---------------------------------------------------------------------	// End walkPCFG //


/**
 * @name   addPCFGNode
 * 
 * @short  This function adds nodes to the PCFG
 * @param  flowNodeIn 
 * @return last vertex 
 */
FlowVertex addPCFGNode(const FlowNode &flowNodeIn)
{
   /* Variable Declaration */
   BOOL found;
   BOOL inserted;
   BOOL unique;
   ConfObject *statConf = new ConfObject;
   THREAD_ID threadID = flowNodeIn.return_threadID();

   graph_traits <PCFG>::edge_descriptor edgeDesc;
   flowNode_name_map_t flowNode = get(flowNode_t(), myPCFG);
   flow_edgeWeight_name_map_t edgeWeight = get(edge_weight, myPCFG);

   /* Processes */
   myPCFG_VertexA = add_vertex(myPCFG);      //place the node
   flowNode[myPCFG_VertexA] = flowNodeIn;    //assign the value

   //If this is the first node in a thread, we need to find its parent; it is assumed that there is at least
   //one other node in the graph. However, if this is not the first node in a thread there should be a check
   //to ensure that there is something else in the graph to hook.
   if(num_vertices(myPCFG) > 1 && lastInsertedNode[threadID] != graph_traits<PCFG>::null_vertex())
   {
      //create an edge for the new node if it isn't the first node of the graph
      tie(edgeDesc, inserted) = add_edge(lastInsertedNode[threadID], myPCFG_VertexA, myPCFG);
      if(inserted)
      {
         edgeWeight[edgeDesc] = threadID;
      }
   }

   lastInsertedNode[threadID] = myPCFG_VertexA;       //set up for next iteration -- need per-thread

   delete statConf;

   return myPCFG_VertexA;
}

/**
 * @name   finalizePCFGNode
 * 
 * @short  Only called when the thread is finished.
 * @see    addPCFGNode()
 * @param  flowNodeIn 
 * @return last vertex 
 */
void finalizePCFG()
{
   /* Variable Declaration */
   BOOL found;
   BOOL inserted;
   BOOL unique;
   ConfObject *statConf = new ConfObject;

   graph_traits <PCFG>::edge_descriptor edgeDesc;
   flowNode_name_map_t flowNode = get(flowNode_t(), myPCFG);
   flowName_name_map_t  flowNodeName  = get(vertex_name, myPCFG);
   flow_edgeWeight_name_map_t edgeWeight = get(edge_weight, myPCFG);

   /* Processes */

   for(UINT_32 threadID = 1; threadID < totalNumThreads; threadID++)
   {
      THREAD_ID parentThreadID = 0;

      //Find the in the graph that spawned this thread
      //Iterate through the graph checking for nodes with children
      //If a node has children, iterate through the child list until a match is found
      graph_traits<PCFG>::vertex_iterator vertexIterator, vertexEnd, parentVertex;

      tie(parentVertex, vertexEnd) = vertices(myPCFG);                           //make sure that the parent exists even if it is incorrect

      for(tie(vertexIterator, vertexEnd) = vertices(myPCFG); vertexIterator != vertexEnd; ++vertexIterator)
      {
         if(flowNode[*vertexIterator].childThreads.size() > 0)
         {
            for(std::vector< UINT_32 >::iterator thisIter = flowNode[*vertexIterator].childThreads.begin(); thisIter != flowNode[*vertexIterator].childThreads.end(); thisIter++)
            {
               if(*thisIter == threadID)
               {
                  parentVertex = vertexIterator;
                  parentThreadID = flowNode[*vertexIterator].return_threadID();
                  break;
               }
            }
         }
      }
std::cerr << "---------->   parentThreadID:  " << parentThreadID << "(" << threadID << ")\n" << std::endl;
      //We link from parent to child when the child terminates
      //Iterate through the graph to find the first node of this thread -- insert edge from parent to this node
      //NOTE Sometimes the parentVertex does not exist and can cause a seg fault when trying to insert an edge with a false node
      for(tie(vertexIterator, vertexEnd) = vertices(myPCFG); vertexIterator != vertexEnd; ++vertexIterator)
      {
         if(flowNode[*vertexIterator].return_threadID() == threadID && flowNode[*vertexIterator].return_isFirst() == 1)
         {
            tie(edgeDesc, inserted) = add_edge(*parentVertex, *vertexIterator, myPCFG);
            if(inserted)
            {
               edgeWeight[edgeDesc] = threadID;
            }
         }
      }

      //NOTE  For now we are linking it to the spawn node
      //FIXME We need to relink this sequence of nodes to the parent
      tie(edgeDesc, inserted) = add_edge(lastInsertedNode[threadID], *parentVertex, myPCFG);
      if(inserted)
      {
         edgeWeight[edgeDesc] = threadID;
      }

//       lastInsertedNode[threadID] = myPCFG_VertexA;       //set up for next iteration -- need per-thread
   }

   delete statConf;

}
//END PCFG--------------------------------------------------------------------------------------------------


}
//END GraphManipulation----------------------------------------------------------------------------------------------------------------------------
