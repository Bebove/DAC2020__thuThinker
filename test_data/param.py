import numpy as np
import pdb
import os

def get_file(filepath):
    file_list=[]
    for root, _, files in(os.walk(filepath)):
        for f in files:
            (name,expand)=os.path.splitext(f)
            if expand =='.npy':
                file_list.append(os.path.join(root,name).replace('\\','/'))
            #file_list.append(f)
    return file_list


def tran(param):
    olocation=param+".npy"
    dlocation=param+".bin"

    weight=np.load(olocation)
    with open(dlocation , 'wb') as fd :
        fd.write(weight)

file_list=get_file('./w1')

for p in file_list:
    tran(p)
#tran()