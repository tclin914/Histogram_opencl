import subprocess
import time

def run_serial():
    subprocess.call(["../serial/image-histogram"]);

def run_opencl():
    subprocess.call(["../opencl/histogram"]);

def run_diff():
    subprocess.call(["diff", "../serial/0356100.out", "../opencl/0356100.out"]);

def run():

    start_time1 = time.time()
    run_serial()
    end_time1 = time.time()

    start_time2 = time.time()
    run_opencl()
    end_time2 = time.time()

    print "opencl output diffs with serial output"
    run_diff()
    print "serial program run %s seconds" % (end_time1 - start_time1)
    print "opencl program run %s seconds" % (end_time2 - start_time2)

if __name__ == '__main__':
    run()
