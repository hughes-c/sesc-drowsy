/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Karin Strauss

This file is part of SESC.

SESC is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2, or (at your option) any later version.

SESC is    distributed in the  hope that  it will  be  useful, but  WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should  have received a copy of  the GNU General  Public License along with
SESC; see the file COPYING.  If not, write to the  Free Software Foundation, 59
Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <stdlib.h>

#include <vector>

// sesc internal stuff
#include "ReportGen.h"
#include "SescConf.h"
#include "callback.h"

// sesc OS model
#include "OSSim.h"

// hardware models
#include "Processor.h"
#include "SMTProcessor.h"
#include "SMemorySystem.h"

// debugging defines
#include "SMPDebug.h"

#if (defined TM)
#include "transReport.h"
#include "transCoherence.h"

transReport *tmReport = 0;
#endif

// statistics and synthesis
#if defined(STAT)
#include "stat_synthesis.h"
#endif

// statistics and synthesis
#if defined(PROFILE)
#include "stat_profile.h"
#endif

int main(int argc, char**argv, char **envp)
{
  osSim = new OSSim(argc, argv, envp);

  int nProcs = SescConf->getRecordSize("","cpucore");

  GLOG(SMPDBG_CONSTR, "Number of Processors: %d", nProcs);

  #if defined(DEBUG)
  MSG("Simulator in Debug Mode\n");
  #endif

  #if defined(STAT)
  std:cout << "STAT Enabled\n";
  Synthesis::init();
  #elif defined(PROFILE)
  std:cout << "PROFILING Enabled\n";
  Profiling::init();
  #else
  std::cout << "Compiled on " << __DATE__ << " at " << __TIME__ << std::endl;
  #endif

   // processor and memory build
   std::vector<GProcessor *>    pr(nProcs);
   std::vector<GMemorySystem *> ms(nProcs);

   for(int i = 0; i < nProcs; i++) {
      GLOG(SMPDBG_CONSTR, "Building processor %d and its memory subsystem", i);
      GMemorySystem *gms = new SMemorySystem(i);
      gms->buildMemorySystem();
      ms[i] = gms;
      pr[i] = 0;
      if(SescConf->checkInt("cpucore","smtContexts",i)) {
         if( SescConf->getInt("cpucore","smtContexts",i) > 1 )
         pr[i] =new SMTProcessor(ms[i], i);
      }
      if (pr[i] == 0)
         pr[i] =new Processor(ms[i], i);
   }

   #if defined(TRANS_DVFS)
   transGCM->set_prPointer(&pr);
   #endif

  GLOG(SMPDBG_CONSTR, "I am booting now");
  osSim->boot();
  GLOG(SMPDBG_CONSTR, "Terminating simulation");

  for(int i = 0; i < nProcs; i++) {
    delete pr[i];
    delete ms[i];
  }

  #if defined(STAT)
  Synthesis::finished();
  #elif defined(PROFILE)
  Profiling::finished();
  #endif

  delete osSim;

  return 0;
}
