//
// C++ Interface: codeGenerator
//
// Description: 
//
//
/// @author: Clay Hughes <>, (C) 2008
/// @date:           01/15/08
/// Last Modified:   02/05/08
//
// Copyright: See COPYING file that comes with this distribution
//
//////////////////////////////////////////////////////////////////////

#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <stdio.h>
#include <algorithm>

#include <boost/config.hpp>
#include <boost/random.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_iterator.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/depth_first_search.hpp>

#include "ProcessId.h"
#include "Synthetic.h"
#include "ConfObject.h"
#include "BasicBlock.h"
#include "InstructionMix.h"
#include "memoryOperations.h"
#include "stat-types.h"
#include "stat-boost-types.h"
#include "opcodes.h"
#include "statPaths.h"
#include "transReport.h"

#define MAX_VALUE 2147483648

#define LOOP_SIZE 1500 
//#define STREAM_SIZE (524288 / 4)
#define STREAM_SIZE (4096 / 4)
#define MEM_MULTIPLIER 4

#define ACC_MAX 70.0

extern std::string IntToString(INT_64 number);
extern std::string HexToString(INT_64 number);

namespace CodeGenerator
{
   class OperandList
   {
      public:
         OperandList(): rs(""),rt(""),rd(""),imm(""),rs_variable(""),rt_variable(""),rd_variable(""), clobberList("") {}

         string rs;
         string rt;
         string rd;
         string imm;

         string rs_variable;
         string rt_variable;
         string rd_variable;

         string clobberList;
   };

   class CodeLogic
   {
      public:
         CodeLogic(UINT_32 totalNumThreads)
         {
            lastLock.resize(totalNumThreads, 0);
            lastTrans.resize(totalNumThreads, 0);
            accumulatedValue.resize(totalNumThreads, 0);
         }

         std::map< ADDRESS_INT, ADDRESS_INT > virtual_to_physical;
         std::vector< IntRegValue > lastLock;
         std::vector< ADDRESS_INT > lastTrans;
         std::vector< UINT_32 > accumulatedValue;
   };

   class MemoryPerformance
   {
      public:
         MemoryPerformance()
         {
            precision = 0.10;

            readSetSize = (float)tmReport->return_summaryReadSetSize();
            writeSetSize = (float)tmReport->return_summaryWriteSetSize();

            readSet_reads = (float)tmReport->return_summaryReadSetSize() / (float)tmReport->return_summaryLoadCount();
            writeSet_writes = (float)tmReport->return_summaryWriteSetSize() / (float)tmReport->return_summaryStoreCount();
            writeSet_readSet = (float)tmReport->return_summaryWriteSetSize() / (float)tmReport->return_summaryReadSetSize();
            writes_reads = (float)tmReport->return_summaryStoreCount() / (float)tmReport->return_summaryLoadCount();
         }

         MemoryPerformance(MemoryPerformance &objectIn)
         {
             precision = objectIn.precision;
             readSet_reads = objectIn.readSet_reads;
             writeSet_writes = objectIn.writeSet_writes;
             writeSet_readSet = objectIn.writeSet_readSet;
             writes_reads = objectIn.writes_reads;
         }

         float return_precision() { return this->precision; }
         std::pair<float, float> readSet_reads_HiLo() { return std::make_pair(readSet_reads + readSet_reads * precision, readSet_reads - readSet_reads * precision); }
         std::pair<float, float> writeSet_writes_HiLo() { return std::make_pair(writeSet_writes + writeSet_writes * precision, writeSet_writes - writeSet_writes * precision); }
         std::pair<float, float> writeSet_readSet_HiLo() { return std::make_pair(writeSet_readSet + writeSet_readSet * precision, writeSet_readSet - writeSet_readSet * precision); }
         std::pair<float, float> writes_reads_HiLo() { return std::make_pair(writes_reads + writes_reads * precision, writes_reads - writes_reads * precision); }

         float readSetSize;
         float writeSetSize;

         float readSet_reads;
         float writeSet_writes;
         float writeSet_readSet;
         float writes_reads;

      private:
         float precision;
   };

   void anaylzeSynthetic(Synthetic *syntheticThreads[], UINT_32 arraySize);
   void anaylzeTransactionMemoryReferences(Synthetic *syntheticThreads[], UINT_32 arraySize);

   void writeOutSynthetic(Synthetic *syntheticThreads[], UINT_32 arraySize);
   void writeBasicBlock(CodeLogic *syntheticCodeBlock, const BasicBlock &localBasicBlock, THREAD_ID threadID, UINT_32 nodeID, std::ofstream &outputFile);
   void writeInstruction(CodeLogic *syntheticCodeBlock, const InstructionContainer &instructionIn, THREAD_ID threadID, UINT_32 nodeID, std::ofstream &outputFile, const std::map< ADDRESS_INT, UINT_32 > &conflictMap);
   void writeLabel(THREAD_ID threadID, UINT_32 nodeID, std::ofstream &outputFile);

   string translateInstruction(CodeLogic *synthContext, InstructionContainer instructionIn, OperandList &operandList, THREAD_ID threadID, UINT_32 nodeID, std::map< ADDRESS_INT, UINT_32 > conflictMap);
   string getIntVariable(RegType registerIn);
   string getFPVariable(RegType registerIn);

   void startLockSection(UINT_32 lockID, std::ofstream &outputFile);
   void endLockSection(UINT_32 lockID, std::ofstream &outputFile);
   void startTransSection(UINT_32 transID, std::ofstream &outputFile);
   void endTransSection(UINT_32 transID, std::ofstream &outputFile);
   void writeBarrier(string barrID, UINT_32 numProcs, std::ofstream &outputFile);
   void writeWait(std::ofstream &outputFile);
   void writeSpawn(THREAD_ID threadID, std::ofstream &outputFile);
   void writeSpawn(std::vector < UINT_32 > childThreads, std::ofstream &outputFile);
   void writeSpawn(std::vector < UINT_32 > childThreads, std::ofstream &outputFile, UINT_32 diffThreads);

   void startAccumulationLoop(UINT_32 threadID, UINT_32 accumulation, std::ofstream &outputFile);
   void endAccumulationLoop(std::ofstream &outputFile);

   void headerGen(std::ofstream &outputFile);
   void trailerGen(CodeLogic *synthContext, THREAD_ID threadID, std::ofstream &outputFile);
   void funcHeaderGen(THREAD_ID threadID, std::ofstream &outputFile);
   void funcTrailerGen(CodeLogic *synthContext, THREAD_ID threadID, std::ofstream &outputFile);

   BasicBlock instructionGenerator(float numInstructions, InstructionMix instructionMix);
}
#endif

