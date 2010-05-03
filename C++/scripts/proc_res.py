#!/usr/bin/env python

import math
import sys

arq = []
num = []

for i in range(1,41):
    if(i < 10):
        arq += [open(sys.argv[1] + "la0" + str(i) + ".res")]
    else:
        arq += [open(sys.argv[1] + "la" + str(i) + ".res")]

for k in range (0, 40):
    y = 0
    for line in arq[k]:
        x = line.split("\t")
        num.append(float(x[0]))
        y += float(x[0])

    med = y/len(num)
    print k+1, ":", med

    num = []

    arq[k].close()
