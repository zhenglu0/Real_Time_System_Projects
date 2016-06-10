#!/usr/bin/python

import re
import sys
import glob

def main():
    if len(sys.argv) < 3: raise Exception("Too few arguments")

    filenameArg = sys.argv[1].rpartition(".")
    if filenameArg[1] == ".":
        outputFilename = (filenameArg[0], "." + filenameArg[2])
    else:
        outputFilename = (filenameArg[2], ".txt")
    
    with open(outputFilename[0] + outputFilename[1], 'w') as output:    
        with open(outputFilename[0] + "_verbose" + outputFilename[1], 'w') as verboseOutput:
            for experimentDir in sys.argv[2:]:
                experimentPath = experimentDir.split("/")
                output.write(experimentPath[-1] + "\n")
                verboseOutput.write(experimentPath[-1] + "\n")
            
                numTotalTasksets = 0
                numTotalFailures = 0
                numTotalPeriods = 0
                numTotalMissedDeadlines = 0
                
                realnumSchedulableFailures = 0
                realnumTotalFailures = 0
                realnumNotGuaranteedFailures = 0
                
                numSchedulableTasksets = 0
                numSchedulableFailures = 0
                numSchedulablePeriods = 0
                numSchedulableMissedDeadlines = 0
                
                numNotGuaranteedTasksets = 0
                numNotGuaranteedFailures = 0
                numNotGuaranteedPeriods = 0
                numNotGuaranteedMissedDeadlines = 0
                
                numUnschedulableTasksets = 0
                
                for tasksetDir in sorted(glob.glob(experimentDir + "/taskset*")):
                    tasksetPath = tasksetDir.split("/")
                    #LJ#
                    tmp=re.match(r'taskset(?P<id>\d+)$',tasksetPath[-1])
                    #print tasksetPath[-1]
                    #if int(tmp.group('id')) <= 25 or int(tmp.group('id')) >= 96:
                    #    continue
                    verboseOutput.write("\t" + tasksetPath[-1] + ": ")
                
                    schedulabilityFile = tasksetDir + "/result/schedulability.txt"
                    try:
                    	with open(schedulabilityFile, 'r') as f:
                            m = re.match(r"^\s*(?P<schedulable>\d)\s*$", f.readline())
		#LJ#
                    except:
			continue

                    #if m == None: raise Exception("Bad file: " + schedulabilityFile)
		#LJ#
                    if m == None: continue
                    schedulable = int(m.group(1))
                    succeeded = True
                    realsucceeded = 0
                    
                    if schedulable == 0:
                        verboseOutput.write("schedulable\n")
                    else:
                        verboseOutput.write("not schedulable\n")
                                
                    for taskResultFile in sorted(glob.glob(tasksetDir + "/result/task*_result.txt")):
                        taskResultPath = taskResultFile.split("/")
                        verboseOutput.write("\t\t" + taskResultPath[-1] + ": ")
                    
                        with open(taskResultFile, 'r') as f:
                            line = f.readline()
                #LJ#
                            #multiline = f.readlines()
                            #if len(multiline) == 3:
                            #    print multiline[0]
                            #    line = multiline[2]
                            if line == "Program affinity:\n":
                                line = f.readline()
                                line = f.readline()

                        m = re.match(r"^\s*(?P<numMissed>\d+)\s+(?P<numPossible>\d+)\s+(?P<numCores>\d+)\s*(?P<maxRuntime>\d+.\d+)?\s*$", line)
                        #if m == None: raise Exception("Bad file: " + taskResultFile)
		#LJ#
                        if m == None: continue

                        numMissed = int(m.group(1))
                        numPossible = int(m.group(2))
                        
                        verboseOutput.write(line[0:-1])
                        if numMissed > 0 and schedulable == 0:
                            verboseOutput.write(" *****")
                            print '*',
                        elif numMissed == 1 and schedulable != 0 and numPossible != 0:
                            verboseOutput.write(" +++++")
                        elif numMissed == 2 and schedulable != 0 and numPossible != 0:
                            verboseOutput.write(" +++++")
                        elif numMissed > 2 and schedulable != 0:
                            verboseOutput.write(" ?????")
                        verboseOutput.write("\n")
                        
                        if numMissed > 0 or numPossible == 0: succeeded = False
                        numTotalPeriods += numPossible
                        numTotalMissedDeadlines += numMissed
                        if numMissed > 2 and numPossible != 0: realsucceeded = 3
                        elif realsucceeded != 3 and (numMissed == 1 or numMissed == 2) and numPossible != 0: realsucceeded = 2
                        
                        if schedulable == 0:
                            numSchedulablePeriods += numPossible
                            numSchedulableMissedDeadlines += numMissed
                        elif schedulable == 1:
                            numNotGuaranteedPeriods += numPossible
                            numNotGuaranteedMissedDeadlines += numMissed
                            
                    numTotalTasksets += 1
                    if not succeeded: numTotalFailures += 1
                    if realsucceeded == 2: realnumTotalFailures += 1
                    
                    if schedulable == 0:
                        numSchedulableTasksets += 1
                        if not succeeded: numSchedulableFailures += 1
                        if realsucceeded == 2: realnumSchedulableFailures += 1
                    elif schedulable == 1:
                        numNotGuaranteedTasksets += 1
                        if not succeeded: numNotGuaranteedFailures += 1
                        if realsucceeded == 2: realnumNotGuaranteedFailures += 1
                    else:
                        numUnschedulableTasksets += 1
                
                output.write("\tnumTotalTasksets: " + str(numTotalTasksets) + "\n")
                output.write("\tnumTotalFailures: " + str(numTotalFailures)+" - "+str(realnumTotalFailures)+ "\n")
                output.write("\tnumTotalPeriods: " + str(numTotalPeriods) + "\n")
                output.write("\tnumTotalMissedDeadlines: " + str(numTotalMissedDeadlines) + "\n\n")
                
                output.write("\tnumSchedulableTasksets: " + str(numSchedulableTasksets) + "\n")
                output.write("\tnumSchedulableFailures: " + str(numSchedulableFailures) +" - "+str(realnumSchedulableFailures)+ "\n")
                output.write("\tnumSchedulablePeriods: " + str(numSchedulablePeriods) + "\n")
                output.write("\tnumSchedulableMissedDeadlines: " + str(numSchedulableMissedDeadlines) + "\n\n")
                
                output.write("\tnumNotGuaranteedTasksets: " + str(numNotGuaranteedTasksets) + "\n")
                output.write("\tnumNotGuaranteedFailures: " + str(numNotGuaranteedFailures) +" - "+str(realnumNotGuaranteedFailures)+ "\n")
                output.write("\tnumNotGuaranteedPeriods: " + str(numNotGuaranteedPeriods) + "\n")
                output.write("\tnumNotGuaranteedMissedDeadlines: " + str(numNotGuaranteedMissedDeadlines) + "\n\n")
                output.write("\tnumUnschedulableTasksets: " + str(numUnschedulableTasksets) + "\n\n")
                                
main()

