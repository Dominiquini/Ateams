#!/usr/bin/env python

import math
import sys

arq = []

for i in range(1,41):
    if(i < 10):
        arq += [open(sys.argv[1] + "la0" + str(i) + ".res")]
    else:
        arq += [open(sys.argv[1] + "la" + str(i) + ".res")]

for k in range (0, 40):
    num = 0
    sum = 0
    min = 0
    max = 0
    
    for line in arq[k]:
        val = float(line.split("\t")[0])
        
        num += 1
        sum += val

        if min == 0 or min > val:
            min = val

        if max == 0 or max < val:
            max = val

    med = sum/num
    print k+1, ":", med, ":", min, ":", max

    arq[k].close()
