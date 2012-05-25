//
// C++ Implementation: Synthetic
//
// Description: 
//
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Synthetic.h"

Synthetic::Synthetic(const Synthetic& objectIn)
{
   node_ID = objectIn.node_ID;
   coreList = objectIn.coreList;
}

/**
 * @ingroup Synthetic
 * @brief This fucntion prints the contents of the input list.
 * @param detail unsigned 8b integer
 * @param streamIn output stream
 * @return unsigned 8b integer
 * Detail level:
 *  - 0:  Prints starting address for each basic block
 *  - 1:  Prints address and synchronization flags
 *  - 2:  Prints address, synchronization flags, and instruction list
 */
UINT_8 Synthetic::print_coreList(UINT_8 detail, std::ostream &streamIn)
{
   std::list <BasicBlock>::iterator basicBlockListIterator;

   switch(detail)
   {
      case 0 :
         streamIn << "\n";
         for(basicBlockListIterator = this->coreList.begin(); basicBlockListIterator != this->coreList.end(); basicBlockListIterator++)
         {
            streamIn << std::hex << basicBlockListIterator->return_bbAddress();
            streamIn << "\n";
         }
      break;
      case 1 :
         streamIn << "\n";
         for(basicBlockListIterator = this->coreList.begin(); basicBlockListIterator != this->coreList.end(); basicBlockListIterator++)
         {
            streamIn << std::hex << basicBlockListIterator->return_bbAddress() << std:: dec << " <> ";
            streamIn << basicBlockListIterator->return_isThreadEvent() << " <> ";
            streamIn << basicBlockListIterator->return_isSpawn() << " <> ";
            streamIn << basicBlockListIterator->return_isDestroy() << " <> ";
            streamIn << basicBlockListIterator->return_isCritical() << " <> ";
            streamIn << basicBlockListIterator->return_isWait() << " <> ";
            streamIn << basicBlockListIterator->return_isBarrier();
            streamIn << "\n";
         }
      break;
      case 2 :
         streamIn << "\n";
         for(basicBlockListIterator = this->coreList.begin(); basicBlockListIterator != this->coreList.end(); basicBlockListIterator++)
         {
            streamIn << std::hex << basicBlockListIterator->return_bbAddress() << std:: dec << " <> ";
            streamIn << basicBlockListIterator->return_isThreadEvent() << " <> ";
            streamIn << basicBlockListIterator->return_isSpawn() << " <> ";
            streamIn << basicBlockListIterator->return_isDestroy() << " <> ";
            streamIn << basicBlockListIterator->return_isCritical() << " <> ";
            streamIn << basicBlockListIterator->return_isWait() << " <> ";
            streamIn << basicBlockListIterator->return_isBarrier();
            streamIn << "\n";
            basicBlockListIterator->print_instructionList(streamIn);
         }
      break;
   }


   streamIn << std::dec << std::flush;

   return 1;
}

UINT_8 Synthetic::copy_coreList(std::list <BasicBlock> listIn)
{
   std::list <BasicBlock>::iterator basicBlockListIterator;

   for(basicBlockListIterator = listIn.begin(); basicBlockListIterator != listIn.end(); basicBlockListIterator++)
   {
      this->coreList.push_back(*basicBlockListIterator);
   }

   return 1;
}

UINT_8 Synthetic::clear_coreList()
{
   this->coreList.clear();
   return 1;
}

UINT_8 Synthetic::update_coreList(BasicBlock basicBlockIn)
{
   this->coreList.push_back(basicBlockIn);
   return 1;
}

UINT_8 Synthetic::inc_node_ID(void)
{
   this->node_ID = this->node_ID + 1;
   return 1;
}

BasicBlock Synthetic::return_front_of_coreList(void)
{
   BasicBlock temp;

   if(this->coreList.empty() == 1)
      return temp;
   else
      return this->coreList.front();
}

BasicBlock Synthetic::return_front_of_coreList_pop(void)
{
   BasicBlock temp;

   if(this->coreList.empty() == 1)
      return temp;
   else
   {
      BasicBlock front = this->coreList.front();
      this->coreList.pop_front();

      return front;
   }
}

BasicBlock Synthetic::return_back_of_coreList(void)
{
   BasicBlock temp;

   if(this->coreList.empty() == 1)
      return temp;
   else
      return this->coreList.back();
}

std::list <BasicBlock> Synthetic::return_coreList(void)
{
   return this->coreList;
}

std::list <BasicBlock> & Synthetic::return_coreListRef(void)
{
   return this->coreList;
}

UINT_32 Synthetic::return_size_of_coreList(void)
{
   return this->coreList.size();
}

UINT_32 Synthetic::return_node_ID(void)
{
   return this->node_ID;
}
