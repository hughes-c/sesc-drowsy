/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2004 University of Illinois.

   Contributed by Jose Renau

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

#ifndef MARKOVPREFETCHER_H
#define MARKOVPREFETCHER_H

#include <queue>

#include "Port.h"
#include "MemRequest.h"
#include "CacheCore.h"
#include "MemObj.h"

class MarkovPfState : public StateGeneric<PAddr> {
 public:
  PAddr predAddr1;
  PAddr predAddr2;
  PAddr predAddr3;
  PAddr predAddr4;
};

class MarkovQState : public StateGeneric<> {
};

/*
class MarkovTState : public StateGeneric<> {
 public:
  PAddr missAddr;
  PAddr predAddr1;
  PAddr predAddr2;
  PAddr predAddr3;
  int tag;
  };*/

class MarkovPrefetcher : public MemObj {
protected:
  GMemorySystem *gms;
  PortGeneric *cachePort;

  typedef CacheGeneric<MarkovPfState,PAddr> MarkovTable;
  MarkovTable::CacheLine *tEntry;
  PAddr lastAddr;

  typedef CacheGeneric<MarkovQState,PAddr> BuffType;
  typedef CacheGeneric<MarkovQState,PAddr>::CacheLine bLine;

  typedef HASH_MAP<PAddr, std::queue<MemRequest *> *> penReqMapper;
  typedef HASH_SET<PAddr> penFetchSet;

  penReqMapper pendingRequests;

  penFetchSet pendingFetches;

  void read(MemRequest *mreq);

  int defaultMask;
  
  int lineSize;

  BuffType *buff;
  MarkovTable *table;

  PortGeneric *buffPort;
  PortGeneric *tablePort;

  int numBuffPorts;
  int numTablePorts;
  int buffPortOccp;
  int tablePortOccp;
  int hitDelay;
  int missDelay;
  int depth;
  int width;
  int ptr;
  int age;

  GStatsCntr halfMiss;
  GStatsCntr miss;
  GStatsCntr hit;
  GStatsCntr predictions;
  GStatsCntr accesses;

  static const int tEntrySize = 8; // size of an entry in the prefetching table

public:
  MarkovPrefetcher(MemorySystem* current, const char *device_descr_section,
      const char *device_name = NULL);
  ~MarkovPrefetcher() {}
  void access(MemRequest *mreq);
  void returnAccess(MemRequest *mreq);
  bool canAcceptStore(PAddr addr);
  virtual void invalidate(PAddr addr,ushort size,MemObj *oc);
  Time_t getNextFreeCycle() const;
  void prefetch(PAddr addr, Time_t lat);
  void insertTable(PAddr paddr);
  void TESTinsertTable(PAddr paddr);
  void processAck(PAddr paddr);
  typedef CallbackMember1<MarkovPrefetcher, PAddr, &MarkovPrefetcher::processAck> processAckCB;

  Time_t nextBuffSlot() {
    return buffPort->nextSlot();
  }

  Time_t nextTableSlot() {
    return tablePort->nextSlot();
  }

#ifdef SESC_ENERGY
  void fakeWrite(unsigned int processorID, PAddr address) {};
  unsigned int fakeAbort(unsigned int processorID, unsigned int versioning) {};
  unsigned int fakeCommit(unsigned int processorID, unsigned int versioning) {};
#endif

  void sleepCacheLines(CPU_t Id) {};
};

#endif // MARKOVPREFETCHER_H
