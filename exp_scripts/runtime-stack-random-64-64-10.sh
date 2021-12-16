!/bin/bash
source `dirname $0`/util.sh
start_date=`getDate`

scen_start=1
scen_end=100
force=0
map="random-64-64-10.map"
agents=120
time_limit=300000
exec_option="_"
exec_repetation=0

solvers=(
    "PP -m 1"
    "PP -m 100000"
    "DBS"
)

for solver in "${solvers[@]}"
do
    bash `dirname $0`/run.sh \
       $map \
       "$agents" \
       "$solver" \
       $scen_start \
       $scen_end \
       $time_limit \
       $exec_option \
       $exec_repetation \
       $force
done
