#!/bin/bash

FREQ=$1

sed -i 's/Energy=/Energy_'${FREQ}'=/g' power_list
sed -i 's/Energy\ =/Energy_'${FREQ}'\ =/g' power_list

echo "" >> power.states
echo "" >> power.states

cat power_list >> power.states

