# With credit to https://github.com/jrosebr1/imutils

from threading import Thread
import cv2

class VideoStream:
  def __init__(self, src=0, resolution=(800,600)):
    self.stream = cv2.VideoCapture(src)
    self.stream.set(cv2.CAP_PROP_FRAME_WIDTH,resolution[0])
    self.stream.set(cv2.CAP_PROP_FRAME_HEIGHT,resolution[1])
    (self.grabbed, self.frame) = self.stream.read()

    self.stopped = False

  def start(self):
    t = Thread(target=self.update, args=())
    t.daemon = True
    t.start()
    return self

  def update(self):
    while True:
      if self.stopped:
        return

      (self.grabbed, self.frame) = self.stream.read()

  def read(self):
    return self.frame

  def stop(self):
    self.stopped = True
