import sys
import random

def gen(n):
    m = int(n) / 3
    f = open("input", "w")
    f.write(str(m * 3))
    f.write("\n")
    for i in range(m):
        r = random.randrange(0, 256, 1);
        g = random.randrange(0, 256, 1);
        b = random.randrange(0, 256, 1);
        f.write(str(r))
        f.write(" ")
        f.write(str(g))
        f.write(" ")
        f.write(str(b))
        f.write("\n")
    f.close()

if __name__ == "__main__":
    
    if len(sys.argv) < 2:
        print "Usage: python data.py num\n"
        sys.exit()

    gen(sys.argv[1]);
