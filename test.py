from tst import *

t = TST()

o = '1'*10000

t['a']=o

print t['a']=='1'*10000
print t['a']=='1'*10000

print t.get('a')=='1'*10000