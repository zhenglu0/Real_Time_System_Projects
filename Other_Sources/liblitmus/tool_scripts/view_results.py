#!/usr/bin/python

import os
import sys
import glob

def main():
    if len(sys.argv) < 2: raise Exception("Too few arguments")
    for experimentDir in sys.argv[1:]:
        experimentPath = experimentDir.split("/")
        print experimentPath[-1] + "\n"
        for tasksetDir in sorted(glob.glob(experimentDir + "/taskset*")):
            tasksetPath = tasksetDir.split("/")
            print "\t" + tasksetPath[-1] + ": "
            for fname in sorted(glob.glob(tasksetDir + "/result/*")):
                taskResultPath = fname.split("/")
                print "\t\t" + taskResultPath[-1] + ": ",
                with open(fname) as f:
                    try:
                        print f.readline()
                    except:
                        pass
main()

