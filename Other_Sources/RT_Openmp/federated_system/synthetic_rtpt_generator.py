#!/usr/bin/python

import re
import sys
import glob

def toTimespecStr(timeStr, timeScaleInNSec):
    nsec = int(timeStr) * int(timeScaleInNSec)
    nsecPerSec = 1000000000
    sec = nsec // nsecPerSec
    nsec = nsec % nsecPerSec
    return str(sec) + " " + str(nsec)
    
def main():
    if len(sys.argv) != 7: raise Exception("Wrong number of arguments")
    
    rtptFile = sys.argv[1]
    tasksetDir = sys.argv[2]
    firstCore = sys.argv[3]
    lastCore = sys.argv[4]
    totalTime = sys.argv[5]
    timeScaleInNSec = sys.argv[6]

    with open(rtptFile + ".rtpt", 'w') as output:
        output.write(firstCore + " " + lastCore + "\n")
        
        for taskFile in sorted(glob.glob(tasksetDir + "/task*.txt")):
            with open(taskFile, 'r') as f:
                lines = f.readlines()
            if len(lines) == 0: raise Exception("Bad task file: " + taskFile)
            
            m = re.match(r"^\s*(?P<period>\d+)\s+(?P<numSegments>\d+)\s*$", lines[0])
            if m == None: raise Exception("Bad task file: " + taskFile)
            period, numSegments = m.groups()
            output.write("synthetic_task " + numSegments + " ")
            if (len(lines) != 2*int(numSegments) + 1): raise Exception("Bad task file: " + taskFile)
            
            work = 0
            span = 0
            for i in range(0, int(numSegments)):
                m = re.match(r"^\s*(?P<numStrands>\d+)\s+(?P<segmentLength>\d+)\s+(?P<priority>\d+)\s+(?P<release>(\d+.\d+)|\d+)\s*$", lines[1 + 2*i])
                if m == None: raise Exception("Bad task file: " + taskFile)
                numStrands, segmentLength = m.group(1, 2)
                output.write(numStrands + " " + toTimespecStr(segmentLength, timeScaleInNSec) + " ")
                work += int(numStrands) * int(segmentLength)
                span += int(segmentLength)
                
                m = re.match(r"^\s*(\d+\s*){" + numStrands + "}$", lines[2 + 2*i])
                if m == None: raise Exception("Bad task file: " + taskFile)
                
            output.write("\n")
            output.write(toTimespecStr(work, timeScaleInNSec) + " ")
            output.write(toTimespecStr(span, timeScaleInNSec) + " ")
            output.write(toTimespecStr(period, timeScaleInNSec) + " ")
            output.write(toTimespecStr(period, timeScaleInNSec) + " ")
            output.write("0 0 ")
            numIters = int((float(totalTime) * 1000000) / (int(period) * int(timeScaleInNSec)))
            output.write(str(numIters) + "\n")
                
main()

