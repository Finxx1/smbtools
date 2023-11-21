import os
import sys

file = open(sys.argv[1], "rb")

filedata = file.read()

file.close()

texturecount = int.from_bytes(filedata[0:4], "little")

print(str(texturecount) + " textures in Texture Package")

offsets = []
sizes = []


for i in range(texturecount):
    offsets.append(int.from_bytes(filedata[i * 17 + 4:i * 17 + 4 + 4], "little"))
    sizes.append(int.from_bytes(filedata[i * 17 + 8:i * 17 + 8 + 4], "little"))
    
print("Offsets: " + str(offsets))
print("Sizes:   " + str(sizes))

try:
    os.mkdir("_" + sys.argv[1])
    os.chdir("_" + sys.argv[1])
except:
    os.chdir("_" + sys.argv[1])

for i in range(texturecount):
    file = open(str(i) + ".png", "wb+")
    
    file.write(filedata[offsets[i]:offsets[i]+sizes[i]])
    
    file.close()