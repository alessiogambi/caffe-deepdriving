The cpu folder contains a basic docker file which builds *this* code (i.e., it checksout the latest commit of this git repo) into a docker container.

This repo contains a patched version of the code provided by Andre Netband. The patch enables communication with the AI using named pipes.

The input pipe is used by the AI to receive the frame and the current speed value, using a basic protocol:
= Speed value, size of the image in bytes, bytes of the image as RGB-3 channel

The output pips is used by the AI to output driving commands, using a basic protocol:
= value for Acc (0-1), value for brake (0-1), value for steering angle (-1,+1)

To build the container run:

```docker build -t deepdriving-caffe-cpu-single:1.0 .```

This will take a while since there are plenty of dependencies to download. Many of them might not be needed in practice, by I cannot tell which one :D

Once the container is ready, the following commands enable to run it:

```
mkdir pipes
mkfifo pipes/in
mkfifo pipes/out

docker run -v $(pwd)/pipes:/pipes -t deepdriving-caffe-cpu-single:1.0
```

If you wish to "see" what happens inside the docker container you must forward the 5901 port of the container and connect to it using VNC

For example the following code enable to access the container on the port 5903.
```
docker run -v -p 5903:5901 $(pwd)/pipes:/pipes -t deepdriving-caffe-cpu-single:1.0
```

At this point, assuming you are on a Linux platform, whatever you send to pipes/in will be read by the AI,
and whatever the Ai writes can be read from pipes/out.

A simple pyhon code that illustrates how to communicate with the running container can be found under then 
python folder in this repo.
