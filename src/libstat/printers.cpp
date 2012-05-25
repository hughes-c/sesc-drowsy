//
// C++ Implementation: printers
//
// Description: 
//
//
/// @author: hughes  <hughes@fraidy2-uf>, (C) 200
///
/// @date:          05/23/07
/// Last Modified:  02/05/08
//
// Copyright: See COPYING file that comes with this distribution
//
//////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

#include "printers.h"

extern UINT_32 totalNumThreads;
extern std::deque  < BBGraph * > myCFG;
extern PCFG myPCFG;

extern std::vector < std::vector < UINT_32 > > per_threadReadBins;
extern std::vector < std::vector < UINT_32 > > per_threadWriteBins;

namespace Synthesis
{
   extern StatPaths statPaths;
}

namespace SynthPrinters
{

/**
 * @name SFGprinter 
 * 
 * @param threadCount 
 * @return 
 */
void SFGprinter(UINT_32 threadCount)
{
   /* Variable Declaraion */
   string threadNum;
   string fileName;

   graph_traits <BBGraph>::vertex_iterator vertexIterator, vertexEnd;
   graph_traits <BBGraph>::out_edge_iterator outEdgeIterator, outEdgeEnd;

   /* Processes */
   cout << "\nPrinting SFG Contents" << flush;
   for(UINT_32 counter = 0; counter < threadCount; counter++)
   {
      cout << "..." << counter << flush;

      basicBlock_name_map_t basicBlockLocal = get( basicBlock_t(), *myCFG[counter]  );
      edgeWeight_name_map_t edgeWeight = get( edge_weight, *myCFG[counter] );

      fileName = Synthesis::statPaths.return_rootDirectory() + Synthesis::statPaths.return_dataDirectory() + Synthesis::statPaths.return_outputFileName() + "." + "SFG" + ".";
      threadNum = ".T" + IntToString(counter) + ".out";

      fileName = fileName + "outPut" + threadNum;
      ofstream outputFile(fileName.c_str(), ios::trunc);      //open a file for writing (truncate the current contents)
      if ( !outputFile )  //check to be sure file is open
         cout << "Error opening file.";

      //Begin writing to file
      //The contents of each BB, the number of instructions, the average dependency distance, and the number or memory ops
      outputFile << "\n\n";
      outputFile << "Number of threads:  " << "\n";
      outputFile << "Number of instructions:  " << "\n";
      //since we use the zero index for the total count, loop must be <=
// 		for ( UINT_32 moo = 0; moo <= numThreads; moo++ )
// 		{
// 			if ( moo == 0 )
// 			{
// 				outputFile << std::right << std::setw ( 11 ) << "Total " << ":  " << numInstructions[moo] << "\n";
// 			}
// 			else
// 				outputFile << std::right << std::setw ( 10 ) << "Thread " << std::left << moo << ":  " << numInstructions[moo] << "\n";
// 		}

// 		outputFile << "Number of basic blocks:  " << numBasicBlocks << "\n\n";
// 		outputFile << "\n\n";

      //Begin writing to file
      //The contents of each BB, the number of instructions, the average dependency distance, and the number or memory ops
      outputFile << "NUM - Number of Instructions in each BB\n";
      outputFile << "MOP - Number of memory operations in each BB\n";
      outputFile << "AVG - Average distance between instruction dependancies in each BB\n";
      outputFile << "CNT - Number of times BB is accessed\n";
      outputFile << "BRA - Probability taken\n";
      outputFile << "CS  - Is a critical section\n";
      outputFile << "SP  - Spawns a new thread\n";
      outputFile << "TG  - Target thread\n";
      outputFile << "DS  - Destroy a thread\n";
      outputFile << "WT  - Is waiting\n";
      outputFile << "BR  - Is a barrier\n";
      outputFile << "TID - Thread ID\n";
      outputFile << "LID - Lock ID\n";
      outputFile << "\n\n\n";

      //initialize uniform RV over [0,1)
      static boost::lagged_fibonacci607 rng(static_cast<unsigned> (std::time(0)));
      uniform_real<> uniformDistribution ( 0,1 );
      variate_generator<boost::lagged_fibonacci607&, boost::uniform_real<double> > chooseEdge ( rng, uniformDistribution );

      for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[counter]); vertexIterator != vertexEnd; ++vertexIterator)
      {
         outputFile << std::setw ( 17 )	<< std::left << std::hex << basicBlockLocal[*vertexIterator].return_bbAddress() << std::dec;
         outputFile << std::left << "TID: "  << std::setw ( 4 )   << basicBlockLocal[*vertexIterator].return_threadID();
         outputFile << std::left << "CNT: "  << std::setw ( 10 )  << basicBlockLocal[*vertexIterator].return_bbCount();
         outputFile << std::left << "AVG: "  << std::setw ( 6 )   << std::setprecision ( 3 ) << basicBlockLocal[*vertexIterator].return_avgDistance();
         outputFile << std::left << "TX:  "  << std::setw ( 4 )   << basicBlockLocal[*vertexIterator].return_isTrans();
         outputFile << std::left << "CS:  "  << std::setw ( 4 )   << basicBlockLocal[*vertexIterator].return_isCritical();
         outputFile << std::left << "SP:  "  << std::setw ( 4 )   << basicBlockLocal[*vertexIterator].return_isSpawn();
         outputFile << std::left << "TG:  "  << std::setw ( 4 )   << basicBlockLocal[*vertexIterator].return_targetThread();
         outputFile << std::left << "WT:  "  << std::setw ( 4 )   << basicBlockLocal[*vertexIterator].return_isWait();
         outputFile << std::left << "BR:  "  << std::setw ( 4 )   << basicBlockLocal[*vertexIterator].return_isBarrier();
         outputFile << std::left << "DS:  "  << std::setw ( 4 )   << basicBlockLocal[*vertexIterator].return_isDestroy();
         outputFile << std::left << "SH:  "  << std::setw ( 4 )   << basicBlockLocal[*vertexIterator].return_isShared();
         //outputFile << std::left << "NUM:  "	<< std::setw(6) << basicBlockLocal[*vertexIterator].return_numInstructions();
         outputFile << std::left << "DEP:  " << std::setw ( 9 ) << basicBlockLocal[*vertexIterator].return_numDependancies();
         //outputFile << std::left << "MOP: " << std::setw(4) << basicBlockLocal[*vertexIterator].return_numMemoryOps();
         if(basicBlockLocal[*vertexIterator].return_isCritical() == 1)
            outputFile << std::left << "LID:  "  << std::setw ( 4 )   << std::hex << basicBlockLocal[*vertexIterator].return_lockID() << std::dec;
         if(basicBlockLocal[*vertexIterator].return_isTrans() == 1)
            outputFile << std::left << "TxID:  "  << std::setw ( 4 )   << std::hex << basicBlockLocal[*vertexIterator].return_transID() << std::dec;
         outputFile << "\n";

         std::list <InstructionContainer>::iterator instructionListIterator;
         std::list <InstructionContainer> tempInstructionList = basicBlockLocal[*vertexIterator].return_instructionList();

         //cout << "\nList Size  " << tempInstructionList.size();
         for(instructionListIterator = tempInstructionList.begin(); instructionListIterator !=  tempInstructionList.end(); instructionListIterator++)
         {
            outputFile << std::hex << (ADDRESS_INT)instructionListIterator->return_instructionID();
            outputFile << "::";
            outputFile << Instruction::opcode2Name(instructionListIterator->return_opCode());
            outputFile << "-" << Instruction::subCode_to_Name(instructionListIterator->return_subCode());
            outputFile << "|";
         }
// 
// 			//outputFile << std::setw ( 40 ) << std::left << basicBlockLocal[*vertexIterator].return_insDisassembled();
         outputFile << endl;
      }

      outputFile << "\n\n#EOF";
      outputFile.close();
   }

   std::cout << "...Finished" << std::flush;
}//---------------------------------------------------------------------	// End SFGprinter //


/**
 * @name SFGprinter 
 * 
 * @param threadCount 
 * @param newFileName 
 * @return 
 */
void SFGprinter(UINT_32 threadCount, string newFileName)
{
   /* Variable Declaraion */
   string threadNum;
   string fileName;

   graph_traits <BBGraph>::vertex_iterator vertexIterator, vertexEnd;
   graph_traits <BBGraph>::out_edge_iterator outEdgeIterator, outEdgeEnd;

   /* Processes */
   cout << "\nPrinting SFG Contents" << flush;
   for(UINT_32 counter = 0; counter < threadCount; counter++)
   {
      cout << "..." << counter << flush;

      basicBlock_name_map_t basicBlockLocal = get( basicBlock_t(), *myCFG[counter]  );
      edgeWeight_name_map_t edgeWeight = get( edge_weight, *myCFG[counter] );

      fileName = Synthesis::statPaths.return_rootDirectory() + Synthesis::statPaths.return_dataDirectory() + Synthesis::statPaths.return_outputFileName() + "." + "SFG" + ".";
      threadNum = ".T" + IntToString(counter) + ".out";

      fileName = fileName + newFileName + threadNum;
      ofstream outputFile(fileName.c_str(), ios::trunc);      //open a file for writing (truncate the current contents)

      if ( !outputFile )  //check to be sure file is open
         cout << "Error opening file.";

      //Begin writing to file
      //The contents of each BB, the number of instructions, the average dependency distance, and the number or memory ops
      outputFile << "\n\n";
      outputFile << "Number of threads:  " << "\n";
      outputFile << "Number of instructions:  " << "\n";
      //since we use the zero index for the total count, loop must be <=
// 		for ( UINT_32 moo = 0; moo <= numThreads; moo++ )
// 		{
// 			if ( moo == 0 )
// 			{
// 				outputFile << std::right << std::setw ( 11 ) << "Total " << ":  " << numInstructions[moo] << "\n";
// 			}
// 			else
// 				outputFile << std::right << std::setw ( 10 ) << "Thread " << std::left << moo << ":  " << numInstructions[moo] << "\n";
// 		}

// 		outputFile << "Number of basic blocks:  " << numBasicBlocks << "\n\n";
// 		outputFile << "\n\n";

      //Begin writing to file
      //The contents of each BB, the number of instructions, the average dependency distance, and the number or memory ops
      outputFile << "NUM - Number of Instructions in each BB\n";
      outputFile << "MOP - Number of memory operations in each BB\n";
      outputFile << "AVG - Average distance between instruction dependancies in each BB\n";
      outputFile << "CNT - Number of times BB is accessed\n";
      outputFile << "BRA - Probability taken\n";
      outputFile << "CS  - Is a critical section\n";
      outputFile << "SP  - Spawns a new thread\n";
      outputFile << "TG  - Target thread\n";
      outputFile << "DS  - Destroy a thread\n";
      outputFile << "WT  - Is waiting\n";
      outputFile << "BR  - Is a barrier\n";
      outputFile << "TID - Thread ID\n";
      outputFile << "LID - Lock ID\n";
      outputFile << "\n\n\n";

      //initialize uniform RV over [0,1)
      static boost::lagged_fibonacci607 rng(static_cast<unsigned> (std::time(0)));
      uniform_real<> uniformDistribution ( 0,1 );
      variate_generator<boost::lagged_fibonacci607&, boost::uniform_real<double> > chooseEdge ( rng, uniformDistribution );

      for(tie( vertexIterator, vertexEnd) = vertices(*myCFG[counter]); vertexIterator != vertexEnd; ++vertexIterator)
      {
         outputFile << std::setw ( 17 )      << std::left         << std::hex << basicBlockLocal[*vertexIterator].return_bbAddress() << std::dec;
         outputFile << std::left << "TID: "  << std::setw ( 4 )   << basicBlockLocal[*vertexIterator].return_threadID();
         outputFile << std::left << "CNT: "  << std::setw ( 10 )  << basicBlockLocal[*vertexIterator].return_bbCount();
         outputFile << std::left << "AVG: "  << std::setw ( 6 )   << std::setprecision ( 3 ) << basicBlockLocal[*vertexIterator].return_avgDistance();
         outputFile << std::left << "TX:  "  << std::setw ( 4 )   << basicBlockLocal[*vertexIterator].return_isTrans();
         outputFile << std::left << "CS:  "  << std::setw ( 4 )   << basicBlockLocal[*vertexIterator].return_isCritical();
         outputFile << std::left << "SP:  "  << std::setw ( 4 )   << basicBlockLocal[*vertexIterator].return_isSpawn();
         outputFile << std::left << "TG:  "  << std::setw ( 4 )   << basicBlockLocal[*vertexIterator].return_targetThread();
         outputFile << std::left << "WT:  "  << std::setw ( 4 )   << basicBlockLocal[*vertexIterator].return_isWait();
         outputFile << std::left << "BR:  "  << std::setw ( 4 )   << basicBlockLocal[*vertexIterator].return_isBarrier();
         outputFile << std::left << "DS:  "  << std::setw ( 4 )   << basicBlockLocal[*vertexIterator].return_isDestroy();
         outputFile << std::left << "SH:  "  << std::setw ( 4 )   << basicBlockLocal[*vertexIterator].return_isShared();
         //outputFile << std::left << "NUM:  "	<< std::setw(6) << basicBlockLocal[*vertexIterator].return_numInstructions();
         outputFile << std::left << "DEP:  " << std::setw ( 9 )   << basicBlockLocal[*vertexIterator].return_numDependancies();
         //outputFile << std::left << "MOP: " << std::setw(4) << basicBlockLocal[*vertexIterator].return_numMemoryOps();
         if(basicBlockLocal[*vertexIterator].return_isCritical() == 1)
            outputFile << std::left << "LID:  "  << std::setw ( 4 )  << std::hex << basicBlockLocal[*vertexIterator].return_lockID()  << std::dec;
         if(basicBlockLocal[*vertexIterator].return_isTrans() == 1)
            outputFile << std::left << "TxID:  "  << std::setw ( 4 ) << std::hex << basicBlockLocal[*vertexIterator].return_transID() << std::dec;
         outputFile << "\n";

         std::list <InstructionContainer>::iterator instructionListIterator;
         std::list <InstructionContainer> tempInstructionList = basicBlockLocal[*vertexIterator].return_instructionList();

         //cout << "\nList Size  " << tempInstructionList.size();
         for(instructionListIterator = tempInstructionList.begin(); instructionListIterator !=  tempInstructionList.end(); instructionListIterator++)
         {
            outputFile << std::hex << (ADDRESS_INT)instructionListIterator->return_instructionID();
            outputFile << "::";
            outputFile << Instruction::opcode2Name(instructionListIterator->return_opCode());
            outputFile << "-" << Instruction::subCode_to_Name(instructionListIterator->return_subCode());
            outputFile << "|";
         }
// 
// 			//outputFile << std::setw ( 40 ) << std::left << basicBlockLocal[*vertexIterator].return_insDisassembled();
         outputFile << endl;
      }

      outputFile << "\n\n#EOF";
      outputFile.close();
   }

   std::cout << "...Finished" << std::flush;
}//---------------------------------------------------------------------	// End SFGprinter //


/**
 * @name printSFGStructure
 * 
 * @return 
This function prints out the address of each node
and the address of its direct descendents.
**/
void printSFGStructure()
{
   /* Variable Declaration */
   ConfObject* statConf = new ConfObject;
   UINT_32 numThreads = totalNumThreads;
   graph_traits <BBGraph>::vertex_iterator vertexIterator, vertexEnd;
   graph_traits <BBGraph>::out_edge_iterator outEdgeIterator, outEdgeEnd;

   bool print_to_screen = 0;
   string fileName = Synthesis::statPaths.return_rootDirectory() + Synthesis::statPaths.return_dataDirectory() + Synthesis::statPaths.return_outputFileName() + ".";
   int rSize = (UINT_32)statConf->return_reductionFactor();
   fileName = fileName + "SFG_structure" + "." + IntToString(rSize) + ".out";
   ofstream graphOutputFile;

   /* Processes */
   cout << "\nPrinting SFG Layout" << flush;
   if(print_to_screen == 0)
   {
      graphOutputFile.open(fileName.c_str(), ios::trunc);         //open a file for writing (truncate the current contents)
      if(!graphOutputFile)  //check to be sure file is open
         cout << "Error opening file.";
   }

   for(UINT_32 threadCounter = 0; threadCounter < numThreads; threadCounter++)
   {
      cout << "..." << threadCounter << flush;

      basicBlock_name_map_t basicBlockLocal = get(basicBlock_t(), *myCFG[threadCounter]);
      edgeWeight_name_map_t edgeWeight = get(edge_weight, *myCFG[threadCounter]);

      if(print_to_screen == 1)
      {
         cout << "\nGraph Trace  ---  Thread " << threadCounter << "\n";
         cout << "\n***************************************************************************************\n";
      }
      else
      {
         graphOutputFile << "\nGraph Trace  ---  Thread " << threadCounter << "\n";
         graphOutputFile << "\n***************************************************************************************\n";
      }

      for(tie(vertexIterator, vertexEnd) = vertices(*myCFG[threadCounter]); vertexIterator != vertexEnd; ++vertexIterator)
      {
         if(print_to_screen == 1)
         {
            cout << "Vertex:  " << hex << basicBlockLocal[*vertexIterator].return_bbAddress() << dec;
            cout << "   Out Degree:  " << out_degree(*vertexIterator, *myCFG[threadCounter]);
            cout << "   Count:  " << basicBlockLocal[*vertexIterator].return_bbCount();
            cout << "\n";
            cout << "Children [Value]  :";
         }
         else
         {
            graphOutputFile << "Vertex:  " << hex << basicBlockLocal[*vertexIterator].return_bbAddress() << dec;
            graphOutputFile << "   Out Degree:  " << out_degree(*vertexIterator, *myCFG[threadCounter]);
            graphOutputFile << "   Count:  " << basicBlockLocal[*vertexIterator].return_bbCount();
            graphOutputFile << "\n";
            graphOutputFile << "Children [Value]  :";
         }

         for(tie(outEdgeIterator, outEdgeEnd) = out_edges(*vertexIterator, *myCFG[threadCounter]); outEdgeIterator != outEdgeEnd; ++outEdgeIterator)
         {
            if(print_to_screen == 1)
            {
               cout << hex << basicBlockLocal[target(*outEdgeIterator, *myCFG[threadCounter])].return_bbAddress() << dec;
               cout << " [" << edgeWeight[*outEdgeIterator] << "] ";
            }
            else
            {
               graphOutputFile << hex << basicBlockLocal[target(*outEdgeIterator, *myCFG[threadCounter])].return_bbAddress() << dec;
               graphOutputFile << " [" << edgeWeight[*outEdgeIterator] << "] ";
            }
         }

         if(print_to_screen == 1)
            cout << endl;
         else
            graphOutputFile << endl;
      }
   }

   if(print_to_screen == 1)
      cout << "\n***************************************************************************************\n";
   else
      graphOutputFile << "\n***************************************************************************************\n";

   graphOutputFile.close();
   delete statConf;

   std::cout << "...Finished" << std::flush;
}//---------------------------------------------------------------------	// End printSFGStructure //

/**
 * @name printBins
 * 
 * @return 
 */
void printBins()
{
   for(size_t i = 0; i < totalNumThreads; i++)
   {
      for(size_t j = 0; j < BIN_SIZE; j++)
         cout << per_threadReadBins[i][j] << "  ";
      cout << "  <>  ";
      for(size_t j = 0; j < BIN_SIZE; j++)
         cout << per_threadWriteBins[i][j] << "  ";
      cout << "\n";
   }
}//---------------------------------------------------------------------	// End printBins //


/**
 * @name PCFGprinter
 *
 * @return 
 */
void PCFGprinter()
{
   /* Variable Declaraion */
   string fileName;

   graph_traits <PCFG>::vertex_iterator vertexIterator, vertexEnd;
   graph_traits <PCFG>::out_edge_iterator outEdgeIterator, outEdgeEnd;
   flowNode_name_map_t flowNode = get(flowNode_t(), myPCFG);
   flow_edgeWeight_name_map_t edgeWeight = get(edge_weight, myPCFG);
   flowName_name_map_t  nodeName  = get(vertex_name, myPCFG);

   /* Processes */
   std::cout << "\nPrinting PCFG Contents" << flush;

   fileName = Synthesis::statPaths.return_rootDirectory() + Synthesis::statPaths.return_dataDirectory() + Synthesis::statPaths.return_outputFileName() + "." + "PCFG" + ".out";
   ofstream outputFile(fileName.c_str(), ios::trunc);      //open a file for writing (truncate the current contents)
   if(!outputFile)  //check to be sure file is open
      std::cout << "Error opening file.";

   //Begin writing to file
   //The contents of each BB, the number of instructions, the average dependency distance, and the number or memory ops
   outputFile << "\n\n";

   //Begin writing to file
   //The contents of each BB, the number of instructions, the average dependency distance, and the number or memory ops
   outputFile << "TID - Thread ID\n";
   outputFile << "INS - Number of Instructions\n";
   outputFile << "TG  - Target thread\n";
   outputFile << "WT  - Is waiting\n";
   outputFile << "BR  - Is a barrier\n";
   outputFile << "TID - Thread ID\n";
   outputFile << "LID - Lock ID\n";
   outputFile << "\n\n\n";

   //initialize uniform RV over [0,1)
   static boost::lagged_fibonacci607 rng(static_cast<unsigned> (std::time(0)));
   uniform_real<> uniformDistribution ( 0,1 );
   variate_generator<boost::lagged_fibonacci607&, boost::uniform_real<double> > chooseEdge ( rng, uniformDistribution );

   for(tie( vertexIterator, vertexEnd) = vertices(myPCFG); vertexIterator != vertexEnd; ++vertexIterator)
   {
      outputFile << std::setw( 8 )        << std::left         << std::hex << nodeName[*vertexIterator] << std::dec;
      outputFile << std::left << "ADDR: " << std::setw ( 10 )  << std::hex << flowNode[*vertexIterator].return_startPC() << std::dec;
      outputFile << std::left << "TID: "  << std::setw ( 4 )   << flowNode[*vertexIterator].return_threadID();
      outputFile << std::left << "INS: "  << std::setw ( 10 )  << flowNode[*vertexIterator].return_numInstructions();
      outputFile << std::left << "WT_INS: "  << std::setw ( 10 )  << round(flowNode[*vertexIterator].return_weighted_numInstructions());
      if(flowNode[*vertexIterator].return_isSpawn() == 1)
      {
         outputFile << std::left << "SP:  "  << std::setw ( 4 ) << flowNode[*vertexIterator].return_isSpawn() << "<>  ";
         for(std::vector < UINT_32 >::iterator thisIter = flowNode[*vertexIterator].childThreads.begin(); thisIter != flowNode[*vertexIterator].childThreads.end(); thisIter++)
         {
            outputFile << *thisIter << " ";
         }
      }
      outputFile << std::left << "WT:  "  << std::setw ( 4 )   << flowNode[*vertexIterator].return_isWait();
      outputFile << std::left << "BR:  "  << std::setw ( 4 )   << flowNode[*vertexIterator].return_isBarrier();
      outputFile << std::left << "TX:  "  << std::setw ( 4 )   << flowNode[*vertexIterator].return_isTrans();
      outputFile << std::left << "CS:  "  << std::setw ( 4 )   << flowNode[*vertexIterator].return_isCritical();
      if(flowNode[*vertexIterator].return_isCritical() == 1)
         outputFile << std::left << "LID:  "  << std::setw ( 4 )   << std::hex << flowNode[*vertexIterator].return_lockID() << std::dec;
      if(flowNode[*vertexIterator].return_isTrans() == 1)
         outputFile << std::left << "TxID:  "  << std::setw ( 4 )   << std::hex << flowNode[*vertexIterator].return_transID() << std::dec;
      outputFile << "\n";

      outputFile << endl;
   }

   outputFile << "\n\n#EOF";
   outputFile.close();

   std::cout << "...Finished" << std::flush;
}//---------------------------------------------------------------------	// End PCFGprinter //


/**
 * @name PCFGprinter
 * 
 * @param newFileName 
 * @return 
 */
void PCFGprinter(string newFileName)
{
   /* Variable Declaraion */
   string fileName;

   graph_traits <PCFG>::vertex_iterator vertexIterator, vertexEnd;
   graph_traits <PCFG>::out_edge_iterator outEdgeIterator, outEdgeEnd;
   flowNode_name_map_t flowNode = get(flowNode_t(), myPCFG);
   flow_edgeWeight_name_map_t edgeWeight = get(edge_weight, myPCFG);
   flowName_name_map_t  nodeName  = get(vertex_name, myPCFG);

   /* Processes */
   cout << "\nPrinting PCFG Contents" << flush;

   fileName = Synthesis::statPaths.return_rootDirectory() + Synthesis::statPaths.return_dataDirectory() + Synthesis::statPaths.return_outputFileName() + "." + "PCFG" + ".";
   fileName = fileName + newFileName + ".out";
   ofstream outputFile(fileName.c_str(), ios::trunc);      //open a file for writing (truncate the current contents)

   if ( !outputFile )  //check to be sure file is open
      cout << "Error opening file.";

   //Begin writing to file
   //The contents of each BB, the number of instructions, the average dependency distance, and the number or memory ops
   outputFile << "\n\n";

   //Begin writing to file
   //The contents of each BB, the number of instructions, the average dependency distance, and the number or memory ops
   outputFile << "TID - Thread ID\n";
   outputFile << "INS - Number of Instructions\n";
   outputFile << "TG  - Target thread\n";
   outputFile << "WT  - Is waiting\n";
   outputFile << "BR  - Is a barrier\n";
   outputFile << "TID - Thread ID\n";
   outputFile << "LID - Lock ID\n";
   outputFile << "\n\n\n";

   //initialize uniform RV over [0,1)
   static boost::lagged_fibonacci607 rng(static_cast<unsigned> (std::time(0)));
   uniform_real<> uniformDistribution ( 0,1 );
   variate_generator<boost::lagged_fibonacci607&, boost::uniform_real<double> > chooseEdge ( rng, uniformDistribution );

   for(tie(vertexIterator, vertexEnd) = vertices(myPCFG); vertexIterator != vertexEnd; ++vertexIterator)
   {
      outputFile << std::setw( 8 )        << std::left         << std::hex << nodeName[*vertexIterator] << std::dec;
      outputFile << std::left << "ADDR: " << std::setw ( 10 )  << std::hex << flowNode[*vertexIterator].return_startPC() << std::dec;
      outputFile << std::left << "TID: "  << std::setw ( 4 )   << flowNode[*vertexIterator].return_threadID();
      outputFile << std::left << "INS: "  << std::setw ( 10 )  << flowNode[*vertexIterator].return_numInstructions();
      outputFile << std::left << "WT_INS: "  << std::setw ( 10 )  << round(flowNode[*vertexIterator].return_weighted_numInstructions());
      if(flowNode[*vertexIterator].return_isSpawn() == 1)
      {
         outputFile << std::left << "SP:  "  << std::setw ( 4 ) << flowNode[*vertexIterator].return_isSpawn() << "<>  ";
         for(std::vector < UINT_32 >::iterator thisIter = flowNode[*vertexIterator].childThreads.begin(); thisIter != flowNode[*vertexIterator].childThreads.end(); thisIter++)
         {
            outputFile << *thisIter << " ";
         }
      }
      outputFile << std::left << "WT:  "  << std::setw ( 4 )   << flowNode[*vertexIterator].return_isWait();
      outputFile << std::left << "BR:  "  << std::setw ( 4 )   << flowNode[*vertexIterator].return_isBarrier();
      outputFile << std::left << "TX:  "  << std::setw ( 4 )   << flowNode[*vertexIterator].return_isTrans();
      outputFile << std::left << "CS:  "  << std::setw ( 4 )   << flowNode[*vertexIterator].return_isCritical();
      if(flowNode[*vertexIterator].return_isCritical() == 1)
         outputFile << std::left << "LID:  "  << std::setw ( 4 ) << std::hex << flowNode[*vertexIterator].return_lockID() << std::dec;
      if(flowNode[*vertexIterator].return_isTrans() == 1)
         outputFile << std::left << "TxID:  "  << std::setw ( 4 ) << std::hex << flowNode[*vertexIterator].return_transID() << std::dec;
      outputFile << "\n";

      outputFile << endl;
   }

   outputFile << "\n\n#EOF";
   outputFile.close();

   std::cout << "...Finished" << std::flush;
}//---------------------------------------------------------------------	// End PCFGprinter //


/**
 * @name printPCFGStructure
 * 
 * @return 
This function prints out the address of each node
and the address of its direct descendents.
**/
void printPCFGStructure()
{
   /* Variable Declaration */
   ConfObject* statConf = new ConfObject;
   UINT_32 numThreads = totalNumThreads;
   graph_traits <PCFG>::vertex_iterator vertexIterator, vertexEnd;
   graph_traits <PCFG>::out_edge_iterator outEdgeIterator, outEdgeEnd;
   flowNode_name_map_t flowNode = get(flowNode_t(), myPCFG);
   flow_edgeWeight_name_map_t edgeWeight = get(edge_weight, myPCFG);
   flowName_name_map_t  nodeName  = get(vertex_name, myPCFG);

   bool print_to_screen = 0;
   string fileName = Synthesis::statPaths.return_rootDirectory() + Synthesis::statPaths.return_dataDirectory() + Synthesis::statPaths.return_outputFileName() + ".";
   int rSize = (UINT_32)statConf->return_reductionFactor();
   fileName = fileName + "PCFG_structure" + "." + IntToString(rSize) + ".out";
   ofstream graphOutputFile;

   /* Processes */
   cout << "\nPrinting PCFG Layout" << flush;
   if(print_to_screen == 0)
   {
      graphOutputFile.open(fileName.c_str(), ios::trunc);         //open a file for writing (truncate the current contents)
      if(!graphOutputFile)  //check to be sure file is open
         cout << "Error opening file.";
   }

   if(print_to_screen == 1)
   {
      cout << "\nGraph Trace\n";
      cout << "\n***************************************************************************************\n";
   }
   else
   {
      graphOutputFile << "\nGraph Trace\n";
      graphOutputFile << "\n***************************************************************************************\n";
   }

   for(tie(vertexIterator, vertexEnd) = vertices(myPCFG); vertexIterator != vertexEnd; ++vertexIterator)
   {
      if(print_to_screen == 1)
      {
         cout << "Vertex:  " << nodeName[*vertexIterator];
         cout << "   Out Degree:  " << out_degree(*vertexIterator, myPCFG);
         cout << "\n";
         cout << "Children [Value]  :";
      }
      else
      {
         graphOutputFile << "Vertex:  " << nodeName[*vertexIterator];
         graphOutputFile << "   Out Degree:  " << out_degree(*vertexIterator, myPCFG);
         graphOutputFile << "\n";
         graphOutputFile << "Children [Value]  :";
      }

      for(tie(outEdgeIterator, outEdgeEnd) = out_edges(*vertexIterator, myPCFG); outEdgeIterator != outEdgeEnd; ++outEdgeIterator)
      {
         if(print_to_screen == 1)
         {
            cout << hex << nodeName[target(*outEdgeIterator, myPCFG)] << dec;
            cout << " [" << edgeWeight[*outEdgeIterator] << "] ";
         }
         else
         {
            graphOutputFile << hex << nodeName[target(*outEdgeIterator, myPCFG)] << dec;
            graphOutputFile << " [" << edgeWeight[*outEdgeIterator] << "] ";
         }
      }

      if(print_to_screen == 1)
         cout << endl;
      else
         graphOutputFile << endl;
   }

   if(print_to_screen == 1)
      cout << "\n***************************************************************************************\n";
   else
      graphOutputFile << "\n***************************************************************************************\n";

   graphOutputFile.close();
   delete statConf;

   std::cout << "...Finished" << std::flush;
}

}//end SynthPrinters
