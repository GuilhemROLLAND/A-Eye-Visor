from msilib.schema import File
import random # for random value
import numpy as np # linear algebra
import matplotlib
import pandas as pd # data processing, CSV file I/O (e.g. pd.read_csv)
import matplotlib.pyplot as plt # plotting 
from PIL import Image # Image processing
import json  #json file I/O
from mpl_toolkits.basemap import Basemap
import torch
from torch import nn
from torch.utils.data import DataLoader
from torchvision import datasets
from torchvision.transforms import ToTensor
from torch.utils.data import Dataset
import os
from torchvision.io import read_image
from torchvision.io import decode_image
from torch.utils.data import DataLoader
import time
from PIL import Image
import PIL
import pathlib
import csv, shutil
import pandas as pd
import glob

def saveAsJPEG(array,pos):
    pixels = np.asarray(array['data'][pos])
    pix = pixels.reshape((3,6400))
    pix_T = pix.T
    imge = pix_T.reshape((224,224,3)).astype('uint8')
    img = Image.fromarray(imge)
    imgJPEG = img.save('bateau{}.jpg'.format(pos))

# Get images that have ships
df = pd.read_csv('C:/Users/duboisrouvray/Documents/STAGE_ELSYS_2022/dataset_airbus/labels.csv')
has_ship = [isinstance(_, str) for _ in df['EncodedPixels']]  # true if has ship
df_with_ships = df.loc[has_ship]
print(df_with_ships.index)
print(df_with_ships.at[2,'ImageId'])
print('n with ships:', np.sum(has_ship))
print('n without ships:', len(has_ship)-np.sum(has_ship))


# for n in (df_with_ships.index) : 
    # shutil.copyfile('C:/Users/duboisrouvray/Documents/STAGE_ELSYS_2022/dataset_airbus/{}'.format(df.at[n,'ImageId']),'C:/Users/duboisrouvray/Documents/STAGE_ELSYS_2022/dataset_airbus/delete/{}'.format(df.at[n,'ImageId']))
    # try :
    #     shutil.move('C:/Users/duboisrouvray/Documents/STAGE_ELSYS_2022/dataset_airbus/{}'.format(df.at[n,'ImageId']),'C:/Users/duboisrouvray/Documents/STAGE_ELSYS_2022/dataset_airbus/bateau/{}'.format(df.at[n,'ImageId']))
    # except :
    #     pass

# for jpgfile in glob.iglob(os.path.join('C:/users/duboisrouvray/Documents/STAGE_ELSYS_2022/dataset_airbus', "*.jpg")):
#     shutil.move(jpgfile, 'C:/users/duboisrouvray/Documents/STAGE_ELSYS_2022/dataset_airbus/pas_bateau')

# shutil.rmtree('C:/Users/duboisrouvray/Documents/STAGE_ELSYS_2022/dataset_airbus/delete/')
path = pathlib.Path('C:/Users/duboisrouvray/Documents/STAGE_ELSYS_2022/dataset_airbus/bateau')
size = (240,240)
# for i,file in enumerate(glob.iglob(os.path.join('C:/users/duboisrouvray/Documents/STAGE_ELSYS_2022/dataset_airbus/bateau', "*.jpg"))):
#     img = Image.open(r'{}'.format(file))
#     img = img.resize(size)
#     img.save('{}'.format(file))
for i,file in enumerate(glob.iglob(os.path.join('C:/users/duboisrouvray/Documents/STAGE_ELSYS_2022/dataset_airbus/pas_bateau', "*.jpg"))):
    img = Image.open(r'{}'.format(file))
    img = img.resize(size)
    img.save('{}'.format(file))


"""
csvfile = open('labels.csv')
reader = csv.reader(csvfile)
next(reader)
for row in reader:
    if int(row[1]):
        shutil.copyfile("./224x224/" + row[0] + ".jpg", "./224x224/bateau/" + row[0] + ".jpg")
    else:
        shutil.copyfile("./224x224/" + row[0] + ".jpg", "./224x224/pas_bateau/" + row[0] + ".jpg")"""