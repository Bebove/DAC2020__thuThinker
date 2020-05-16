import numpy as np
import pdb

olocation="./w1/320.npy"
dlocation="./w1/320.bin"

weight=np.load(olocation)
with open(dlocation , 'wb') as fd :
    fd.write(weight)
