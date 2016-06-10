#!/usr/bin/python

import sys
import re
import glob
import copy
import math

#from cluster import *
from lib_synthetic import *

##rootdir = "/Users/user/Desktop/project/cluster/cilk_clustering/exp/old_openmp_results"
#rootdir = "/Users/user/Desktop/project/cluster/cilk_clustering/exp/new_exp_results"
rootdir = "/Users/user/Desktop/project/cluster/cilk_clustering/exp/36cores"


#./analyze_result.py ../result/exp_results/experiment_summary_1024_no_balance_verbose.txt
def parse_result(resultdir):
	resultfile = open(resultdir, 'r')
	result = resultfile.readlines()

	#[[analysis], [realresult]]
	alltask = [[],[],[],[],[]]
	for i in range(0, 7):
		alltask[0].append([])
		alltask[1].append([])
		alltask[2].append([])
		alltask[3].append([])
		alltask[4].append([])
		for j in range(0, 101):
			alltask[0][i].append(-1)
			alltask[1][i].append(-1)
			alltask[2][i].append([])
			alltask[3][i].append([])
			alltask[4][i].append([])
	#print len(alltask)

	i = 0
	utilnum = 0
	tasknum = 1
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
				if result[i+1] == '		task0_result.txt: 1 0 0\n' or result[i+1] == '		task0_result.txt: 1 0 \n':
					#print utilnum,tasknum
					alltask[0][utilnum][tasknum] = 2
					alltask[1][utilnum][tasknum] = 2
					i += 1
				else:
					#print utilnum, tasknum
					alltask[0][utilnum][tasknum] = 1
					alltask[1][utilnum][tasknum] = 0
			else:
					alltask[0][utilnum][tasknum] = 0
					alltask[1][utilnum][tasknum] = 0
			i += 1
			continue

		real = re.match(r'^\s+task(?P<name>\d+)_result.txt: (?P<miss>\d+) \d+ (?P<high>\d+) .*$', eachline)
		if real:
			realnum = int(real.group('miss'))
			#LJ#
			if realnum > 0:
				alltask[1][utilnum][tasknum] = 1
			if realnum > 1:
				alltask[2][utilnum][tasknum].append(int(real.group('name')))
				#print alltask[2][utilnum][tasknum],utilnum,tasknum
				if int(real.group('high')) > 1:
					alltask[3][utilnum][tasknum].append([int(real.group('name')), int(real.group('high'))])
			elif int(real.group('high')) > 1:
				alltask[4][utilnum][tasknum].append([int(real.group('name')), int(real.group('high'))])
			i += 1
			continue
	return alltask

#sort according to first (high to low)
def sortfirst(e1, e2):
	return -1*cmp(e1[0], e2[0])

#sort according to priority (high to low)
def sortpriority(e1, e2):
	return -1*cmp(e1[-3], e2[-3])


def compare():
	result = []
	number = 0
	##for diffperiod in sorted(glob.glob(rootdir + "/experiment_summary_*.txt")):
	##	period = re.match(rootdir+r'/experiment_summary_(?P<period>\d+)_balance_verbose\.txt$', diffperiod)
		#period = re.match(rootdir+r'/experiment_summary_(?P<period>\d+)_.*_seq_.*_verbose\.txt$', diffperiod)
	#for diffperiod in sorted(glob.glob(rootdir + "/cilk_*.txt")):
	#	period = re.match(rootdir+r'/cilk_(?P<period>\d+)_overhead_seq_5.*_verbose\.txt$', diffperiod)
	for diffperiod in sorted(glob.glob(rootdir + "/*.txt")):
		#period = re.match(rootdir+r'/experiment_summary_36cores_(?P<period>\d+).*_verbose\.txt$', diffperiod)
		#period = re.match(rootdir+r'/cilk_36cores_(?P<period>\d+).*_verbose\.txt$', diffperiod)
		period = re.match(rootdir+r'/.*_new36cores_(?P<period>\d+).*_verbose\.txt$', diffperiod)
		if period:
			print diffperiod
			fq = int(period.group('period'))
			alltask = parse_result(diffperiod)
			result.append([fq, alltask])
			number += 1
	#stop

	result.sort(sortfirst)
	overhead = []
	overslack = []
	overslack2 = []
	count1 = []
	count2 = []
	countmiss = 0
	for k in range(0, number):
		overhead.append(-1)
		overslack.append(-1)
		overslack2.append(10000000)
		count1.append(0)
		count2.append(0)
	alltaskset = readtaskset()

	compareover = []
	for k in range(0, number):
		compareover.append([])
		for i in range(0, 7):
			compareover[k].append(0)

	for i in range(0, 7):
		for j in range(1, 101):
		#	for k in range(0, number-1):
		#		if result[k+1][1][1][i][j] == -1:
		#			continue
		#		if result[k][1][1][i][j] == -1:
		#			continue
		#		if result[k][1][1][i][j] < result[k+1][1][1][i][j]:
		#			print 'util',10*(i+2),'task',j,result[k][0],':', result[k][1][1][i][j], result[k+1][1][1][i][j],'\t',result[k][1][0][i][j], result[k+1][1][0][i][j]

			for k in range(0, number):
				fqq = result[k][0]
				#if k == 4 or k == 5:
				#	[outinfo, corestr, sched] = syn_partition(fqq, alltaskset[i][j], 4, 12)
				#else:
			#	[outinfo, corestr, sched] = syn_partition(fqq, alltaskset[i][j], 3, 12)
				[outinfo, corestr, sched] = syn_partition(fqq, alltaskset[i][j], 7, 36)

				minslack = -1
				for each in result[k][1][3][i][j]:
					if k != 0:
						continue
					countmiss+=1
					nuncore = each[1]
					numseg = alltaskset[i][j][each[0]-1][5]
					period = outinfo[each[0]-1][1]
					work = outinfo[each[0]-1][3]
					span = outinfo[each[0]-1][4]
					slack = period - span - (work-span)*1.0/(each[1]*0.9)
				#	slack = slack*1.0/(each[1]*work/span)
				#	slack = slack*1.0/(math.log(each[1],2)*work/span)
				#	slack = slack/((8*25*work/span+110)*1000.0)
					tmpslack = slack/(1000.0*(0.15*work/span*(numseg+7)+10*numseg+80))
					slack = slack/(1000.0*((0.15*nuncore+1.1)*work/span/nuncore*numseg+(10.5*nuncore+10)*numseg))
					if k == 0:
						if slack < 0:
							continue 
						print len(alltaskset[i][j]),'\t',slack, each[1], numseg, work/span, period - span - (work-span)*1.0/each[1],'\t',work/span*(numseg),numseg*(each[1])
					#if tmpslack < 1 and slack > 1 and each[1]-1.0*(work-span)/(period-span)<1:
					#	print '\t\t',slack, each[1], numseg, work/span, period - span - (work-span)*1.0/each[1]

				#	print slack,each,work*1.0/span
				#	if slack > 10000:
						#continue
				#		print slack,each, k,result[k][0], i, j
						#print period-span, (work-span)*1.0/each[1]
						#print outinfo
					if slack > minslack:
						minslack = slack
				minslack2 = 10000000
				for each in result[k][1][4][i][j]:
					if k != 0:
						continue
					numseg = alltaskset[i][j][each[0]-1][5]
					period = outinfo[each[0]-1][1]
					work = outinfo[each[0]-1][3]
					span = outinfo[each[0]-1][4]
					slack = period - span - (work-span)*1.0/each[1]
				#	slack = slack*1.0/(each[1]*work/span)
				#	slack = slack*1.0/(math.log(each[1],2)*work/span)
					#slack = slack/(each[1]*math.log(1.0*work/span, 2))
					slack = slack/(1000.0*(0.15*work/span*(numseg+7)+10*numseg+100))
					count2[k]+=1
				#	if slack < 70000:
					if slack < 1.8:
						count1[k] += 1
					#if k == 0 and i == 4 and j == 15 and each[0]-1==6:
					#	slack = period - span - (work-span)*1.0/5
					#	slack = slack*1.0/(math.log(5)*work/span)
					#	print '\t',slack,each, k,result[k][0], i, j
				#		continue
					if slack < minslack2:
						minslack2 = slack

				if minslack > overslack[k]:
					overslack[k] = minslack
				if minslack2 < overslack2[k]:
					overslack2[k] = minslack2

				if result[k][1][2][i][j] == []:
					continue

		#		flag = 0
		#		for core in corestr:
		#			sumutil = 0
		#			for each in core:
		#				sumutil += each[2]
		#			if sumutil >= 0.95:
		#				flag = 1
		#		if flag == 1:
		#			continue

				#print i, j
				#print alltaskset[i][j]
				#print outinfo
				tmp3 = 10000000
				for each in result[k][1][2][i][j]:
					misscore = outinfo[each-1][-2]
					if outinfo[each-1][-1] != misscore:
						#print 'highmiss'
						continue
					misstask = outinfo[each-1]
					missall = []
					for task in outinfo:
						if task[-2] == misscore:
							missall.append(task)
					missall.sort(sortpriority)
					inter = 0
					preempt = 2
					preempt2 = 1
					preempt3 = 2.0/missall[0][1]
					flag = 0
					missutil = 0
					eachold = missall[0][1]
					for each in missall:
						missutil += each[2]
						if each == misstask:
							flag = 1
						elif flag == 0:
							inter += misstask[1]/each[1]*each[3]
							preempt += 2*misstask[1]/each[1]
						else:
							preempt += 2*each[1]/misstask[1]
						preempt2 += 1*each[1]/missall[0][1]
						preempt3 += 1.0/eachold
						eachold = each[1]
					tmp2 = (misstask[1]*0.95-misstask[3]-inter)/1000.0
					tmp2 = 1.0*tmp2/(preempt2*50)
					tmp = len(missall)*(preempt2*100*1000.0)/missall[-1][1]
					tmp = 1.5*(preempt3*100*1000.0)
					tmp = 0.95-missutil-tmp
					if missutil >= 0.9 and len(missall) == 1:
						continue
					#if tmp2 > overhead[k]:
					#	overhead[k] = tmp2
					if i==6 and j==15 and k==4 and len(missall)==1:
						continue
					if tmp < tmp3:
						tmp3 = tmp

					if tmp == 0.046104944984646831:
						print i, j, k, result[k][0], result[k][1][2][i][j]
						print misstask
						print missall
						print tmp, preempt
						print outinfo
				if tmp3 > overhead[k] and tmp3 != 10000000:
					overhead[k] = tmp3

			#for k in range(0, number):
			#	if result[k][1][2][i][j] == []:
			#		[outinfo, corestr, sched] = syn_partition(result[k][0], alltaskset[i][j], 7, 12)
			#		if result[k][1][0][i][j] == 0 and sched > 0:
			#			compareover[k][i]+=1
			for k in range(0, number):
			##	[outinfo, corestr, sched] = syn_partition(result[k][0], alltaskset[i][j], 7, 12)
				[outinfo, corestr, sched] = syn_partition(result[k][0], alltaskset[i][j], 7, 36)
			#	[outinfo, corestr, sched] = syn_partition(1024, alltaskset[i][j], 7, 36)
				if sched < 1:
					compareover[k][i]+=1
				#if corestr == []:
				#	continue
				#if sched == 0 and corestr[0][0][1] < 8000000 and corestr[0][0][2] > 0.82 and corestr[0][0][2] < 0.9:
				#	smallperiod = corestr[0][0][1]
				#if k == 0 and i == 3 and sched == 0:
				#	print j, '~~~~'
				if k == 0 and i == 3 and sched == 0 and j == 87:
					#continue
					task = 0
					period = outinfo[task][1]
					work = outinfo[task][3]
					span = outinfo[task][4]
					nuncore = outinfo[task][-1]-outinfo[task][-2]+1
				#	print period,work,span,nuncore
					print corestr
					slack = period - span - (work-span)*1.0/nuncore
					slack1 = slack*1.0/(nuncore*work/span)
					slack2 = slack*1.0/(8*(1000.0*(0.15*work/span*(numseg+7)+10*numseg+80)))
					print '\t',slack,slack2,work/span,nuncore,alltaskset[i][j][task][5],(work-span)/(1.0*period - span),(alltaskset[i][j][task][5]*(8.2*nuncore+130))*1000
				#	print corestr

				#print i, j, k
	print overhead
	print overslack
	print overslack2
	print countmiss
	print count1
	print count2
	print compareover
				#stop	

compare()

#[100 100 100 95 68 25 3 ], [100 100 100 100 90 67 14 ], [], [100 100 100 100 100 97 72 ]
#70 84
#40 100


#####for cilk
[[93, 85, 73, 70, 45, 5, 0]]
#theslack = period-span-1.0*(work - span)/(numcore*0.95)
#theslack < (numseg*(8.2*numcore+130))*1000
[[87, 79, 64, 52, 22, 0, 0], [93, 90, 75, 79, 57, 0, 0], [100, 100, 98, 99, 75, 2, 0]]
#theslack = period-span-1.0*(work - span)/(numcore*0.95)
#theslack < (185+32*math.log(numcore,2))*1000
[[100, 99, 98, 98, 78, 1, 0], [100, 100, 100, 100, 92, 6, 0], [100, 100, 100, 100, 95, 11, 0]]


######!!!!!!!!core36 is running cilk 256!!!!!!!!

#sudo rm /export/austen/experiment/testcore36aug2avg3/bestaug5util50/taskset17/result/*
#sudo vi /export/austen/experiment/testcore36aug2avg3/bestaug5util50/taskset17/result/ta


#####for omp
[[48, 24, 15, 9, 0, 0, 0]]
#theslack = period-span-1.0*(work - span)/(numcore*0.95)
#while theslack < (8*25*work/span+120)*1000.0
[[48, 28, 17, 10, 0, 0, 0], [75, 66, 50, 42, 9, 0, 0], [95, 86, 80, 75, 39, 1, 0]]
#theslack = period-span-1.0*(work - span)/(numcore*0.95)
#theslack<3*((0.15*numcore+1.1)*(work/span/numcore)*numseg+(10.5*numcore+10)*numseg)*1000.0
[[72, 48, 37, 19, 14, 0, 0], [91, 83, 70, 62, 49, 0, 0], [94, 92, 79, 80, 62, 2, 0]]
#2*
[[84, 72, 56, 44, 33, 0, 0], [93, 90, 75, 73, 57, 1, 0], [98, 98, 94, 91, 69, 5, 0]]
#theslack<((0.15*numcore+1.1)*(work/span/numcore)*numseg+(10.5*numcore+10)*numseg)*1000.0
[[93, 90, 75, 73, 57, 0, 0], [98, 98, 94, 91, 69, 5, 0], [100, 100, 100, 100, 88, 9, 0]]

#period-span-1.0*(work - span)/numcore <10000000
#	1024			512			256			128
[[4, 1, 2, 0, 0, 0, 0], [15, 10, 3, 1, 0, 0, 0], [43, 30, 16, 6, 1, 0, 0], [71, 57, 41, 38, 13, 1, 0], [97, 91, 90, 84, 56, 1, 0], [100, 99, 98, 97, 86, 12, 0]]


