import numpy as np
import pdb

olocation="./w1/313bs.npy"
dlocation="./w1/313bs.bin"

weight=np.load(olocation)
with open(dlocation , 'wb') as fd :
    fd.write(weight)
