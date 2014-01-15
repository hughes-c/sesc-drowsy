/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Luis Ceze
                  Karin Strauss

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

#include <iostream>
#include <iomanip>

#include <sys/time.h>
#include <unistd.h>

#include "GProcessor.h"
#include "OSSim.h"

#include "FetchEngine.h"
#include "ExecutionFlow.h"
#include "GMemoryOS.h"
#include "GMemorySystem.h"
#include "LDSTBuffer.h"
#include "opcodes.h"

#if (defined TM)
#include "transReport.h"
#endif

#ifdef XACTION
#include "XactionManager.h"
#endif


//stat
#if defined(STAT)
#include "ConfObject.h"
#include "graphManipulation.h"

namespace Synthesis
{
   extern std::vector < BOOL > isTransaction;
   extern void checkContainerSizes(THREAD_ID threadID);
   extern void analysis(tuple<DInst, Time_t>  tempTuple);
}

#endif

#if defined(PROFILE)
#include <deque>
#include "ConfObject.h"
#include "workloadCharacteristics.h"
extern   UINT_16 totalNumThreads;

namespace Profiling
{
   extern std::deque< UINT_32 > transactionDistance;
   extern std::vector< BOOL > firstTransaction;
   extern std::vector< WorkloadCharacteristics * > currBBStats;
   extern std::vector< BOOL > isTransaction;
   extern void analysis(tuple<DInst, Time_t>  tempTuple);
}
#endif

#if defined(STAT_COMMON)
#include <boost/tuple/tuple.hpp>
std::vector< std::deque< tuple<DInst, Time_t> > * > instructionQueueVector;
#endif

GProcessor::GProcessor(GMemorySystem *gm, CPU_t i, size_t numFlows)
  :Id(i)
  ,FetchWidth(SescConf->getInt("cpucore", "fetchWidth",i))
  ,IssueWidth(SescConf->getInt("cpucore", "issueWidth",i))
  ,RetireWidth(SescConf->getInt("cpucore", "retireWidth",i))
  ,RealisticWidth(RetireWidth < IssueWidth ? RetireWidth : IssueWidth)
  ,InstQueueSize(SescConf->getInt("cpucore", "instQueueSize",i))
  ,InOrderCore(SescConf->getBool("cpucore","inorder",i))
  ,MaxFlows(numFlows)
  ,MaxROBSize(SescConf->getInt("cpucore", "robSize",i))
  ,memorySystem(gm)
  ,ROB(MaxROBSize)
  ,replayQ(2*MaxROBSize)
  ,lsq(this, i)
  ,clusterManager(gm, this)
  ,robUsed("Proc(%d)_robUsed", i)
  ,noFetch("Processor(%d)_noFetch", i)
  ,noFetch2("Processor(%d)_noFetch2", i)
  ,retired("ExeEngine(%d)_retired", i)
  ,notRetiredOtherCause("ExeEngine(%d):noRetOtherCause", i)
  ,nLocks("Processor(%d):nLocks", i)
  ,nLockContCycles("Processor(%d):nLockContCycles", i)
{
  // osSim should be already initialized
  I(osSim);
  osSim->registerProc(this);

  SescConf->isInt("cpucore", "issueWidth",i);
  SescConf->isLT("cpucore", "issueWidth", 1025,i); // no longer than unsigned short

  SescConf->isInt("cpucore"    , "retireWidth",i);
  SescConf->isBetween("cpucore" , "retireWidth", 0, 32700,i);

  SescConf->isInt("cpucore"    , "robSize",i);
  SescConf->isBetween("cpucore" , "robSize", 2, 262144,i);

  SescConf->isInt("cpucore"    , "intRegs",i);
  SescConf->isBetween("cpucore" , "intRegs", 16, 262144,i);

  SescConf->isInt("cpucore"    , "fpRegs",i);
  SescConf->isBetween("cpucore" , "fpRegs", 16, 262144,i);

  regPool[0] = SescConf->getInt("cpucore", "intRegs",i);
  regPool[1] = SescConf->getInt("cpucore", "fpRegs",i);
  regPool[2] = 262144; // Unlimited registers for invalid output

#ifdef SESC_MISPATH
  for (int j = 0 ; j < INSTRUCTION_MAX_DESTPOOL; j++)
    misRegPool[j] = 0;
#endif

  nStall[0] = 0 ; // crash if used
  nStall[SmallWinStall]     = new GStatsCntr("ExeEngine(%d):nSmallWin",i);
  nStall[SmallROBStall]     = new GStatsCntr("ExeEngine(%d):nSmallROB",i);
  nStall[SmallREGStall]     = new GStatsCntr("ExeEngine(%d):nSmallREG",i);
  nStall[OutsLoadsStall]    = new GStatsCntr("ExeEngine(%d):nOutsLoads",i);
  nStall[OutsStoresStall]   = new GStatsCntr("ExeEngine(%d):nOutsStores",i);
  nStall[OutsBranchesStall] = new GStatsCntr("ExeEngine(%d):nOutsBranches",i);
  nStall[ReplayStall]       = new GStatsCntr("ExeEngine(%d):nReplays",i);
  nStall[PortConflictStall] = new GStatsCntr("ExeEngine(%d):PortConflict",i);
  nStall[SwitchStall]       = new GStatsCntr("ExeEngine(%d):switch",i);

  for(unsigned r = 0; r < MaxNoRetResp; r++) {
    for(unsigned s = 0; s < MaxInstType; s++) {
      for(unsigned t = 0; t < MaxRetOutcome; t++) {
        notRetired[r][s][t] = &notRetiredOtherCause;
      }
    }
  }

  notRetired[Self][iOpInvalid][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetSelf_iOpInvalid_NotExecuted",i);
  notRetired[Self][iALU][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetSelf_iALU_NotExecuted",i);
  notRetired[Self][iMult][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetSelf_iMult_NotExecuted",i);
  notRetired[Self][iDiv][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetSelf_iDiv_NotExecuted",i);
  notRetired[Self][iBJ][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetSelf_iBJ_NotExecuted",i);
  notRetired[Self][iLoad][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetSelf_iLoad_NotExecuted",i);
  notRetired[Self][iStore][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetSelf_iStore_NotExecuted",i);
  notRetired[Self][fpALU][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetSelf_fpALU_NotExecuted",i);
  notRetired[Self][fpMult][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetSelf_fpMult_NotExecuted",i);
  notRetired[Self][fpDiv][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetSelf_fpDiv_NotExecuted",i);
  notRetired[Self][iFence][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetSelf_iFence_NotExecuted",i);
  notRetired[Self][iLoad][NotFinished] =
    new GStatsCntr("ExeEngine(%d):noRetSelf_iLoad_NotFinished",i);
  notRetired[Self][iStore][NoCacheSpace] =
    new GStatsCntr("ExeEngine(%d):noRetSelf_iStore_NoCacheSpace",i);
  notRetired[Self][iStore][NoCachePorts] =
    new GStatsCntr("ExeEngine(%d):noRetSelf_iStore_NoCachePorts",i);
  notRetired[Self][iStore][WaitForFence] =
    new GStatsCntr("ExeEngine(%d):noRetSelf_iStore_WaitForFence",i);
  notRetired[Self][iFence][NoCacheSpace] =
    new GStatsCntr("ExeEngine(%d):noRetSelf_iFence_NoCacheSpace",i);
  notRetired[Self][iFence][WaitForFence] =
    new GStatsCntr("ExeEngine(%d):noRetSelf_iFence_WaitForFence",i);

  notRetired[Other][iOpInvalid][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetOther_iOpInvalid_NotExecuted",i);
  notRetired[Other][iALU][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetOther_iALU_NotExecuted",i);
  notRetired[Other][iMult][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetOther_iMult_NotExecuted",i);
  notRetired[Other][iDiv][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetOther_iDiv_NotExecuted",i);
  notRetired[Other][iBJ][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetOther_iBJ_NotExecuted",i);
  notRetired[Other][iLoad][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetOther_iLoad_NotExecuted",i);
  notRetired[Other][iStore][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetOther_iStore_NotExecuted",i);
  notRetired[Other][fpALU][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetOther_fpALU_NotExecuted",i);
  notRetired[Other][fpMult][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetOther_fpMult_NotExecuted",i);
  notRetired[Other][fpDiv][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetOther_fpDiv_NotExecuted",i);
  notRetired[Other][iFence][NotExecuted] =
    new GStatsCntr("ExeEngine(%d):noRetOther_iFence_NotExecuted",i);
  notRetired[Other][iLoad][NotFinished] =
    new GStatsCntr("ExeEngine(%d):noRetOther_iLoad_NotFinished",i);
  notRetired[Other][iStore][NoCacheSpace] =
    new GStatsCntr("ExeEngine(%d):noRetOther_iStore_NoCacheSpace",i);
  notRetired[Other][iStore][NoCachePorts] =
    new GStatsCntr("ExeEngine(%d):noRetOther_iStore_NoCachePorts",i);
  notRetired[Other][iStore][WaitForFence] =
    new GStatsCntr("ExeEngine(%d):noRetOther_iStore_WaitForFence",i);
  notRetired[Other][iFence][NoCacheSpace] =
    new GStatsCntr("ExeEngine(%d):noRetOther_iFence_NoCacheSpace",i);
  notRetired[Other][iFence][WaitForFence] =
    new GStatsCntr("ExeEngine(%d):noRetOther_iFence_WaitForFence",i);

  clockTicks=0;

  char cadena[100];
  sprintf(cadena, "Proc(%d)", (int)i);

#if defined(SEP_DVFS)
   std::string component, entry;
   std::string fullComponent, fullEntry;

   component = "renameEnergy";
   for(size_t counter = 0; counter < NUM_STATES; counter++)
   {
      fullComponent = component + GProcessor::state_to_freq(counter);
      renameEnergy[counter] = new GStatsEnergy(fullComponent.c_str(), cadena, i, IssuePower
            ,EnergyMgr::get(fullComponent.c_str(),i));
   }

   entry = "ROBEnergy";
   component = "robEnergy";
   for(size_t counter = 0; counter < NUM_STATES; counter++)
   {
      fullEntry = entry + GProcessor::state_to_freq(counter);
      fullComponent = component + GProcessor::state_to_freq(counter);
      robEnergy[counter] = new GStatsEnergy(fullEntry.c_str(), cadena, i, IssuePower
            , EnergyMgr::get(fullComponent.c_str(), i));
   }

   entry = "wrIRegEnergy";
   component = "wrRegEnergy";
   for(size_t counter = 0; counter < NUM_STATES; counter++)
   {
      fullEntry = entry + GProcessor::state_to_freq(counter);
      fullComponent = component + GProcessor::state_to_freq(counter);
      wrRegEnergy[0][counter] = new GStatsEnergy(fullEntry.c_str(), cadena, i, ExecPower
            ,EnergyMgr::get(fullComponent.c_str(), i));
   }

   entry = "wrFPRegEnergy";
   component = "wrRegEnergy";
   for(size_t counter = 0; counter < NUM_STATES; counter++)
   {
      fullEntry = entry + GProcessor::state_to_freq(counter);
      fullComponent = component + GProcessor::state_to_freq(counter);
      wrRegEnergy[1][counter] = new GStatsEnergy(fullEntry.c_str(), cadena, i, ExecPower
            ,EnergyMgr::get(fullComponent.c_str(), i));
   }

   for(size_t counter = 0; counter < NUM_STATES; counter++)
   {
      wrRegEnergy[2][counter] = new GStatsEnergyNull();
   }

   entry = "rdIRegEnergy";
   component = "rdRegEnergy";
   for(size_t counter = 0; counter < NUM_STATES; counter++)
   {
      fullEntry = entry + GProcessor::state_to_freq(counter);
      fullComponent = component + GProcessor::state_to_freq(counter);
      rdRegEnergy[0][counter] = new GStatsEnergy(fullEntry.c_str(), cadena, i, ExecPower
            ,EnergyMgr::get(fullComponent.c_str(), i));
   }

   entry = "rdFPRegEnergy";
   component = "rdRegEnergy";
   for(size_t counter = 0; counter < NUM_STATES; counter++)
   {
      fullEntry = entry + GProcessor::state_to_freq(counter);
      fullComponent = component + GProcessor::state_to_freq(counter);
      rdRegEnergy[1][counter] = new GStatsEnergy(fullEntry.c_str(), cadena, i, ExecPower
            ,EnergyMgr::get(fullComponent.c_str(), i));
   }

   for(size_t counter = 0; counter < NUM_STATES; counter++)
   {
      rdRegEnergy[2][counter] = new GStatsEnergyNull();
   }

#else
  renameEnergy = new GStatsEnergy("renameEnergy", cadena, i, IssuePower
                                  ,EnergyMgr::get("renameEnergy",i));

  robEnergy = new GStatsEnergy("ROBEnergy",cadena, i, IssuePower, EnergyMgr::get("robEnergy",i));

  wrRegEnergy[0] = new GStatsEnergy("wrIRegEnergy", cadena, i, ExecPower
                                    ,EnergyMgr::get("wrRegEnergy",i));

  wrRegEnergy[1] = new GStatsEnergy("wrFPRegEnergy", cadena, i, ExecPower
                                    ,EnergyMgr::get("wrRegEnergy",i));

  wrRegEnergy[2] = new GStatsEnergyNull();

  rdRegEnergy[0] = new GStatsEnergy("rdIRegEnergy", cadena , i, ExecPower
                                    ,EnergyMgr::get("rdRegEnergy",i));

  rdRegEnergy[1] = new GStatsEnergy("rdFPRegEnergy", cadena , i, ExecPower
                                    ,EnergyMgr::get("rdRegEnergy",i));

  rdRegEnergy[2] = new GStatsEnergyNull();
#endif

#if (defined TM)
  signatureEnergy = new GStatsEnergy("sigEnergy", cadena, i, ExecPower
        ,EnergyMgr::get("signatureEnergy",i));
#endif

  I(ROB.size() == 0);

  // CHANGE "PendingWindow" instead of "Proc" for script compatibility reasons
  buildInstStats(nInst, "PendingWindow");


#ifdef SESC_MISPATH
  buildInstStats(nInstFake, "FakePendingWindow");
#endif


#ifdef XACTION
  xaction = new XactionManager(Id);
#endif

#ifdef SESC_ENERGY
  set_executeState(DEFAULT_DVFS);
  for(size_t counter = 0; counter < NUM_STATES; counter++)
  {
     proc_stateCycles[Id][counter] = 0;
     global_stateCycles[counter] = 0;
     proc_gateCycles[Id][counter] = 0;
  }
#endif

#if (defined TM)
  instCountTM = 0;
#endif
}

GProcessor::~GProcessor()
{

#ifdef SESC_ENERGY
   std::cerr << Id << ", ";
   for(size_t counter = 0; counter < NUM_STATES; counter++)
   {
      std::cerr << "State:  " << counter << " -- ";
      std::cerr << proc_stateCycles[Id][counter] << ", ";
   }

   std::cerr << std::endl;
#endif

#if defined(SEP_DVFS)
   for(unsigned int counter = 0; counter < NUM_STATES; counter++)
   {
      delete renameEnergy[counter];
      delete robEnergy[counter];
      delete wrRegEnergy[0][counter];
      delete wrRegEnergy[1][counter];
      delete wrRegEnergy[2][counter];
      delete rdRegEnergy[0][counter];
      delete rdRegEnergy[1][counter];
      delete rdRegEnergy[2][counter];
   }
#else
   delete renameEnergy;
   delete robEnergy;
   delete wrRegEnergy[0];
   delete wrRegEnergy[1];
   delete wrRegEnergy[2];
   delete rdRegEnergy[0];
   delete rdRegEnergy[1];
   delete rdRegEnergy[2];
#endif

#if (defined TM)
   delete signatureEnergy;
#endif

#if defined(STAT_COMMON)
   for(UINT_32 counter = 0; counter < instructionQueueVector.size(); counter++)
   {
      while(instructionQueueVector[counter]->empty() == 0)
      {
#if defined(PROFILE)
         Profiling::analysis(instructionQueueVector[counter]->front());
#elif defined(STAT)
         Synthesis::analysis(instructionQueueVector[counter]->front());
#endif
         instructionQueueVector[counter]->pop_front();
      }
   }
#endif
}

void GProcessor::buildInstStats(GStatsCntr *i[MaxInstType], const char *txt)
{
  bzero(i, sizeof(GStatsCntr *) * MaxInstType);

  i[iOpInvalid] = new GStatsCntr("%s(%d)_iOpInvalid:n", txt, Id);
  i[iALU]   = new GStatsCntr("%s(%d)_iALU:n", txt, Id);
  i[iMult]  = new GStatsCntr("%s(%d)_iComplex:n", txt, Id);
  i[iDiv]   = i[iMult];

  i[iBJ]    = new GStatsCntr("%s(%d)_iBJ:n", txt, Id);

  i[iLoad]  = new GStatsCntr("%s(%d)_iLoad:n", txt, Id);
  i[iStore] = new GStatsCntr("%s(%d)_iStore:n", txt, Id);

  i[fpALU]  = new GStatsCntr("%s(%d)_fpALU:n", txt, Id);
  i[fpMult] = new GStatsCntr("%s(%d)_fpComplex:n", txt, Id);
  i[fpDiv]  = i[fpMult];

  i[iFence] = new GStatsCntr("%s(%d)_other:n", txt, Id);
  i[iEvent] = i[iFence];

  IN(forall((int a=1;a<(int)MaxInstType;a++), i[a] != 0));
}

StallCause GProcessor::sharedAddInst(DInst *dinst)
{
  // rename an instruction. Steps:
  //
  // 1-Get a ROB entry
  //
  // 2-Get a register
  //
  // 3-Try to schedule in Resource (window entry...)

  if( ROB.size() >= MaxROBSize ) {
    return SmallROBStall;
  }

#if SESC_INORDER
  if (switching) {
    if(!ROB.empty())
      return SwitchStall; // New stall Type to add
    switching = false;
  }
#endif

  const Instruction *inst = dinst->getInst();

  // Register count
  I(inst->getDstPool()<3);
#ifdef SESC_MISPATH
  if ((regPool[inst->getDstPool()] - misRegPool[inst->getDstPool()]) == 0)
    return SmallREGStall;
#else
  if (regPool[inst->getDstPool()] == 0)
    return SmallREGStall;
#endif

  Resource *res = clusterManager.getResource(inst->getOpcode());
  I(res);

  const Cluster *cluster = res->getCluster();
  StallCause sc = cluster->canIssue(dinst);
  if (sc != NoStall)
    return sc;

  sc = res->canIssue(dinst);
  if (sc != NoStall)
    return sc;

  // BEGIN INSERTION (note that schedule already inserted in the window)
  dinst->markIssued();

#ifdef SESC_BAAD
  dinst->setRenameTime();
#endif

#ifdef SESC_MISPATH
  if (dinst->isFake()) {
    nInstFake[inst->getOpcode()]->inc();
    misRegPool[inst->getDstPool()]++;
  }else{
    nInst[inst->getOpcode()]->inc();
    regPool[inst->getDstPool()]--;
  }
#else
  nInst[inst->getOpcode()]->inc();
  regPool[inst->getDstPool()]--;
#endif

#if defined(SEP_DVFS)
  renameEnergy[executeState]->inc();  // Rename RAT
  robEnergy[executeState]->inc();     // one for insert

  rdRegEnergy[inst->getSrc1Pool()][executeState]->inc();
  rdRegEnergy[inst->getSrc2Pool()][executeState]->inc();
  wrRegEnergy[inst->getDstPool()][executeState]->inc();
#else
  renameEnergy->inc();  // Rename RAT
  robEnergy->inc();     // one for insert

  rdRegEnergy[inst->getSrc1Pool()]->inc();
  rdRegEnergy[inst->getSrc2Pool()]->inc();
  wrRegEnergy[inst->getDstPool()]->inc();
#endif

  ROB.push(dinst);


  dinst->setResource(res);

  return NoStall;
}

int GProcessor::issue(PipeQueue &pipeQ)
{
  int i=0; // Instructions executed counter
  int j=0; // Fake Instructions counter

  I(!pipeQ.instQueue.empty());

  if(!replayQ.empty()) {
    issueFromReplayQ();
    nStall[ReplayStall]->add(RealisticWidth);
    return 0;  // we issued 0 from the instQ;
    // FIXME:check if we can issue from replayQ and
    // fetchQ during the same cycle
  }

  do{
    IBucket *bucket = pipeQ.instQueue.top();
    do{
      I(!bucket->empty());
      if( i >= IssueWidth ) {
        return i+j;
      }

      I(!bucket->empty());

      DInst *dinst = bucket->top();

#if defined(SEP_DVFS)
      dinst->procExecuteState = get_executeState();
#endif

#ifdef TASKSCALAR
      if (!dinst->isFake()) {
        if (dinst->getLVID()==0 || dinst->getLVID()->isKilled()) {
          // Task got killed. Just swallow the instruction
          dinst->killSilently();
          bucket->pop();
          j++;
          continue;
        }
      }
#endif

      StallCause c = addInst(dinst);
      if (c != NoStall) {
        if (i < RealisticWidth)
          nStall[c]->add(RealisticWidth - i);
        return i+j;
      }
      i++;

      bucket->pop();

    }while(!bucket->empty());

    pipeQ.pipeLine.doneItem(bucket);
    pipeQ.instQueue.pop();
  }while(!pipeQ.instQueue.empty());

  return i+j;
}

int GProcessor::issueFromReplayQ()
{
  int nIssued = 0;

  while(!replayQ.empty()) {
    DInst *dinst = replayQ.top();

#if defined(SEP_DVFS)
    dinst->procExecuteState = get_executeState();
#endif

    StallCause c = addInst(dinst);
    if (c != NoStall) {
      if (nIssued < RealisticWidth)
        nStall[c]->add(RealisticWidth - nIssued);
      break;
    }
    nIssued++;
    replayQ.pop();
    if(nIssued >= IssueWidth)
      break;
  }
  return nIssued;
}

void GProcessor::replay(DInst *dinst)
{
  //traverse the ROB for instructions younger than dinst
  //and add them to the replayQ.

#ifndef DOREPLAY
  return;
#endif

  if(dinst->isDeadInst())
    return;

  bool pushInst = false;
  unsigned int robPos = ROB.getIdFromTop(0); // head or top
  while(1) {
    DInst *robDInst = ROB.getData(robPos);
    if(robDInst == dinst)
      pushInst = true;

    if(pushInst && !robDInst->isDeadInst()) {
      replayQ.push(robDInst->clone());
      robDInst->setDeadInst();
    }

    robPos = ROB.getNextId(robPos);
    if (ROB.isEnd(robPos))
      break;
  }
}

void GProcessor::report(const char *str)
{
  Report::field("Proc(%d):clockTicks=%lld", Id, clockTicks);
  memorySystem->getMemoryOS()->report(str);

}

void GProcessor::addEvent(EventType ev, CallbackBase *cb, int vaddr)
{
  currentFlow()->addEvent(ev,cb,vaddr);
}

void GProcessor::retire()
{

#ifdef DEBUG

  // Check for progress. When a processor gets stuck, it sucks big time
  if ((((int)globalClock) & 0x1FFFFFL) == 0) {
    if (ROB.empty()) {
      // ROB should not be empty for lots of time
      if (prevDInstID == 1) {
        MSG("GProcessor::retire CPU[%d] ROB empty for long time @%lld", Id, globalClock);
      }
      prevDInstID = 1;
    }else{
      DInst *dinst = ROB.top();
      if (prevDInstID == dinst->getID()) {
        I(0);
        MSG("ExeEngine::retire CPU[%d] no forward progress from pc=0x%x with %d @%lld"
            ,Id, (uint)dinst->getInst()->getAddr()
            ,(uint)dinst->getInst()->currentID(), globalClock );
        dinst->dump("HEAD");
        LDSTBuffer::dump("");
      }
      prevDInstID = dinst->getID();
    }
  }
#endif

//BEGIN DROWSY ---------------------------------------------------------------------------------------------------------

   if(globalClock >0 && globalClock% 2000 == 0)
   {
      MemObj *localSource = this->memorySystem->getDataSource();

      if(std::string(localSource->getSymbolicName()).find("_D") != std::string::npos)//test for data cache
         localSource->sleepCacheLines();

      //  std::cout << globalClock << "  " << localSource->getSymbolicName() << std::endl;
   }

//END DROWSY -----------------------------------------------------------------------------------------------------------

  robUsed.sample(ROB.size());

  ushort i;

  for(i=0;i<RetireWidth && !ROB.empty();i++) {
    DInst *dinst = ROB.top();

    if( !dinst->isExecuted() ) {
      addStatsNoRetire(i, dinst, NotExecuted);
      return;
    }

    // save it now because retire can destroy DInst
    int rp = dinst->getInst()->getDstPool();



//BEGIN STAT --------------------------------------------------------------------------------------------------------
#if defined(STAT)

   ConfObject* statConf = new ConfObject;
   THREAD_ID threadID = dinst->get_threadID();

   //Check to see if we're profling or not
   if(statConf->return_enableSynth() == 1)
   {
      Synthesis::checkContainerSizes(threadID);

//FIXME Bug with flushing the instructionQueues
      tuple<DInst, Time_t> instruction_cycle(*dinst, globalClock);
      Synthesis::analysis(instruction_cycle);
//       instructionQueueVector[threadID]->push_back(instruction_cycle);
//       if((INT_32)instructionQueueVector[threadID]->size() > statConf->return_windowSize())
//       {
//          Synthesis::analysis(instructionQueueVector[threadID]->front());
//          instructionQueueVector[threadID]->pop_front();
//       }
//
//       //if this is the end of the thread, we need to flush the instruction queue
//       if(dinst->getInst()->getICode()->func == mint_exit)
//       {
//          while(instructionQueueVector[threadID]->empty() == 0)
//          {
//             Synthesis::analysis(instructionQueueVector[threadID]->front());
//             instructionQueueVector[threadID]->pop_front();
//          }
//       }
   }

   delete statConf;
#endif

//END STAT ----------------------------------------------------------------------------------------------------------

//BEGIN PROFILING --------------------------------------------------------------------------------------------------------
#if defined(PROFILE)
   ConfObject *statConf = new ConfObject;
   THREAD_ID threadID = dinst->get_threadID();
   if(statConf->return_enableProfiling() == 1)
   {
      tuple<DInst, Time_t> instruction_cycle(*dinst, globalClock);

      //Need to ensure that the vector is large enough to hold the next thread
      if(threadID >= Profiling::transactionDistance.size())
      {
         if(threadID == Profiling::transactionDistance.size())
         {
            std::cerr << "Profiling::Push back to transactionDistance with " << threadID;
            UINT_32 temp_1 = 0;
            Profiling::transactionDistance.push_back(temp_1);
            std::cerr << " and new size of " << Profiling::transactionDistance.size() << "*" << std::endl;
         }
         else
         {
            std::cerr << "Profiling::Resizing transactionDistance with " << threadID;
            Profiling::transactionDistance.resize(threadID + 1);
            std::cerr << " and new size of " << Profiling::transactionDistance.size() << "*" << std::endl;
         }
      }

      if(threadID >= Profiling::isTransaction.size())
      {
         if(threadID == Profiling::isTransaction.size())
         {
            std::cerr << "Profiling::Push back to isTransaction with " << threadID;
            BOOL temp_1 = 0;
            Profiling::isTransaction.push_back(temp_1);
            std::cerr << " and new size of " << Profiling::isTransaction.size() << "*" << std::endl;
         }
         else
         {
            std::cerr << "Profiling::Resizing isTransaction with " << threadID;
            Profiling::isTransaction.resize(threadID + 1);
            std::cerr << " and new size of " << Profiling::isTransaction.size() << "*" << std::endl;
         }
      }

      if(threadID >= instructionQueueVector.size())
      {
         if(threadID == instructionQueueVector.size())
         {
            std::cerr << "Profiling::Push back to instructionQueueVector with " << threadID;
            instructionQueueVector.push_back(new std::deque< tuple<DInst, Time_t> >);
            std::cerr << " and new size of " << instructionQueueVector.size() << " and capacity of " << instructionQueueVector.capacity() << "*" << std::endl;
         }
         else
         {
            std::cerr << "Profiling::Resizing instructionQueueVector with " << threadID;
            instructionQueueVector.resize(threadID + 1, new std::deque< tuple<DInst, Time_t> >);
            std::cerr << " and new size of " << instructionQueueVector.size() << " and capacity of " << instructionQueueVector.capacity() << "*" << std::endl;
         }
      }

      if(threadID >= Profiling::currBBStats.size())
      {
         if(threadID == Profiling::currBBStats.size())
         {
            std::cerr << "Profiling::Push back to currBBStats with " << threadID;
            Profiling::currBBStats.push_back(new WorkloadCharacteristics());
            std::cerr << " and new size of " << Profiling::currBBStats.size() << " and capacity of " << Profiling::currBBStats.capacity() << "*" << std::endl;
         }
         else
         {
            std::cerr << "Profiling::Resizing currBBStats with " << threadID;
            Profiling::currBBStats.resize(threadID + 1, new WorkloadCharacteristics());
            std::cerr << " and new size of " << Profiling::currBBStats.size() << " and capacity of " << Profiling::currBBStats.capacity() << "*" << std::endl;
         }
      }

      if(threadID >= Profiling::firstTransaction.size())
      {
         if(threadID == Profiling::firstTransaction.size())
         {
            std::cerr << "Profiling::Push back to firstTransaction with " << threadID;
            UINT_32 temp_1 = 1;
            Profiling::firstTransaction.push_back(temp_1);
            std::cerr << " and new size of " << Profiling::firstTransaction.size() << " and capacity of " << Profiling::firstTransaction.capacity() << "*" << std::endl;
         }
         else
         {
            std::cerr << "Profiling::Resizing firstTransaction with " << threadID;
            Profiling::firstTransaction.resize(threadID + 1);
            std::cerr << " and new size of " << Profiling::firstTransaction.size() << " and capacity of " << Profiling::firstTransaction.capacity() << "*" << std::endl;
            Profiling::firstTransaction[threadID] = 1;
         }
      }

      instructionQueueVector[threadID]->push_back(instruction_cycle);
      if((INT_32)instructionQueueVector[threadID]->size() > statConf->return_windowSize())
      {
         instruction_cycle = instructionQueueVector[threadID]->front();
         instructionQueueVector[threadID]->pop_front();
         Profiling::analysis(instruction_cycle);
      }
   }
   delete statConf;
#endif
//END PROFILING --------------------------------------------------------------------------------------------------------



#if (defined TM)
   // We must grab the type here since we will not be able to use it past the retirement phase
   transInstType tempTransType = dinst->transType;
   sType synchType = dinst->synchType;
   int transPid = dinst->transPid;
   int transTid = dinst->transTid;
   int transBCFlag = dinst->transBCFlag;

   //this has to happen before the dinst is actually retired to keep hit counts correct
#ifdef SESC_ENERGY
   int versioning = transGCM->getVersioning();
   int conflictDetection = transGCM->getConflictDetection();
   MemObj *localSource = this->memorySystem->getDataSource();

   if(transBCFlag == 1)                                                                //ABORT
   {
      localSource->fakeAbort(transPid, versioning);                                    //Used to add energy counters

      if(DVFS_DEBUG == 1)
      {
         std::cout << "ABORT (" << getId() << ")  ";

         std::cout << get_executeState() << std::endl;
         std::cout << "A-CURRENT POWER:  " << GStatsEnergy::getTotalPower();
         std::cout << std::endl;
      }

   }
   else if(tempTransType == transCommit && transBCFlag != 2)                           //COMMIT
   {
      localSource->fakeCommit(transPid, versioning);                                   //Used to add energy counters

      if(DVFS_DEBUG == 1)
      {
         std::cout << "COMMIT (" << getId() << ")  ";

         std::cout << get_executeState() << std::endl;
         std::cout << "C-CURRENT POWER:  " << GStatsEnergy::getTotalPower();
         std::cout << std::endl;
      }
   }

   //If the conflict detection scheme is lazy, we need to perform a sig. check
   if(conflictDetection == 0)
   {
      incrementSignatureEnergy();
   }

//end TM
#endif

//end SESC_ENERGY
#endif

    bool fake = dinst->isFake();

    I(dinst->getResource());
    RetOutcome retOutcome = dinst->getResource()->retire(dinst);
    if( retOutcome != Retired) {
      addStatsNoRetire(i, dinst, retOutcome);
      return;
    }
    // dinst CAN NOT be used beyond this point

#if (defined TM)
    if(transBCFlag == 1)
      instCountTM++;
      // Call the proper reporting function based on the type of instruction
      switch(tempTransType){
      case transCommit:
        if(transBCFlag != 2)
        {
          tmReport->reportCommit(transPid);
        }
        break;
      case transBegin:
        if(transBCFlag != 2)
          tmReport->reportBegin(transPid, this->Id);
        break;
      case transLoad:
        tmReport->reportLoad(transPid);
        break;
      case transStore:
        tmReport->reportStore(transPid);
        break;
      case transAbort:
         tmReport->beginTMStats(instCountTM);
        break;
      case transInt:
      case transFp:
      case transBJ:
      case transFence:
        tmReport->registerTransInst(transPid,tempTransType);
        break;
      case transOther:
        break;
      case transNT:
        tmReport->incrementCommittedInstCountByCpu(this->Id);
        break;
       }

    if(synchType == barrier)
    {
      tmReport->reportBarrier ( this->Id);
    }
#endif

    if (!fake)
      regPool[rp]++;

    ROB.pop();

#if defined(SEP_DVFS)
    robEnergy[executeState]->inc(); // read ROB entry (finished?, update retirement rat...)
#else
    robEnergy->inc(); // read ROB entry (finished?, update retirement rat...)
#endif
  }

  if(!ROB.empty() || i != 0)
    addStatsRetire(i);

}

#ifdef SESC_ENERGY
std::string GProcessor::state_to_freq(unsigned int state)
{
   switch(state)
   {
      case 0 :
         return "_3_73";
      case 1 :
         return "_3_47";
      case 2 :
         return "_3_20";
      case 3 :
         return "_2_93";
      case 4 :
         return "_2_67";
      case 5 :
         return "_2_40";
      case 6 :
         return "_2_27";
      case 7 :
         return "_2_13";
      case 8 :
         return "_2_00";
      case 9 :
         return "_1_87";
      case 10 :
         return "_1_73";
      case 11 :
         return "_1_60";
      case 12 :
         return "_1_47";
      case 13 :
         return "_1_33";
      case 14 :
         return "_1_20";
      case 15 :
         return "_1_07";
      default :
         return "";
   }
}

double GProcessor::state_to_freq_dbl(unsigned int state)
{
   switch(state)
   {
      case 0 :
         return 3.73e9;
      case 1 :
         return 3.47e9;
      case 2 :
         return 3.20e9;
      case 3 :
         return 2.93e9;
      case 4 :
         return 2.67e9;
      case 5 :
         return 2.40e9;
      case 6 :
         return 2.27e9;
      case 7 :
         return 2.13e9;
      case 8 :
         return 2.00e9;
      case 9 :
         return 1.87e9;
      case 10 :
         return 1.73e9;
      case 11 :
         return 1.60e9;
      case 12 :
         return 1.47e9;
      case 13 :
         return 1.33e9;
      case 14 :
         return 1.20e9;
      case 15 :
         return 1.07e9;
      default :
         return 0.00;
   }
}

unsigned int GProcessor::freq_to_state(std::string freq)
{
   std::string frequencies[NUM_STATES] = {"_3_73", "_3_47", "_3_20", "_2_93", "_2_67", "_2_40", "_2_27", "_2_13", "_2_00", "_1_87", "_1_73", "_1_60", "_1_47", "_1_33", "_1_20", "_1_07"};

   for(unsigned int state = 0; state < NUM_STATES; state++)
   {
      if(frequencies[state] == freq)
      {
         return state;
      }
   }

   return 0;
}

double GProcessor::totalExecTime()
{
   /* Variables */
   double runTime = 0;

   /* Processes */
   for(unsigned int state = 0; state < NUM_STATES; state++)
   {
      runTime = runTime + (global_stateCycles[state] / state_to_freq_dbl(state));
   }

   return runTime;
}

double GProcessor::procGateTime(size_t procID)
{
   /* Variables */
   double gateTime = 0;

   /* Processes */
   for(unsigned int state = 0; state < NUM_STATES; state++)
   {
      gateTime = gateTime + (proc_gateCycles[procID][state] / state_to_freq_dbl(state));
   }

   return gateTime;
}

#endif

