import os
import shutil
import pathlib
import glob
from PIL import Image
import numpy 
from numpy import asarray
import csv
dataset_path = 'C:/Users/duboisrouvray/Documents/STAGE_ELSYS_2022/SHIPSNET_DATASET/test'
path_boat = dataset_path+"/bateau"
path_not_boat = dataset_path+"/pas_bateau"
path_boat = pathlib.Path(path_boat)
path_not_boat = pathlib.Path(path_not_boat)
print(path_boat)
print(path_not_boat)
nb_boat = len(list(path_boat.glob('*')))
nb_not_boat = len(list(path_not_boat.glob('*')))
print(nb_boat)
print(nb_not_boat)
f = open('C:/Users/duboisrouvray/Documents/STAGE_ELSYS_2022/SHIPSNET_DATASET/shipsnet_test.csv', 'w', newline='\n')
writer = csv.writer(f)
i=0
# firstLine = ['label', 'pxR1','pxV1','pxB1','pxR2','pxV2', 'pxB2', '...']
# writer.writerow(firstLine)
for i in range(nb_boat) :
    label = 1
    image = Image.open('C:/Users/duboisrouvray/Documents/STAGE_ELSYS_2022/SHIPSNET_DATASET/test/bateau/bateau{}.jpg'.format(i+899))
    image = asarray(image)
    strBuilder = []
    strBuilder.append(str(label))
    for n in range(224) :
        for m in range(224) :
            for c in range(3) :
                strBuilder.append(str(image[n][m][c]))
    writer.writerow(strBuilder)



i=0
for i in range(nb_not_boat) :
    label = 0
    image = Image.open('C:/Users/duboisrouvray/Documents/STAGE_ELSYS_2022/SHIPSNET_DATASET/test/pas_bateau/bateau{}.jpg'.format(3599+i))
    image = asarray(image)
    strBuilder = []
    strBuilder.append(str(label))
    for n in range(224) :
        for m in range(224) :
            for c in range(3) :
                strBuilder.append(str(image[n][m][c]))
    writer.writerow(strBuilder)
f.close()