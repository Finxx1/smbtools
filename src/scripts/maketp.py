import sys

def intb(a):
    return int.to_bytes(a, 4, 'little')

length = len(sys.argv)-2

file = open(sys.argv[1], "wb")
file.write(intb(length))

off = 0

for arg in sys.argv[2:]:
    tfile = open(arg, "rb")
    file.write(intb(4 + 17 * length + off))
    tfile.seek(0,2)
    file.write(intb(tfile.tell()))
    off += tfile.tell()
    tfile.seek(0,0)
    file.write(int.to_bytes(0, 9, 'little'))
    
for arg in sys.argv[2:]:
    tfile = open(arg, "rb")
    file.write(tfile.read())