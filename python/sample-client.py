#!/usr/bin/env python3
import sys
import signal
from PIL import Image
from PIL import ImageOps
import os
from os.path import basename
import time
import numpy as np
import math
import json

# Basic parsing of arguments
print ("This is the name of the script: ", sys.argv[0])
print ("Number of arguments: ", len(sys.argv))
print ("The arguments are: " , str(sys.argv))

# We send images to in/fifo
input_fifo_name = sys.argv[1]
# We read commands from out/control_commands_pipe
control_commands_pipe = sys.argv[2]
# We take images and data from the following folder
dir = sys.argv[3]

size = 640, 480

def main():
    try:
        os.mkfifo(input_fifo_name)
        os.mkfifo(control_commands_pipe)
    except FileExistsError:
        pass

    with open(input_fifo_name, 'wb') as pipe, open(control_commands_pipe, 'rt') as control_pipe:

        def signal_handler(*args):
            print('You pressed Ctrl+C!')
            pipe.write('-1\n'.encode())
            pipe.close()
            control_pipe.close()
            print('closing pipes')
            sys.exit(0)

        signal.signal(signal.SIGINT, signal_handler)

        for filename in sorted([os.path.join(dir, file) for file in os.listdir(dir)]):
            if filename.endswith("_screen.png"):
                jsonFile = filename[:-len('_screen.png')] + ".json"
                print ("Reading file", jsonFile)

                with open(jsonFile) as f:
                    data = json.load(f)
                    # Input speed is m/s
                    speed = math.sqrt( data["vel"][0]*data["vel"][0]+data["vel"][1]*data["vel"][1] );
                    print("Send speed", speed)

                with Image.open(filename).convert('RGB') as imageFile:
                    #print ("Reading file", filename)
                    #print ("Original size", imageFile.size)
                    imageFile = imageFile.resize(size);
                    print ("New size", imageFile.size)
                    # Flip the image vertically because BMP are stored that way...
                    imageFile = ImageOps.flip(imageFile)
                    b = imageFile.tobytes()

                    #print("Send speed", speed)
                    pipe.write('{}\n'.format(speed).encode())

                    print ("Send Image", filename)
                    pipe.write('{}\n'.format(len(b)).encode())
                    pipe.write(b)

                # Ensure not faster than max freq.
                # Assume this is enough to get the next control data.
                time.sleep(1.0)
                print("Read driving commands")
		# Read from the control pipe. Note this is not blocking
                line = control_pipe.readline(); #read()
                print("Accel", line)
                line = control_pipe.readline()
                print("Brake", line)
                line = control_pipe.readline()
                print("Steer", line)
            else:
                continue

if __name__ == '__main__':
    main()
