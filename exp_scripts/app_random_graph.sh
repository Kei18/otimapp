#!/bin/bash
source `dirname $0`/util.sh

## args
vertex_num=$1
prob=$2
agents_list=$3
solver=$4
eval_num=$5
time_limit=$6
force=$7

PROJECT_DIR=`dirname $0`/..
map_trimed=${map%.map}

## check git status
if [ ${force} -ne 1 ]
then
   GIT_STATUS_RESULT=`git status -s`
   if [ ${#GIT_STATUS_RESULT} -ne 0 ]
   then
       echo "Untracked changes exist. Commit them beforehand."
       git status -s
       exit 1
   fi
fi
GIT_RECENT_COMMIT=`git log -1 --pretty=format:"%H"`

## print experimental info
echo "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*"
echo "start experiment"
echo "- vertex: ${vertex_num}"
echo "- prob: ${prob}"
echo "- agents: ${agents_list}"
echo "- solver: ${solver}"
echo "- eval_num: ${eval_num}"

## create output directory
EXP_DATE=`getDate`
OUTPUT_DIR=$PROJECT_DIR/../data/$EXP_DATE/
mkdir -p $OUTPUT_DIR

## build
(cd $PROJECT_DIR/build;
 make > /dev/null 2>&1;)

## main
for agent_num in ${agents_list[@]}
do
    echo "\nagents=${agent_num}"
    scen=1
    while [ $scen -lt `expr $eval_num + 1` ]
    do
        # plan
        plan_file="G-${vertex_num}-${prob}_${agent_num}agents_${scen}_plan.txt"
        cmd="$PROJECT_DIR/build/app_random_graph \
                    -n $vertex_num \
                    -p $prob \
                    -k $agent_num \
                    -r $scen \
                    -o $OUTPUT_DIR/$plan_file \
                    -s $solver \
                    -T $time_limit"
        eval $cmd
        scen=`expr $scen + 1`
    done
done

## create status file
STATUS_FILE=$OUTPUT_DIR/status.txt
{
    echo start:$EXP_DATE
    echo end:`date +%Y-%m-%d-%H-%M-%S`
    echo used-commit:$GIT_RECENT_COMMIT
    echo G:$vertex_num,$prob
    echo agents:$agents_list
    echo solver:$solver
    echo eval_num:$eval_num
} > $STATUS_FILE

echo "\nfinish experiment"
echo "result -> ${OUTPUT_DIR}"
echo "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"
