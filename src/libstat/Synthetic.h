//
// C++ Interface: Synthetic
//
// Description: 
//
//
/// @author: hughes  <hughes@fraidy2-uf>, (C) 2008
///
/// @date:          01/15/08
/// Last Modified: 01/15/08
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SYNTHETIC_H
#define SYNTHETIC_H

#include <list>

#include "stat-types.h"
#include "BasicBlock.h"

class Synthetic
{
public:
   /* Constructor */
   Synthetic(): node_ID(0) {};
   Synthetic(const Synthetic& objectIn);

   /* Variables */


   /* Functions */
   UINT_8                  print_coreList(UINT_8 detail, std::ostream &streamIn);

   UINT_8                  clear_coreList(void);
   UINT_8                  copy_coreList(std::list <BasicBlock> listIn);
   UINT_8                  update_coreList(BasicBlock basicBlockIn);
   UINT_8                  inc_node_ID(void);

   std::list <BasicBlock>  return_coreList(void);
   std::list <BasicBlock> &return_coreListRef(void);
   BasicBlock              return_front_of_coreList(void);
   BasicBlock              return_front_of_coreList_pop(void);
   BasicBlock              return_back_of_coreList(void);
   UINT_32                 return_size_of_coreList(void);
   UINT_32                 return_node_ID(void);

protected:


private:
   UINT_32                 node_ID;
   std::list <BasicBlock>  coreList;


};

#endif
