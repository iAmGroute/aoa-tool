FROM python:3-slim-bookworm

RUN  apt-get -y update
RUN  apt-get -y install --no-install-recommends build-essential
RUN  apt-get -y install --no-install-recommends libsdl2-dev
RUN  apt-get -y install --no-install-recommends libusb-1.0-0-dev
RUN  apt-get -y install --no-install-recommends ninja-build

RUN  python3 -m pip install -U meson

ENTRYPOINT []
CMD [ "/bin/bash" ]
