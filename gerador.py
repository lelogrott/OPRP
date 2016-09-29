import random
import numpy as np

a=[]
NLINHAS=100
NCOLUNAS=100
print "%i %i"%(NLINHAS+1, NCOLUNAS + NLINHAS +1)
for i in range(0,NLINHAS):
	a.append(random.sample(range(0,NCOLUNAS+1), NCOLUNAS))
	if(i==0):
		a[i].append(1)
		a[i]+=[int(k) for k in np.zeros(NLINHAS-1)]
	else:
		a[i]+=([int(k) for k in np.zeros(i)])
		a[i].append(1)
		a[i]+=([int(k) for k in np.zeros(NLINHAS-i-1)])


for i in range(0,NLINHAS):
	a[i].append(random.randint(1,50))


a.append([-1 * int(x) for x in random.sample(range(1,NCOLUNAS+1), NCOLUNAS)]+[int(k) for k in np.zeros(NLINHAS+1)])

for x in a:
	print(" ".join([str(i) for i in x]))

