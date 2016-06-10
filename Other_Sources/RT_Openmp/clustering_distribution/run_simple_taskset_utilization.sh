#!/bin/bash

program_name=simple_task_utilization
first_core=0
last_core=3
bucket_width_sec=0
bucket_width_ns=10000
num_repetitions=1000
args="1000 1000"

./$program_name $first_core $last_core $bucket_width_sec $bucket_width_ns $num_repetitions $args 