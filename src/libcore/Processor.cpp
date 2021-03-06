/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Basilio Fraguela
                  James Tuck
                  Milos Prvulovic
                  Luis Ceze

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

#include "SescConf.h"

#include "Processor.h"

#include "Cache.h"
#include "FetchEngine.h"
#include "GMemorySystem.h"
#include "ExecutionFlow.h"
#include "OSSim.h"

#if (defined TM)
#include "transReport.h"
#endif

extern bool awake[1024];//*********************************************new

Processor::Processor(GMemorySystem *gm, CPU_t i)
  :GProcessor(gm, i, 1)
  ,IFID(i, i, gm, this)
  ,pipeQ(i)
{
  spaceInInstQueue = InstQueueSize;

  bzero(RAT,sizeof(DInst*)*NumArchRegs);

  l1Cache = gm->getDataSource();

}

Processor::~Processor()
{
  // Nothing to do
}

DInst **Processor::getRAT(const int contextId)
{
  I(contextId == Id);
  return RAT;
}

FetchEngine *Processor::currentFlow()
{
  return &IFID;
}

#if !(defined MIPS_EMUL)
void Processor::saveThreadContext(Pid_t pid)
{
  I(IFID.getPid()==pid);
  IFID.saveThreadContext();
}

void Processor::loadThreadContext(Pid_t pid)
{
  I(IFID.getPid()==pid);
  IFID.loadThreadContext();
}

ThreadContext *Processor::getThreadContext(Pid_t pid)
{
  I(IFID.getPid()==pid);
  return IFID.getThreadContext();
}

void Processor::setInstructionPointer(Pid_t pid, icode_ptr picode)
{
  I(IFID.getPid()==pid);
  IFID.setInstructionPointer(picode);
}

icode_ptr Processor::getInstructionPointer(Pid_t pid)
{
  I(IFID.getPid()==pid);
  return IFID.getInstructionPointer();
}
#endif

void Processor::switchIn(Pid_t pid)
{
  IFID.switchIn(pid);
}

void Processor::switchOut(Pid_t pid)
{
  IFID.switchOut(pid);
}

size_t Processor::availableFlows() const
{
  return IFID.getPid() < 0 ? 1 : 0;
}

long long Processor::getAndClearnGradInsts(Pid_t pid)
{
  I(IFID.getPid() == pid);

  return IFID.getAndClearnGradInsts();
}

long long Processor::getAndClearnWPathInsts(Pid_t pid)
{
  I(IFID.getPid() == pid);

  return IFID.getAndClearnWPathInsts();
}

Pid_t Processor::findVictimPid() const
{
  return IFID.getPid();
}

void Processor::goRabbitMode(long long n2Skip)
{
  IFID.goRabbitMode(n2Skip);
}



void Processor::advanceClock()
{
#ifdef SESC_ENERGY
   size_t modAmount = 15;
   size_t remainder = 0;
#endif

#ifdef TS_STALL
  if (isStall()) return;
#endif

//Check to see if we're stalled or should have a different exeuction interval
#ifdef SESC_ENERGY

#if defined(SEP_DVFS)

   switch(get_executeState())
   {
      case 0 :
         modAmount = 1;
         break;
      case 1 :
         modAmount = 11;
         break;
      case 2 :
         modAmount = 12;
         break;
      case 3 :
         modAmount = 13;
         break;
      case 4 :
         modAmount = 14;
         break;
      case 5 :
         modAmount = 15;
         break;
      case 6 :
         modAmount = 16;
         break;
      case 7 :
         modAmount = 18;
         break;
      case 8 :
         modAmount = 19;
         break;
      case 9 :
         modAmount = 20;
         break;
      case 10 :
         modAmount = 21;
         break;
      case 11 :
         modAmount = 23;
         break;
      case 12 :
         modAmount = 25;
         break;
      case 13 :
         modAmount = 25;
         break;
      case 14 :
         modAmount = 31;
         break;
      case 15 :
         modAmount = 35;
         break;
      case 16 :
         modAmount = 99;
         break;
      default :
         modAmount = 15;
         break;
   }

//FIXME
   if(modAmount ==  99)
   {
      set_skipAmount(1000);
   }
   else if(modAmount > 1 && globalClock % modAmount == 0 && globalClock != 0)
   {
      set_skipAmount(modAmount - 10);
   }

   int eState = get_executeState();
   int booHoo = globalClock % modAmount;
   int clk = globalClock;
   int skippy = get_skipAmount();

   int l_clk = proc_stateCycles[Id][get_executeState()];


   if(get_skipAmount() > 0)
   {
      set_skipAmount(get_skipAmount() - 1);
      return;
   }

//    if(globalClock % modAmount == 0)
//    {
//       std::cout << globalClock << "   " << modAmount << std::endl;
//       return;
//    }

// std::cout << getId() << std::setw(4) << get_executeState() << " - ";
// std::cout << "clock:  " << setw(12) << globalClock << "   mod:  " << modAmount << "  fmod:  " << fmod(globalClock, modAmount);

//    if(globalClock % modAmount != 0)
//    {
//       switchOut(IFID.getPid());
//       IFID.switchIn(3);
//       return;
//    }

#endif

#endif

  clockTicks++;
 // if (globalClock % 2000==0)  //this needs to go somewhere else (put all lines to sleep every 2000cc)
 //        {for (int i = 0; i < 1025; i++)
  //           	{awake[i]=false;}}
//   std::cout << "PROC(" << Id << "):  " << clockTicks << "\t-    ";

  global_stateCycles[get_executeState()] = global_stateCycles[get_executeState()] + 1;
  proc_stateCycles[Id][get_executeState()] = proc_stateCycles[Id][get_executeState()] + 1;

//   for(size_t counter = 0; counter < NUM_STATES; counter++)
//   {
//      std::cout <<  proc_stateCycles[Id][counter] << "  ";
//   }
//   std::cout << std::endl;

  // This checks to see if we should avoid the entire Loop because we are currently
  // stalling on a memory location NACK
#if (defined TM)
  if(!transGCM->checkStall(this->IFID.getPid()))
  {
#endif

  //  GMSG(!ROB.empty(),"robTop %d Ul %d Us %d Ub %d",ROB.getIdFromTop(0)
  //       ,unresolvedLoad, unresolvedStore, unresolvedBranch);

  // Fetch Stage
  if (IFID.hasWork() ) {
    IBucket *bucket = pipeQ.pipeLine.newItem();
    if( bucket ) {
      IFID.fetch(bucket);
    }
  }

#if(defined TM)
  }
#endif

  // ID Stage (insert to instQueue)
  if (spaceInInstQueue >= FetchWidth) {
    IBucket *bucket = pipeQ.pipeLine.nextItem();
    if( bucket ) {
      I(!bucket->empty());
      //      I(bucket->top()->getInst()->getAddr());

      spaceInInstQueue -= bucket->size();
      pipeQ.instQueue.push(bucket);
    }else{
      noFetch2.inc();
    }
  }else{
    noFetch.inc();
  }

  // RENAME Stage
  if ( !pipeQ.instQueue.empty() ) {
    spaceInInstQueue += issue(pipeQ);
    //    spaceInInstQueue += issue(pipeQ);
  }

  retire();
}


StallCause Processor::addInst(DInst *dinst)
{
  const Instruction *inst = dinst->getInst();

  if (InOrderCore) {
    if(RAT[inst->getSrc1()] != 0 || RAT[inst->getSrc2()] != 0
       || RAT[inst->getDest()] != 0
       ) {
      return SmallWinStall;
    }
  }

  StallCause sc = sharedAddInst(dinst);
  if (sc != NoStall)
    return sc;


  I(dinst->getResource() != 0); // Resource::schedule must set the resource field

  if(!dinst->isSrc2Ready()) {
    // It already has a src2 dep. It means that it is solved at
    // retirement (Memory consistency. coherence issues)
    if( RAT[inst->getSrc1()] )
      RAT[inst->getSrc1()]->addSrc1(dinst);
  }else{
    if( RAT[inst->getSrc1()] )
      RAT[inst->getSrc1()]->addSrc1(dinst);

    if( RAT[inst->getSrc2()] )
      RAT[inst->getSrc2()]->addSrc2(dinst);
  }

  dinst->setRATEntry(&RAT[inst->getDest()]);
  RAT[inst->getDest()] = dinst;

  I(dinst->getResource());
  dinst->getResource()->getCluster()->addInst(dinst);

  return NoStall;
}

bool Processor::hasWork() const
{
  if (IFID.hasWork())
    return true;

  return !ROB.empty() || pipeQ.hasWork();
}

#ifdef SESC_MISPATH
void Processor::misBranchRestore(DInst *dinst)
{
  clusterManager.misBranchRestore();

  for (unsigned i = 0 ; i < INSTRUCTION_MAX_DESTPOOL; i++)
    misRegPool[i] = 0;

  I(dinst->getFetch() == &IFID);

  pipeQ.pipeLine.cleanMark();

  // TODO: try to remove from ROB
}
#endif
