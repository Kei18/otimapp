#!/bin/bash
source `dirname $0`/util.sh
start_date=`getDate`

scen_start=1
scen_end=5
force=0
map="random-32-32-10.map"
agents_list="35"
time_limit=10000
exec_option="-P MAPF_DP -l"
exec_repetation=100
solver="PP -m 100000 -f 8"

delay_probs=(
    "0.2"
    "0.5"
    "0.8"
)

for dp in "${delay_probs[@]}"
do
    option="${exec_option} -u ${dp}"
    bash `dirname $0`/run.sh \
         $map \
         "$agents_list" \
         "$solver" \
         $scen_start \
         $scen_end \
         $time_limit \
         "$option" \
         $exec_repetation \
         $force
done

MESSAGE="*-----------------------------------
fin experiment\nm-tolerant\ndate=${start_date}\nmap=${map}\nagents=${agents_list}\n\nsolver=${solver}
-----------------------------------*"

bash `dirname $0`/slack_notification.sh "$MESSAGE"
