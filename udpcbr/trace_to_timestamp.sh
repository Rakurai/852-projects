#!/bin/bash

trace=$1
tmp_out=$trace.tmp
ts_out=$trace.ts.tmp
order_out=$trace.order.tmp
trace_out=$trace.txt

sudo tcpdump -X -r $trace > $tmp_out
cat $tmp_out |grep client|tr '.' ' '|awk '{print $2}' > $ts_out
cat $tmp_out |grep 0x0020:|tr 'a' 'A'|tr 'b' 'B'|tr 'c' 'C'|tr 'd' 'D'|tr 'e' 'E'|tr 'f' 'F'|awk '{printf "ibase=16; %s\n",$5}'|bc 2>/dev/null 1> $order_out
paste $ts_out $order_out > $trace_out

rm $tmp_out $ts_out $order_out
