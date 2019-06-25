import struct
import numpy as np

def floatToHex(f):
    return hex(struct.unpack('<I', struct.pack('<f', f))[0])[2:]

def hexToFloat(f):
    return struct.unpack('!f', bytes.fromhex(f))[0]


oFileA = open("testBRAM_A.coe", 'w')
oFileB = open("testBRAM_B.coe", 'w')
oFileRes = open("testResults.txt", 'w')
# pre-text 

oFileA.write("""memory_initialization_radix=16;
memory_initialization_vector=\n""")
oFileB.write("""memory_initialization_radix=16;
memory_initialization_vector=\n""")

for (a,b) in zip(np.arange(1.0,2.0,0.1), np.arange(0.0,1.0,0.1)):
    oFileA.write(floatToHex(a) + ",\n")
    oFileB.write(floatToHex(b) + ",\n")
    oFileRes.write(floatToHex(a) + ' ' + floatToHex(b) + ' ' + floatToHex(a/b) + '\n')

oFileA.write(floatToHex(0.0) + ';\n')
oFileB.write(floatToHex(0.0) + ';\n')

oFileA.close()
oFileB.close()
oFileRes.close()