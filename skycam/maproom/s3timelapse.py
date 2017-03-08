from threading import Timer
from datetime import datetime
import os

import cv2
import boto3

from . import constants as c

class S3Timelapse:
  def __init__(self, camera, interval=c.timelapseCaptureTime, profileName=c.timelapseProfile, bucket=c.timelapseBucket):
    self.camera = camera
    self.bucket = bucket
    self.interval = interval
    self.timer = None
    self.running = False

    self.session = boto3.Session(profile_name=profileName)
    self.s3 = self.session.client('s3')

  def start(self):
    self.running = True

    self.timer = Timer(self.interval, self.update)
    self.timer.start()

    return self

  def update(self):
    frame, gray = self.camera.lastFrame()

    now = datetime.now()
    filename = now.strftime('%Y%m%d-%H%M%S-%f.png')

    try:
      cv2.imwrite(filename, frame)
      self.s3.upload_file(filename, self.bucket, filename)
      print('Uploaded ' + filename)
    except Exception as e:
      print(e)
    finally:
      os.remove(filename)

    if self.running:
      self.timer = Timer(self.interval, self.update)
      self.timer.start()

  def stop(self):
    self.running = False
    if self.timer is not None:
      self.timer.cancel()

  def isRunning(self):
    return self.running
