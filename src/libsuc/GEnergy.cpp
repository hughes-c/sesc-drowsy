
/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by  Smruti Sarangi

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

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <cmath>

#include "EnergyMgr.h"
#include "ReportGen.h"

#ifdef SESC_THERM
#include "ReportTherm.h"
#endif

#ifdef SESC_ENERGY
#include "GProcessor.h"
#endif

GStatsEnergy::EProcStoreType  GStatsEnergy::eProcStore;
GStatsEnergy::EGroupStoreType GStatsEnergy::eGroupStore;

double GStatsEnergyNull::getDouble() const
{
  return 0;
}

void GStatsEnergyNull::inc()
{
}

void GStatsEnergyNull::add(int v)
{
}

GStatsEnergy::GStatsEnergy(const char *field, const char *blk
			   ,int procId, PowerGroup grp
			   ,double energy)
  :StepEnergy(energy)
  ,steps(0)
  ,gid(grp)
{
  if( eProcStore.size() <= static_cast<size_t>(procId) )
  {
    eProcStore.resize(procId+1);
  }
  eProcStore[procId].push_back(this);

  if(eGroupStore.size() <= static_cast<size_t>(grp) )
    eGroupStore.resize(grp + 1);
  eGroupStore[grp].push_back(this);

  char cadena[1024];
  sprintf(cadena,"%s:%s", blk, field) ;
  name = strdup(cadena);
  subscribe();
}


#ifdef SESC_THERM
void GStatsEnergy::setupDump(int procId) 
{
  I((unsigned int)procId < eProcStore.size());

  for(size_t i = 0; i < eProcStore[procId].size(); i++)
  {
    GStatsEnergy *e = eProcStore[procId][i];

    ReportTherm::field("%s\t", e->name);

    //Trans Power
    std::string fieldName = e->name;
    ReportTherm::field_to_file(fieldName + "\t", ReportTherm::powerTraceFile);

  }

  ReportTherm::field("\n");

  //Trans Power
  ReportTherm::field_to_file("\n", ReportTherm::powerTraceFile);
#if 0
  for (size_t i = 0; i < eProcStore[procId].size(); i++) {
    GStatsEnergy *e = eProcStore[procId][i];

    e->inc(); // charge unit energy to compute power densities
    double pwr = EnergyMgr::etop(e->getDouble());
    ReportTherm::field("%g\t", pwr);
  }
  ReportTherm::field("\n");
#endif
}

void GStatsEnergy::printDump(int procId) 
{
   I((unsigned int)procId < eProcStore.size());

   // dump values
   for(size_t i = 0; i < eProcStore[procId].size(); i++)
   {
      double pwr;
      GStatsEnergy *e = eProcStore[procId][i];

      #if defined(SEP_DVFS)
      double myFrequency;
      size_t myState;
      std::string tempName = e->name;
//       std::cout << tempName << "    ";

      if(tempName.find("_3_") != std::string::npos)
      {
         std::string freq_str = tempName.substr(tempName.find("_3_"));
//          std::cout << freq_str;
//          std::cout << "   " << GProcessor::freq_to_state(freq_str);

         myState = GProcessor::freq_to_state(freq_str);
         myFrequency = GProcessor::state_to_freq_dbl(myState);
//          std::cout << "   " << myFrequency;
      }
      else if(tempName.find("_2_") != std::string::npos)
      {
         std::string freq_str = tempName.substr(tempName.find("_2_"));
//          std::cout << freq_str;
//          std::cout << "   " << GProcessor::freq_to_state(freq_str);

         myState = GProcessor::freq_to_state(freq_str);
         myFrequency = GProcessor::state_to_freq_dbl(myState);
//          std::cout << "   " << myFrequency;
      }
      else if(tempName.find("_1_") != std::string::npos)
      {
         std::string freq_str = tempName.substr(tempName.find("_1_"));
//          std::cout << freq_str;
//          std::cout << "   " << GProcessor::freq_to_state(freq_str);

         myState = GProcessor::freq_to_state(freq_str);
         myFrequency = GProcessor::state_to_freq_dbl(myState);
//          std::cout << "   " << myFrequency;
      }
      else
      {
         myState = DEFAULT_DVFS;
         myFrequency = GProcessor::state_to_freq_dbl(myState);
      }

//       std::cout << "\n";

      if(proc_stateCycles[procId][myState] > 0)
         pwr = EnergyMgr::etop(e->getDouble(), proc_stateCycles[procId][myState] - 1, myFrequency);
      else
         pwr = EnergyMgr::etop(e->getDouble());

//       std::cout << std::setw(48) << e->name << std::setw(16) << e->getDouble() << std::setw(6) << proc_stateCycles[procId][myState] << std::setw(16) << myFrequency << std::setw(6) << pwr << std::endl;

      #else
      pwr = EnergyMgr::etop(e->getDouble());
      #endif

      ReportTherm::fieldRaw(pwr);

   }

   ///NOTE The dump to *power* begins here
   bool triggered = 0;
   size_t nProcs = SescConf->getRecordSize("","cpucore");

  for(size_t threadID = 0; threadID < nProcs; threadID++)
  {
      I(threadID < eProcStore.size());

      //print list of structures -- skip P0
      if(globalClock == 1 && triggered == 0)
      {
         for(size_t threadID = 1; threadID < nProcs; threadID++)
         {
            ReportTherm::field_to_file("\n", ReportTherm::powerTraceFile);
            for(size_t i = 0; i < eProcStore[threadID].size(); i++)
            {
               GStatsEnergy *e = eProcStore[threadID][i];

               //Trans Power
               std::string fieldName = e->name;
               ReportTherm::field_to_file(fieldName + "\t", ReportTherm::powerTraceFile);
            }

            ReportTherm::field_to_file("\n", ReportTherm::powerTraceFile);
         }

         triggered = 1;
         ReportTherm::field_to_file("VALUES\n", ReportTherm::powerTraceFile);
      }

      //Add PE ID to file
      ReportTherm::field_to_file(ReportTherm::IntToString(threadID), ReportTherm::powerTraceFile);

      //Add clock to file
      std::string clockDump = ":" + ReportTherm::IntToString(globalClock) + ":";
      ReportTherm::field_to_file(clockDump, ReportTherm::powerTraceFile);

      for(size_t i = 0; i < eProcStore[threadID].size(); i++)
      {
         double pwr;
         GStatsEnergy *e = eProcStore[threadID][i];

         #if defined(SEP_DVFS)
         size_t myState;
         double myFrequency;
         std::string tempName = e->name;

         if(tempName.find("_3_") != std::string::npos)
         {
            std::string freq_str = tempName.substr(tempName.find("_3_"));
            myState = GProcessor::freq_to_state(freq_str);
            myFrequency = GProcessor::state_to_freq_dbl(myState);
         }
         else if(tempName.find("_2_") != std::string::npos)
         {
            std::string freq_str = tempName.substr(tempName.find("_2_"));
            myState = GProcessor::freq_to_state(freq_str);
            myFrequency = GProcessor::state_to_freq_dbl(myState);
         }
         else if(tempName.find("_1_") != std::string::npos)
         {
            std::string freq_str = tempName.substr(tempName.find("_1_"));
            myState = GProcessor::freq_to_state(freq_str);
            myFrequency = GProcessor::state_to_freq_dbl(myState);
         }
         else
         {
            myState = DEFAULT_DVFS;
            myFrequency = GProcessor::state_to_freq_dbl(myState);
         }

         if(proc_stateCycles[threadID][myState] > 0)
            pwr = EnergyMgr::etop(e->getDouble(), proc_stateCycles[threadID][myState] - 1, myFrequency);
         else
            pwr = EnergyMgr::etop(e->getDouble());

//          std::cout << threadID << " - " << myState << ":  ";
//          std::cout << std::setw(48) << e->name << std::setw(16) << e->getDouble() << std::setw(6) << proc_stateCycles[threadID][myState] << std::setw(16) << myFrequency << std::setw(16) << pwr << std::endl;


         #else
         pwr = EnergyMgr::etop(e->getDouble());
         #endif

         ReportTherm::fieldRaw_to_file(pwr, ReportTherm::powerTraceFile);
      }

      ReportTherm::field_to_file("\n", ReportTherm::powerTraceFile);
      GStatsEnergy::dumpPowerTrace(threadID, GStatsEnergy::get_sampleTime(), ReportTherm::powerTraceFile);
//          GStatsEnergy::dumpEnergyTrace(threadID, GStatsEnergy::get_sampleTime(), ReportTherm::powerTraceFile);

      ReportTherm::field_to_file("\n", ReportTherm::powerTraceFile);
  }
}

void GStatsEnergy::reportValueDumpSetup() const
{
}

void GStatsEnergy::reportValueDump() const
{
}
#endif

void GStatsEnergy::dump(int procId)
{
   double pVals[MaxPowerGroup];
   for(int c=0; c < MaxPowerGroup; c++)
      pVals[c] = 0.0;

   I((unsigned int)procId < eProcStore.size());

   // calculate the values
   for(size_t i = 0; i < eProcStore[procId].size(); i++)
   {
      GStatsEnergy *e = eProcStore[procId][i];

      #if defined(SEP_DVFS)
      size_t myState;
      double myFrequency;
      std::string tempName = e->name;

      if(tempName.find("_3_") != std::string::npos)
      {
         std::string freq_str = tempName.substr(tempName.find("_3_"));
         myState = GProcessor::freq_to_state(freq_str);
         myFrequency = GProcessor::state_to_freq_dbl(myState);
      }
      else if(tempName.find("_2_") != std::string::npos)
      {
         std::string freq_str = tempName.substr(tempName.find("_2_"));
         myState = GProcessor::freq_to_state(freq_str);
         myFrequency = GProcessor::state_to_freq_dbl(myState);
      }
      else if(tempName.find("_1_") != std::string::npos)
      {
         std::string freq_str = tempName.substr(tempName.find("_1_"));
         myState = GProcessor::freq_to_state(freq_str);
         myFrequency = GProcessor::state_to_freq_dbl(myState);
      }
      else
      {
         myState = DEFAULT_DVFS;
         myFrequency = GProcessor::state_to_freq_dbl(myState);
      }

      if(proc_stateCycles[procId][myState] > 0)
         pVals[e->getGid()] += EnergyMgr::etop(e->getDouble(), proc_stateCycles[procId][myState] - 1, myFrequency);
      else
         pVals[e->getGid()] += EnergyMgr::etop(e->getDouble());
      
      size_t debugging = 0;
      if(debugging == 1)
      {
         std::cout << procId << " - " << myState << ":  ";
         std::cout << std::setw(48) << e->name << std::setw(16) << e->getDouble() << std::setw(18) << proc_stateCycles[procId][myState] << std::setw(14) << myFrequency << std::setw(18);
         
         if(proc_stateCycles[procId][myState] > 0)
            std::cout << EnergyMgr::etop(e->getDouble(), proc_stateCycles[procId][myState] - 1, myFrequency);
         else
            std::cout << EnergyMgr::etop(e->getDouble());
         
         std::cout <<  "        " << pVals[e->getGid()] << std::endl;
      }

      #else
      pVals[e->getGid()] += EnergyMgr::etop(e->getDouble());
      #endif
   }

   // dump the values
   for(int j=1; j < ClockPower;j++)
   {
      Report::field("Proc(%d):%s=%g",procId, EnergyStore::getStr(static_cast<PowerGroup>(j)), pVals[j]);
   }
}

void GStatsEnergy::dumpPowerTrace(int procId, Time_t interval, std::ostream &outStream)
{
   /* Variables */
   double pVals[MaxPowerGroup];
   double tempEnergy[MaxPowerGroup];

   /* Processes */
   for(int c = 0; c < MaxPowerGroup; c++)
   {
      pVals[c] = 0.0;
      tempEnergy[c] = 0.0;
   }

   I((unsigned int)procId < eProcStore.size());


   // calculate the values
   for(size_t i = 0; i < eProcStore[procId].size(); i++)
   {
      GStatsEnergy *e = eProcStore[procId][i];

      #if defined(SEP_DVFS)
      size_t myState;
      double myFrequency;
      std::string tempName = e->name;

      if(tempName.find("_3_") != std::string::npos)
      {
         std::string freq_str = tempName.substr(tempName.find("_3_"));
         myState = GProcessor::freq_to_state(freq_str);
         myFrequency = GProcessor::state_to_freq_dbl(myState);
      }
      else if(tempName.find("_2_") != std::string::npos)
      {
         std::string freq_str = tempName.substr(tempName.find("_2_"));
         myState = GProcessor::freq_to_state(freq_str);
         myFrequency = GProcessor::state_to_freq_dbl(myState);
      }
      else if(tempName.find("_1_") != std::string::npos)
      {
         std::string freq_str = tempName.substr(tempName.find("_1_"));
         myState = GProcessor::freq_to_state(freq_str);
         myFrequency = GProcessor::state_to_freq_dbl(myState);
      }
      else
      {
         myState = DEFAULT_DVFS;
         myFrequency = GProcessor::state_to_freq_dbl(myState);
      }

      if(proc_stateCycles[procId][myState] > 0)
      {
         pVals[e->getGid()] =  pVals[e->getGid()] + EnergyMgr::etop(e->getDouble(), proc_stateCycles[procId][myState] - 1, myFrequency);
         tempEnergy[e->getGid()] =  tempEnergy[e->getGid()] + e->getDouble();
      }
      else
      {
         pVals[e->getGid()] =  pVals[e->getGid()] + EnergyMgr::etop(e->getDouble());
         tempEnergy[e->getGid()] =  tempEnergy[e->getGid()] + e->getDouble();
      }

      #else
      pVals[e->getGid()] =  pVals[e->getGid()] + EnergyMgr::etop(e->getDouble());
      tempEnergy[e->getGid()] =  tempEnergy[e->getGid()] + e->getDouble();
      #endif
   }

   // dump the values
   for(int j = 1; j < ClockPower; j++)
   {
      outStream << "P:" << procId << ":" << EnergyStore::getStr(static_cast<PowerGroup>(j)) << ":" << pVals[j] << ":" <<  EnergyStore::getEnergyStr(static_cast<PowerGroup>(j)) << ":" << tempEnergy[j] <<  "\n";
   }
}

void GStatsEnergy::dumpEnergyTrace(int procId, Time_t interval, std::ostream &outStream)
{
   /* Variables */
   static double perPEEnergy[MaxPowerGroup][32];
   double pVals[MaxPowerGroup];

   /* Processes */
   for(int c = 0; c < MaxPowerGroup; c++)
   {
      pVals[c] = 0.0;
   }

   I((unsigned int)procId < eProcStore.size());

   // calculate the values
   for(size_t i = 0; i < eProcStore[procId].size(); i++)
   {
      GStatsEnergy *e = eProcStore[procId][i];

      pVals[e->getGid()] =  pVals[e->getGid()] + e->getDouble();
   }

   // dump the values
   for(int j = 1; j < ClockPower; j++)
      outStream << "Proc(" << procId << "):" << EnergyStore::getEnergyStr(static_cast<PowerGroup>(j)) << "=" << pVals[j] << " <> " <<  pVals[j] - perPEEnergy[j][procId] << "\n";

   for(size_t c = 0; c < MaxPowerGroup; c++)
   {
      perPEEnergy[c][procId] = perPEEnergy[c][procId] + pVals[c];
   }
}

void GStatsEnergy::dump()
{
   double pVals[MaxPowerGroup];
   double eVals[MaxPowerGroup];
   for(int i=0; i < MaxPowerGroup; i++)
   {
      pVals[i] = 0.0;
      eVals[i] = 0.0;
   }

   // iterate through the groups and calculate the values
   for(size_t i = 1; i < MaxPowerGroup; i++)
   {
      PowerGroup pg = static_cast<PowerGroup>(i);

      #if defined(SEP_DVFS)
      double    pwr = GStatsEnergy::getTotalGroupPower(pg);
      double    nrg = GStatsEnergy::getTotalGroupEnergy(pg);
      #else
      double    pwr = EnergyMgr::etop(GStatsEnergy::getTotalGroup(pg));
      double    nrg = 0;
      #endif

      pVals[pg] += pwr;
      eVals[pg] += nrg;
   }

   // dump the values
   for(int j=1; j < MaxPowerGroup;j++)
   {
      Report::field("PowerMgr:%s=%g",EnergyStore::getStr(static_cast<PowerGroup>(j)),pVals[j]);
   }
   for(int j=1; j < MaxPowerGroup;j++)
   {
      #if defined(SEP_DVFS)
      Report::field("EnergyMgr:%s=%g",EnergyStore::getEnergyStr(static_cast<PowerGroup>(j)),eVals[j]);
      #else
      Report::field("EnergyMgr:%s=%g",EnergyStore::getEnergyStr(static_cast<PowerGroup>(j)),EnergyMgr::ptoe(pVals[j]));
      #endif
   }
}

double GStatsEnergy::getTotalPower()
{
   double totalPower = 0.0;

   double pVals[MaxPowerGroup];
   for(int i=0; i < MaxPowerGroup; i++)
   {
      pVals[i] = 0.0;
   }

   // iterate through the groups and calculate the values
   for(size_t i = 1; i < MaxPowerGroup; i++)
   {
      PowerGroup pg = static_cast<PowerGroup>(i);

      #if defined(SEP_DVFS)
      double    pwr = GStatsEnergy::getTotalGroupPower(pg);
      #else
      double    pwr = EnergyMgr::etop(GStatsEnergy::getTotalGroup(pg));
      #endif

      pVals[pg] += pwr;
   }

   #if defined(SEP_DVFS)
   for(int j=1; j < MaxPowerGroup;j++)
      totalPower += pVals[j];
   #else
   for(int j=1; j < MaxPowerGroup;j++)
      totalPower += EnergyMgr::etop(pVals[j]);
   #endif

   return totalPower;
}

double GStatsEnergy::getTotalEnergy()
{
  double totalEnergy = 0.0;

  double pVals[MaxPowerGroup];
  double eVals[MaxPowerGroup];
  for(int i=0; i < MaxPowerGroup; i++)
  {
     pVals[i] = 0.0;
     eVals[i] = 0.0;
  }

  // calculate the values
  for(size_t i = 1; i < MaxPowerGroup; i++)
  {
    PowerGroup pg = static_cast<PowerGroup>(i);

    #if defined(SEP_DVFS)
    double    pwr = GStatsEnergy::getTotalGroupPower(pg);
    double    nrg = GStatsEnergy::getTotalGroupEnergy(pg);
    #else
    double    pwr = EnergyMgr::etop(GStatsEnergy::getTotalGroup(pg));
    double    nrg = 0;
    #endif

    pVals[pg] += pwr;
    eVals[pg] += nrg;
  }

   // dump the values
   #if defined(SEP_DVFS)
  for(int j=1; j < MaxPowerGroup;j++)
     totalEnergy += eVals[j];
   #else
   for(int j=1; j < MaxPowerGroup;j++)
      totalEnergy += EnergyMgr::ptoe(pVals[j]);
   #endif

  return totalEnergy;
}

void GStatsEnergy::reportValue() const
{
  Report::field("%s=%g", name, getDouble());
}

double GStatsEnergy::getTotalProc(int procId)
{
  double total=0;

  I((unsigned int)procId < eProcStore.size());

  for(size_t i = 0; i < eProcStore[procId].size(); i++)
  {
    GStatsEnergy *e = eProcStore[procId][i];
    total += e->getDouble();
//std::cout << e->name << "\n";
  }

  return total;
}

double GStatsEnergy::getTotalGroup(PowerGroup grp)
{
  double total=0;

  if(eGroupStore.size() <= static_cast<size_t>(grp))
    return 0.0;

  for(size_t i = 0; i < eGroupStore[grp].size(); i++)
  {
    total += eGroupStore[grp][i]->getDouble();
  }

  return total;
}

#if defined(SEP_DVFS)
double GStatsEnergy::getTotalProcPower(int procId)
{
   double time = 0.0;
   double totalEnergy = 0.0;

   double total = 0.0;

   totalEnergy = getTotalProcEnergy(procId);

   //Need to calculate the total execution time for the proc based on cycle count per state
   for(size_t counter = 0; counter < NUM_STATES; counter++)
   {
      time = time + (proc_stateCycles[procId][counter] / GProcessor::state_to_freq_dbl(counter));
   }

   total = (totalEnergy * pow(10, -9)) / time;                                         //convert from J to nJ

   return total;
}

double GStatsEnergy::getTotalProcEnergy(int procId)
{
   double total = 0;
   double tempTotal[NUM_STATES] = {0.0};

   I((unsigned int)procId < eProcStore.size());

   for(size_t i = 0; i < eProcStore[procId].size(); i++)
   {
      GStatsEnergy *e = eProcStore[procId][i];

      double power;
      size_t myState;
      double myFrequency;
      std::string tempName = e->name;

      size_t procID;
      size_t location;
      location = tempName.find_first_of("(") + 1;
      procID = atoi(&tempName[location]);

      if(tempName.find("_3_") != std::string::npos)
      {
         std::string freq_str = tempName.substr(tempName.find("_3_"));
         myState = GProcessor::freq_to_state(freq_str);
      }
      else if(tempName.find("_2_") != std::string::npos)
      {
         std::string freq_str = tempName.substr(tempName.find("_2_"));
         myState = GProcessor::freq_to_state(freq_str);
      }
      else if(tempName.find("_1_") != std::string::npos)
      {
         std::string freq_str = tempName.substr(tempName.find("_1_"));
         myState = GProcessor::freq_to_state(freq_str);
      }
      else
      {
         myState = DEFAULT_DVFS;
      }

      tempTotal[myState] = tempTotal[myState] + e->getDouble();
   }

   for(size_t counter = 0; counter < NUM_STATES; counter++)
   {
      total = total + tempTotal[counter];
   }

   return total;
}

double GStatsEnergy::getTotalGroupPower(PowerGroup grp)
{
   double time = 0.0;
   double totalEnergy = 0.0;

   double totalPower = 0.0;

   totalEnergy = getTotalGroupEnergy(grp);

   //Need to calculate the total execution time for the chip based on cycle count per state
   for(size_t counter = 0; counter < NUM_STATES; counter++)
   {
      time = time + (global_stateCycles[counter] / GProcessor::state_to_freq_dbl(counter));
   }

   totalPower = (totalEnergy * pow(10, -9)) / time;                                         //convert from J to nJ

   return totalPower;
}

double GStatsEnergy::getTotalGroupEnergy(PowerGroup grp)
{
   double total = 0;
   double tempTotal[NUM_STATES] = {0.0};

   if(eGroupStore.size() <= static_cast<size_t>(grp))
      return 0.0;

   for(size_t i = 0; i < eGroupStore[grp].size(); i++)
   {
      double power;
      size_t myState;
      double myFrequency;
      std::string tempName = eGroupStore[grp][i]->name;

      size_t procID;
      size_t location;
      location = tempName.find_first_of("(") + 1;
      procID = atoi(&tempName[location]);


      if(tempName.find("_3_") != std::string::npos)
      {
         std::string freq_str = tempName.substr(tempName.find("_3_"));
         myState = GProcessor::freq_to_state(freq_str);
      }
      else if(tempName.find("_2_") != std::string::npos)
      {
         std::string freq_str = tempName.substr(tempName.find("_2_"));
         myState = GProcessor::freq_to_state(freq_str);
      }
      else if(tempName.find("_1_") != std::string::npos)
      {
         std::string freq_str = tempName.substr(tempName.find("_1_"));
         myState = GProcessor::freq_to_state(freq_str);
      }
      else
      {
         myState = DEFAULT_DVFS;
      }

      tempTotal[myState] = tempTotal[myState] + eGroupStore[grp][i]->getDouble();
   }

   for(size_t counter = 0; counter < NUM_STATES; counter++)
   {
      total = total + tempTotal[counter];
   }

   return total;
}

double GStatsEnergy::getProcClockPower(int procId)
{
   double clockPower = 0.0;
   const char *procName = SescConf->getCharPtr("","cpucore",0);

   double maxClockEnergy = EnergyMgr::get(procName,"clockEnergy", procId);
   double maxEnergy      = EnergyMgr::get(procName,"totEnergy");

   double totalExecTime = GProcessor::totalExecTime();
   double totalGateTime = GProcessor::procGateTime(procId);
   double runRatio;

   if(totalGateTime > 0)
      runRatio = totalGateTime / totalExecTime;
   else
      runRatio = 1.0;

   //Instead of a 50/50 ratio, clock power is based on 50% utilization and 50 * ratio of nonexecut
//    clockPower = (0.5 * (maxClockEnergy/maxEnergy) * getTotalProcPower(procId) + 0.5 * maxClockEnergy);
   clockPower = (0.5 * (maxClockEnergy/maxEnergy) * getTotalProcPower(procId) + 0.5 * runRatio * maxClockEnergy);

   return clockPower;
}


#endif

double GStatsEnergy::getDouble() const
{
#ifndef ACCESS
  return StepEnergy*steps;
#else
  return static_cast<double>(steps) ;
#endif
}

void GStatsEnergy::inc() 
{
  steps++;
}

void GStatsEnergy::add(int v)
{
  I(v >= 0);
  steps += v;
}

/*****************************************************
  *           GStatsEnergyCGBase
  ****************************************************/

GStatsEnergyCGBase::GStatsEnergyCGBase(const char* str,int procId)
  : lastCycleUsed(0) 
  ,numCycles(0)
{
  id = procId;
  name = strdup(str);
}

void GStatsEnergyCGBase::use()
{
  if(lastCycleUsed != globalClock) {
    numCycles++;
    lastCycleUsed = globalClock;
  }
}

/*****************************************************
  *           GStatsEnergyCG
  ****************************************************/
GStatsEnergyCG::GStatsEnergyCG(const char *name, 
			       const char* block, 
			       int procId, 
			       PowerGroup grp, 
			       double energy, 
			       GStatsEnergyCGBase *b)
  : GStatsEnergy(name, block, procId, grp, energy) 
{
  eb = b;
  localE = energy*0.95;
  clockE = energy*0.05;
}

double GStatsEnergyCG::getDouble() const
{
#ifndef ACCESS
  return (steps * localE + eb->getNumCycles()*clockE);
#else
  return static_cast<double>(steps);
#endif
}

void GStatsEnergyCG::inc()
{
  steps++;
  eb->use();
}

void GStatsEnergyCG::add(int v)
{
  I(v >= 0);
  steps += v;
  eb->use();
}

// Energy Store functions
EnergyStore::EnergyStore() 
{
   //Changed to read per-processor energy profiles instead of relying on CPU0
   size_t nProcs = SescConf->getRecordSize("","cpucore");
   proc.resize(nProcs);

   for(size_t counter = 0; counter < nProcs; counter++)
      proc[counter] = SescConf->getCharPtr("","cpucore", counter) ;

   ///DEPRECATED
   //proc = SescConf->getCharPtr("","cpucore", 0);
}

double EnergyStore::get(const char *name, int procId)
{
   //Changed to read per-processor energy profiles instead of relying on CPU0
   return get(proc[procId], name, procId);

   ///DEPRECATED
   //return get(proc, name, procId);
}

double EnergyStore::get(const char *block, const char *name, int procId)
{
  return SescConf->getDouble(block, name);
}

const char* EnergyStore::getStr(PowerGroup p)
{
  switch(p) {
  case FetchPower:
    return "fetchPower";
  case IssuePower:
    return "issuePower";
  case MemPower:
    return "memPower";
  case ExecPower:
    return "execPower";
  case ClockPower:
    return "clockPower";
  case  Not_Valid_Power:
  default:
    I(0);
    return "";
  }

  return 0;
}

const char *EnergyStore::getEnergyStr(PowerGroup p)
{
   switch(p) {
  case FetchPower:
    return "fetchEnergy";
  case IssuePower:
    return "issueEnergy";
  case MemPower:
    return "memEnergy";
  case ExecPower:
    return "execEnergy";
  case ClockPower:
    return "clockEnergy";
  case  Not_Valid_Power:
  default:
    I(0);
    return "";
  }

  return 0;

}
