//
// C++ Interface: programStatistics
//
// Description: 
//
//
/// @author: hughes  <hughes@fraidy2-uf>, (C) 2008
///
/// @date:          04/14/08
/// Last Modified:  04/30/08
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef PROGRAMSTATISTICS_H
#define PROGRAMSTATISTICS_H

#include <ctime>
#include "workloadCharacteristics.h"

class ProgramStatistics
{
public:
   ~ProgramStatistics()
   {
      for(UINT_32 counter = 0; counter < threadCharacteristics.size(); counter++)
      {
         delete threadCharacteristics[counter];
      }
   }

   WorkloadCharacteristics                   programCharacteristics;
   std::vector<WorkloadCharacteristics *>    threadCharacteristics;
   WorkloadCharacteristics                   transactionCharacteristics;

   UINT_8 set_reportFileName(string reportFileName)
   {
      if(reportFileName == "")
      {
         this->reportFileName = "";
         return 1;
      }
      else
      {
         string tempName;
         char buffer[40];
         time_t rawtime;
         struct tm *myTime;

         time(&rawtime);
         myTime = localtime(&rawtime);

         strftime(buffer,40,"-%b%d.%Y-%H.%M.%S-profile",myTime);
         tempName = buffer;

         this->reportFileName = reportFileName + buffer;
         return 1;
      }
   }

   string return_reportFileName(void)
   {
      return this->reportFileName;
   }

protected:

private:
   string reportFileName;
};

#endif

