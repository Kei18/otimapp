#!/bin/bash
source `dirname $0`/util.sh

force=0
map="random-32-32-10.map"
time_limit=300000
exec_option="-P MAPF_DP -l -u 0.5"
exec_repetation=50
solver="PP -m 100000"

scen_start=1
scen_end=10
agents_list="20 40"
bash `dirname $0`/run.sh \
     $map \
     "$agents_list" \
     "$solver" \
     $scen_start \
     $scen_end \
     $time_limit \
     "$exec_option" \
     $exec_repetation \
     $force

# PP+ fails to solve the first instance with 60 agents
scen_start=3
scen_end=12
agents_list="60"
bash `dirname $0`/run.sh \
     $map \
     "$agents_list" \
     "$solver" \
     $scen_start \
     $scen_end \
     $time_limit \
     "$exec_option" \
     $exec_repetation \
     $force
