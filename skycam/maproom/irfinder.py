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
  def __init__(self, irCam, regCam, clientIPs=[], clientPort=5201):
    self.irCam = irCam
    self.regCam = regCam
    self.clients = u.clientsFromIPs(clientIPs, clientPort)
    self.running = False
    self.polygons = None

  def start(self):
    self.running = True

    self.irCam.setPreundistort(True)
    self.regCam.setPreundistort(True)
    self.irCam.setFrameSync(8.0)
    self.regCam.setFrameSync(8.0)

    t = Thread(target=self.update, args=(), daemon=True)
    t.start()
    return self

  def findCircle(self, ir, reg, draw=False):
    # TODO: use constants
    irROI = ir[c.mappedImageUL[0]:c.mappedImageBR[0], c.mappedImageUL[1]:c.mappedImageBR[1]]
    regROI = reg[c.mappedImageUL[0]:c.mappedImageBR[0], c.mappedImageUL[1]:c.mappedImageBR[1]]

    # Quasi-match the two scenes...
    meanIR = cv2.mean(ir)
    meanReg = cv2.mean(regROI)
    regROI = cv2.multiply(regROI, meanIR[0] / meanReg[0])

    # Blur the difference between IR and normal
    diff = cv2.subtract(irROI, regROI)
    diff = cv2.blur(diff, (31,31))

    # The flashlight is at the brightest point
    minVal, maxVal, minLoc, maxLoc = cv2.minMaxLoc(diff)

    if maxVal > 20:
      if draw:
        cv2.circle(ir, u.tup(maxLoc + np.array(c.mappedImageUL)), 50, 0, 2)
      return (np.array(maxLoc, np.float32) / np.array(diff.shape, np.float32)).tolist()
    else:
      if draw:
        cv2.circle(ir, u.tup(maxLoc + np.array(c.mappedImageUL)), 50, 127, 2)
      return None

  def update(self):
    fps = FPS()
    while True:
      if not self.running:
        return

      ir = self.irCam.grayUndistorted
      reg = self.regCam.grayUndistorted

      if ir is None:
        continue
      if reg is None:
        continue

      self.circleCenter = self.findCircle(ir, reg)
      u.sendToClients(self.clients, "/ir", { 'circleCenter': self.circleCenter })

      p, f, frameidx = fps.update()
      if p:
        print('IR FPS', f)

  def stop(self):
    self.running = False

    self.irCam.setFrameSync(-1)
    self.regCam.setFrameSync(-1)
    self.irCam.setPreundistort(False)
    self.irCam.setPreundistort(False)

  def isRunning(self):
    return self.running

if __name__ == "__main__":
  cam1 = MaproomCamera(0)
  cam1.load('./calibrations')
  cam2 = MaproomCamera(1)
  cam2.load('./calibrations', loadHeight=False)

  cam1.setPreundistort(True)
  cam2.setPreundistort(True)
  cam1.setFrameSync(8.0)
  cam2.setFrameSync(8.0)

  irFinder = IRFinder(None, None, clientIPs=['192.168.7.20', '192.168.7.22'])

  cam1.start()
  cam2.start()

  draw = True

  fps = FPS()
  while True:
    p, f, frameidx = fps.update()

    cam1.update()
    cam2.update()

    ir = cam1.grayUndistorted
    reg = cam2.grayUndistorted

    if ir is None:
      continue
    if reg is None:
      continue

    circleCenter = irFinder.findCircle(ir, reg, draw=draw)
    u.sendToClients(irFinder.clients, "/ir", { 'circleCenter': circleCenter })

    if draw:
      frame = ir

      cv2.putText(frame, str(math.floor(frameidx / 10) % 5), (20, 40), cv2.FONT_HERSHEY_PLAIN, 1, 255)
      cv2.imshow('frame', frame)
      if cv2.waitKey(1) & 0xFF == ord('q'):
        break

    if p:
      print('FPS', f)
