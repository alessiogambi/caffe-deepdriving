FROM ubuntu:16.04 AS Builder
LABEL maintainer caffe-maint@googlegroups.com

# Install original caffe dependencies, dependencies from Andre' Netzeband implementation, and VNC server
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        git \
        curl \
        libatlas-base-dev \
        libboost-all-dev \
        libgflags-dev \
        libgoogle-glog-dev \
        libhdf5-serial-dev \
        libleveldb-dev \
        liblmdb-dev \
        libopencv-dev \
        libprotobuf-dev \
        libsnappy-dev \
        protobuf-compiler \
        python-dev \
        python-numpy \
        python-pip \
        python-setuptools \
        python-scipy \
        libplib-dev \
        libopenal-dev \
        libalut-dev \
        libvorbis-dev \
        libxxf86vm-dev \
        libxmu-dev \
        libxrender-dev \
        libxrandr-dev \
        libz-dev \
        libpng-dev \
   	x11vnc \
        xvfb

RUN curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh | bash

RUN apt-get install git-lfs && git lfs install

ENV CAFFE_ROOT=/caffe-deepdriving

# Clone the latest commit from the repo

RUN git clone --depth 1 https://github.com/alessiogambi/caffe-deepdriving.git ${CAFFE_ROOT}

WORKDIR ${CAFFE_ROOT}

RUN mkdir -p ${CAFFE_ROOT}/build/linux-64-cpu/release && \
        mkdir -p ${CAFFE_ROOT}/install/linux-64-cpu/release

WORKDIR ${CAFFE_ROOT}/build/linux-64-cpu/release

RUN ls

RUN cmake -DCPU_ONLY=1 \
                -DCMAKE_INSTALL_PREFIX=../../../install/linux-64-cpu/release \
                -DCMAKE_BUILD_TYPE=Release \
                        ../../.. && \
    make && \
    make install

ENV DEEPDRIVING_CAFFE_PATH=${CAFFE_ROOT}/install/linux-64-cpu/release

# Note this is hardcoded
EXPOSE 5901

WORKDIR ${CAFFE_ROOT}/torcs

# Start the servers: virtual display, vncserver, and torcs_run.sh 1 command
COPY startup.sh .

CMD ["./startup.sh"]
