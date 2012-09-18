/**
 * @file
 * @author  chughes   <>, (C) 2012
 * @date    09/18/12
 * @brief   
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Interface: transCoherence \n
 * This object provides the functional coherence. 
 *
 * @note
 *
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEM_INFO
#define MEM_INFO

struct memInfo
{
   size_t lastClock;
   std::vector<size_t> memBins;
};


#endif