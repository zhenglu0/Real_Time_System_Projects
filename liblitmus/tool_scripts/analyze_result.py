#!/usr/bin/python

import sys
import re
import glob
import copy
import math

from cluster import *

#./analyze_result.py ../result/exp_results/experiment_summary_1024_no_balance_verbose.txt
def parse_result():
	if len(sys.argv) < 1:
		print "./analyze_result.py result_file"
		raise Exception("Too few arguments")

	resultdir = sys.argv[1]
	resultfile = open(resultdir, 'r')
	result = resultfile.readlines()

	#[[analysis], [realresult]]
	alltask = [[],[],[]]
	for i in range(0, 7):
		alltask[0].append([])
		alltask[1].append([])
		alltask[2].append([])
		for j in range(0, 101):
			alltask[0][i].append(0)
			alltask[1][i].append(0)
			alltask[2][i].append([])
	#print len(alltask)

	i = 0
	utilnum = 0
	tasknum = 1
	highmiss = []
	while i < len(result):
		eachline = result[i]
		#print eachline

		util = re.match(r'^bestaug5util(?P<util>\d)0\s+$', eachline)
		if util:
			utilnum = int(util.group('util')) - 2
			#print utilnum
			i += 1
			continue

		task = re.match(r'^\s+taskset(?P<tasknum>\d+): (?P<schedulable>\w+)\s+.*$', eachline)
		if task:
			#print task.group('tasknum'), task.group('schedulable')
			tasknum = int(task.group('tasknum'))
			#0: Guaranteed schedulable.
			#1: Not guaranteed schedulable, partition available, may try.
			#2: Not schedulable, no partition available.
			if task.group('schedulable') == 'not':
				if result[i+1] == '		task0_result.txt: 1 0 0\n':
					#print utilnum,tasknum
					alltask[0][utilnum][tasknum] = 2
					alltask[1][utilnum][tasknum] = 2
					i += 1
				else:
					#print utilnum, tasknum
					alltask[0][utilnum][tasknum] = 1
		real = re.match(r'^\s+task\d+_result.txt: (?P<miss>\d+) \d+ (?P<high>\d+) .*$', eachline)
		if real:
			realnum = int(real.group('miss'))
			if realnum > 0:
				alltask[1][utilnum][tasknum] = 1
				alltask[2][utilnum][tasknum].append(eachline)
				#print eachline	
				if int(real.group('high')) > 1:
				#if eachline[-2] == '*':
				#	print utilnum, tasknum, eachline
					highmiss.append([utilnum, tasknum])
		i += 1
	return alltask, highmiss

#rtptdir = '../rtpt_files/'
rtptdir = '../new_36/'
core36 = [100, 83, 58, 51, 37, 26, 0]
def analyze(alltask, highmiss):
	utilnum = 0
	tasknum = 1

	count = 0
	countthr1 = 0
	countthr2 = 0
	exp = []
	balance = []
	threshold = []
	thranalysis = []
	analysis = []
	over = []
	overanalysis = []
	canfit = []
	notfit = []
	taskaveutil = []
	taskavecore = []
	taskavenum = []
	taskavenum2 = []
	taskcanfit = []
	tasknotfit = []
	numcanfit = []
	numnotfit = []
	utilcanfit = []
	utilnotfit = []
	corecanfit = []
	corenotfit = []

	for i in range(0, 7):
		exp.append(0)
		balance.append(0)
		threshold.append(0)
		thranalysis.append(0)
		analysis.append(0)
		over.append(0)
		overanalysis.append(0)
		canfit.append(0)
		notfit.append(0)
		taskaveutil.append(0)
		taskavecore.append(0)
		taskavenum.append(0)
		taskavenum2.append(0)
		taskcanfit.append(0)
		tasknotfit.append(0)
		numcanfit.append(0)
		numnotfit.append(0)
		utilcanfit.append(0)
		utilnotfit.append(0)
		corecanfit.append(0)
		corenotfit.append(0)

	for tasksetdir in glob.glob(rtptdir+'bestaug5util*'):
		#print tasksetdir
		taskset = re.match(r'^\.\./rtpt_files/bestaug5util(?P<util>\d+)0_taskset(?P<taskset>\d+)\.rtpt$', tasksetdir)
		taskset = re.match(r'^\.\./new_36/bestaug5util(?P<util>\d+)0_taskset(?P<taskset>\d+)\.rtpt$', tasksetdir)

		if not taskset:
			continue

		utilnum = int(taskset.group('util')) - 2
		tasknum = int(taskset.group('taskset'))

		(info, corenum, rawinfo) = readinput(tasksetdir[0:-5])
		prognum = len(info)
		info2 = copy.deepcopy(info)
		info1 = copy.deepcopy(info)
		info3 = copy.deepcopy(info)
		(sched, corestr, outinfo) = cluster_partition(info, prognum, corenum, 2)
		(sched2, corestr2, outinfo2) = cluster_partition(info2, prognum, corenum, 3)
		#(sched3, corestr3, outinfo3) = cluster_partition(info3, prognum, corenum, 7)
		(sched3, corestr3, outinfo3) = cluster_partition(info3, prognum, corenum, 6)
		taskavenum[utilnum]+=len(info3)
		taskavenum2[utilnum]+=len(outinfo3)
		#print outinfo3
		flag=0
		for each in outinfo3:
			if sched3 > 0:
				break
			assign = 1.0*(each[3]-each[4])/(each[1]-each[4])
			ceil = each[7]-each[6]+1
			taskavecore[utilnum]+=ceil
			#if ceil-assign <= 0.05*(2+math.log(ceil,2)):
			if ceil-assign <= 0.02*(5+ceil):
				#print utilnum,tasknum,ceil, assign
				#print each
				if alltask[2][utilnum][tasknum] != []:
					flag = 1
					print utilnum,tasknum,ceil, assign
				#	print each
				#	print alltask[2][utilnum][tasknum] 
					countthr1+=1
				else:
					countthr2+=1
				break
		if flag==0 and sched3==0 and alltask[2][utilnum][tasknum] != []:
			#print alltask[2][utilnum][tasknum] 
			for each in outinfo3:
				assign = 1.0*(each[3]-each[4])/(each[1]-each[4])
				ceil = each[7]-each[6]+1
			#	print utilnum,tasknum,ceil, assign,ceil-assign,0.05*(2+math.log(ceil,2)),0.0125*(8+ceil)

		misscore = []
		if alltask[2][utilnum][tasknum] != []:
			#print utilnum, tasknum,alltask[2][utilnum][tasknum]
			for each in alltask[2][utilnum][tasknum]:
				real = re.match(r'^\s+task\d+_result.txt: (?P<miss>\d+) (?P<period>\d+) (?P<high>\d+) .*$', each)
				if real:
					misscore.append([int(real.group('high')),int(real.group('period'))])
		for each in outinfo3:
			canfitflag = 0
			if each[7]>11 and each[6]<=11:
				numnotfit[utilnum] += 1
				utilnotfit[utilnum] += each[2]
				corenotfit[utilnum] += each[7]-each[6]+1
			elif each[7]>23 and each[6]<=23:
				numnotfit[utilnum] += 1
				utilnotfit[utilnum] += each[2]
				corenotfit[utilnum] += each[7]-each[6]+1
			else:
			#elif each[7]-each[6]+1 > 5:
				numcanfit[utilnum] += 1
				canfitflag = 1
				utilcanfit[utilnum] += each[2]
				corecanfit[utilnum] += each[7]-each[6]+1
			#else:
			#	canfitflag = 1
			if misscore != []:
				for eachmiss in misscore:
					if eachmiss[0] == each[7]-each[6]+1 and eachmiss[1]==327680000000/each[1]:
					#if eachmiss[0] == each[7]-each[6]+1 and eachmiss[1]==163840000000/each[1]:
						if canfitflag == 0:
							tasknotfit[utilnum] += 1
						else:
						#elif each[7]-each[6]+1 > 5:
							taskcanfit[utilnum] += 1
						#print '\t',each, misscore
						break

		for each in info3:
			taskaveutil[utilnum]+=each[2]

		if sched2 > 0:
			thranalysis[utilnum] += 0.01
		if sched3 > 0:
			overanalysis[utilnum] += 0.01
		if alltask[1][utilnum][tasknum] == 1 or alltask[1][utilnum][tasknum] == 2:
			if core36[utilnum] >= tasknum:
				canfit[utilnum] += 0.01
			else:
				notfit[utilnum] += 0.01

		if sched2 != 0:
		#if sched2 > 0:
			(sched1, corestr1, outinfo1) = cluster_partition(info1, prognum, corenum, 4)
			if sched1 == 0 or (sched2 == 2 and sched1 == 1):
			#if sched1 <= 0 or (sched2 == 2 and sched1 < 2):
				sched2 = sched1
				corestr2 = corestr1
				outinfo2 = outinfo1
			elif sched == 0 or (sched2 == 2 and sched == 1):
			#elif sched <= 0 or (sched2 == 2 and sched < 2):
				sched2 = sched
				corestr2 = corestr
				outinfo2 = outinfo
		#if sched2 == -1:
		#	print utilnum, tasknum

		if alltask[0][utilnum][tasknum] != 0:
			analysis[utilnum] += 0.01

		#need to change cluster.py #run_cluster()
		#need to add the core[1] > 0.9: return -1, corestr, outinfo
		if alltask[1][utilnum][tasknum] == 1:
			#print utilnum, tasknum
			exp[utilnum] += 0.01

			utilmiss = -1
			if sched != 0:
				balance[utilnum] += 0.01
			else:
				for eachmiss in highmiss:
					if eachmiss[0] == utilnum and eachmiss[1] == tasknum:
						utilmiss = utilnum
						balance[utilnum] += 0.01
						break

			#if sched2 != 0:
			if sched2 == -1 or sched2 == 2:
				threshold[utilnum] += 0.01
			elif utilmiss == -1:
				for eachmiss in highmiss:
					if eachmiss[0] == utilnum and eachmiss[1] == tasknum:
						utilmiss = utilnum
						threshold[utilnum] += 0.01
						break
			else:
				threshold[utilmiss] += 0.01
			if sched3 == 0:
				over[utilnum] += 0.01
		else:
			if sched2 > 0:
				threshold[utilnum] += 0.01

		if alltask[1][utilnum][tasknum] == 2:
			#over[utilnum] += 0.01
			exp[utilnum] += 0.01

		#if sched == 0 and alltask[0][utilnum][tasknum] != 0:
		#	print sched, alltask[0][utilnum][tasknum]

	output = 'experiment = ['
	for each in exp:
		output += str(round(each, 2)) + ' '
	output += '];'
	print output
	output = 'balance = ['
	for each in balance:
		output += str(round(each, 2)) + ' '
	output += '];'
	print output
	output = 'threshold = ['
	for each in threshold:
		output += str(round(each, 2)) + ' '
	output += '];'
	print output
	output = 'thranalysis = ['
	for each in thranalysis:
		output += str(round(each, 2)) + ' '
	output += '];'
	print output
	output = 'analysis = ['
	for each in analysis:
		output += str(round(each, 2)) + ' '
	output += '];'
	print output
	output = 'over = ['
	for each in over:
		output += str(round(each, 2)) + ' '
	output += '];'
	print output
	output = 'overanalysis = ['
	for each in overanalysis:
		output += str(round(each, 2)) + ' '
	output += '];'
	print output
	output = 'canfit = ['
	for each in canfit:
		output += str(round(each, 2)) + ' '
		tmp = canfit.index(each)
		tmp = core36[tmp]
		if tmp == 0:
			tmp = 1
		output += str(round(100.0*each/tmp, 4)) + ' '
	output += '];'
	print output
	output = 'notfit = ['
	for each in notfit:
		output += str(round(each, 2)) + ' '
		tmp = notfit.index(each)
		tmp = 100-core36[tmp]
		if tmp == 0:
			tmp = 1
		output += str(round(100.0*each/tmp, 4)) + ' '
	output += '];'
	print output
	output = 'numcanfit = ['
	for each in numcanfit:
		output += str(round(each, 2)) + ' '
	output += '];'
	print output
	output = 'numnotfit = ['
	for each in numnotfit:
		output += str(round(each, 2)) + ' '
	output += '];'
	print output
	output = 'utilcanfit = ['
	for each in utilcanfit:
		tmp = utilcanfit.index(each)
		tmp = numcanfit[tmp]
		if tmp == 0:
			tmp = 1
		output += str(round(each/tmp, 4)) + ' '
	output += '];'
	print output
	output = 'utilnotfit = ['
	for each in utilnotfit:
		tmp = utilnotfit.index(each)
		tmp = numnotfit[tmp]
		if tmp == 0:
			tmp = 1
		output += str(round(each/tmp, 4)) + ' '
	output += '];'
	print output
	output = 'corecanfit = ['
	for each in corecanfit:
		tmp = corecanfit.index(each)
		tmp = numcanfit[tmp]
		if tmp == 0:
			tmp = 1
		output += str(round(1.0*each/tmp, 4)) + ' '
	output += '];'
	print output
	output = 'corenotfit = ['
	for each in corenotfit:
		tmp = corenotfit.index(each)
		tmp = numnotfit[tmp]
		if tmp == 0:
			tmp = 1
		output += str(round(1.0*each/tmp, 4)) + ' '
	output += '];'
	print output
	output = 'taskcanfit = ['
	for each in taskcanfit:
		output += str(round(each, 2)) + ' '
	output += '];'
	print output
	output = 'tasknotfit = ['
	for each in tasknotfit:
		output += str(round(each, 2)) + ' '
	output += '];'
	print output
	output = 'ratiocanfit = ['
	for each in taskcanfit:
		tmp = taskcanfit.index(each)
		tmp = numcanfit[tmp]
		if tmp == 0:
			tmp = 1
		output += str(round(100.0*each/tmp, 4)) + ' '
	output += '];'
	print output
	output = 'rationotfit = ['
	for each in tasknotfit:
		tmp = tasknotfit.index(each)
		tmp = numnotfit[tmp]
		if tmp == 0:
			tmp = 1
		output += str(round(100.0*each/tmp, 4)) + ' '
	output += '];'
	print output


	print '\n'
	output = 'taskavenum = ['
	for i in range(0, 7):
		each = taskavenum[i]
		output += str(round(each/100.0, 2)) + ' '
	output += '];'
	print output
	output = 'taskaveutil = ['
	for i in range(0, 7):
		each = taskaveutil[i]
		output += str(round(each/taskavenum[i], 2)) + ' '
	output += '];'
	print output
	output = 'taskavecore = ['
	for i in range(0, 7):
		if taskavenum2[i] != 0:
			each = 1.0*taskavecore[i]/taskavenum2[i]
		else:
			each = 0
		output += str(round(each, 2)) + ' '
	output += '];'
	print output
	print countthr1, countthr2


(alltask, highmiss) = parse_result()
analyze(alltask, highmiss)

