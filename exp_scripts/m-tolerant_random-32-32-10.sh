#!/bin/bash
source `dirname $0`/util.sh
start_date=`getDate`

scen_start=1
scen_end=100
force=0
map="random-32-32-10.map"
agents_list="30 50 70 90"
time_limit=5000
exec_option="-P PRIMITIVE"
exec_repetation=100
solver="PP -m 100000 -f 4"

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

# send message
MESSAGE="*-----------------------------------
fin experiment\nm-tolerant\ndate=${start_date}\nmap=${map}\nagents=${agents_list}\n${solver}
-----------------------------------*"

bash `dirname $0`/slack_notification.sh "$MESSAGE"
