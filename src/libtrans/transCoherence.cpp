/**
 * @file
 * @author  jpoe   <>, (C) 2008, 2009
 * @date    09/19/08
 * @brief   This is the implementation for the global coherence module.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Implementation: transCoherence
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#include "transCoherence.h"

#include <cmath>
#include <iostream>

#include "ThreadContext.h"
#include "transReport.h"
#include "GProcessor.h"

transCoherence *transGCM = 0;

size_t transCoherence::stallTime = 5000;


/**
 * @ingroup transCoherence
 * @brief   Global Coherence Module
 */
transCoherence::transCoherence()
{
}


/**
 * @ingroup transCoherence
 * @brief   Constructor
 */
transCoherence::transCoherence(FILE* out, int conflicts, int versioning, int cacheLineSize)
{
   this->conflictDetection = conflicts;
   this->versioning = versioning;
   this->cacheLineSize = cacheLineSize;
   this->out = out;

   utid = 0; // Set Global Transaction ID = 0

   // Eager/Eager
   if(versioning && conflictDetection)
   {
      this->readPtr = &transCoherence::readEE;
      this->writePtr = &transCoherence::writeEE;
      this->beginPtr = &transCoherence::beginEE;
      this->commitPtr = &transCoherence::commitEE;
      this->abortPtr = &transCoherence::abortEE;

      alpha = 0.60;

      MSG("\nTransactional Memory Enabled -- Eager Eager");
   }
   // Eager/Lazy rides on top of Eager/Eager with
   // different stall times on commit/abort
   else if(!versioning && conflictDetection)
   {
      this->readPtr = &transCoherence::readEE;
      this->writePtr = &transCoherence::writeEE;
      this->beginPtr = &transCoherence::beginEE;
      this->commitPtr = &transCoherence::commitEE;
      this->abortPtr = &transCoherence::abortEE;

      alpha = 0.60;

      MSG("\nTransactional Memory Enabled -- Eager Lazy");
   }
   // Eager/Lazy
   else if(!versioning && !conflictDetection)
   {
      this->readPtr = &transCoherence::readLL;
      this->writePtr = &transCoherence::writeLL;
      this->beginPtr = &transCoherence::beginLL;
      this->commitPtr = &transCoherence::commitLL;
      this->abortPtr = &transCoherence::abortLL;

      alpha = 0.60;

      MSG("\nTransactional Memory Enabled -- Lazy Lazy");
   }
   // Lazy/Lazy
   else
   {
      fprintf(stderr,"Unsupported Versioning/Conflict Detection combination provided!\n");
      exit(0);
   }

   for(int i = 0; i < MAX_CPU_COUNT; i++)
   {
      transState[i].timestamp = ((~0ULL) - 1024);
      transState[i].cycleFlag = 0;
      transState[i].state = INVALID;
      transState[i].beginPC = 0;
      stallCycle[i] = 0;
      abortCount[i] = 0;
      abortReason[i].first = 0;
      abortReason[i].second = 0;
      tmDepth[i] = 0;
      currentCommitter = -1;

      conflictProbability[i] = 0.35;
   }

   activeTx = 0;

   useDVFS =  SescConf->getBool("TransactionalMemory","useDVFS");
   useConflictProbability = SescConf->getBool("TransactionalMemory","applySmartBackoff");
   useTMSerialization = SescConf->getBool("TransactionalMemory","useTMSerialize");
   useAbortGating = SescConf->getBool("TransactionalMemory","useAbortGating");

   if(useDVFS == 1 && useConflictProbability == 1)
      MSG("Advanced Scheduling Enabled  -- DVFS Enabled/Smart Backoff Enabled");
   else if(useDVFS == 1 && useConflictProbability == 0)
      MSG("Advanced Scheduling Enabled  -- DVFS Enabled/Smart Backoff Disabled");
   else if(useDVFS == 0 && useConflictProbability == 1)
      MSG("Advanced Scheduling Enabled  -- DVFS Disabled/Smart Backoff Enabled");
   else
      MSG("Advanced Scheduling Disabled");

   if(useTMSerialization == 1)
      MSG("Transaction Serialization Enabled");

   if(useAbortGating == 1)
      MSG("Clock/PLL Gate on All Aborts Enabled");

   numProcs = SescConf->getInt("","procsPerNode");
   numProcs = 4;

   abortThreshold = numProcs / 2;

   for(size_t counter = 0; counter < MAX_CPU_COUNT; counter++)
   {
      nackArray[counter] = -1;
      nackState[counter] = 0;

      writeSetList[counter] = new std::set< RAddr >;

      readPredictionSet[counter]  = new std::set< RAddr >;

      for(size_t counterB = 0; counterB < SHRINK_HISTORY; counterB++)
      {
         readPredictionSetList[counter].push_back(new std::set< RAddr >);
      }
   }
}

/**
 * @ingroup transCoherence
 * @brief   Create new cache state reference with Read bit set
 *
 * @param pid   Process ID
 * @return     Cache state
 */
struct cacheState transCoherence::newReadState(int pid)
{
   struct cacheState tmp;
   tmp.state = R;
   tmp.readers.insert(pid);
   return tmp;
}

/**
 * @ingroup transCoherence
 * @brief   Create new cache state reference with Write bit set
 *
 * @param pid   Process ID
 * @return Cache state
 */
struct cacheState transCoherence::newWriteState(int pid)
{
   struct cacheState tmp;
   tmp.state = W;
   tmp.writers.insert(pid);
   return tmp;
}

/**
 * @ingroup transCoherence
 * @brief check to see if thread has been ordered to abort
 *
 * @param pid Process ID
 * @param tid Thread ID
 * @return Abort?
 */
bool transCoherence::checkAbort(int pid, int tid)
{
   if(transState[pid].state == DOABORT)
   {
      tmReport->reportAbort(transState[pid].utid, pid, tid, abortReason[pid].first, abortReason[pid].second, abortReason[pid].second,transState[pid].timestamp, 0);
      transState[pid].state = ABORTING;
      return true;
   }
   else
      return false;
}

/**************************************
 *   Standard Eager / Eager Methods   *
 **************************************/

/**
 * @ingroup transCoherence
 * @brief   eager eager read
 *
 * @param pid   Process ID
 * @param tid   Thread ID
 * @param raddr Real address
 * @return Coherency status
 */
GCMRet transCoherence::readEE(int pid, int tid, RAddr raddr)
{
   RAddr caddr = addrToCacheLine(raddr);
   GCMRet retval = SUCCESS;

   std::map<RAddr, cacheState>::iterator it;
   it = permCache.find(caddr);

   //! If the cache line has been instantiated in our Map
   if(it != permCache.end())
   {
      struct cacheState per = it->second;
      if(per.writers.size() >= 1 && (per.writers.count(pid) != 1))
      {
         int nackPid = *per.writers.begin();

         Time_t nackTimestamp = transState[nackPid].timestamp;
         Time_t myTimestamp = transState[pid].timestamp;

         if(nackTimestamp <= myTimestamp && transState[pid].cycleFlag)
         {
         tmReport->reportNackLoad(transState[pid].utid,pid, tid, nackPid, raddr, caddr, myTimestamp, nackTimestamp);
         tmReport->reportAbort(transState[pid].utid,pid, tid, nackPid, raddr, caddr, myTimestamp, nackTimestamp);
         transState[pid].state = ABORTING;
         return ABORT;
         }

         if(nackTimestamp >= myTimestamp)
         transState[nackPid].cycleFlag = 1;

         tmReport->reportNackLoad(transState[pid].utid, pid, tid, nackPid, raddr, caddr, myTimestamp, nackTimestamp);
         transState[pid].state = NACKED;
         set_nackArray(pid, nackPid);//nackArray[pid] = nackPid;
         retval = NACK;
      }
      else
      {
         per.readers.insert(pid);
         tmReport->registerLoad(transState[pid].utid,transState[pid].beginPC,pid,tid,raddr,caddr,transState[pid].timestamp);
         permCache[caddr] = per;
         transState[pid].state = RUNNING;
         retval = SUCCESS;
      }
   }
   //! We haven't, so create a new one
   else
   {
      tmReport->registerLoad(transState[pid].utid, transState[pid].beginPC,pid,tid,raddr,caddr,transState[pid].timestamp);
      permCache[caddr] = newReadState(pid);
      transState[pid].state = RUNNING;
      retval = SUCCESS;
   }

   //check to see if the address is present in n of the four previous read sets
   if(checkReadPredictionSetList(pid, caddr) != 0)
      updatePredictionSet(caddr);

   return retval;
}

/**
 * @ingroup transCoherence
 * @brief   eager eager write
 *
 * @param pid   Process ID
 * @param tid   Thread ID
 * @param raddr Real address
 * @return Coherency status
 */
GCMRet transCoherence::writeEE(int pid, int tid, RAddr raddr)
{
   RAddr caddr = addrToCacheLine(raddr);
   GCMRet retval = SUCCESS;

   std::map<RAddr, cacheState>::iterator it;
   it = permCache.find(caddr);

   //! If the cache line has been instantiated in our Map
   if(it != permCache.end())
   {
      struct cacheState per = it->second;
      //! If there is more than one reader, or there is a single reader who happens not to be us
      if(per.readers.size() > 1 || ((per.readers.size() == 1) && (per.readers.count(pid) != 1)))
      {
         std::set<int>::iterator it = per.readers.begin();
         int nackPid = *it;
         //!  Grab the first reader than isn't us
         if(nackPid == pid)
         {
         ++it;
         nackPid = *it;
         }
         //!  Take our timestamp as well as the readers
         Time_t nackTimestamp = transState[nackPid].timestamp;
         Time_t myTimestamp = transState[pid].timestamp;

         //!  If the process that is going to nack us is older than us, and we have cycle flag set, abort
         if(nackTimestamp <= myTimestamp && transState[pid].cycleFlag)
         {
         tmReport->reportNackStore(transState[pid].utid,pid, tid, nackPid, raddr, caddr, myTimestamp, nackTimestamp);
         tmReport->reportAbort(transState[pid].utid,pid, tid, nackPid, raddr, caddr, myTimestamp, nackTimestamp);
         transState[pid].state = ABORTING;
         return ABORT;
         }

         //!  If we are older than the guy we're nacking on, then set her cycle flag to indicate possible deadlock
         if(nackTimestamp >= myTimestamp)
         transState[nackPid].cycleFlag = 1;

         tmReport->reportNackStore(transState[pid].utid,pid, tid, nackPid, raddr, caddr, myTimestamp, nackTimestamp);
         transState[pid].state = NACKED;
         set_nackArray(pid, nackPid);//nackArray[pid] = nackPid;
         retval = NACK;
      }
      else if((per.writers.size() > 1) || ((per.writers.size() == 1) && (per.writers.count(pid) != 1)))
      {

         std::set<int>::iterator it = per.writers.begin();
         int nackPid = *it;

         //!  Grab the first reader than isn't us
         if(nackPid == pid)
         {
         ++it;
         nackPid = *it;
         }

         Time_t nackTimestamp = transState[nackPid].timestamp;
         Time_t myTimestamp = transState[pid].timestamp;

         if(nackTimestamp <= myTimestamp && transState[pid].cycleFlag)
         {
         tmReport->reportNackStore(transState[pid].utid,pid, tid, nackPid, raddr, caddr, myTimestamp, nackTimestamp);
         tmReport->reportAbort(transState[pid].utid,pid, tid, nackPid, raddr, caddr, myTimestamp, nackTimestamp);
         transState[pid].state = ABORTING;
         return ABORT;
         }

         if(nackTimestamp >= myTimestamp)
         transState[nackPid].cycleFlag = 1;

         tmReport->reportNackStore(transState[pid].utid,pid, tid, nackPid, raddr, caddr, myTimestamp, nackTimestamp);
         transState[pid].state = NACKED;
         set_nackArray(pid, nackPid);//nackArray[pid] = nackPid;
         retval = NACK;
      }
      else
      {
         per.writers.insert(pid);
         tmReport->registerStore(transState[pid].utid, transState[pid].beginPC,pid,tid,raddr,caddr,transState[pid].timestamp);
         permCache[caddr] = per;
         transState[pid].state = RUNNING;
         retval = SUCCESS;
      }

   }
   //!  We haven't, so create a new one
   else
   {
      tmReport->registerStore(transState[pid].utid,transState[pid].beginPC,pid,tid,raddr,caddr,transState[pid].timestamp);
      permCache[caddr] = newWriteState(pid);
      transState[pid].state = RUNNING;
      retval = SUCCESS;
   }

   return retval;
}

/**
 * @ingroup transCoherence
 * @brief   eager eager begin
 *
 * @param pid   Process ID
 * @param picode Instruction code
 * @return  Final coherency status
 */
GCMFinalRet transCoherence::beginEE(int pid, icode_ptr picode)
{
   struct GCMFinalRet retVal;

   //!  Subsume all nested transactions for now
   if(tmDepth[pid]>0)
   {
      //tmReport->registerBegin(transState[pid].utid,pid,tid,transState[pid].timestamp);
      tmDepth[pid]++;
      retVal.ret = IGNORE;

      //!  This is a subsumed begin, set BCFlag = 2
      retVal.BCFlag = 2;
      retVal.tuid = transState[pid].utid;
      return retVal;
   }
   else
   {
      //!  If we had just aborted, we need to now invalidate all the memory addresses we touched
      if(transState[pid].state == ABORTING)
      {
         delete writeSetList[pid];
         writeSetList[pid] = new std::set< RAddr >;

         std::map<RAddr, cacheState>::iterator it;
         for(it = permCache.begin(); it != permCache.end(); ++it)
         {
            //add all of the write addresses to the write set list before deleting them
            if(it->second.writers.count(pid) > 0)
               writeSetList[pid]->insert(it->first);

            it->second.writers.erase(pid);
            it->second.readers.erase(pid);
         }

         transState[pid].state = ABORTED;
         abortCount[pid] = abortCount[pid] + 1;
      }

      //!  If we just finished an abort, it's time to backoff
      if(transState[pid].state == ABORTED)
      {
         retVal.abortCount = abortCount[pid];
         retVal.ret = BACKOFF;
         transState[pid].state = RUNNING;

         if(useConflictProbability == 1 || useTMSerialization == 1 || useAbortGating == 1 || useDVFS == 1)
            activeTx = activeTx - 1;
      }
      else
      {
         if(useConflictProbability == 1)
         {
            activeTx = activeTx + 1;
            #if defined(CONFLICT_DEBUG)
            std::cerr << "B(" << pid << ")-ACTIVE TX:  " << activeTx << "   ABORTED:  " << abortCount[pid] << "\n";
            #endif

            double myProcs = numProcs;
            double abortScaler = (double)abortCount[pid] / ((double)abortCount[pid] + numProcs);
            double conflictScaler = abortScaler * (activeTx / myProcs);
            conflictProbability[pid] = (alpha * conflictProbability[pid]) + (1.0 - alpha) * conflictScaler;

            #if defined(CONFLICT_DEBUG)
            std::cerr << "*************Conflict Probability(" << pid << "):  " << abortCount[pid] << "   " << activeTx << "  " << conflictProbability[pid] << "\n";
            #endif

            if(conflictProbability[pid] > 0.20 && activeTx > 1)
            {
               #if defined(CONFLICT_DEBUG)
               std::cerr << "D(" << pid << ")-ACTIVE TX(" << pid << "):  " << activeTx << "   ABORTED:  " << abortCount[pid] << "  " << conflictProbability[pid] << "\n";
               #endif

               retVal.ret = BEGIN_DELAY;
               activeTx = activeTx - 1;
               return retVal;
            }
         }
         else if(useTMSerialization == 1 || useAbortGating == 1)
         {
            activeTx = activeTx + 1;
            #if defined(CONFLICT_DEBUG)
            std::cerr << "B(" << pid << ")-ACTIVE TX:  " << activeTx << "   ABORTED:  " << abortCount[pid] << "\n";
            #endif

            if(get_totalAborts() > 0 && activeTx > 1)
            {
               #if defined(CONFLICT_DEBUG)
               std::cerr << "D(" << pid << ")-ACTIVE TX(" << pid << "):  " << activeTx << "   ABORTED:  " << abortCount[pid] << "\n";
               #endif

               retVal.ret = BEGIN_DELAY;
               activeTx = activeTx - 1;
               return retVal;
            }
         }

         if(useDVFS == 1)
         {
            if(useConflictProbability == 0)
            {
               activeTx = activeTx + 1;
               #if defined(CONFLICT_DEBUG)
               std::cerr << "B(" << pid << ")-ACTIVE TX:  " << activeTx << "   ABORTED:  " << abortCount[pid] << "\n";
               #endif
            }

            if(get_totalAborts() > abortThreshold && activeTx > 1)
            {
               #if defined(CONFLICT_DEBUG)
               std::cerr << "D(" << pid << ")-ACTIVE TX(" << pid << "):  " << activeTx << "   ABORTED:  " << abortCount[pid] << "\n";
               #endif

               retVal.ret = BEGIN_DELAY;
               activeTx = activeTx - 1;
               return retVal;
            }
         }

         //!  Pass whether this is the begining of an aborted replay back to the context
         if(abortCount[pid] > 0)
         {
            retVal.BCFlag = 1;  //!  Replay
         }
         else
         {
            retVal.BCFlag = 0;
         }

         transState[pid].timestamp = globalClock;
         transState[pid].beginPC = picode->addr;
         transState[pid].cycleFlag = 0;
         transState[pid].state = RUNNING;
         transState[pid].utid = transCoherence::utid++;

         tmDepth[pid]++;

         tmReport->registerBegin(transState[pid].utid, pid, picode->immed, picode->addr, transState[pid].timestamp);

         retVal.ret = SUCCESS;
         retVal.tuid = transState[pid].utid;
      }

      return retVal;
   }
}

/**
 * @ingroup transCoherence
 * @brief   eager eager abort
 *
 * @param pthread SESC pointer to thread
 * @param tid    Thread ID
 * @return Final coherency status
 */
struct GCMFinalRet transCoherence::abortEE(thread_ptr pthread, int tid)
{
   struct GCMFinalRet retVal;

   int pid = pthread->getPid();
   int writeSetSize = 0;
   transState[pid].timestamp = ((~0ULL) - 1024);
   transState[pid].beginPC = 0;
   stallCycle[pid] = 0;
   transState[pid].cycleFlag = 0;

   //!  We can't just decriment because we should be going back to the original begin, so tmDepth[pid] = 0
   tmDepth[pid] = 0;

   if(useConflictProbability == 1 || useTMSerialization == 1 || useAbortGating == 1 || useDVFS == 1)
   {
      #if defined(CONFLICT_DEBUG)
      std::cerr << "A(" << pid << ")-ACTIVE TX:  " << activeTx << "   ABORTED:  " << abortCount[pid] << "\n";
      #endif
   }

   std::map<RAddr, cacheState>::iterator it;
   for(it = permCache.begin(); it != permCache.end(); ++it)
   {
      writeSetSize += it->second.writers.count(pid);

      //add all of the write addresses to the prediction set list before deleting them
      if(it->second.writers.count(pid) > 0)
         updatePredictionSet(it->first);

      //add read addresses to the read predictionSet
      if(it->second.readers.count(pid) > 0)
         readPredictionSet[pid]->insert(it->first);
   }

   updateReadPredictionSetList(pid, readPredictionSet[pid]);
   readPredictionSet[pid] = new std::set< RAddr >;

   retVal.writeSetSize = writeSetSize;

   transState[pid].state = ABORTING;
   retVal.ret = SUCCESS;
   return retVal;
}

/**
 * @ingroup transCoherence
 * @brief   eager eager commit
 *
 * @param pid   Process ID
 * @param tid    Thread ID
 * @return Final coherency status
 */
struct GCMFinalRet transCoherence::commitEE(int pid, int tid)
{

   struct GCMFinalRet retVal;

   //!  Set the default BCFlag to 0, since the only other option for Commit is subsumed 2
   retVal.BCFlag = 0;

   if(tmDepth[pid] > 1)
   {
      //tmReport->registerCommit(transState[pid].utid,pid,tid,transState[pid].timestamp); // Register Commit in Report
      tmDepth[pid]--;
      retVal.ret = IGNORE;
      //!  This commit is subsumed, set the BCFlag to 2
      retVal.BCFlag = 2;
      retVal.tuid = transState[pid].utid;
      return retVal;
   }
   else
   {
      if(useConflictProbability == 1)
      {
         conflictProbability[pid] = alpha * conflictProbability[pid];
      }

      //!  If we have already stalled for the commit, our state will be COMMITTING, Complete Commit
      if(transState[pid].state == COMMITTING)
      {
         tmReport->registerCommit(transState[pid].utid,pid,tid,transState[pid].timestamp); //!  Register Commit in Report

         int writeSetSize = 0;
         transState[pid].timestamp = ((~0ULL) - 1024);
         transState[pid].beginPC = 0;
         stallCycle[pid] = 0;
         transState[pid].cycleFlag = 0;
         abortCount[pid] = 0;
         tmDepth[pid] = 0;

         if(useConflictProbability == 1 || useTMSerialization == 1)
         {
            activeTx = activeTx - 1;

            #if defined(CONFLICT_DEBUG)
            std::cerr << "C(" << pid << ")-ACTIVE TX:  " << activeTx << "   ABORTED:  " << abortCount[pid] << "\n";
            #endif
         }
         else if(useAbortGating == 1)
         {
            activeTx = activeTx - 1;
            clearAborts();

            #if defined(CONFLICT_DEBUG)
            std::cerr << "C(" << pid << ")-ACTIVE TX:  " << activeTx << "   ABORTED:  " << abortCount[pid] << "\n";
            #endif
         }

         if(useDVFS == 1)
         {
            if(useConflictProbability == 0)
            {
               activeTx = activeTx - 1;
               #if defined(CONFLICT_DEBUG)
               std::cerr << "C(" << pid << ")-ACTIVE TX:  " << activeTx << "   ABORTED:  " << abortCount[pid] << "\n";
               #endif
            }
            clearAborts();
         }

         //good Tx so delete the previous write set
         delete writeSetList[pid];
         writeSetList[pid] = new std::set< RAddr >;

         std::map<RAddr, cacheState>::iterator it;
         for(it = permCache.begin(); it != permCache.end(); ++it)
         {
            //add all of the write addresses to the write set list before deleting them
            if(it->second.writers.count(pid) > 0)
               writeSetList[pid]->insert(it->first);

            //add read addresses to the read predictionSet
            if(it->second.readers.count(pid) > 0)
               readPredictionSet[pid]->insert(it->first);

            writeSetSize += it->second.writers.erase(pid);
            it->second.readers.erase(pid);
         }

         updateReadPredictionSetList(pid, readPredictionSet[pid]);
         readPredictionSet[pid] = new std::set< RAddr >;

         //clean up the prediction sets for the next tx
         clearPredictionSet();

         retVal.writeSetSize = writeSetSize;
         retVal.ret = SUCCESS;
         transState[pid].state = COMMITTED;
         retVal.tuid = transState[pid].utid;
         return retVal;
      }
      else
      {
         int writeSetSize = 0;
         std::map<RAddr, cacheState>::iterator it;
         for(it = permCache.begin(); it != permCache.end(); ++it)
         writeSetSize += it->second.writers.count(pid);
         transState[pid].state = COMMITTING;
         retVal.writeSetSize = writeSetSize;
         retVal.ret = COMMIT_DELAY;
         retVal.tuid = transState[pid].utid;
         return retVal;
      }

   }

}


/**************************************
 *   Standard Lazy / Lazy Methods   *
 **************************************/

/*
  * The Read function is much simpler in the Lazy approach since
  * we do not have to worry about conflict detection.  We always
  * permit accecss and simply record the information.
*/

/**
 * @ingroup transCoherence
 * @brief   lazy lazy read
 *
 * @param pid   Process ID
 * @param tid    Thread ID
 * @param raddr  Real address
 * @return Coherency status
 */
GCMRet transCoherence::readLL(int pid, int tid, RAddr raddr)
{
   RAddr caddr = addrToCacheLine(raddr);
   GCMRet retval = SUCCESS;

   //!  If we have been forced to ABORT
   if(transState[pid].state == DOABORT)
   {
      tmReport->reportAbort(transState[pid].utid,pid, tid, abortReason[pid].first, abortReason[pid].second, abortReason[pid].second, transState[pid].timestamp, 0);
      transState[pid].state = ABORTING;
      return ABORT;
   }

   std::map<RAddr, cacheState>::iterator it;
   it = permCache.find(caddr);

   //!  If the cache line has been instantiated in our Map
   if(it != permCache.end())
   {
      struct cacheState per = it->second;

      per.readers.insert(pid);
      tmReport->registerLoad(transState[pid].utid, transState[pid].beginPC,pid,tid,raddr,caddr,transState[pid].timestamp);
      permCache[caddr] = per;
      transState[pid].state = RUNNING;
      retval = SUCCESS;
   }
   //!  We haven't, so create a new one
   else
   {
      tmReport->registerLoad(transState[pid].utid, transState[pid].beginPC,pid,tid,raddr,caddr,transState[pid].timestamp);
      permCache[caddr] = newReadState(pid);
      transState[pid].state = RUNNING;
      retval = SUCCESS;
   }

   //check to see if the address is present in n of the four previous read sets
   if(checkReadPredictionSetList(pid, caddr) != 0)
      updatePredictionSet(caddr);

   return retval;
}


/*
  * The Write function is much simpler in the Lazy approach since
  * we do not have to worry about conflict detection.  We always
  * permit accecss and simply record the information.
*/

/**
 * @ingroup transCoherence
 * @brief   lazy lazy write
 *
 * @param pid   Process ID
 * @param tid    Thread ID
 * @param raddr Real address
 * @return Coherency status
 */
GCMRet transCoherence::writeLL(int pid, int tid, RAddr raddr)
{
   RAddr caddr = addrToCacheLine(raddr);
   GCMRet retval = SUCCESS;

   //!  If we have been forced to ABORT
   if(transState[pid].state == DOABORT)
   {
      tmReport->reportAbort(transState[pid].utid,pid, tid, abortReason[pid].first, abortReason[pid].second, abortReason[pid].second,transState[pid].timestamp, 0);
      transState[pid].state = ABORTING;
      return ABORT;
   }

   std::map<RAddr, cacheState>::iterator it;
   it = permCache.find(caddr);

   //!  If the cache line has been instantiated in our Map
   if(it != permCache.end())
   {
      struct cacheState per = it->second;
      per.writers.insert(pid);
      tmReport->registerStore(transState[pid].utid,transState[pid].beginPC,pid,tid,raddr,caddr,transState[pid].timestamp);
      permCache[caddr] = per;
      transState[pid].state = RUNNING;
      retval = SUCCESS;
   }
   //!  We haven't, so create a new one
   else
   {
      tmReport->registerStore(transState[pid].utid,transState[pid].beginPC,pid,tid,raddr,caddr,transState[pid].timestamp);
      permCache[caddr] = newWriteState(pid);
      transState[pid].state = RUNNING;
      retval = SUCCESS;
   }

   return retval;
}

/**
 * @ingroup transCoherence
 * @brief lazy lazy begin
 *
 * @param pid   Process ID
 * @param picode SESC icode pointer
 * @return Final coherency status
 */
GCMFinalRet transCoherence::beginLL(int pid, icode_ptr picode)
{
  struct GCMFinalRet retVal;

  //!  Subsume all nested transactions for now
  if(tmDepth[pid]>0)
  {
    //tmReport->registerBegin(transState[pid].utid,pid,tid,transState[pid].timestamp);
    tmDepth[pid]++;
    retVal.ret = IGNORE;
    //!  This begin is subsumed, set the BCFlag to 2
    retVal.BCFlag = 2;
    retVal.tuid = transState[pid].utid;
    return retVal;
  }
  else
  {
      //!  If we had just aborted, we need to now invalidate all the memory addresses we touched
      if(transState[pid].state == ABORTING)
      {
         transState[pid].state = ABORTED;
         abortCount[pid]++;
      }

      if(useConflictProbability == 1)
      {
         activeTx = activeTx + 1;

         #if defined(CONFLICT_DEBUG)
         std::cerr << "B(" << pid << ")-ACTIVE TX:  " << activeTx << "   ABORTED:  " << abortCount[pid] << "\n";
         #endif

         double myProcs = numProcs;
         double abortScaler = (double)abortCount[pid] / ((double)abortCount[pid] + numProcs);
         double conflictScaler = abortScaler * (activeTx / myProcs);
         conflictProbability[pid] = (alpha * conflictProbability[pid]) + (1.0 - alpha) * conflictScaler;

         #if defined(CONFLICT_DEBUG)
         std::cerr << "*************Conflict Probability(" << pid << "):  " << activeTx << "  " << conflictProbability[pid] << "\n";
         #endif

         if(conflictProbability[pid] > 0.2 && activeTx > 1)
         {
            #if defined(CONFLICT_DEBUG)
            std::cerr << "D(" << pid << ")-ACTIVE TX(" << pid << "):  " << activeTx << "   ABORTED:  " << abortCount[pid] << "  " << conflictProbability[pid] << "\n";
            #endif

            retVal.ret = BEGIN_DELAY;
            activeTx = activeTx - 1;
            return retVal;
         }
      }
      else if(useTMSerialization == 1 || useAbortGating == 1)
      {
         activeTx = activeTx + 1;

         #if defined(CONFLICT_DEBUG)
         std::cerr << "B(" << pid << ")-ACTIVE TX:  " << activeTx << "   ABORTED:  " << abortCount[pid] << "\n";
         #endif

         if(get_totalAborts() > 0 && activeTx > 1)
         {
            #if defined(CONFLICT_DEBUG)
            std::cerr << "D(" << pid << ")-ACTIVE TX(" << pid << "):  " << activeTx << "   ABORTED:  " << abortCount[pid] << "\n";
            #endif

            retVal.ret = BEGIN_DELAY;
            activeTx = activeTx - 1;
            return retVal;
         }
      }

      if(useDVFS == 1)
      {
         if(useConflictProbability == 0)
         {
            activeTx = activeTx + 1;

            #if defined(CONFLICT_DEBUG)
            std::cerr << "B(" << pid << ")-ACTIVE TX:  " << activeTx << "   ABORTED:  " << abortCount[pid] << "\n";
            #endif
         }

         if(get_totalAborts() > abortThreshold && activeTx > 1)
         {
            #if defined(CONFLICT_DEBUG)
            std::cerr << "D(" << pid << ")-ACTIVE TX(" << pid << "):  " << activeTx << "   ABORTED:  " << abortCount[pid] << "\n";
            #endif

            retVal.ret = BEGIN_DELAY;
            activeTx = activeTx - 1;
            return retVal;
         }
      }

      //!  Pass whether this is the begining of an aborted replay back to the context
      if(abortCount[pid] > 0)
      {
         retVal.BCFlag = 1;  //!  Replay
      }
      else
      {
         retVal.BCFlag = 0;
      }

      transState[pid].timestamp = globalClock;
      transState[pid].beginPC = picode->addr;
      transState[pid].cycleFlag = 0;
      transState[pid].state = RUNNING;
      transState[pid].utid = transCoherence::utid++;

      tmDepth[pid]++;

      tmReport->registerBegin(transState[pid].utid, pid, picode->immed, picode->addr, transState[pid].timestamp);

      retVal.ret = SUCCESS;
      retVal.tuid = transState[pid].utid;
   }

   return retVal;
}

/**
 * @ingroup transCoherence
 * @brief lazy lazy abort
 *
 * @param pthread SESC thread pointer
 * @param tid    Thread ID
 * @return Final coherency status
 */
struct GCMFinalRet transCoherence::abortLL(thread_ptr pthread, int tid)
{

   struct GCMFinalRet retVal;

   int pid = pthread->getPid();
   int writeSetSize = 0;
   transState[pid].timestamp = ((~0ULL) - 1024);
   transState[pid].beginPC = 0;
   stallCycle[pid] = 0;
   transState[pid].cycleFlag = 0;

   if(useConflictProbability == 1 || useTMSerialization == 1 || useAbortGating == 1 || useDVFS == 1)
   {
      activeTx = activeTx - 1;

      #if defined(CONFLICT_DEBUG)
      std::cerr << "A(" << pid << ")-ACTIVE TX:  " << activeTx << "   ABORTED:  " << abortCount[pid] << "\n";
      #endif
   }

   std::map<RAddr, cacheState>::iterator it;
   for(it = permCache.begin(); it != permCache.end(); ++it)
   {
      writeSetSize += it->second.writers.count(pid);

      //add all of the write addresses to the write set list before deleting them
      if(it->second.writers.count(pid) > 0)
         updatePredictionSet(it->first);

      //add read addresses to the read predictionSet
      if(it->second.readers.count(pid) > 0)
         readPredictionSet[pid]->insert(it->first);
   }

   updateReadPredictionSetList(pid, readPredictionSet[pid]);
   readPredictionSet[pid] = new std::set< RAddr >;

   //!  We can't just decriment because we should be going back to the original begin, so tmDepth[pid] = 0
   tmDepth[pid]=0;

   //!  Write set size doesn't matter for Lazy/Lazy abort
   retVal.writeSetSize = 0;

   transState[pid].state = ABORTING;
   retVal.ret = SUCCESS;
   return retVal;

}

/**
 * @ingroup transCoherence
 * @brief lazy lazy commit
 *
 * @param pid   Process ID
 * @param tid    Thread ID
 * @return Final coherency status
 */
struct GCMFinalRet transCoherence::commitLL(int pid, int tid)
{
  struct GCMFinalRet retVal;

  //!  Set BCFlag default to 0, since only other option is subsumed BCFlag = 2
  retVal.BCFlag = 0;

  //!  If we have been forced to ABORT
  if(transState[pid].state == DOABORT)
  {
    retVal.ret = ABORT;
    transState[pid].state = ABORTING;
    tmReport->reportAbort(transState[pid].utid,pid, tid, abortReason[pid].first, abortReason[pid].second, abortReason[pid].second,transState[pid].timestamp, 0);
    return retVal;
  }


  if(tmDepth[pid] > 1)
  {
    //tmReport->registerCommit(transState[pid].utid,pid,tid,transState[pid].timestamp); // Register Commit in Report
    tmDepth[pid]--;
    retVal.ret = IGNORE;
    //!  This is a subsumed commit, set BCFlag = 2
    retVal.BCFlag = 2;
    retVal.tuid = transState[pid].utid;
    return retVal;
  }
  else
  {
    //!  If we have already stalled for the commit, our state will be COMMITTING, Complete Commit
    if(transState[pid].state == COMMITTING)
    {
       tmReport->registerCommit(transState[pid].utid,pid,tid,transState[pid].timestamp); //!  Register Commit in Report

      int writeSetSize = 0;
      int didWrite = 0;
      transState[pid].timestamp = ((~0ULL) - 1024);
      transState[pid].beginPC = 0;
      stallCycle[pid] = 0;
      transState[pid].cycleFlag = 0;
      abortCount[pid] = 0;
      tmDepth[pid] = 0;

      if(useConflictProbability == 1 || useTMSerialization == 1)
      {
         activeTx = activeTx - 1;

         #if defined(CONFLICT_DEBUG)
         std::cerr << "C(" << pid << ")-ACTIVE TX:  " << activeTx << "   ABORTED:  " << abortCount[pid] << "\n";
         #endif
      }
      else if(useAbortGating == 1)
      {
         activeTx = activeTx - 1;
         clearAborts();

         #if defined(CONFLICT_DEBUG)
         std::cerr << "C(" << pid << ")-ACTIVE TX:  " << activeTx << "   ABORTED:  " << abortCount[pid] << "\n";
         #endif
      }

      if(useDVFS == 1)
      {
         if(useConflictProbability == 0)
         {
            activeTx = activeTx - 1;
         }
         clearAborts();

         #if defined(CONFLICT_DEBUG)
         std::cerr << "C(" << pid << ")-ACTIVE TX:  " << activeTx << "   ABORTED:  " << abortCount[pid] << "\n";
         #endif
      }

      //good Tx so clean up the write set list
      delete writeSetList[pid];
      writeSetList[pid] = new std::set< RAddr >;

      std::map<RAddr, cacheState>::iterator it;
      std::set<int>::iterator setIt;

      for(it = permCache.begin(); it != permCache.end(); ++it)
      {
         //add read addresses to the read predictionSet
         if(it->second.readers.count(pid) > 0)
            readPredictionSet[pid]->insert(it->first);

        didWrite = it->second.writers.erase(pid);

        //!  If we have written to this address, we must abort everyone who read/wrote to it
        if(didWrite)
        {
          //add all of the write addresses to the write set list before deleting them
          writeSetList[pid]->insert(it->first);

          //!  Increase our write set
          writeSetSize++;
          //!  Abort all who wrote to this
          for(setIt = it->second.writers.begin(); setIt != it->second.writers.end(); ++setIt)
            if(*setIt != pid)
            {
              transState[*setIt].state = DOABORT;
              abortReason[*setIt].first =  pid;
              abortReason[*setIt].second = (RAddr)it->first;
            }
          //!  Abort all who read from this
          for(setIt = it->second.readers.begin(); setIt != it->second.readers.end(); ++setIt)
            if(*setIt != pid)
            {
              transState[*setIt].state = DOABORT;
              abortReason[*setIt].first =  pid;
              abortReason[*setIt].second = (RAddr)it->first;
            }

          it->second.writers.clear();
          it->second.readers.clear();
        }
        else
          it->second.readers.erase(pid);
      }

      updateReadPredictionSetList(pid, readPredictionSet[pid]);
      readPredictionSet[pid] = new std::set< RAddr >;

      //clean up the prediction sets for the next tx
      clearPredictionSet();

      currentCommitter = -1;  //!  Allow other transaction to commit again
      retVal.writeSetSize = writeSetSize;
      retVal.ret = SUCCESS;
      transState[pid].state = COMMITTED;
      retVal.tuid = transState[pid].utid;
      return retVal;
    }
    else if(currentCommitter >= 0)
    {
      retVal.ret = NACK;
      transState[pid].state = NACKED;
      set_nackArray(pid, currentCommitter);//nackArray[pid] = currentCommitter;
      tmReport->reportNackCommit(transState[pid].utid, pid, tid, currentCommitter, transState[pid].timestamp, transState[currentCommitter].timestamp);
      return retVal;
    }
    else
    {
      tmReport->reportNackCommitFN(transState[pid].utid,pid,tid,transState[pid].timestamp); //!  Register Commit in Report
      int writeSetSize = 0;
      currentCommitter = pid; //!  Stop other transactions from being able to commit
      std::map<RAddr, cacheState>::iterator it;
      for(it = permCache.begin(); it != permCache.end(); ++it)
        writeSetSize += it->second.writers.count(pid);
      transState[pid].state = COMMITTING;
      retVal.writeSetSize = writeSetSize;
      retVal.ret = COMMIT_DELAY;
      retVal.tuid = transState[pid].utid;
      return retVal;
    }
  }
}

///-----------------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 *
 * @param processElement
 */
void transCoherence::releaseNackedPE(int processElement)
{
   /* Variables */


   /* Processes */
   for(size_t counter = 0; counter < MAX_CPU_COUNT; counter++)
   {
      if(get_nackArray(counter) == processElement)
      {
         std::cout << "Release " << counter << " by " << processElement << "\n";
         transGCM->prPointer->at(counter)->set_executeState(DEFAULT_DVFS);
      }
   }
}

/**
 *
 * @param procID
 * @param state
 * @return
 */
bool transCoherence::set_nackArray(int procID, int state)
{
   if(procID >= 0 && procID < MAX_CPU_COUNT)
   {
      nackArray[procID] = state;
      return 1;
   }
   else
   {
      return 0;
   }
}

/**
 *
 * @param procID
 * @return
 */
int transCoherence::get_nackArray(int procID)
{
   if(procID >= 0 && procID < MAX_CPU_COUNT)
   {
      return nackArray[procID];
   }
   else
   {
      return -1;
   }
}

size_t transCoherence::get_stallTime()
{
   if(useTMSerialization == 1)
   {
      return 1;
   }
   else if(useAbortGating == 1)
   {
      double yar = get_totalAborts();
      double boo = log(yar);
      double poo = pow(2, boo);
      double nar = get_renews();
      double par = log(nar);
      double poy = pow(2, par);
      double blarg = stallTime * (poy + poo);
      size_t narf = ceil(blarg);
      return ceil(stallTime * (pow(2, log(get_totalAborts())) + pow(2, log(get_renews()))));
   }
   if(useDVFS == 1)
   {
      return stallTime / 10;
   }
   else
   {
      return stallTime;
   }
}

std::map< RAddr, uint32_t >* transCoherence::getCurrentSets(uint32_t log2AddrLs, uint32_t maskSets, uint32_t log2Assoc, int pid)
{
   uint32_t set;
   std::map< RAddr, uint32_t >* currentSets = new std::map< RAddr, uint32_t >;

   for(std::map<RAddr, cacheState>::iterator it = permCache.begin(); it != permCache.end(); ++it)
   {
      set = ((addrToCacheLine(it->first) >> log2AddrLs) & maskSets) << log2Assoc;
      if(it->second.readers.count(pid) > 0 || it->second.writers.count(pid) > 0)
      {
         (*currentSets)[set] = 1;
      }
   }

   return currentSets;
}

uint32_t transCoherence::checkWriteSetList(uint32_t log2AddrLs, uint32_t maskSets, uint32_t log2Assoc, int pid, RAddr caddr)
{
   uint32_t set;

   for(std::set< RAddr >::iterator myIter = writeSetList[pid]->begin(); myIter != writeSetList[pid]->end(); ++myIter)
   {
      set = ((addrToCacheLine(*myIter) >> log2AddrLs) & maskSets) << log2Assoc;
      if(set == caddr)
         return 1;
   }

   return 0;
}

uint32_t transCoherence::clearReadPredictionSet(int pid)
{
   delete readPredictionSet[pid];
   readPredictionSet[pid] = new std::set< RAddr >;

   return 0;
}

uint32_t transCoherence::checkReadPredictionSetList(int pid, RAddr caddr)
{
   uint32_t count;

   count = 0;
   for(std::list< std::set< RAddr > * >::iterator myIter = readPredictionSetList[pid].begin(); myIter !=readPredictionSetList[pid].end(); ++myIter)
   {
      if((*myIter)->count(caddr) != 0)
         count = count + 1;
   }

   if(count >= SHRINK_THRESHOLD)
      return 1;
   else
      return 0;

}

uint32_t transCoherence::updateReadPredictionSetList(int pid, std::set< RAddr > * incList)
{
   delete readPredictionSetList[pid].back();
   readPredictionSetList[pid].pop_back();

   readPredictionSetList[pid].push_front(incList);

   return 0;
}

void transCoherence::updatePredictionSet(std::set< RAddr > * addrList)
{
   for(std::set< RAddr >::iterator myIter = addrList->begin(); myIter != addrList->end(); ++myIter)
   {
      predictionSet.insert(*myIter);
   }
}

uint32_t transCoherence::checkPredictionSet(uint32_t log2AddrLs, uint32_t maskSets, uint32_t log2Assoc, int pid, RAddr caddr)
{
   uint32_t set;

   for(std::set< RAddr >::iterator myIter = predictionSet.begin(); myIter != predictionSet.end(); ++myIter)
   {
      set = ((addrToCacheLine(*myIter) >> log2AddrLs) & maskSets) << log2Assoc;
      if(set == caddr)
      {
         return 1;
      }
   }

   return 0;
}
