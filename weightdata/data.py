import numpy as np
import pdb

olocation="./w1/307bs.npy"
dlocation="./w1/307bs.bin"

weight=np.load(olocation)
with open(dlocation , 'wb') as fd :
    fd.write(weight)
