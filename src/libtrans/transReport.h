/**
 * @file
 * @author  jpoe   <>, (C) 2008, 2009
 * @date    09/19/08
 * @brief   This is the interface for the transaction reporting module.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Interface: transReport \n
 * This file is responsible for creating the transactional trace file that is used to
 * extract useful statistics from the simulation.  Note for most operations there is
 * a "register" and a "report" version.  The purpose of the register is to add the
 * operation to a queue for later print, and then the report function is called at the
 * commit stage to actually print the operation so that we have accurate cycle counts.
 * 
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TRANSACTION_REPORT
#define TRANSACTION_REPORT

#include <time.h>
#include <stdio.h>
#include <queue>
#include "OSSim.h"
#include "ExecutionFlow.h"


typedef uintptr_t RAddr;
typedef unsigned long long ID;
typedef unsigned long long TIMESTAMP;
typedef unsigned long long INSTCOUNT;

struct memRef{
  RAddr caddr;
  RAddr raddr;
  RAddr beginPC;
  int tid;
  int pid;
  ID utid;
  TIMESTAMP timestamp;
};

struct transRef{
  int cpu;
  int pid;
  int tid;
  RAddr PC;
  ID utid;
  TIMESTAMP timestamp;
};

/**
 * @ingroup transReport
 * @brief   Report Module
 *
 * This module is used to calculate the output statistics. It keeps track of cycle counts, instruction counts, etc
 * and writes them to an output file based on the level of granularity (detailed, transactional, global) the user wants
 */
class transReport{
  public:

    // Constructor
    transReport(){
      transReport("");
    }

    transReport(const char *reportFileName);

    // The report functions are used to output the event to the transactional log 
    // at the point of instruction commit to enable accurate cycle information
    void reportCommit(int pid);
    void reportBegin(int pid, int cpu);
    void reportLoad(int pid);
    void reportStore(int pid);
    void reportAbort(ID utid,int pid, int tid, int nackPid, RAddr raddr, RAddr caddr, TIMESTAMP myTimeStamp, TIMESTAMP nackTimestamp);
    void reportNackStore(ID utid,int pid, int tid, int nackPid, RAddr raddr, RAddr caddr, TIMESTAMP myTimestamp, TIMESTAMP nackTimestamp);
    void reportNackLoad(ID utid,int pid, int tid, int nackPid, RAddr raddr, RAddr caddr, TIMESTAMP myTimestamp, TIMESTAMP nackTimestamp);
    void reportNackCommit(ID utid,int pid, int tid, int nackPid, TIMESTAMP myTimestamp, TIMESTAMP nackTimestamp);
    void reportNackCommitFN(ID utid,int pid, int tid, TIMESTAMP begin_timestamp);

    // The register functions are used within the fetch/execution cycle to queue the event
    // that will eventually print out in the instruction commit point
    void registerTransInst(int pid, transInstType type);
    void registerTransInstAbort(int pid, transInstType type);

    void registerCommit(ID utid,int pid, int tid, TIMESTAMP timestamp);
    void registerBegin(ID utid,int pid, int tid, RAddr PC, TIMESTAMP timestamp);
    void registerLoad(ID utid,RAddr beginPC, int pid, int tid, RAddr raddr,RAddr caddr, TIMESTAMP timestamp);
    void registerStore(ID utid,RAddr beginPC, int pid, int tid, RAddr raddr,RAddr caddr, TIMESTAMP timestamp);


    void reportBarrier ( int pid );

    void printClock();
    void print(char *out);

    void registerOut();   // Keeps track of all outputs to fflush after a certain number
    FILE* getOutfile();

    FILE *outfile;

  private:


    std::queue<memRef> loads[MAX_CPU_COUNT];
    std::queue<memRef> stores[MAX_CPU_COUNT];
    std::queue<transRef> begins[MAX_CPU_COUNT];
    std::queue<transRef> commits[MAX_CPU_COUNT];

    int outCount;       // Counter to fflush after a set number of prints
    int maxCount;

    int printDetailedTrace;
    int printRealBCTimes;
    int printAllNacks;

    RAddr nackingAddr[MAX_CPU_COUNT];
    unsigned long long nackingTimestamp[MAX_CPU_COUNT];
    int nackingPid[MAX_CPU_COUNT];
    int tmDepth[MAX_CPU_COUNT];
    int tempInstCount[MAX_CPU_COUNT][6];
    int tempInstCountAbort[MAX_CPU_COUNT][6];

   /************************************************************
    ***** Functions/Data Used For Transactional Statistics *****
    ************************************************************/

   private:

    int transactionalReport;
    int printTransactionalReportDetail;
    int printTransactionalReportSummary;
    int calculateFullReadWriteSet;

    struct conflict {
         int confPid;
         int tid;
         RAddr raddr;
         TIMESTAMP begin;
         TIMESTAMP end;
      };

    struct transData {
      ID utid;
      int pid;
      int tid;
      RAddr PC;
      int aborted;
      int cpu;
      INSTCOUNT instCount;
      unsigned long long reads;
      std::map<RAddr,INSTCOUNT> readSet;
      unsigned long long writes;
      std::map<RAddr,INSTCOUNT> writeSet;
      std::list<conflict> conflicts;
      unsigned long long beginTimestamp;
      unsigned long long endTimestamp;
      std::list<int> conflictDistribution;
     };

      std::map<unsigned long long, transData> transDataReport;
      std::map<int,ID> activeTransactions;

      std::set<RAddr> pReadSet;
      std::set<RAddr> pWriteSet;

   public:

      std::set<RAddr> return_globalReadSet(void);
      std::set<RAddr> return_globalWriteSet(void);

      void transactionalBegin(ID utid, int tid, int pid, RAddr PC,int cpu, TIMESTAMP timestamp);
      void transactionalCommit(ID utid, INSTCOUNT instCount, TIMESTAMP timestamp, INSTCOUNT fpOps);
      void transactionalAbort(ID utid, INSTCOUNT instCount);
      void transactionalNackBegin(ID utid, int tid, int confPid, RAddr raddr, TIMESTAMP timestamp);
      void transactionalNackFinish(ID utid, TIMESTAMP timestamp);
      void transactionalLoad(ID utid, RAddr addr);
      void transactionalStore(ID utid, RAddr addr);
      void transactionalComplete();
      void transactionalCompleteSummary();

   /******************************************************
    ***** Functions/Data Used For Summary Statistics *****
    ******************************************************/

   private:

    int printSummaryReport;

    unsigned long long summaryCommitCount;
    unsigned long long summaryCommitInstCount;
    unsigned long long summaryMinCommitInstCount;
    unsigned long long summaryMaxCommitInstCount;
    unsigned long long summaryCommitCycleCount;
    unsigned long long summaryMinCommitCycleCount;
    unsigned long long summaryMaxCommitCycleCount;

    unsigned long long summaryAbortCount;
    unsigned long long summaryAbortInstCount;    
    unsigned long long summaryMinAbortInstCount;
    unsigned long long summaryMaxAbortInstCount;
    unsigned long long summaryAbortCycleCount;
    unsigned long long summaryMinAbortCycleCount;
    unsigned long long summaryMaxAbortCycleCount;


    unsigned long long summaryReadSetSize;
    unsigned long long summaryWriteSetSize;
    unsigned long long summaryLoadCount;
    unsigned long long summaryStoreCount;

    std::set<RAddr> summaryReadSet[MAX_CPU_COUNT];
    std::set<RAddr> summaryWriteSet[MAX_CPU_COUNT];
    unsigned long long tempLoadCount[MAX_CPU_COUNT];
    unsigned long long tempStoreCount[MAX_CPU_COUNT];

    unsigned long long summaryNackCount[MAX_CPU_COUNT];
    unsigned long long summaryNackCycleCount[MAX_CPU_COUNT];

    unsigned long long summaryUsefulNackCount;
    unsigned long long summaryUsefulNackCycle;

    unsigned long long summaryAbortedNackCount;
    unsigned long long summaryAbortedNackCycle;

    unsigned long long summaryBeginCycle[MAX_CPU_COUNT];
    unsigned long long summaryNackCycle[MAX_CPU_COUNT];

    // Test Implementation for "Useful NACKs" Metric
    unsigned long long summaryTempNackCycleCount[MAX_CPU_COUNT];
    unsigned long long 

    int summaryTransFlag[MAX_CPU_COUNT];

   public:

    void summaryBegin(int pid, TIMESTAMP timestamp);
    void summaryCommit(int pid, INSTCOUNT instCount, TIMESTAMP timestamp);
    void summaryAbort(int pid, INSTCOUNT instCount);
    void summaryNackBegin(int pid, TIMESTAMP timestamp);
    void summaryNackFinish(int pid, TIMESTAMP timestamp);
    void summaryLoad(int pid, RAddr addr);
    void summaryStore(int pid, RAddr addr);
    void summaryComplete();

   unsigned long long return_summaryCommitCount(void) { return this->summaryCommitCount; }
   unsigned long long return_summaryReadSetSize(void) { return this->summaryReadSetSize; }
   unsigned long long return_summaryWriteSetSize(void) { return this->summaryWriteSetSize; }
   unsigned long long return_summaryLoadCount(void) { return this->summaryLoadCount; }
   unsigned long long return_summaryStoreCount(void) { return this->summaryStoreCount; }

   /************************************************
    ***** Functions/Data Used For transMemRefs *****
    ************************************************/

//     This functionality was added for Clay's Synthesis project.
//     It will create 2 maps (one for loads, one for stores). These
//     maps use the PC address of a transactional begin instruction 
//     as their keys and a list of lists of memory addresses as the
//     data.  Thus, for every static transaction a entry in each
//     map is created; and upon each new dynamic instance of that
//     transaction a new list is created and appended to the current
//     list for that static instruction that stores every memory
//     address that was accessed.


   private:
      std::list < std::list <RAddr> > emptyList;

      std::map < RAddr, std::list < std::list < RAddr > > > transMemHistoryLoads;
      std::map < RAddr, std::list < std::list < RAddr > > > transMemHistoryStores;

      int transMemRefState[MAX_CPU_COUNT];
      int recordTransMemRefs; // Flag to enable transMemRefs

      void transMemRef_newBegin(int pid, RAddr PC);
      void transMemRef_newCommit(int pid);
      void transMemRef_newLoad(RAddr PC, RAddr mem);
      void transMemRef_newStore(RAddr PC, RAddr mem);

   public:

      // Print both maps and all values out to standard output
      void transMemRef_printResults();
      // Return the entire load references map 
      std::map < RAddr, std::list < std::list < RAddr > > > transMemRef_getLoadMap();
      // Return the entire store references map
      std::map < RAddr, std::list < std::list < RAddr > > > transMemRef_getStoreMap();
      // Return the read list of lists for a specified PC address
      std::list < std::list < RAddr > > transMemRef_getLoadLists(RAddr PC);
      // Return the store list of lists for a specified 
      std::list < std::list < RAddr > > transMemRef_getStoreLists(RAddr PC);

      std::map < RAddr, std::list < std::list < RAddr > > > *transMemRef_ref_getLoadMap();
      std::map < RAddr, std::list < std::list < RAddr > > > *transMemRef_ref_getStoreMap();

      std::list < std::list < RAddr > > *transMemRef_ref_getLoadLists(RAddr PC);
      std::list < std::list < RAddr > > *transMemRef_ref_getStoreLists(RAddr PC);



   /************************************************
    ***** Functions/Data Used For beginTMStats *****
    ************************************************/

    private:

        INSTCOUNT   beginRecordkeepingInstructionCount;
        INSTCOUNT   beginRecordKeepingCycleCount;
        int         recordTMStart;

    public:

        void beginTMStats(INSTCOUNT insts);


   /*************************************************
    ***** Functions/Data Used to get instructon *****
    ***** counts for the cpu                    *****
    ************************************************/


    private:

        INSTCOUNT   committedInstCountByCpu[MAX_CPU_COUNT];

    public:

        void        incrementCommittedInstCountByCpu( int cpu );
        void        addToCommittedInstCountByCpu( int cpu, INSTCOUNT count );
        INSTCOUNT   getCommittedInstCountbyCpu( int cpu );


};

inline void transReport::print(char *out){
  fprintf(outfile,out);
}

/*
  This function is used to cause an fflush after
  a certain number of fprintf statements have been
  called.
*/

inline void transReport::registerOut(){
  if(outCount-- < 0){
    fflush(outfile);
    outCount = maxCount;
  }
}

inline void transReport::printClock()
{
    if(printDetailedTrace)
      fprintf(outfile,"<Trans> tmTrace: CLK  :99999999999:0:6666:%llu\n",globalClock);
    fflush(outfile);
}

inline FILE* transReport::getOutfile(){
  return this->outfile;
}

extern transReport *tmReport;

#endif


/**
 * @typedef ID
 * unsigned long long.
 */

/**
 * @typedef RAddr
 * uintptr_t.
 */

/**
 * @typedef TIMESTAMP
 * unsigned long long
 */

/**
 * @typedef INSTCOUNT
 * unsigned long long
 */

/**
 * @struct  memRef
 * @ingroup transReport
 * @brief   memory reference structure used for transMemRef
 *
 */

/**
 * @struct  transRef
 * @ingroup transReport
 * @brief   used for transMemRef option
 *
 */

