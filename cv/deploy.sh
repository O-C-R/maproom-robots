#!/bin/bash

set -ex

scp markers.service pi@192.168.1.52:~/markers.service
ssh pi@192.168.1.52 "set -ex && \
  sudo mv markers.service /lib/systemd/system/markers.service && \
  sudo systemctl disable markers.service && \
  sudo systemctl daemon-reload && \
  sudo systemctl enable markers.service && \
  sudo systemctl reload-or-restart markers.service"
