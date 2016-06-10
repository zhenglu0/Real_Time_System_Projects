#!/usr/bin/python

import re
import sys

#LJ#
#for experiments, set openmp = 0, cilk = 1
exp_version = 0
#exp_version = 1

#clustered scheduling
#assign core to each task
#info: ['prog_name', period, util, worst_case, span]
#outinfo: ['prog_name', period, util, worst_case, span, *priority, *firstc, *lastc]
#(sched, corestr, outinfo) = cluster_partition(info, prognum, corenum)
from lib_cluster import *
#INPUT file:
#Input format for Python script (real-time parallel taskset file .rtpt):
#A: system_first_core system_last_core
#B: task_program_name task_arg1 task_arg2 task_arg3 ...
#C: work_sec work_ns span_sec span_ns period_sec period_ns deadline_sec deadline_ns relative_release_sec relative_release_ns num_iters

#OUTPUT file:
#Output format for Python script (real-time parallel schedule file .rtps):
#A: system_first_core system_last_core
#B: task_program_name task_arg1 task_arg2 task_arg3 ...
#C: work_sec work_ns span_sec span_ns period_sec period_ns deadline_sec deadline_ns relative_release_sec relative_release_ns num_iters
#D: task_first_core task_last_core task_priority

#read the input file
#provide the prefix only to inputname
#generate info: ['prog_name', period, util, worst_case, span]
def readinput(inputname):
	error = 0
	#open input file
	p1 = open(inputname+'.rtpt', 'r');
	infile = p1.readlines()
	p1.close()
	#print infile

	#first core, last core, num core
	cores = re.match(r'^\s*(?P<firstc>\d+)\s*(?P<lastc>\d+)\s*', infile[0])
	if cores:
		firstc = int(cores.group('firstc'))
		lastc = int(cores.group('lastc'))
		corenum = lastc - firstc + 1
		#corenum = lastc - firstc
		#print corenum
		if corenum < 1:
			print 'First line should be: first_core last_core'
			error = 1
	else:
		print 'First line should be: first_core last_core'
		error = 1

	#get each program parameters
	i = 1
	numtask = 0
	count = 0
	info = []
	rawinfo = [[],[],[firstc, lastc]]
	while i < len(infile):
		line = re.findall(r'\S+', infile[i])
		#line B
		if line and count == 0:
			count = 1
			#get program name
			line[0] += '_'+str(numtask)
			name = line[0]
			numSegments = int(line[1])
			#print name
			rawinfo[0].append(line)
		#line C
		elif line and count == 1:
			count = 0
			rawinfo[1].append([name]+line)
			if len(line) != 11:
				print 'Invalid paraneters:', infile[i]
				error = 1
#C: work_sec work_ns span_sec span_ns period_sec period_ns deadline_sec deadline_ns relative_release_sec relative_release_ns num_iters
			#get work, period, span
			work = int(line[0])*1000000000+int(line[1])
			span = int(line[2])*1000000000+int(line[3])
			period = int(line[4])*1000000000+int(line[5])
			deadline = int(line[6])*1000000000+int(line[7])
			if period != deadline:
				print 'period not equal deadline:', infile[i]
				error = 1
			util = 1.0*work/period
			#print util
			if util >= 1.0 and 2*span > period:
				print 'critical path length too long:', span, period
				error = 1
			#info: ['prog_name', period, util, worst_case, span]
			#prog = [name, period, util, work, span]
#alltaskset: ['prog_name', period, util, worst_case, span]+[numSegments]
			prog = [name, period, util, work, span, numSegments]
			info.append(prog)
			numtask += 1
		#empty line
		#else:
			#print 'end', infile[i]
		i += 1
	#print info#,rawinfo
	#no parameters for the last program
	if count == 1:
		print 'no parameters for the last program'
		error = 1
	if error:
		return [], 0, infile
	else:
		info.sort(sortutil)
		#print info
		#print rawinfo
		return info, corenum, rawinfo

#partition the task set
#info: ['prog_name', period, util, worst_case, span]
def partition(inputname, rawinfo, info, corenum, balance):
	prognum = len(info)
	#invalid input file
	if prognum == 0:
		print 'Invalid rtps file, not schedulable.'
		sched = 1
		outinfo = []
		return
	#can run partition

	#outinfo: ['prog_name', period, util, worst_case, span, *priority, *firstc, *lastc]

	if balance != 3 and balance != 7:
		(sched, corestr, outinfo) = cluster_partition(info, prognum, corenum, balance)
	elif balance == 3:
		info1 = copy.deepcopy(info)
		info2 = copy.deepcopy(info)
		(sched, corestr, outinfo) = cluster_partition(info, prognum, corenum, balance)
		if sched != 0:
			(sched1, corestr1, outinfo1) = cluster_partition(info1, prognum, corenum, 4)
			if sched1 == 0 or (sched == 2 and sched1 == 1):
			#if sched1 == 0 or (sched2 == 2 and sched1 < 2):
				#print "!", sched1
				if sched == 2:
					sched = 1
				corestr = corestr1
				outinfo = outinfo1
			elif sched1 == 2:
				(sched2, corestr2, outinfo2) = cluster_partition(info2, prognum, corenum, 2)
				if sched2 == 0 or (sched == 2 and sched2 == 1):
					#print "!!", sched2
					if sched == 2:
						sched = 1
					corestr = corestr2
					outinfo = outinfo2
					#print sched, sched1, sched2
	else:
		info1 = copy.deepcopy(info)
		info2 = copy.deepcopy(info)
		(sched, corestr, outinfo) = cluster_partition(info, prognum, corenum, balance)
		if sched != 0:
			(sched1, corestr1, outinfo1) = cluster_partition(info1, prognum, corenum, 8)
			if sched1 == 0 or (sched == 2 and sched1 == 1):
			#if sched1 == 0 or (sched2 == 2 and sched1 < 2):
				#print "!", sched1
				if sched == 2:
					sched = 1
				corestr = corestr1
				outinfo = outinfo1
			elif sched1 == 2:
				(sched2, corestr2, outinfo2) = cluster_partition(info2, prognum, corenum, 9)
				if sched2 == 0 or (sched == 2 and sched2 == 1):
					#print "!!", sched2
					if sched == 2:
						sched = 1
					corestr = corestr2
					outinfo = outinfo2
					#print sched, sched1, sched2
			sched = 0
			corestr = []
			outinfo = []

	#print sched, outinfo

	#first line options:
	#0: Guaranteed schedulable.
	#1: Not guaranteed schedulable, partition available, may try.
	#2: Not schedulable, no partition available.
	options = 0
	if sched == 2 or outinfo == []:
		options = 2
		print 'Not schedulable, no partition available.'
	elif sched == 1 and len(outinfo) > 0:
		options = 1
		print 'Not guaranteed schedulable, partition available, may try.'
	else:
		options = 0
		print 'Guaranteed schedulable.'
	p2 = open(inputname+'.rtps', 'w');
	#if guaranteed, first line is 1
	#if not guaranteed, first line is 0
	p2.write(str(options)+'\n')
	p2.write(str(rawinfo[2][0])+' '+str(rawinfo[2][1])+'\n')
	#if partition available, more lines follow
	#if cannot partition, no more lines
	#print outinfo
	if outinfo != []:
		firstc = rawinfo[2][0]
		rawinfo[0].sort(sortname)
		rawinfo[1].sort(sortname)
		outinfo.sort(sortname)
		#print rawinfo
		#print outinfo
		for i in range(0, len(outinfo)):
			if rawinfo[0][i][0] != outinfo[i][0]:
				print 'Weird, names not match', info[i][0], outinfo[i][0]
			else:
				line = ''
				fakename = rawinfo[0][i][0]
				name = re.match(r'^(?P<name>.+)_(\d+)$', fakename)
				#print name.group('name')
				if name:
					if outinfo[i][6] != outinfo[i][7] and exp_version == 0:
						line = line + name.group('name') + ' '
					elif outinfo[i][6] != outinfo[i][7] and exp_version == 1:
						line = line + name.group('name') + '_cilk '
					else:
						line = line + name.group('name') + '_sequential '
				else:
					print 'Weird, names not match', fakename
				for each in rawinfo[0][i][1:]:
					line = line + each + ' '
				line = line.strip(' ')+'\n'
				for each in rawinfo[1][i][1:]:
					line = line + each + ' '
				line = line.strip(' ')+'\n'
				line = line+str(firstc+outinfo[i][6])+' '+str(firstc+outinfo[i][7])+' '+str(outinfo[i][5])+'\n'
				#print line
				#break
				p2.write(line)
	p2.close()


#./cluster.py egtaskset [balance]
#provide only the prefix of input .rtpt file name, and it will automatically generate corresponding .rtps file
#with balance option, it will do additional load balancing
	#0: Guaranteed schedulable.
	#1: Not guaranteed schedulable, partition available, may try.
	#2: Not schedulable, no partition available.
#lib_cluster.py includes functions needed to do partition

def run_cluster():
	inputname = 'egtaskset'
	inputname = sys.argv[1]
	print inputname
	(info, corenum, rawinfo) = readinput(inputname)
	balance = 0
	if len(sys.argv) > 2:
		if sys.argv[2] == 'balance':
			balance = 3
		elif sys.argv[2] == 'test':
			balance = 2
		elif sys.argv[2] == 'threshold':
			balance = 5
		elif sys.argv[2] == 'socket':
			balance = 6
		elif sys.argv[2] == 'threshold95':
			balance = 4
		elif sys.argv[2] == 'overhead':
			balance = 7
	partition(inputname, rawinfo, info, corenum, balance)
run_cluster()




#['prog_name', period, 'seg_id', sub_count, worst_case, rel_tm, rel_dead, *priority, [core]]
#[1, 8003584, segid, numthread, work, 0, 0]
#[2, 16007168, segid, numthread, work, 0, 0]
#[3, 2000896, segid, numthread, work, 0, 0]
#[4, 4001792, segid, numthread, work, 0, 0]
#[5, 2000896, segid, numthread, work, 0, 0]
#[6, 16007168, segid, numthread, work, 0, 0]

#taskset = [[1, 8003584, 1, 1, 181722, 0, 0],[1, 8003584, 2, 2, 492408, 0, 0],[1, 8003584, 3, 25, 319479, 0, 0],[2, 16007168, 1, 16, 987747, 0, 0],[2, 16007168, 2, 1, 383961, 0, 0],[3, 2000896, 1, 15, 199308, 0, 0],[4, 4001792, 1, 9, 466029, 0, 0],[5, 2000896, 1, 8, 354651, 0, 0],[6, 16007168, 1, 8, 226664, 0, 0],[6, 16007168, 2, 2, 214940, 0, 0],[6, 16007168, 3, 1, 394708, 0, 0],[6, 16007168, 4, 2, 406432, 0, 0],[6, 16007168, 5, 5, 290169, 0, 0],[6, 16007168, 6, 2, 426949, 0, 0],[6, 16007168, 7, 5, 254997, 0, 0],[6, 16007168, 8, 12, 799186, 0, 0]]
#(sched, corestr, outinfo) = cluster_assign_core(taskset, 6, 12, 0)
#print outinfo
#simu5 = cluster_simulation(corestr, taskset)


