import os
import sys
import struct
from PIL import Image

file = open(sys.argv[1], "rb")

filedata = file.read()

file.close()

image = Image.open(sys.argv[2])

itxs = struct.unpack('f' * int(len(filedata) / 4), filedata)

w = image.width
h = image.height


objs = int(len(itxs) / 5)

for i in range(objs):
    cropsize = (int(itxs[i * 5] * w), int(itxs[i * 5 + 1] * h), int((itxs[i * 5 + 2] + itxs[i * 5]) * w), int((itxs[i * 5 + 3] + itxs[i * 5 + 1]) * h))
    rotation = itxs[i * 5 + 4]
    try:
        ci = image.crop(cropsize)
        ci = ci.rotate(rotation * (180.0/3.14159265), expand=True)
        ci.save(str(i) + ".png")
    except:
        continue
    
