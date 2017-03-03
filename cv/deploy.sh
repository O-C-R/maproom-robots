#!/bin/bash

set -ex

PI_HOST=192.168.7.52

scp ./*.py pi@$PI_HOST:~/cv/
scp markers.service pi@$PI_HOST:~/markers.service
ssh pi@$PI_HOST "set -ex && \
  sudo mv markers.service /lib/systemd/system/markers.service && \
  sudo systemctl disable markers.service && \
  sudo systemctl daemon-reload && \
  sudo systemctl enable markers.service && \
  sudo systemctl reload-or-restart markers.service"
