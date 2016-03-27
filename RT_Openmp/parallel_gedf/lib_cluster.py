#!/usr/bin/python

import string
import math
import copy
import re

#LJ#
#for experiments, set openmp = 0, cilk = 1
exp_version = 0
#exp_version = 1

#sort according to segments' prog_name, sub_id
def sortname(e1, e2):
	if e1[0] != e2[0]:
		return cmp(e1[0], e2[0])
	else:
		return cmp(e1[2], e2[2])
		#return 1

#sort according to segments' prog_name id
def sortid(e1, e2):
	a1 = re.match(r'^.+_(?P<id>\d+)', e1[0])
	b1 = int(a1.group('id'))
	a2 = re.match(r'^.+_(?P<id>\d+)', e2[0])
	b2 = int(a2.group('id'))
	return cmp(b1, b2)

#sort according to progs' util (high to low)
def sortutil(e1, e2):
	return -1*cmp(e1[2], e2[2])

#sort according to progs' period (low to high)
def sortperiod(e1, e2):
	return cmp(e1[1], e2[1])

#sort according to progs' releasetime (low to high)
def sortrelease(e1, e2):
	if e1[8] != e2[8]:
		return cmp(e1[8], e2[8])
	else:
		return cmp(e1[9], e2[9])

#sort according to progs' priority (high to low)
def sortpriority(e1, e2):
	return -1*cmp(e1[5], e2[5])

#sort possible according to remaining util (high to low)
def sortposs(e1, e2):
	return -1*cmp(e1[1], e2[1])

#for 36#
#sorthigh for highinfo
def sorthigh(e1, e2):
	return -1*cmp(e1[0], e2[0])


#partition low util tasks
#original or threshold
#0: Guaranteed schedulable.
#1: Not guaranteed schedulable, partition available, may try.
#2: Not schedulable, no partition available.
def overhead(lowcore, lowinfo, posscore, highid, corestat, corestr, outinfo, thr):
	sched = 0
	lowcorenum = len(lowcore)
	curcore = 0
	#overhead#
	#ratio = 1
	ratio = 2
	if thr == 0.9:
		#old#ratio = 2
		ratio = 0.3
		#old#thr = 0.95
		thr = 0.9
	elif thr == 0.95:
		##ratio = 1.5
		ratio = 0.2
		#thr = 0.98
	elif thr == 0.98:
		##ratio = 1.5
		ratio = 0.1
		#thr = 1.0
	else:
		thr = 0.95

	overcore = []
	for each in corestat:
		overcore.append([0,1])

	for i in range(0, len(lowinfo)):
		#info: ['prog_name', period, util, worst_case, span]
		util = lowinfo[i][2]
		period = lowinfo[i][-1]
		#min sumutil, min core
		#print curcore, corestat
		minutil = corestat[lowcore[curcore]][1]
		mincore = curcore
		#next fit ring
		count = 0
		while count < lowcorenum:
			#corestat: [[np(numprog), sumutil, mint, preempt], ]
			if corestat[lowcore[curcore]][0] == 0:
				corestat[lowcore[curcore]][0] += 1
				corestat[lowcore[curcore]][1] += util
				corestat[lowcore[curcore]][2] = period
				#overhead#
				#overcore[lowcore[curcore]][0] = 2*ratio
				overcore[lowcore[curcore]][0] = 2.0*ratio/lowinfo[i][1]
				overcore[lowcore[curcore]][1] = lowinfo[i][1]
				newprog = lowinfo[i][0:-1]+[98-corestat[lowcore[curcore]][0], lowcore[curcore], lowcore[curcore]]
				outinfo.append(newprog)
				corestr[lowcore[curcore]].append(newprog)
				break
			else:
				#bound = np(rp^(1/np)-1)+2/rp-1
				#sumutil <= bound
				np = corestat[lowcore[curcore]][0]+1.0
				rp = 1.0*period/corestat[lowcore[curcore]][2]
				sumutil = corestat[lowcore[curcore]][1] + util
				#overhead#
				#preempt = overcore[lowcore[curcore]][0]+ratio*lowinfo[i][1]/overcore[lowcore[curcore]][1]
				preempt = overcore[lowcore[curcore]][0]+2.0*ratio/overcore[lowcore[curcore]][1]
				#sumutil += np*(preempt*100*1000.0)/overcore[lowcore[curcore]][1]
				#overhead#
				##sumutil += (preempt*100*1000.0)
				sumutil += (preempt*100*1000.0+1-thr)
				bound = np*(math.pow(rp,(1/np))-1)+2/rp-1
				##if sumutil < thr and sumutil <= bound:
				if sumutil <= bound:
					corestat[lowcore[curcore]][0] += 1
					corestat[lowcore[curcore]][1] += util
					#overhead#
					#overcore[lowcore[curcore]][0] += ratio*lowinfo[i][1]/overcore[lowcore[curcore]][1]
					overcore[lowcore[curcore]][0] += 2.0*ratio/overcore[lowcore[curcore]][1]
					overcore[lowcore[curcore]][1] = lowinfo[i][1]
					newprog = lowinfo[i][0:-1]+[98-corestat[lowcore[curcore]][0], lowcore[curcore], lowcore[curcore]]
					outinfo.append(newprog)
					corestr[lowcore[curcore]].append(newprog)
					break
				else:
					if minutil > corestat[lowcore[curcore]][1]:
						minutil = corestat[lowcore[curcore]][1]
						mincore = curcore
					count += 1
					if curcore == lowcorenum - 1:
						curcore = 0
					else:
						curcore += 1
				#print bound, sumutil, curcore
		if count >= lowcorenum:
			sched = 1
			if corestat[lowcore[mincore]][1] + util >= 1:
				#print "--------"
				#add one posscore to lowcore
				if len(posscore) > 0:
					addcore = posscore[0]
					progid = highid[0]
					posscore.remove(addcore)
					highid.remove(progid)
					lowcore.append(addcore)
					lowcorenum += 1
					corestat[addcore][0] += 1
					corestat[addcore][1] += util
					corestat[addcore][2] = period
					#overhead#
					#overcore[addcore][0] = 2*ratio
					overcore[lowcore[curcore]][0] = 2.0*ratio/lowinfo[i][1]
					overcore[addcore][1] = lowinfo[i][1]
					newprog = lowinfo[i][0:-1]+[98-corestat[addcore][0], addcore, addcore]
					outinfo.append(newprog)
					corestr[addcore].append(newprog)
					outinfo[progid][-1] = addcore - 1
					corestr[outinfo[progid][-2]][0][-1] = addcore - 1
					#print "try............."
					#print possible
					#print addcore, newprog
					#print outinfo
					#print corestr
					#print corestat
				else:
					return 2
			#put to minutil core
			else:
				corestat[lowcore[mincore]][0] += 1
				corestat[lowcore[mincore]][1] += util
				#overhead#
				#overcore[lowcore[mincore]][0] += ratio*lowinfo[i][1]/overcore[lowcore[curcore]][1]
				overcore[lowcore[curcore]][0] += 2.0*ratio/overcore[lowcore[curcore]][1]
				overcore[lowcore[curcore]][1] = lowinfo[i][1]
				newprog = lowinfo[i][0:-1]+[98-corestat[lowcore[mincore]][0], lowcore[mincore], lowcore[mincore]]
				outinfo.append(newprog)
				corestr[lowcore[mincore]].append(newprog)
	#print "\t",outinfo
	#if sched == 1:
		#print "!!!!!!!!!!original\t"#, corestr, partedcore, lowcore, corestat
	#print corestr
	return sched


#partition low util tasks
#original or threshold
#0: Guaranteed schedulable.
#1: Not guaranteed schedulable, partition available, may try.
#2: Not schedulable, no partition available.
def original(lowcore, lowinfo, posscore, highid, corestat, corestr, outinfo, thr):
	sched = 0
	lowcorenum = len(lowcore)
	curcore = 0

	for i in range(0, len(lowinfo)):
		#info: ['prog_name', period, util, worst_case, span]
		util = lowinfo[i][2]
		period = lowinfo[i][-1]
		#min sumutil, min core
		#print curcore, corestat
		minutil = corestat[lowcore[curcore]][1]
		mincore = curcore
		#next fit ring
		count = 0
		while count < lowcorenum:
			#corestat: [[np(numprog), sumutil, mint], ]
			if corestat[lowcore[curcore]][0] == 0:
				corestat[lowcore[curcore]][0] += 1
				corestat[lowcore[curcore]][1] += util
				corestat[lowcore[curcore]][2] = period
				newprog = lowinfo[i][0:-1]+[98-corestat[lowcore[curcore]][0], lowcore[curcore], lowcore[curcore]]
				outinfo.append(newprog)
				corestr[lowcore[curcore]].append(newprog)
				break
			else:
				#bound = np(rp^(1/np)-1)+2/rp-1
				#sumutil <= bound
				np = corestat[lowcore[curcore]][0]+1.0
				rp = 1.0*period/corestat[lowcore[curcore]][2]
				sumutil = corestat[lowcore[curcore]][1] + util
				bound = np*(math.pow(rp,(1/np))-1)+2/rp-1
				if sumutil < thr and sumutil <= bound:
					corestat[lowcore[curcore]][0] += 1
					corestat[lowcore[curcore]][1] += util
					newprog = lowinfo[i][0:-1]+[98-corestat[lowcore[curcore]][0], lowcore[curcore], lowcore[curcore]]
					outinfo.append(newprog)
					corestr[lowcore[curcore]].append(newprog)
					break
				else:
					if minutil > corestat[lowcore[curcore]][1]:
						minutil = corestat[lowcore[curcore]][1]
						mincore = curcore
					count += 1
					if curcore == lowcorenum - 1:
						curcore = 0
					else:
						curcore += 1
				#print bound, sumutil, curcore
		if count >= lowcorenum:
			sched = 1
			if corestat[lowcore[mincore]][1] + util >= 1:
				#print "--------"
				#add one posscore to lowcore
				if len(posscore) > 0:
					addcore = posscore[0]
					progid = highid[0]
					posscore.remove(addcore)
					highid.remove(progid)
					lowcore.append(addcore)
					lowcorenum += 1
					corestat[addcore][0] += 1
					corestat[addcore][1] += util
					corestat[addcore][2] = period
					newprog = lowinfo[i][0:-1]+[98-corestat[addcore][0], addcore, addcore]
					outinfo.append(newprog)
					corestr[addcore].append(newprog)
					outinfo[progid][-1] = addcore - 1
					corestr[outinfo[progid][-2]][0][-1] = addcore - 1
					#print "try............."
					#print possible
					#print addcore, newprog
					#print outinfo
					#print corestr
					#print corestat
				else:
					return 2
			#put to minutil core
			else:
				corestat[lowcore[mincore]][0] += 1
				corestat[lowcore[mincore]][1] += util
				newprog = lowinfo[i][0:-1]+[98-corestat[lowcore[mincore]][0], lowcore[mincore], lowcore[mincore]]
				outinfo.append(newprog)
				corestr[lowcore[mincore]].append(newprog)
	#print "\t",outinfo
	#if sched == 1:
		#print "!!!!!!!!!!original\t"#, corestr, partedcore, lowcore, corestat
	#print corestr
	return sched

#partition low util tasks
#worstfit
#0: Guaranteed schedulable.
#1: Not guaranteed schedulable, partition available, may try.
#2: Not schedulable, no partition available.
def worstfit(lowcore, lowinfo, posscore, highid, corestat, corestr, outinfo):
	sched = 0
	lowcorenum = len(lowcore)
	curcore = 0

	for i in range(0, len(lowinfo)):
		#info: ['prog_name', period, util, worst_case, span]
		util = lowinfo[i][2]
		period = lowinfo[i][-1]
		#min sumutil, min core
		#print curcore, corestat
		minutil = corestat[lowcore[curcore]][1]
		mincore = curcore
		#next fit ring
		count = 0
		find = 0
		while count < lowcorenum:
			#corestat: [[np(numprog), sumutil, mint], ]
			if corestat[lowcore[curcore]][0] == 0:
			#	corestat[lowcore[curcore]][0] += 1
			#	corestat[lowcore[curcore]][1] += util
			#	corestat[lowcore[curcore]][2] = period
			#	newprog = lowinfo[i][0:-1]+[98-corestat[lowcore[curcore]][0], lowcore[curcore], lowcore[curcore]]
			#	outinfo.append(newprog)
			#	corestr[lowcore[curcore]].append(newprog)
				find = 1
				minutil = corestat[lowcore[curcore]][1]
				mincore = curcore
				break
			else:
				#bound = np(rp^(1/np)-1)+2/rp-1
				#sumutil <= bound
				np = corestat[lowcore[curcore]][0]+1.0
				rp = 1.0*period/corestat[lowcore[curcore]][2]
				sumutil = corestat[lowcore[curcore]][1] + util
				bound = np*(math.pow(rp,(1/np))-1)+2/rp-1
				if sumutil <= bound:
					find = 2
			#		corestat[lowcore[curcore]][0] += 1
			#		corestat[lowcore[curcore]][1] += util
			#		newprog = lowinfo[i][0:-1]+[98-corestat[lowcore[curcore]][0], lowcore[curcore], lowcore[curcore]]
			#		outinfo.append(newprog)
			#		corestr[lowcore[curcore]].append(newprog)
			#		break
			#	else:
				if minutil > corestat[lowcore[curcore]][1]:
					minutil = corestat[lowcore[curcore]][1]
					mincore = curcore
				count += 1
				if curcore == lowcorenum - 1:
					curcore = 0
				else:
					curcore += 1
				#print '\t',bound, sumutil, curcore
		if find == 0:
			sched = 1
			if corestat[lowcore[mincore]][1] + util >= 1:
				#print "--------"
				#add one posscore to lowcore
				if len(posscore) > 0:
					addcore = posscore[0]
					progid = highid[0]
					posscore.remove(addcore)
					highid.remove(progid)
					lowcore.append(addcore)
					lowcorenum += 1
					corestat[addcore][0] += 1
					corestat[addcore][1] += util
					corestat[addcore][2] = period
					newprog = lowinfo[i][0:-1]+[98-corestat[addcore][0], addcore, addcore]
					outinfo.append(newprog)
					corestr[addcore].append(newprog)
					outinfo[progid][-1] = addcore - 1
					corestr[outinfo[progid][-2]][0][-1] = addcore - 1
					#print "try............."
					#print possible
					#print addcore, newprog
					#print outinfo
					#print corestr
					#print corestat
				else:
					return 2
			#put to minutil core
			else:
				corestat[lowcore[mincore]][0] += 1
				corestat[lowcore[mincore]][1] += util
				newprog = lowinfo[i][0:-1]+[98-corestat[lowcore[mincore]][0], lowcore[mincore], lowcore[mincore]]
				outinfo.append(newprog)
				corestr[lowcore[mincore]].append(newprog)
		elif find == 1:
			corestat[lowcore[mincore]][0] += 1
			corestat[lowcore[mincore]][1] += util
			corestat[lowcore[mincore]][2] = period
			newprog = lowinfo[i][0:-1]+[98-corestat[lowcore[mincore]][0], lowcore[mincore], lowcore[mincore]]
			outinfo.append(newprog)
			corestr[lowcore[mincore]].append(newprog)
		elif find == 2:
			corestat[lowcore[mincore]][0] += 1
			corestat[lowcore[mincore]][1] += util
			newprog = lowinfo[i][0:-1]+[98-corestat[lowcore[mincore]][0], lowcore[mincore], lowcore[mincore]]
			outinfo.append(newprog)
			corestr[lowcore[mincore]].append(newprog)

	#print "\t",outinfo
	#if sched == 1:
	#	print "!!!!!!!!!!worstfit\t"#, corestr, partedcore, lowcore, corestat
	#print corestr
	return sched

#partition low util tasks
#load balance
#0: Guaranteed schedulable.
#1: Not guaranteed schedulable, partition available, may try.
#2: Not schedulable, no partition available.
def loadbalance(lowcore, info, posscore, highid, corestat, corestr, outinfo):
	#print '\n\n'
	#print lowcore
	#print info
	#print corestat
	#print corestr
	#print outinfo

	change = 0
	available = []
	availablestr = []
	used = []
	for core in lowcore:
		if corestr[core] == []:
			available.append(core)
		else:
			used.append(core)
	for core in corestr:
		availablestr.append([])
	if available == []:
		return change

	while(1):
		utilmax = 0
		coremax = 0
		for core in used:
			if corestat[core][0] > 1 and corestat[core][1] > utilmax:
				utilmax = corestat[core][1]
				coremax = core
		if utilmax == 0:
			#return change
			break

		utilmin = 1
		coremin = 0
		for core in available:
			if corestat[core][1] < utilmin:
				utilmin = corestat[core][1]
				coremin = core
			if utilmin == 0:
				break

		if utilmin >= utilmax:
			#return change
			break

		#print coremax, utilmax, corestr[coremax]
		flag = 0
		corestr[coremax].sort(sortutil)
		for each in corestr[coremax]:
			util = each[2]
			a = re.match(r'^.+_(?P<id>\d+)', each[0])
			nameid = int(a.group('id'))
			period = info[nameid][5]
			#print period, nameid
			if corestat[coremin][0] == 0:
				corestat[coremin][0] += 1
				corestat[coremin][1] += util
				corestat[coremin][2] = period
				corestat[coremax][0] -= 1
				corestat[coremax][1] -= util
				#newprog = info[nameid][0:5]+[98-corestat[coremin][0], coremin, coremin]
				#outinfo.append(newprog)
				#corestr[coremin].append(newprog)
				newprog = info[nameid][0:6]
				availablestr[coremin].append(newprog)
				oldprog = each
				flag = 1
				change = 1
				break
			else:
				if corestat[coremin][2] > period:
					corestat[coremin][2] = period
				np = 1.0
				sumutil = util
				canadd = 0
				for eachprog in availablestr[coremin]:
					if period > eachprog[5]:
						np += 1.0
						sumutil += eachprog[2]
						continue
					elif canadd == 0:
						rp = 1.0*period/corestat[coremin][2]
						bound = np*(math.pow(rp,(1/np))-1)+2/rp-1
						if sumutil >= utilmax or sumutil > bound:
							canadd = 2
							break
						#print '!',np, rp, sumutil, bound,coremin
						canadd = 1 
						#continue
					np += 1.0
					rp = 1.0*eachprog[5]/corestat[coremin][2]
					sumutil += eachprog[2]
					bound = np*(math.pow(rp,(1/np))-1)+2/rp-1
					#print '!!',np, rp, sumutil, bound,coremin
					#print each
					if sumutil >= utilmax or sumutil > bound:
						#print '!!!',sumutil, utilmax, utilmin
						canadd = 2
						break
				#np = corestat[coremin][0]+1.0
				#rp = 1.0*period/corestat[coremin][2]
				#sumutil = corestat[coremin][1] + util
				#bound = np*(math.pow(rp,(1/np))-1)+2/rp-1
				if canadd == 0:
					rp = 1.0*period/corestat[coremin][2]
					bound = np*(math.pow(rp,(1/np))-1)+2/rp-1
					if sumutil >= utilmax or sumutil > bound:
						canadd = 2
					else:
						canadd = 1 
				if canadd == 1:
					corestat[coremin][0] += 1
					corestat[coremin][1] += util
					corestat[coremax][0] -= 1
					corestat[coremax][1] -= util
					#newprog = info[nameid][0:5]+[98-corestat[coremin][0], coremin, coremin]
					#outinfo.append(newprog)
					#corestr[coremin].append(newprog)
					newprog = info[nameid][0:6]
					#if period < availablestr[coremin][0][5]:
					#	print each, coremin, availablestr#"!!"
					#	change = 10
					availablestr[coremin].append(newprog)
					availablestr[coremin].sort(sortperiod)
					oldprog = each
					flag = 1
					change = 1
					break

		if flag == 0:
			#return change
			break
		else:
			corestr[coremax].remove(oldprog)
			outinfo.remove(oldprog)
		#if change == 1:
		#	print coremax, coremin
		#	print corestat
		#	print corestr
		#	print outinfo
		#	return change
	#print '\t', availablestr, available
	for core in available:
		if availablestr[core] != []:
			availablestr[core].sort(sortperiod)
			for i in range(0, len(availablestr[core])):
				newprog = availablestr[core][i][0:-1]+[97-i, core, core]
				outinfo.append(newprog)
				corestr[core].append(newprog)
	#if change == 10:
	#	print outinfo
	return change


#clustered scheduling
#assign core to each task
#info: ['prog_name', period, util, worst_case, span]
#outinfo: ['prog_name', period, util, worst_case, span, *priority, *firstc, *lastc]
#option:
#0. original, no load balancing
#1. worst fit, no load balancing
#2. original + load balancing
#3. original + load balancing + 0.90 threshold
#4. original + load balancing + 0.95 threshold
####5. original + load balancing + 0.90 threshold + 0.95 threshold + ori
#5. original + 0.90 threshold
#6. for 36 (corenum = 36) + load balancing + 0.90 threshold
#7. 0.95 + 2*overhead
#8. 0.98 + overhead
#return sched
	#0: Guaranteed schedulable.
	#1: Not guaranteed schedulable, partition available, may try.
	#2: Not schedulable, no partition available.
##def cluster_partition(info, prognum, corenum, option):
####for overhead
def cluster_partition(overheadinfo, prognum, corenum, option):
	info = []
	for each in overheadinfo:
		info.append(copy.deepcopy(each[0:5]))
	overheadinfo.sort(sortutil)

	#sort util from high to low
	info.sort(sortutil)
	#print info
	outinfo = []

	#corestat: [[np(numprog), sumutil, mint], ]
	#store stat for RM partition
	#next fit ring: 
	#bound = np(rp^(1/np)-1)+2/rp-1
	#sumutil <= bound

	#corestr is the structure of tasks on a core
	#corestr: [[['prog_name', period, util, worst_case, span, *priority, *firstc, *lastc], ], ]

	#initialize corestat, corestr
	corestat = []
	corestr = []
	for m in range(0, corenum):
		corestat.append([0, 0.0, 0])
		corestr.append([])

	#if option == 7:
	#	option = 4
	#	for each in info:
	#		each[3] += 409600
			#each[3] += 819200
	#		each[4] *= 1.40
	#		each[2] = 1.0*each[3]/each[1]
	#	info.sort(sortutil)
	#print info

	#for 36#
	if option == 3 or option == 5 or option == 6 or option == 7:
		threshold = 0.9
	elif option == 4 or option == 8:
		threshold = 0.95
	elif option == 9:
		threshold = 0.98
	else:
		threshold = 1.0

	#low task start id
	lowid = len(info)
	#partition high util tasks
	partedcore = -1
	#remember possible cores from high util
	#instead of take the ceiling, take the floor
	#[[posscoreid, remaining_util, highid], ]
	possible = []
	for i in range(0, len(info)):
		period = info[i][1]
		work = info[i][3]
		span = info[i][4]
		util = info[i][2]
		#high util >= 1?
		if option == 7:
			threshold = 0.95 - 2.0*(100*1000.0)/period

		if util >= threshold:
			if partedcore+1 == corenum:
				#print "not core for high tasks", info
				return 2, [], []
			#number of cores assigned to high tasks
			#numcore = int(math.ceil(2*util))
			#number = (C-L)(P-L)
			tmp = 1.0*(work - span)/(period - span)
			#print work, span, period
			#print util, tmp
			numcore = int(math.ceil(tmp))
			if numcore == 1:
				numcore = 2
			#LJ#
			if option == 6:
				if numcore - tmp <= 0.02*(5+numcore):
					numcore+=1
#	while theslack < 50.0*1000*numcore*work/span and numcore-tmp<10:
#c_try	while theslack < (numseg*(8.2*numcore+130))*1000 and numcore-tmp<10:
###	while theslack <((15*work/span+10.5*numcore)*numseg+120)*1000.0 and numcore-tmp<10:
#12c	while theslack < 10.0*1000*math.log(numcore,2)*work/span and numcore-tmp<10:
##omp	while theslack < (8*25.0*work/span+120)*1000.0 and numcore-tmp<10:
			if option == 7 and exp_version == 1:
				theslack = period-span-1.0*(work - span)/(numcore*0.95)
				numseg = overheadinfo[i][5]
				while theslack < (200+32*math.log(numcore,2))*1000 and numcore-tmp<10:
					numcore+=1
					theslack = period-span-1.0*(work - span)/(numcore*0.95)
				if numcore-tmp>= 4:
					return 2, [], []
				print i, numcore
			elif option == 8 and exp_version == 1:
				theslack = period-span-1.0*(work - span)/(numcore)
				numseg = overheadinfo[i][5]
                        	while theslack < (20.0+3.2*math.log(numcore,2))*numseg*1000 and numcore-tmp<10:
					numcore+=1
					theslack = period-span-1.0*(work - span)/numcore
				if numcore-tmp>= 4:
					return 2, [], []
			elif option == 7 and exp_version == 0:
				theslack = period-span-1.0*(work - span)/(numcore*0.95)
				numseg = overheadinfo[i][5]
				while theslack <10000000 and numcore-tmp<10 and ((0.15*numcore+1.10)*(work/span/numcore)*numseg+(10.5*numcore+10)*numseg)*1000.0:
					numcore+=1
					theslack = period-span-1.0*(work - span)/(numcore*0.95)
				if numcore-tmp>= 4:
					return 2, [], []
			elif option == 8 and exp_version == 0:
				theslack = period-span-1.0*(work - span)/(numcore*0.95)
				numseg = overheadinfo[i][5]
				while theslack < 1000.0*((15*numcore+110)*(work/span/numcore)*numseg+(10.5*numcore+10)*numseg) and numcore-tmp<10:
					numcore+=1
					theslack = period-span-1.0*(work - span)/(numcore*0.95)
				if numcore-tmp>= 4:
					return 2, [], []
			elif option == 9 and exp_version == 0:
				return 2, [], []

			#print numcore, info[i]
			#put the possible core to posscore list
			if numcore > tmp and numcore > 2:
				possible.append([partedcore+numcore,numcore-tmp,i])
			newprog = info[i]+[97, partedcore+1, partedcore+numcore]
			outinfo.append(newprog)
			corestr[partedcore+1].append(newprog)
			partedcore += numcore
			#print partedcore, corenum
			#not core for high tasks
			if partedcore >= corenum:
				#print "not core for high tasks"
				return 2, [], []
		else:
			lowid = i
			break
	#print outinfo

	#for 36#
	if option == 3 or option == 5 or option == 6 or option == 7:
		threshold = 0.9
	elif option == 4 or option == 8:
		threshold = 0.95
	elif option == 9:
		threshold = 0.98
	else:
		threshold = 1.0

	partedcore += 1
	numlowcore = corenum - partedcore
	#print numlowcore
	#if len(info) == 1:
	#	print numlowcore, i, info, lowid, len(info)

	#print i+1, len(info)
	#if i+1 >= len(info):
	if lowid == len(info):
		#print 'no low'
		#for 36#
		if (option == 6 or option == 7) and corenum == 36:
			#print "36 cores, four sockets"
			#print corestr
			#print outinfo
			#print possible
			needchange = 0
			highinfo = []
			for eachhigh in outinfo:
				highinfo.append([eachhigh[7]-eachhigh[6]+1,eachhigh])
				if eachhigh[6] <= 11 and eachhigh[7] > 11:
					needchange = 1
				elif eachhigh[6] <= 23 and eachhigh[7] > 23:
					needchange = 1
			if needchange == 1:
				highinfo.sort(sorthigh)
				socket = [12, 12, 12]
				for eachhigh in highinfo:
					if eachhigh[0] <= socket[0]:
						socket[0] -= eachhigh[0]
					elif eachhigh[0] <= socket[1]:
						socket[1] -= eachhigh[0]
					elif eachhigh[0] <= socket[2]:
						socket[2] -= eachhigh[0]
					else:
						return 0, corestr, outinfo
				socket = [12, 12, 12]
				for eachhigh in highinfo:
					if eachhigh[0] <= socket[0]:
						sid = 0
						socket[0] -= eachhigh[0]
					elif eachhigh[0] <= socket[1]:
						sid = 1
						socket[1] -= eachhigh[0]
					elif eachhigh[0] <= socket[2]:
						sid = 2
						socket[2] -= eachhigh[0]
					newhigh = copy.deepcopy(eachhigh[1])
					newhigh[6] = (sid+1)*12-socket[sid]-eachhigh[0]
					newhigh[7] = (sid+1)*12-socket[sid]-1
					#print sid, socket[sid],newhigh
					#possible
					outinfo.remove(eachhigh[1])
					outinfo.append(newhigh)
					corestr[eachhigh[1][6]].remove(eachhigh[1])
					corestr[newhigh[6]].append(newhigh)
				#print highinfo
				#print outinfo
				#print corestr
			#for analysis#
			#return -10, corestr, outinfo
		return 0, corestr, outinfo
	if numlowcore == 0 and i < len(info):
		return 2, [], []

	#sort low tasks according to period
	lowinfo = info[lowid:len(info)]
	lowinfo.sort(sortperiod)
	#print lowinfo
	#scale low tasks
	tmax = lowinfo[-1][1]
	tmin = lowinfo[0][1]
	scale = 1.0*tmax/tmin
	if scale >= 2.0:
		for task in lowinfo:
			#Ti'=(ti-min)/(max-min) + 1
			period = task[1]
			t = 1.0*(period-tmin)/(tmax-tmin)+1
			task.append(t)
	else:
		for task in lowinfo:
			task.append(task[1])

	#for 36#
	fitsocket = 0
	if (option == 6 or option == 7) and corenum == 36:
		#print "36 cores, four sockets"
		#print corestr
		#print outinfo
		#print possible
		needchange = 0
		highinfo = []
		for eachhigh in outinfo:
			highinfo.append([eachhigh[7]-eachhigh[6]+1,eachhigh])
			if eachhigh[6] <= 11 and eachhigh[7] > 11:
				needchange = 1
			elif eachhigh[6] <= 23 and eachhigh[7] > 23:
				needchange = 1
		if needchange == 1:
			highinfo.sort(sorthigh)
			socket = [12, 12, 12]
			for eachhigh in highinfo:
				if eachhigh[0] <= socket[0]:
					socket[0] -= eachhigh[0]
				elif eachhigh[0] <= socket[1]:
					socket[1] -= eachhigh[0]
				elif eachhigh[0] <= socket[2]:
					socket[2] -= eachhigh[0]
				else:
					fitsocket = 1
			if fitsocket == 0:
				possible = []
				socket = [12, 12, 12]
				fitsocket = [0, 12, 24]
				for eachhigh in highinfo:
					if eachhigh[0] <= socket[0]:
						sid = 0
						socket[0] -= eachhigh[0]
					elif eachhigh[0] <= socket[1]:
						sid = 1
						socket[1] -= eachhigh[0]
					elif eachhigh[0] <= socket[2]:
						sid = 2
						socket[2] -= eachhigh[0]
					newhigh = copy.deepcopy(eachhigh[1])
					newhigh[6] = (sid+1)*12-socket[sid]-eachhigh[0]
					newhigh[7] = (sid+1)*12-socket[sid]-1
					#print sid, socket[sid],newhigh
					fitsocket[sid] = newhigh[7]+1
					tmpid = info.index(eachhigh[1][0:5])
					possible.append([newhigh[7],newhigh[2],tmpid])
					outinfo.remove(eachhigh[1])
					outinfo.append(newhigh)
					corestr[eachhigh[1][6]].remove(eachhigh[1])
					corestr[newhigh[6]].append(newhigh)
				#print highinfo
				#print outinfo
				#print corestr
		else: 
			fitsocket = 2

	#generate possible core id and program id list
	posscore = []
	highid = []
	if len(possible) > 0:
		possible.sort(sortposs)
		for each in possible:
			posscore.append(each[0])
			highid.append(each[2])

	#id of all lowcores
	lowcore = []
	for i in range(partedcore, corenum):
		lowcore.append(i)
	#for 36#
	if fitsocket != 0 and fitsocket != 1 and fitsocket != 2:
		#print "=========",fitsocket,"========="
		#print possible
		#print posscore
		#print highid
		lowcore = []
		for j in range(0, 3):
			for i in range(fitsocket[j], (j+1)*12):
				lowcore.append(i)
		#print "lowcore",lowcore
	#print "lowcore",lowcore

	if option == 0 or option == 5:
		sched = original(lowcore, lowinfo, posscore, highid, corestat, corestr, outinfo, threshold)
	elif option == 1:
		sched = worstfit(lowcore, lowinfo, posscore, highid, corestat, corestr, outinfo)
	#for 36#
	elif option == 2 or option == 3 or option == 4 or option == 6:
		sched = original(lowcore, lowinfo, posscore, highid, corestat, corestr, outinfo, threshold)
		#print corestat
		if sched == 0:
			info.sort(sortid)
			change = loadbalance(lowcore, info, posscore, highid, corestat, corestr, outinfo)
		#print corestat
			#if change == 10:
				#print '!',
			#	return -1, corestr, outinfo
		#if sched != 2:
		#	for core in corestat:
		#		if core[1] > 0.9:
		#			return -1, corestr, outinfo
	elif option == 7 or option == 8 or option == 9:
		sched = overhead(lowcore, lowinfo, posscore, highid, corestat, corestr, outinfo, threshold)

	#print "\t",outinfo
	if sched == 2:
		return sched, [], []
	#print sched, corestat
	#print outinfo
	#for 36#
	#for analysis#
	#if fitsocket != 0 and fitsocket != 1:
	#	if sched == 0:
	#		return -10, corestr, outinfo
	#	else:
	#		return -11, corestr, outinfo
	return sched, corestr, outinfo




#clustered scheduling
#generate needed info for assignment
#['prog_name', period, 'seg_id', sub_count, worst_case, rel_tm, rel_dead, *priority, [core]]
#option:
#0. original, no load balancing
#1. worst fit, no load balancing
#2. original + load balancing
#3. original + load balancing + 0.90 threshold
#4. original + load balancing + 0.95 threshold
#5. original + load balancing + 0.90 threshold + 0.95 threshold + ori
#6. for 36 (corenum = 36) + load balancing + 0.90 threshold
def cluster_assign_core(allsub, prognum, corenum, option):
	#output
	#info: ['prog_name', period, util, worst_case, span]
	info = []

	#sort according to name
	allsub.sort(sortname)
	#print '\t', allsub

	name = ''
	period = allsub[0][1]
	worstcase = 0
	span = 0
	
	#add the worst case
	for i in range(0, len(allsub)):
		#check if new program
		oldname = name
		name = allsub[i][0]
		if i != 0 and name != oldname:
			#add previous program to info
			util = 1.0*worstcase/period
			info.append([oldname, period, util, worstcase, span])
			#set period, set worstcase span to 0
			period = allsub[i][1]
			worstcase = 0
			span = 0
		#get ei, subcount value for each segment
		ei = allsub[i][4]
		subcount = allsub[i][3]
		worstcase += ei*subcount
		span += ei
	util = 1.0*worstcase/period
	info.append([name, period, util, worstcase, span])
	#print info

	#do partition
	if option != 5:
		(sched, corestr, outinfo) = cluster_partition(info, prognum, corenum, option)
	else:
		info1 = copy.deepcopy(info)
		info2 = copy.deepcopy(info)
		(sched, corestr, outinfo) = cluster_partition(info, prognum, corenum, 3)
		if sched != 0:
			#print info
			#print outinfo
			(sched1, corestr1, outinfo1) = cluster_partition(info1, prognum, corenum, 4)
			if sched1 == 0 or (sched == 2 and sched1 == 1):
			#if sched1 == 0 or (sched2 == 2 and sched1 < 2):
				#print 'sched1'
				if sched == 2:
					sched = 1
				corestr = corestr1
				outinfo = outinfo1
			elif sched1 == 2:
				(sched2, corestr2, outinfo2) = cluster_partition(info2, prognum, corenum, 2)
				if sched2 == 0 or (sched == 2 and sched2 == 1):
					#print 'sched2'
					if sched == 2:
						sched = 1
					corestr = corestr2
					outinfo = outinfo2
	return sched, corestr, outinfo


#simulate multiple jobs on single execution
#alljobs: ['prog_name', period, util, worst_case, span, *priority, *firstc, *lastc, release, deadline]
def single_simu(alljobs, hyper):
	time = 0
	nextreltm = hyper
	wait = []
	waitcount = 0
	finish = []
	flag = 1
	i = 0
	while i < len(alljobs):
		while i < len(alljobs) and alljobs[i][8] == time:
			#add new jobs to wait list
			wait.append(alljobs[i])
			i += 1
		if i < len(alljobs):
			nextreltm = alljobs[i][8]
		wait.sort(sortpriority)
	#	print 'wait', wait
		#execute high priority
	#	print 'time', time, 'nextreltm', nextreltm
		exetime = nextreltm - time
		for job in wait:
			work = job[3]
			if work < exetime-0.0000000001:
				exetime -= work
				time += work
				finish.append(job+[time])
				waitcount -= 1
				if waitcount==0:
					break
			elif work <= exetime+0.0000000001 and work >= exetime-0.0000000001:
			#elif work == exetime:
				time+=work
				finish.append(job+[time])
				waitcount-=1
				break
			else:
				job[3]-=exetime
				time += exetime
				break
	#	print 'finish', finish
		for job in finish:
			if job[9] < job[10]:
				#print 'alljobs',alljobs
				#print job
				return 1
			wait.remove(job[0:10])
		finish = []
		time = nextreltm
		nextreltm = hyper
	if len(wait) != 0:
		#print 'alljobs',alljobs
		#print 'wait', wait
		return 1
	#print 'success'
	return 0

#simulate the scheduling to check if it can really be scheduled
#only for harmonic tasks
#simulate high and low utilization seperately
	#if all can meet deadlines return 0; else return 1
def cluster_simulation(corestr, allsub):
	#corestr is the structure of tasks on a core
	#corestr: [[['prog_name', period, util, worst_case, span, *priority, *firstc, *lastc], ], ]

	#if cannot partition
	if len(corestr) == 0:
		#print 'len corestr == 0'
		return 1
	for core in corestr:
		if len(core) == 0:
			continue
		#single task on single core
		elif len(core) == 1 and core[0][6] == core[0][7]:
			if core[0][2] > 1:
				print '!!!impossible: high on single core'
				print corestr
			else:
				continue
		#if high util on multicore
		elif len(core) == 1 and core[0][6] < core[0][7]:
			#find high task structure
			#['prog_name', period, 'seg_id', sub_count, worst_case, rel_tm, rel_dead, *priority, [core]]
			high = []
			for sub in allsub:
				if sub[0] == core[0][0]:
					high.append(sub)
			high.sort(sortname)
			#print high
			numcore = core[0][7] - core[0][6]+1
		#	numcore = 12
			#calculate execution time
			exetime = 0
			for sub in high:
				segtime = int(math.ceil(1.0*sub[3]/numcore))
				exetime += segtime*sub[4]
				#print '   ',segtime
			#print core[0][0], exetime, core[0][1]
			if exetime > core[0][1]:
				tmp = 1.0*(core[0][3]-core[0][4])/(core[0][1]-core[0][4])
				if math.ceil(tmp) <= numcore:
					print core[0][1], exetime
					print '!!!impossible: high task unschedulable'
					print numcore, high
				else:
					print 'f'
				return 1
			#print '\t',exetime, core[0][1]
		else:
			#get the hyperperiod
			hyper = 0
			for prog in core:
				if prog != [] and prog[1] > hyper:
					hyper = prog[1]
			#print 'hyper', hyper
			#generate all the jobs
			alljobs = []
			for prog in core:
				if prog == []:
					continue
				period = prog[1]
				pround = hyper/period
				for i in range(0, pround):
					job = prog[:]+[i*period, (i+1)*period]
					alljobs.append(job)
			alljobs.sort(sortrelease)
			#print alljobs
			#simulate the execution
			if single_simu(alljobs, hyper) == 1:
				return 1
	return 0



