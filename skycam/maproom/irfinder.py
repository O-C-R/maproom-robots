from threading import Thread

import cv2
import numpy as np
import math

from .camera import MaproomCamera
from .fps import FPS
from . import constants as c
from . import util as u

MIN_CONTOUR_AREA = 500

class IRFinder:
  def __init__(self, irCam, clientIPs=[], clientPort=5201):
    self.irCam = irCam
    self.clients = u.clientsFromIPs(clientIPs, clientPort)
    self.running = False

    self.avgPos = np.array([0,0], np.float32)
    self.normAvgPos = np.array([0,0], np.float32)

  def start(self):
    self.running = True
    self.irCam.setPreundistort(True)

    t = Thread(target=self.update, args=(), daemon=True)
    t.start()
    return self

  def findCircle(self, ir, draw=False):
    irROI = ir[c.mappedImageUL[0]:c.mappedImageBR[0], c.mappedImageUL[1]:c.mappedImageBR[1]]
    irROI = cv2.blur(irROI, (25,25))

    minVal, maxVal, minLoc, maxLoc = cv2.minMaxLoc(irROI)

    pos = np.array(maxLoc, np.float32)
    normPos = pos / np.array(irROI.shape, np.float32)
    ret = None
    if maxVal > 25:
      self.avgPos += (pos - self.avgPos) * 0.3
      self.normAvgPos = self.avgPos / np.array(irROI.shape, np.float32)
      dist = np.linalg.norm(self.normAvgPos - normPos)

      ret = self.normAvgPos

    if draw:
      cv2.circle(ir, u.tup(pos + c.mappedImageUL), 50, 255, 2)
      cv2.circle(ir, u.tup(self.avgPos + c.mappedImageUL), 50, 255, 8)

    if ret is None:
      return ret
    else:
      return ret

  def update(self):
    fps = FPS()
    while True:
      if not self.running:
        return

      ir = self.irCam.grayUndistorted
      if ir is None:
        continue

      self.circleCenter = self.findCircle(ir)
      u.sendToClients(self.clients, "/ir", { 'circleCenter': self.circleCenter.tolist() })

      p, f, frameidx = fps.update()
      if p:
        print('IR FPS', f)

  def stop(self):
    self.running = False
    self.irCam.setPreundistort(False)

  def isRunning(self):
    return self.running

if __name__ == "__main__":
  cam1 = MaproomCamera(0)
  cam1.load('./calibrations')
  cam1.setPreundistort(True)
  cam1.start()

  irFinder = IRFinder(cam1, clientIPs=['192.168.7.20', '192.168.7.22', '192.168.7.23'])

  draw = True
  fps = FPS()
  lastcam1frameidx = 0
  lastcam2frameidx = 0
  while True:
    p, f, frameidx = fps.update()

    cam1.update()
    ir = cam1.grayUndistorted
    if ir is None:
      continue

    circleCenter = irFinder.findCircle(ir, draw=draw)
    u.sendToClients(irFinder.clients, "/ir", { 'circleCenter': circleCenter })

    if draw:
      frame = ir

      cv2.imshow('frame', frame)
      if cv2.waitKey(1) & 0xFF == ord('q'):
        break

    if p:
      print('FPS', f)
