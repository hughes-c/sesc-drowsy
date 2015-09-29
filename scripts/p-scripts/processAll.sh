#!/bin/bash

currentDirectory=`pwd`

for i in `ls -1`
do 
  cd $i/results
  processResults
  cd $currentDirectory
done