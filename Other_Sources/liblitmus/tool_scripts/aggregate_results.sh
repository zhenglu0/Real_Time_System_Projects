#!/bin/bash

#rootDir=/export/austen/home/kieselbachk/core36aug2util05-90
##rootDir=/export/austen/experiment/aug5util20-100n100
#rootDir=/export/austen/experiment/core36aug2util05-90
#?rootDir=/home/jing/cluster/core14aug2avg3
#rootDir=/home/jing/cluster/core14aug2avg2
#rootDir=/home/jing/cluster/core14aug2avg1
#rootDir=/home/jing/cluster/core14aug2avg3speedup
#rootDir=/home/jing/cluster/core14aug2avg2speedup
#rootDir=/home/jing/cluster/core14aug2avg3exp1speedup
#rootDir=/home/jing/cluster/core14aug2avg2exp1speedup
#rootDir=/home/jing/cluster/core14aug2avg1exp1speedup
#rootDir=/home/jing/cluster/core14aug2avg3exp1arbispeedup
#rootDir=/home/jing/cluster/core14aug2avg3hardspeedup
#rootDir=/home/jing/cluster/core7aug2avg2exp1speedup
rootDir=/home/jing/cluster/core7aug2avg3speedup

#?python tmpaggregate_results.py experiment_summary.txt $rootDir/bestaug5util20 $rootDir/bestaug5util30 $rootDir/bestaug5util40 $rootDir/bestaug5util50 $rootDir/bestaug5util60 $rootDir/bestaug5util70 $rootDir/bestaug5util80 
python tmpaggregate_results.py experiment_summary.txt $rootDir/bestaug5util20 $rootDir/bestaug5util30 $rootDir/bestaug5util40 $rootDir/bestaug5util50 $rootDir/bestaug5util55 $rootDir/bestaug5util60 $rootDir/bestaug5util70 $rootDir/bestaug5util80 
#python tmpaggregate_results.py experiment_summary.txt $rootDir/bestaug5util30 $rootDir/bestaug5util40 $rootDir/bestaug5util50 $rootDir/bestaug5util60 $rootDir/bestaug5util70 $rootDir/bestaug5util80 
#python tmpaggregate_results.py experiment_summary.txt $rootDir/bestaug5util40 $rootDir/bestaug5util50 $rootDir/bestaug5util60 $rootDir/bestaug5util70 $rootDir/bestaug5util80 
#python tmpaggregate_results.py experiment_summary.txt $rootDir/bestaug5util60 $rootDir/bestaug5util70 $rootDir/bestaug5util80 
#python tmpaggregate_results.py experiment_summary.txt $rootDir/bestaug5util40  

