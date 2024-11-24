FROM debian:bookworm-slim

RUN  apt-get -y update
RUN  apt-get -y install --no-install-recommends libsdl2-2.0-0
RUN  apt-get -y install --no-install-recommends libusb-1.0-0

COPY main.out /usr/bin/aoa-tool
