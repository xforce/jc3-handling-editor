# -*- coding: utf-8 -*-
import sys
import glob
import os
import glob2

def DoMain(argv):
    meow = []
    for f in glob2.glob(argv[0]):
        # gyp needs either double backslashes (\\) or forward slashes (/) as path separator
        # go with forward slashes to be platform independent
        f = f.replace('\\', '/')
        meow.append(f)
    return " ".join(['"%s"' % x for x in meow])
  
  
if __name__ == '__main__':
    results = DoMain(sys.argv[1:])
    if results:
        print results