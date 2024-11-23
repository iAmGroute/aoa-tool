FROM debian:12-slim
RUN  apt-get -y update
RUN  apt-get -y install --no-install-recommends libsdl2-2.0-0 libusb-1.0-0
COPY main.out /usr/bin/aoa-tool
