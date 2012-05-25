#!/bin/bash

# AWK_DIR="/home/cmhug/bin/powerScripts/awk/"
#AWK_DIR="/home/hughes/IDEAL/powerScripts/awk/"
#AWK_DIR="/home/hughes/IDEAL/bin/powerScripts/awk/"


P_FIN="power_final.txt"
P_MIN="power_min_out.txt"
P_LK="power_out.lk.txt"
P_TM="power_out.tm.txt"

LK_TMP="lk.tmp"
TM_TMP="tm.tmp"

# Variables to calculate cuts
END=0
START=0
TOTAL=0

# Default SESC output file
NAME=""

RPT_NAME="${1}"
RPT_NAME=`pwd | awk '/sesc/ { split($1, boo, "/"); sub("sesc.power.dvfs", "", boo[5]); sub("-", "", boo[5]); print boo[5] }'`
echo ${RPT_NAME}

##LOCKS
echo "Name, Power, Energy, Time, EDP" > EDP.TM.${RPT_NAME}.csv
for x in `ls -l | grep -v "tmDebug" | grep -v "therm" | grep -v "power" | grep -v ".pwr."`
do
   NAME=`echo $x | grep "LK-"`
   if [ "${NAME}" != "" ]
   then
      tmpFile=`echo $NAME | sed "s/^\([^.]*\)[.].*/\1/"`
      echo -n $NAME > file.tmp
      echo -n "," >> file.tmp

#       cat $NAME | awk 'BEGIN { execTime = 0; totalEnergy = 0; totalPower = 0; EDP = 0 } /OSSim:totalExecTime/ { split($1, time, "=") } /PowerMgr:totPower/ { split($1, power, "=") }  /EnergyMgr:totEnergy/ { split($1, energy, "=") } END { execTime = time[2]; totalPower = power[2]; totalEnergy = energy[2]; EDP = totalPower * execTime^2; printf("%e\n", EDP)  }' >> file.tmp

#       cat $NAME | awk 'BEGIN { execTime = 0; totalEnergy = 0; totalPower = 0; EDP = 0 } /OSSim:totalExecTime/ { split($1, time, "=") } /PowerMgr:totPower/ { split($1, power, "=") }  /EnergyMgr:totEnergy/ { split($1, energy, "=") } END { execTime = time[2]; totalPower = power[2]; totalEnergy = energy[2]; EDP = totalEnergy * execTime^2; printf("%e\n", EDP); printf("%e -- %e\n", totalEnergy, execTime)  }' >> file.tmp

      cat $NAME | awk 'BEGIN { execTime = 0; totalEnergy = 0; totalPower = 0; EDP = 0 } /OSSim:totalExecTime/ { split($1, time, "=") } /PowerMgr:totPower/ { split($1, power, "=") }  /EnergyMgr:totEnergy/ { split($1, energy, "=") } END { execTime = time[2]; totalPower = power[2]; totalEnergy = energy[2]; EDP = totalEnergy * execTime^2; printf("%e,%e,%e,%e\n", totalPower, totalEnergy, execTime, EDP) }' >> file.tmp

      cat file.tmp >> EDP.LK.${RPT_NAME}.csv
      rm  file.tmp

   fi
done

##TM
echo "Name, Power, Energy, Time, EDP" > EDP.TM.${RPT_NAME}.csv
for x in `ls -l | grep -v "tmDebug" | grep -v "therm" | grep -v "power" | grep -v ".pwr."`
do
   NAME=`echo $x | grep "TM-"`
   if [ "${NAME}" != "" ]
   then
      tmpFile=`echo $NAME | sed "s/^\([^.]*\)[.].*/\1/"`
      echo -n $NAME > file.tmp
      echo -n "," >> file.tmp

#       cat $NAME | awk 'BEGIN { execTime = 0; totalEnergy = 0; totalPower = 0; EDP = 0 } /OSSim:totalExecTime/ { split($1, time, "=") } /PowerMgr:totPower/ { split($1, power, "=") }  /EnergyMgr:totEnergy/ { split($1, energy, "=") } END { execTime = time[2]; totalPower = power[2]; totalEnergy = energy[2]; EDP = totalPower * execTime^2; printf("%e\n", EDP)  }' >> file.tmp

#       cat $NAME | awk 'BEGIN { execTime = 0; totalEnergy = 0; totalPower = 0; EDP = 0 } /OSSim:totalExecTime/ { split($1, time, "=") } /PowerMgr:totPower/ { split($1, power, "=") }  /EnergyMgr:totEnergy/ { split($1, energy, "=") } END { execTime = time[2]; totalPower = power[2]; totalEnergy = energy[2]; EDP = totalEnergy * execTime^2; printf("%e\n", EDP) }' >> file.tmp

      cat $NAME | awk 'BEGIN { execTime = 0; totalEnergy = 0; totalPower = 0; EDP = 0 } /OSSim:totalExecTime/ { split($1, time, "=") } /PowerMgr:totPower/ { split($1, power, "=") }  /EnergyMgr:totEnergy/ { split($1, energy, "=") } END { execTime = time[2]; totalPower = power[2]; totalEnergy = energy[2]; EDP = totalEnergy * execTime^2; printf("%e,%e,%e,%e\n", totalPower, totalEnergy, execTime, EDP) }' >> file.tmp

      cat file.tmp >> EDP.TM.${RPT_NAME}.csv
      rm  file.tmp

   fi
done
