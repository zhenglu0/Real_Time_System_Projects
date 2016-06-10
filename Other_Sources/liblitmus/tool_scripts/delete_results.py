#!/usr/bin/python

import os
import sys
import glob

def main():
    if len(sys.argv) < 2: raise Exception("Too few arguments")
    for experimentDir in sys.argv[1:]:
        for tasksetDir in glob.glob(experimentDir + "/taskset*"):
            #LJ#
            #tasksetPath = tasksetDir.split("/")
            #tmp=re.match(r'taskset(?P<id>\d+)$',tasksetPath[-1])
            #if int(tmp.group('id')) < 50:
            #    continue
            for f in glob.glob(tasksetDir + "/result/*"):
                os.remove(f)

main()

