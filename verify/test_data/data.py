import numpy as np
import pdb

def tran():
    olocation="./modle_param/l2b.npy"
    dlocation="./modle_param/l2b.bin"

    weight=np.load(olocation)
    with open(dlocation , 'wb') as fd :
        fd.write(weight)

img1=np.load("./conv_2/out1_3.npy")
img2=np.load("./conv_2/out2_3.npy")
img3=np.load("./conv_2/out3_3.npy")
img4=np.load("./conv_2/out4_3.npy")

l=len(img1.shape)
imgsize=(img1.shape[l-3],img1.shape[l-2],img1.shape[l-1])

img1=np.pad(np.reshape(img1,imgsize),((0,0),(1,1),(1,1)),'constant')
img2=np.pad(np.reshape(img2,imgsize),((0,0),(1,1),(1,1)),'constant')
img3=np.pad(np.reshape(img3,imgsize),((0,0),(1,1),(1,1)),'constant')
img4=np.pad(np.reshape(img4,imgsize),((0,0),(1,1),(1,1)),'constant')

channel=img1.shape[0]
high=img1.shape[1]
width=img1.shape[2]

img=np.zeros((channel,high*2,width*2))

for i in range(channel):
    img_t1=np.c_[img1[i],img2[i]]
    img_t2=np.c_[img3[i],img4[i]]
    img[i]=np.r_[img_t1,img_t2]

img=img.reshape((-1,))
imgpath='./data/conv_2.bin'
print('375: ',img[375])
print('784: ',img[784])
print('972: ',img[972])
print('1023: ',img[1023])
print('5431: ',img[5431])
print('8920: ',img[8920])
print('11021: ',img[11021])
print('80202: ',img[80202])
print('112342: ',img[112342])
print('248222: ',img[248222])
'''
with open(imgpath , 'wb') as fd :
    fd.write(img)
'''