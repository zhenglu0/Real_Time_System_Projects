#!/usr/bin/python

import re
import sys
import glob

from lib_cluster import *

def toTimespecStr(time, timeScaleInNSec):
	nsec = time * timeScaleInNSec
	#nsecPerSec = 1000000000
	#sec = nsec // nsecPerSec
	#nsec = nsec % nsecPerSec
	return nsec

def readtaskset():
	##rootdir = '/Users/user/Desktop/project/cluster/aug5util20-100n100/'
	#rootdir = '/Users/user/Desktop/project/cluster/core36aug2util05-90/'
	rootdir = '/Users/user/Desktop/project/cluster/core36aug2avg3/'
	name = 'synthetic_task'

	alltaskset = []
	for i in range(0, 7):
		alltaskset.append([])
		for j in range(0, 101):
			alltaskset[i].append([])

	for i in range(0, 7):
		for j in range(1, 101):
			info = alltaskset[i][j]
			tasksetDir = rootdir+'bestaug5util'+str(10*(i+2))+'/taskset'+str(j)
			nameid = 0
			for taskFile in sorted(glob.glob(tasksetDir + "/task*.txt")):
				with open(taskFile, 'r') as f:
					lines = f.readlines()
					m = re.match(r"^\s*(?P<period>\d+)\s+(?P<numSegments>\d+)\s*$", lines[0])
					period, numSegments = m.groups()
					work = 0
					span = 0
					for item in range(0, int(numSegments)):
						m = re.match(r"^\s*(?P<numStrands>\d+)\s+(?P<segmentLength>\d+)\s+(?P<priority>\d+)\s+(?P<release>(\d+.\d+)|\d+)\s*$", lines[1 + 2*item])
						numStrands, segmentLength = m.group(1, 2)
						work += int(numStrands) * int(segmentLength)
						span += int(segmentLength)
					util = 1.0*int(work)/int(period)
#alltaskset: ['prog_name', period, util, worst_case, span]+[numSegments]
					prog = [name+'_'+str(nameid), int(period), util, int(work), int(span), int(numSegments)]
					info.append(prog)
					nameid += 1
	return alltaskset

def syn_partition(fq, rawinfo, balance, corenum):
	timeScaleInNSec = 999999/fq + 1
	prognum = len(rawinfo)
	info = []
	for each in rawinfo:
		period = toTimespecStr(each[1], timeScaleInNSec)
		work = toTimespecStr(each[3], timeScaleInNSec)
		span = toTimespecStr(each[4], timeScaleInNSec)
#info: ['prog_name', period, util, worst_case, span]+[numSegments]
		prog = [each[0], period, each[2], work, span, each[5]]
		info.append(prog)

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
	#print sched, outinfo

	#first line options:
	#0: Guaranteed schedulable.
	#1: Not guaranteed schedulable, partition available, may try.
	#2: Not schedulable, no partition available.
	options = 0
	if sched == 2 or outinfo == []:
		options = 2
#		print 'Not schedulable, no partition available.'
	elif sched == 1 and len(outinfo) > 0:
		options = 1
#		print 'Not guaranteed schedulable, partition available, may try.'
	else:
		options = 0
#		print 'Guaranteed schedulable.'
	if outinfo != []:
		outinfo.sort(sortname)
	else:
		info.sort(sortname)
		outinfo = info
	return outinfo, corestr, sched


