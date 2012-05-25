//
// C++ Interface: statPaths
//
// Description: 
//
//
/// @author: hughes  <hughes@fraidy2-uf>, (C) 2008
///
/// @date:          06/07/08
/// Last Modified:  06/07/08
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef STATPATHS_H
#define STATPATHS_H

#include <ctime>
#include "OSSim.h"

class StatPaths
{
public:
   StatPaths()
   {
      synthDirectory   = "synth/";
      dataDirectory    = "raw/";
   }

   UINT_8 set_reportFileName(string reportFileName)
   {
      if(reportFileName == "")
      {
         this->rootDirectory = "/home/hughes/Benchies/MIPS/asmTesting/newTMSynth/";
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

         strftime(buffer,40,"-%b%d.%Y-%H.%M.%S",myTime);
         tempName = buffer;

         size_t cutOff = reportFileName.find_last_of("/");

         this->reportFileName = reportFileName.substr(cutOff + 1) + buffer;

         if(cutOff == string::npos)
            this->rootDirectory = "/home/hughes/Benchies/MIPS/asmTesting/newTMSynth/";
         else
            this->rootDirectory = reportFileName.substr(0, cutOff + 1);

         return 1;
      }
   }

   UINT_8 set_outputFileName(string outputFileName)
   {
      if(this->reportFileName == "")
      {
         this->outputFileName = outputFileName + ".synthetic" + ".c";
      }
      else
      {
         size_t cutOff = this->reportFileName.find_last_of(".");

         if(this->reportFileName.substr(5, cutOff - 29) == outputFileName)
            this->outputFileName = OSSim::getBenchName();
         else
            this->outputFileName = this->reportFileName.substr(0, cutOff + 1) + outputFileName;
      }
      return 1;
   }

   inline string return_reportFileName(void)
   {
      return this->reportFileName;
   }

   inline string return_rootDirectory(void)
   {
      return this->rootDirectory;
   }

   inline string return_synthDirectory(void)
   {
      return this->synthDirectory;
   }

   inline string return_dataDirectory(void)
   {
      return this->dataDirectory;
   }

   inline string return_outputFileName(void)
   {
      return this->outputFileName;
   }

protected:

private:
   string reportFileName;

   string rootDirectory;
   string synthDirectory;
   string dataDirectory;
   string outputFileName;

};

#endif
