import sys
import random

def gen(n):
    f = open("input", "w")
    f.write(str(n * 3))
    f.write("\n")
    for i in range(n):
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

    gen(int(sys.argv[1]));
