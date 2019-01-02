#!/usr/bin/python
# -*- coding: UTF-8 -*-
import os
import sys
import subprocess
import time
import csv



res_name = 'data.txt'

fp = open(res_name, 'w')

with open('5.csv')  as f:
    reader = csv.reader(f)
    tmp = '0:'
    cnt = 1
    for  row in reader:
        if cnt % 10 > 0:
            tmp += ''.join(row[0:9])
        else:
            fp.write(tmp+'\n')
            tmp = str(cnt/10) + ':'
        cnt += 1


fp.close()