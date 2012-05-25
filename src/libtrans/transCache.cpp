/**
 * @file
 * @author  jpoe   <>, (C) 2008, 2009
 * @date    09/19/08
 * @brief   This is the implementation for the TM cache manager.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION 
 * C++ Implementation: transactionCache
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#include "transCache.h"

/**
 * @ingroup transCache
 * @brief Default constructor
 */
transactionCache::transactionCache()
{
}

/**
 * @ingroup transCache
 * @brief Default destructor
 */
transactionCache::~transactionCache()
{
}

/**
 * @ingroup transCache
 * @brief Word-size loads
 * 
 * @param addr Real address
 * @return     Memory value (int)
 *
 * Full description.
 */
IntRegValue transactionCache::loadWord(RAddr addr)
{
   if(addr % 4 != 0)
      printf("Potential Memory LW Issue: %#10x\n",addr);

   std::map<RAddr, IntRegValue>::iterator it;
   it = memMap.find(addr);
   if(it != memMap.end()){
      return SWAP_WORD(it->second);
   }
   else{
      return SWAP_WORD(*(int *)addr);
   }
}

/**
 * @ingroup transCache
 * @brief Byte-size loads
 * 
 * @param addr Real address
 * @return     Memory value (int)
 */
IntRegValue transactionCache::loadByte(RAddr addr)
{
  int z = 0;
  RAddr oaddr = addr;

  while(addr % 4 != 0){
    addr--;
    z++;
  }

  std::map<RAddr, IntRegValue>::iterator it;
  it = memMap.find(addr);
  if(it != memMap.end()){

    int mem = it->second;
    int retVal = (mem >> (8 * z)) & 0xFF;
    return retVal;
  }
  else{
    return (int) *(unsigned char *) oaddr;
  }
}

/**
 * @ingroup transCache
 * @brief Byte-size stores
 * 
 * @param addr    Real address
 * @param value   Memory value (int)
 */
void transactionCache::storeByte(RAddr addr, IntRegValue value)
{
  int z = 0;
  IntRegValue curMemValue;

  RAddr oaddr = addr;

  while(addr % 4 != 0){
    addr--;
    z++;
  }

  std::map<RAddr, IntRegValue>::iterator it;
  it = memMap.find(addr);
  if(it != memMap.end())
    curMemValue = it->second;
  else
    curMemValue = *(int *)addr;


  memMap[addr] = (value << (8 * z)) | (curMemValue & ~(0xff << (z * 8)));
}

/**
 * @ingroup transCache
 * @brief Half Word-size stores
 * 
 * @param addr    Real address
 * @param value   Memory value (int)
 */
void transactionCache::storeHalfWord(RAddr addr, IntRegValue value)
{
  int z = 0;
  IntRegValue curMemValue;

  RAddr oaddr = addr;

  while(addr % 4 != 0){
    addr--;
    z++;
  }

  if(z > 2)
    printf("Potential Memory LDFP Issue: %#10x\n",addr);

  std::map<RAddr, IntRegValue>::iterator it;
  it = memMap.find(addr);
  if(it != memMap.end())
    curMemValue = it->second;
  else
    curMemValue = *(int *)addr;

  memMap[addr] = (value << (16 * z)) | (curMemValue & ~(0xff << (z * 16)));
}

/**
 * @ingroup transCache
 * @brief Word-size stores
 * 
 * @param addr    Real address
 * @param value   Memory value (int)
 */
void transactionCache::storeWord(RAddr addr, IntRegValue value)
{
  if(addr % 4 != 0)
    printf("Potential Memory SW Issue: %#10x\n",addr);

  memMap[addr]=value; 
}

/**
 * @ingroup transCache
 * @brief Floating-point stores
 * 
 * @param addr    Real address
 * @param value   Memory value (int)
 */
void transactionCache::storeFPWord(RAddr addr, IntRegValue value)
{
  if(addr % 4!=0)
    printf("Potential Memory SFPW Issue: %#10x\n",addr);

  memMap[addr]=value; 
}

/**
 * @ingroup transCache
 * @brief Double prec. floating-point stores
 * 
 * @param addr    Real address
 * @param value   Memory value (64b)
 */
void transactionCache::storeDFP(RAddr addr, unsigned long long value)
{
  if(addr % 4!=0)
    printf("Potential Memory SDFP Issue: %#10x\n",addr);

  memMap[addr]=(int)(value & 0x00000000FFFFFFFF);
  memMap[addr + 4]=(int)((value & 0xFFFFFFFF00000000LL) >> 32);
}

/**
 * @ingroup transCache
 * @brief Double prec. floating-point loads
 * 
 * @param addr    Real address
 * @return        Memory value (double prec.)
 */
double transactionCache::loadDFP(RAddr addr)
{
  if(addr%4!=0)
    printf("Potential Memory LDFP Issue: %#10x\n",addr);

  double retval;
  unsigned int upper = loadWord(addr);
  unsigned int lower = loadWord(addr+4);

  unsigned long long intval = (((unsigned long long)upper)<<32) + lower;

  memcpy(&retval, &intval, sizeof(double));

  return retval;
}

/**
 * @ingroup transCache
 * @brief Unsigned half word-size loads
 * 
 * @param addr Real address
 * @return     Memory value (int)
 */
IntRegValue transactionCache::loadUnsignedHalfword(RAddr addr)
{
  RAddr oaddr = addr;
  int z = 0;

  while(addr%4 != 0){
    addr--;
    z++;
  }

  if(z>2)
    printf("Potential Memory LUHW Issue: %#10x\n",addr);

  std::map<RAddr, IntRegValue>::iterator it;
  it = memMap.find(addr);
  if(it != memMap.end()){
    int mem = it->second;
    int retVal = (mem >> (8 * z)) & 0xFFFF;
    return SWAP_SHORT(retVal);
  }
  else{
    return SWAP_SHORT((int) *(unsigned short *) oaddr);
  }
}

/**
 * @ingroup transCache
 * @brief Half word-size loads
 * 
 * @param addr Real address
 * @return     Memory value (int)
 */
IntRegValue transactionCache::loadHalfword(RAddr addr)
{

  RAddr oaddr = addr;
  unsigned short val;
  int z = 0;

  while(addr % 4 != 0){
    addr--;
    z++;
  }

  if(z>2)
    printf("Potential Memory LHW Issue: %#10x\n",addr);

  std::map<RAddr, IntRegValue>::iterator it;
  it = memMap.find(addr);
  if(it != memMap.end()){
    int mem = it->second;
    val = (mem >> (8 * z)) & 0xFFFF;
  }
  else{
    val = *(unsigned short *) addr;
  }

  val = SWAP_SHORT(val);

  int retval = (signed short) val;

  return retval;
}


/**
 * @ingroup transCache
 * @brief Floating-point loads
 * 
 * @param addr    Real address
 * @return        Memory contents (float)
 */
float transactionCache::loadFPWord(RAddr addr)
{
  if(addr % 4!=0)
    printf("Potential Memory LFPW Issue: %#10x\n",addr);

  unsigned int intval = loadWord(addr);

  float fpval;

  memcpy(&fpval, &intval, sizeof(float));

  return fpval;
}

/**
 * @ingroup transCache
 * @brief Find word boundary
 * 
 * @param addr Real address
 * @return     Real address
 */
RAddr transactionCache::findWordAddress(RAddr addr)
{
  while(addr % 4 != 0)
    addr = addr-1;
  return addr;
}

void transactionCache::writeBuffer(char *buff,RAddr buffBegin, int count)
{
}

void transactionCache::readBuffer(char *buff,RAddr buffBegin, int count)
{
}

std::map<RAddr, IntRegValue>::iterator transactionCache::getBeginIterator()
{
  return memMap.begin();
}

std::map<RAddr, IntRegValue>::iterator transactionCache::getEndIterator()
{
  return memMap.end();
}

