# With credit to https://github.com/jrosebr1/imutils

from threading import Thread
import cv2
from datetime import datetime
import math

from . import constants as c
from .fps import FPS

class VideoStream:
  def __init__(self, src=0, resolution=(800,600)):
    self.src = src
    self.stream = cv2.VideoCapture(src)
    self.stream.set(cv2.CAP_PROP_FRAME_WIDTH,resolution[0])
    self.stream.set(cv2.CAP_PROP_FRAME_HEIGHT,resolution[1])
    (self.grabbed, self.frame) = self.stream.read()
    self.gray = None

    self.mapx = None
    self.mapy = None
    self.perspectiveTransform = None
    self.grayUndistorted = None

    self.frameTime = None
    self.stopped = False

  def setDistortion(self, mapx, mapy, perspectiveTransform):
    self.mapx = mapx
    self.mapy = mapy
    self.perspectiveTransform = perspectiveTransform

  def setFrameSync(self, fps):
    if fps < 0:
      self.frameTime = None
    else:
      self.frameTime = 1000000 / fps

  def start(self):
    t = Thread(target=self.update, args=())
    t.daemon = True
    t.start()
    return self

  def update(self):
    fps = FPS()
    lastFrameNum = 0
    while True:
      if self.stopped:
        return

      # Enable frame sync if asked
      if self.frameTime is not None:
        now = datetime.now()
        frameNum = math.floor(now.microsecond / self.frameTime)
        if frameNum != lastFrameNum:
          lastFrameNum = frameNum
        else:
          continue

      (self.grabbed, self.frame) = self.stream.read()

      # Convert to grayscale always
      self.gray = cv2.cvtColor(self.frame, cv2.COLOR_BGR2GRAY)

      # If asked, undistort the gray
      g = self.gray
      # if self.mapx is not None and self.mapy is not None:
      #   g = cv2.remap(g, self.mapx, self.mapy, cv2.INTER_LINEAR)
      if self.perspectiveTransform is not None:
        g = cv2.warpPerspective(g, self.perspectiveTransform, c.mappedImageResolution)
      self.grayUndistorted = g

      p, f, _ = fps.update()
      if p:
        print("Camera", self.src, "capturing at", f, "fps")

  def read(self):
    return self.frame, self.gray, self.grayUndistorted

  def stop(self):
    self.stopped = True
