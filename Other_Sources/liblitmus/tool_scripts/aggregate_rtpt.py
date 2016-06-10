#!/usr/bin/python

import os
import sys
import glob
import shutil

def main():
    if len(sys.argv) < 3: raise Exception("Too few arguments")
    rtptDir = sys.argv[1]
    for experimentDir in sys.argv[2:]:
        experimentPath = experimentDir.split("/") 
        for tasksetDir in sorted(glob.glob(experimentDir + "/taskset*")):
            tasksetPath = tasksetDir.split("/")
            newName = rtptDir + "/" + experimentPath[-1] + "_" + tasksetPath[-1] + ".rtpt"
            oldNameList = glob.glob(tasksetDir + "/result/synthetic_taskset.rtpt")
            oldName = oldNameList[0]
            shutil.copyfile(oldName, newName)
main()

