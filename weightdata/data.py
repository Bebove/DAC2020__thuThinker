import numpy as np
import pdb

olocation="./w1/307.npy"
dlocation="./w1/307.bin"

weight=np.load(olocation)
with open(dlocation , 'wb') as fd :
    fd.write(weight)
