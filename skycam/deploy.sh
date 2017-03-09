#!/bin/bash

set -ex

PI_HOST=192.168.7.52

ssh pi@$PI_HOST "set -ex && \
  mkdir -p ~/cv/maproom && \
  mkdir -p ~/systemd"
scp ./*.py pi@$PI_HOST:~/cv/
scp ./maproom/*.py pi@$PI_HOST:~/cv/maproom/
scp ./systemd/* pi@$PI_HOST:~/systemd
ssh pi@$PI_HOST "set -ex && \
  sudo mv ~/systemd/* /lib/systemd/system/ && \
  sudo systemctl disable markers.service && \
  sudo systemctl daemon-reload && \
  sudo systemctl enable markers.service && \
  sudo systemctl reload-or-restart markers.service"
