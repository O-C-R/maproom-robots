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

    self.session = boto3.Session(profile_name=profileName)
    self.s3 = self.session.client('s3')

  def start(self):
    self.stopped = False

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

      if not self.stopped:
        self.timer = Timer(self.interval, self.update)
        self.timer.start()
    except Exception as e:
      print(e)
    finally:
      os.remove(filename)

  def stop(self):
    self.stopped = True
    if self.timer != None:
      self.timer.cancel()
