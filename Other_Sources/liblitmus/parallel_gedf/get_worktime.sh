#!/bin/bash
rm worktime
make
sudo ./run_simple_taskset.sh > worktime
