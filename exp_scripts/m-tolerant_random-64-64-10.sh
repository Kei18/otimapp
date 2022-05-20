#!/bin/bash
source `dirname $0`/util.sh
start_date=`getDate`

scen_start=1
scen_end=25
force=0
map="random-64-64-10.map"
agents_list=$(seq 20 20 200)
time_limit=300000
exec_option="-P PRIMITIVE -l"
exec_repetation=100

solvers=(
    "PP -m 100000"
    "PP -m 100000 -f 2"
    "PP -m 100000 -f 4"
    "PP -m 100000 -f 6"
    "PP -m 100000 -f 8"
    "PP -m 100000 -f 10"
    "DBS"
    "DBS -f 2"
    "DBS -f 4"
    "DBS -f 6"
    "DBS -f 8"
    "DBS -f 10"
)

for solver in "${solvers[@]}"
do
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
done
