//
// C++ Interface: printers
//
// Description: 
//
//
/// @author: hughes  <hughes@fraidy2-uf>, (C) 200
///
/// @date:          05/23/07
/// Last Modified:  02/05/08
//
// Copyright: See COPYING file that comes with this distribution
//
//////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef PRINTERS_H
#define PRINTERS_H

#include <iostream>
#include <sstream>  // Required for stringstreams
#include <string> 

#include "stat-types.h"
#include "stat-boost-types.h"

#include "ProcessId.h"
#include "OSSim.h"
#include "ConfObject.h"
#include "statPaths.h"

#define THREADSIN 3

extern std::string IntToString(INT_64 number);
extern std::string HexToString(INT_64 number);

namespace SynthPrinters
{

void SFGprinter(UINT_32 threadCount);
void SFGprinter(UINT_32 threadCount, string newFileName);
void PCFGprinter(void);
void PCFGprinter(string newFileName);
void printSFGStructure(void);
void printPCFGStructure(void);

void printBins(void);

}
#endif
