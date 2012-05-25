//
// C++ Interface: ConfObject
//
// Description: 
//
//
// Author: hughes,,, <hughes@fraidy2-uf>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//


#ifndef CONF_OBJECT_H
#define CONF_OBJECT_H

#include <iostream>
#include "stat-types.h"
#include "SescConf.h"

class ConfObject
{
   public:
      /* Constructor */
      ConfObject() : printContents(0),verboseOutput(0),debugAll(0),debugUniqueBB(0),debugPrintDOTs(0),debugPrintGraph(0),debugPrintGraphStructure(0),enableSynth(0),reduceGraph(0),reductionFactor(0),maxBasicBlocks(0),cacheLineSize(0),enableProfiling(0), enablePerThreadProfiling(0), enablePerTransProfiling(0), windowSize(0), dumpType(0) { readFile(); }

      /* Variables */

      /* Functions */
      UINT_8 readFile(void)
      {
         update_printContents(SescConf->getBool("StatisticalModel","conf_debug_printContents"));
         update_verboseOutput(SescConf->getBool("StatisticalModel","stat_verbose_output"));

         //Stat
         update_debugAll(SescConf->getBool("StatisticalModel","stat_debug_all"));
         update_debugUniqueBB(SescConf->getBool("StatisticalModel","stat_debug_unique_BB_trace"));
         update_debugPrintDOTs(SescConf->getBool("StatisticalModel","stat_debug_printDOTs"));
         update_debugPrintGraph(SescConf->getBool("StatisticalModel","stat_debug_printGraph"));
         update_debugPrintGraphStructure(SescConf->getBool("StatisticalModel","stat_debug_printGraphStruct"));

         update_enableSynth(SescConf->getBool("StatisticalModel","stat_enable"));
         update_reduceGraph(SescConf->getBool("StatisticalModel","stat_reduceGraph"));
         update_reductionFactor(SescConf->getInt("StatisticalModel","stat_reductionFactor"));
         update_maxBasicBlocks(SescConf->getInt("StatisticalModel","stat_maxBasicBlocks"));

         update_cacheLineSize(SescConf->getInt("","cacheLineSize"));

         //Profiling
         update_enableProfiling(SescConf->getBool("Profiling","profiling_enable"));
         update_enablePerThreadProfiling(SescConf->getBool("Profiling","profiling_enable_per_thread"));
         update_enablePerTransProfiling(SescConf->getBool("Profiling","profiling_enable_per_transaction"));
         update_windowSize(SescConf->getInt("Profiling","profiling_windowSize"));
         update_dumpType(SescConf->getInt("Profiling","profiling_dumpType"));

         return 1;
      }

      void print(void)
      {
         std::cout << "\nContents of ConfObject:" << "\n";
         std::cout << "\tenableSynth " << return_enableSynth() << "\n";
         std::cout << "\treduceGraph " << return_reduceGraph() << "\n";

         std::cout << "\tdebugAll " << return_debugAll() << "\n";
         std::cout << "\tdebugUniqueBB " << return_debugUniqueBB() << "\n";
         std::cout << "\tdebugPrintDOTs " << return_debugPrintDOTs() << "\n";
         std::cout << "\tdebugPrintGraph " << return_debugPrintGraph() << "\n";
         std::cout << "\tdebugPrintGraphStructure " << return_debugPrintGraphStructure() << "\n";

         std::cout << "\treduceGraph " << return_reduceGraph() << "\n";
         std::cout << "\treductionFactor " << return_reductionFactor() << "\n";
         std::cout << "\tmaxBasicBlocks " << return_maxBasicBlocks() << "\n";

         //Profiling
         std::cout << "\tenableProfiling " << return_enableProfiling() << "\n";
         std::cout << "\tenablePerThreadProfiling " << return_enablePerThreadProfiling() << "\n";
         std::cout << "\tenablePerTransProfiling " << return_enablePerTransProfiling() << "\n";
         std::cout << "\twindowSize " << return_windowSize() << "\n";
         std::cout << "\tdumpType " << return_dumpType() << "\n";
         std::cout << std::flush;
      }

      /* UPDATE */
      UINT_8   update_printContents(UINT_32 printContents) { this->printContents = printContents; return 1; }
      UINT_8   update_verboseOutput(UINT_32 verboseOutput) { this->verboseOutput = verboseOutput; return 1; }

      UINT_8   update_debugAll(UINT_32 debugAll) { this->debugAll = debugAll; return 1; }
      UINT_8   update_debugUniqueBB(UINT_32 debugUniqueBB) { this->debugUniqueBB = debugUniqueBB; return 1; }
      UINT_8   update_debugPrintDOTs(UINT_32 debugPrintDOTs) { this->debugPrintDOTs = debugPrintDOTs; return 1; }
      UINT_8   update_debugPrintGraph(UINT_32 debugPrintGraph) { this->debugPrintGraph = debugPrintGraph; return 1; }
      UINT_8   update_debugPrintGraphStructure(UINT_32 debugPrintGraphStructure) { this->debugPrintGraphStructure = debugPrintGraphStructure; return 1; }

      UINT_8   update_enableSynth(UINT_32 enableSynth) { this->enableSynth = enableSynth; return 1; }
      UINT_8   update_reduceGraph(UINT_32 reduceGraph) { this->reduceGraph = reduceGraph; return 1; }
      UINT_8   update_reductionFactor(UINT_32 reductionFactor) { this->reductionFactor = reductionFactor; return 1; }
      UINT_8   update_maxBasicBlocks(UINT_32 maxBasicBlocks) { this->maxBasicBlocks = maxBasicBlocks; return 1; }

      UINT_8   update_cacheLineSize(UINT_32 cacheLineSize) { this->cacheLineSize = cacheLineSize; return 1; }

      //Profiling
      UINT_8   update_enableProfiling(BOOL enableProfiling) { this->enableProfiling = enableProfiling; return 1; }
      UINT_8   update_enablePerThreadProfiling(BOOL enablePerThreadProfiling) { this->enablePerThreadProfiling = enablePerThreadProfiling; return 1; }
      UINT_8   update_enablePerTransProfiling(BOOL enablePerTransProfiling) { this->enablePerTransProfiling = enablePerTransProfiling; return 1; }
      UINT_8   update_windowSize(UINT_32 windowSize) { this->windowSize = windowSize; return 1; }
      UINT_8   update_dumpType(UINT_32 dumpType) { this->dumpType = dumpType; return 1; }

      /* RETURN */
      BOOL     return_printContents(void) { return this->printContents; }
      BOOL     return_verboseOutput(void) { return this->verboseOutput; }

      BOOL     return_debugAll(void) { return this->debugAll; }
      BOOL     return_debugUniqueBB(void) { return this->debugUniqueBB; }
      BOOL     return_debugPrintDOTs(void) { return this->debugPrintDOTs; }
      BOOL     return_debugPrintGraph(void) { return this->debugPrintGraph; }
      BOOL     return_debugPrintGraphStructure(void) { return this->debugPrintGraphStructure; }

      BOOL     return_enableSynth(void) { return this->enableSynth; }
      BOOL     return_reduceGraph(void) { return this->reduceGraph; }
      INT_32   return_reductionFactor(void) { return this->reductionFactor; }
      INT_32   return_maxBasicBlocks(void) { return this->maxBasicBlocks; }

      INT_32   return_cacheLineSize(void) { return this->cacheLineSize; }

      //Profiling
      BOOL     return_enableProfiling(void) { return this->enableProfiling; }
      BOOL     return_enablePerThreadProfiling(void) { return this->enablePerThreadProfiling; }
      BOOL     return_enablePerTransProfiling(void) { return this->enablePerTransProfiling; }
      INT_32   return_windowSize(void) { return this->windowSize; }
      INT_32   return_dumpType(void) { return this->dumpType; }

   protected:
      /* Variables */


   private:
      /* Variables */
      //Stat
      BOOL     printContents;
      BOOL     verboseOutput;

      BOOL     debugAll;
      BOOL     debugUniqueBB;
      BOOL     debugPrintDOTs;
      BOOL     debugPrintGraph;
      BOOL     debugPrintGraphStructure;

      BOOL     enableSynth;
      BOOL     reduceGraph;
      INT_32   reductionFactor;
      INT_32   maxBasicBlocks;

      INT_32   cacheLineSize;

      //Profiling
      BOOL     enableProfiling;
      BOOL     enablePerThreadProfiling;
      BOOL     enablePerTransProfiling;
      INT_32   windowSize;
      INT_32   dumpType;

};

#endif
