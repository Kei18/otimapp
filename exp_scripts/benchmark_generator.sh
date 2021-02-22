#!/bin/sh
BENCHMARK_DIR=../instances/
PARAM_FILE=tmp.txt

mkdir -p $BENCHMARK_DIR

if [ $# != 4 ]
then
    echo "bash ./benchmark_generator.sh {map} {agent_num} {instance_num} {max_comp_time}"
    exit 1
fi

map=$1
map_trimed=${map%.map}
agent_num=$2
instance_num=$3
max_comp_time=$4

seed=0
while [ $seed -lt ${instance_num} ]
do
    touch $PARAM_FILE
    {
        echo map_file=$map
        echo agents=$agent_num
        echo seed=$seed
        echo random_problem=1
        echo goal_avoidance=1
        echo max_comp_time=$max_comp_time
    } > $PARAM_FILE
    filename="${map_trimed}_${agent_num}agents_`expr $seed + 1`.txt"
    ../build/app -P -i $PARAM_FILE -o $BENCHMARK_DIR/$filename
    echo create instance $BENCHMARK_DIR$filename
    seed=`expr $seed + 1`
done

rm $PARAM_FILE
