#!/bin/bash
source `dirname $0`/util.sh

## args
map=$1
agents_list=$2
solver=$3
scen_start=$4
scen_end=$5
time_limit=$6
exec_option=$7
exec_repetation=$8
force=$9

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
echo "- map: ${map}"
echo "- agents: ${agents_list}"
echo "- solver: ${solver}"
echo "- scen: from ${scen_start} to ${scen_end}"

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
    scen=$scen_start
    while [ $scen -lt `expr $scen_end + 1` ]
    do
        scen_file="${map_trimed}_${agent_num}agents_${scen}.txt"

        # plan
        plan_file="${map_trimed}_${agent_num}agents_${scen}_plan.txt"
        cmd="$PROJECT_DIR/build/app \
                    -i $PROJECT_DIR/instances/$scen_file \
                    -o $OUTPUT_DIR/$plan_file \
                    -s $solver \
                    -T $time_limit"
        eval $cmd

        # execution
        exec_seed=0
        while [ $exec_seed -lt `expr $exec_repetation` ]
        do
            exec_file="${map_trimed}_${agent_num}agents_${scen}_exec_${exec_seed}.txt"
            cmd_exec="$PROJECT_DIR/build/exec \
                          -i $PROJECT_DIR/instances/$scen_file \
                          -p $OUTPUT_DIR/$plan_file \
                          -o $OUTPUT_DIR/$exec_file \
                          -s ${exec_seed} \
                          ${exec_option}"
            eval $cmd_exec
            exec_seed=`expr $exec_seed + 1`
        done
        scen=`expr $scen + 1`
    done
done

## create status file
STATUS_FILE=$OUTPUT_DIR/status.txt
{
    echo start:$EXP_DATE
    echo end:`date +%Y-%m-%d-%H-%M-%S`
    echo used-commit:$GIT_RECENT_COMMIT
    echo map:$map
    echo agents:$agents_list
    echo solver:$solver
    echo scen_start:$scen_start
    echo scen_end:$scen_end
    echo exec_option:$exec_option
    echo exec_reperation:$exec_repetation
} > $STATUS_FILE

echo "\nfinish experiment"
echo "result -> ${OUTPUT_DIR}"
echo "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"
