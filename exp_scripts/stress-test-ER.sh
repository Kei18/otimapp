#!/bin/bash
source `dirname $0`/util.sh
start_date=`getDate`

n=1000
p_list=(0.003 0.004 0.005)
agents_list=$(seq 20 20 200)
eval_num=25
time_limit=300000
force=1

solvers=(
    "PP -m 1"
    "PP -m 100000"
    "DBS"
)

for p in "${p_list[@]}"
do
    for solver in "${solvers[@]}"
    do
        bash `dirname $0`/app_random_graph.sh \
             $n \
             $p \
             "$agents_list" \
             "$solver" \
             $eval_num \
             $time_limit \
             $force
    done
done
