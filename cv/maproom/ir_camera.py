from threading import Thread
import cv2

class IRCamera:
  def __init__(self, irCam, regCam):
    self.irCam = irCam
    self.regCam = regCam

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

      ir = self.irCam.undistort(gray=True)
      reg = self.regCam.undistort(gray=True)

      diff = cv2.absdiff(irHSV, regHSV)

      # TODO: finish

  def read(self):
    return self.frame

  def stop(self):
    self.stopped = True
