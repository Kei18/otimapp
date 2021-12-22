#!/bin/bash
source `dirname $0`/util.sh
start_date=`getDate`

n=1000
eval_num=100
time_limit=300000
force=1
solvers=(
    "PP -m 1"
    "PP -m 100000"
    "DBS"
)

p=0.003
agents_list="100"
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

p=0.004
agents_list="100"
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

p=0.005
agents_list="100"
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
