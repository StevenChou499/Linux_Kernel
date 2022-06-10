#!/usr/bin/python

import sys

# print(sys.argv[1])

result = []
result_split = []

with open(sys.argv[1], 'r') as f:
    tmp = f.readline()
    while (tmp):
        result.append(tmp)
        tmp = f.readline()
    f.close()

result_split = result[0].split()
total = int(result_split[-1]) + 1

for i in range(0, total):
    # print('i =', i, ', result_split[', i, '] =', result_split[i], '\n')
    if(int(i) != int(result_split[i])):
        print('%s failed\n' % str(i))
        print('output: %s' % str(result_split[i]))
        print('expected: %s' % str(i))
        exit()
    
print("for file " + sys.argv[1] + ", everything is correct!")