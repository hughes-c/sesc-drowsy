/* Added 11/19/04 DVDB
   The original release notes for this file are below.  This was originally ReportGen.cpp

   Edited by David van der Bokke
*/
/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Basilio Fraguela

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

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <alloca.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <ctype.h>

#include "nanassert.h"
#include "ReportTherm.h"
#include "GProcessor.h"
#include "OSSim.h"
#include "GEnergy.h"

using namespace std;


FILE *ReportTherm::rfd[MAXREPORTSTACK];
int ReportTherm::cyclesPerSample = 0; //number of cycles between stats/thermal dumps
int ReportTherm::tos = 0;
int ReportTherm::rep = 0;

StaticCallbackFunction0<ReportTherm::report> ReportTherm::reportCB;


std::string ReportTherm::fileName_powerTrace;

ReportTherm::ReportTherm()
{
  rfd[0]=stdout;
  tos=1;
}

//  Initializes the report file if this is the first write Dumps Energy and
//  Statistics to the thermal report file
void ReportTherm::report() 
{
  if(rep == 0)
  {
    GStatsEnergy::setupDump(0);
    ReportTherm::flush();
    rep = 1;
  }

transCoherence *boo = transGCM;
  GStatsEnergy::printDump(0);
  ReportTherm::flush();

  //Schedule dumps every so many cycles -- cyclesPerSample in conf
  if(rep == 1)
    reportCB.schedule(ReportTherm::cyclesPerSample);

   //During the callback, check to see if any of the processors are currently stalled due
   //to a conflict.
  #if defined(TRANS_DVFS)
  if(transGCM->useDVFS == 1)
   {
      for(size_t procID = 0; procID < 4; procID++)
      {
         if(transGCM->checkStallState(procID) == 1)
         {
            size_t boo = procID;
            size_t yar = transGCM->get_nackingPE(procID);

            if(DVFS_DEBUG == 1)
            {
               std::cout << "PROC " << procID << " STALLED -----------   " << transGCM->get_nackingPE(procID) << "   " << globalClock;
               std::cout << "   " << transGCM->get_nackState(procID);
               std::cout << "   " << transGCM->prPointer->at(procID)->get_executeState() << "   ";
               std::cout << "   " << transGCM->prPointer->at(transGCM->get_nackingPE(procID))->get_executeState() << "\n";
            }

            transGCM->set_nackState(procID, 1);

            if(transGCM->prPointer->at(procID)->get_executeState() <= 12)
            {
               transGCM->prPointer->at(procID)->set_executeState(transGCM->prPointer->at(procID)->get_executeState() + 3);
            }

            if(transGCM->prPointer->at(transGCM->get_nackingPE(procID))->get_executeState() > 1)
            {
               transGCM->prPointer->at(transGCM->get_nackingPE(procID))->set_executeState(transGCM->prPointer->at(transGCM->get_nackingPE(procID))->get_executeState() - 1);
            }

         }
         else if(transGCM->get_nackState(procID) == 1)
         {
            transGCM->set_nackState(procID, 0);
            transGCM->prPointer->at(procID)->set_executeState(DEFAULT_DVFS);
         }
      }
   }
   #endif

}

// Stops the callback
void ReportTherm::stopCB() 
{
  rep = 2;
}

void ReportTherm::openFile(char *name)
{
  I(tos<MAXREPORTSTACK);

  FILE *ffd1;
  rep = 0;

  if(strstr(name, "XXXXXX")) {
    int fd;

    fd = mkstemp(name);
    ffd1 = fdopen(fd, "a");
  }else{
    ffd1 = fopen(name, "a");
  }

  if(ffd1 == 0) {
    fprintf(stderr, "NANASSERT::REPORT could not open temporal file [%s]\n", name);
    exit(-3);
  }

  rfd[tos++] = ffd1;
  reportCB.schedule(1);

  const char *model = SescConf->getCharPtr("thermal","model");
  cyclesPerSample   = SescConf->getInt(model,"cyclesPerSample");
  SescConf->isBetween(model,"cyclesPerSample", 100, 1e6);
}

void ReportTherm::close()
{
  rep = 2;
  while( tos ) {
    printf(".");
    tos--;
    fclose(rfd[tos]);
  }
}

void ReportTherm::field(int fn, const char *format,...)
{
  va_list ap;

  I( fn < tos );
  FILE *ffd = rfd[fn];
  
  va_start(ap, format);

  vfprintf(ffd, format, ap);

  va_end(ap);

  //fprintf(ffd, "\n");
}

void ReportTherm::field(const char *format, ...)
{
  va_list ap;

  I( tos );
  FILE *ffd = rfd[tos-1];

  va_start(ap, format);

  vfprintf(ffd, format, ap);

  va_end(ap);

  //fprintf(ffd, "\n");
}

void ReportTherm::fieldRaw(float val)
{
  FILE *ffd = rfd[tos-1];

  fwrite(&val, sizeof(float), 1, ffd);
}

void ReportTherm::flush()
{
  if( tos == 0 )
    return;

  fflush(rfd[tos-1]);
}

void ReportTherm::openPowerTraceFile(std::string fileName)
{
   std::ofstream outputFile(fileName.c_str(), ios::trunc);        //open a file for writing (truncate the current contents)
   if(!outputFile)                                                //check to be sure file is open
      std::cerr << "Error opening file.";
}


std::string ReportTherm::IntToString(int64_t input)
{
   std::ostringstream output;
   output << input;

   return output.str();
}

void ReportTherm::field_to_file(std::string fieldIn, std::ostream &outputStream)
{
   outputStream << fieldIn;
}

void ReportTherm::flush_file(std::ostream &outputStream)
{
   outputStream << endl;
}

void ReportTherm::fieldRaw_to_file(double value, std::ostream &outputStream)
{
   std::string delineator = ":";
   outputStream << value;
   outputStream << delineator;
}

