import numpy as np
import pdb
import os

def tran():
    olocation="./w1/323bs.npy"
    dlocation="./w1/323bs.bin"

    weight=np.load(olocation)
    with open(dlocation , 'wb') as fd :
        fd.write(weight)

def get_file(filepath):
    file_list=[]
    for root, _, files in(os.walk(filepath)):
        for f in files:
            if os.path.splitext(f)[1]=='.npy':
                file_list.append(os.path.join(root,f).replace('\\','/'))
            #file_list.append(f)
    return file_list
pdb.set_trace()
source_path='./conv5' #the folder of your .npy files, one folder contains 4 images
save_imgpath='./data/conv5.bin' #bin file to be saved

file_list=get_file(source_path)

img1=np.load(file_list[0])
img2=np.load(file_list[1])
img3=np.load(file_list[2])
img4=np.load(file_list[3])

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


'''
print(img[644*90+120])
print(img[644*90+120+644*388])
print(img[644*90+120+2*644*388])
print(img[644*90+120+3*644*388])
print(img[644*90+120+4*644*388])
print(img[644*90+120+5*644*388])

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

with open(save_imgpath , 'wb') as fd :
    fd.write(img)
