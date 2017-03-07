from threading import Thread

import cv2
import numpy as np

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
    self.stopped = False

  def start(self):
    t = Thread(target=self.update, args=())
    t.daemon = True
    t.start()
    return self

  def findPolygons(self, ir, reg, draw=False):
    # TODO: use constants
    irROI = ir[100:612, 100:612]
    regROI = reg[100:612, 100:612]

    meanIR = cv2.mean(irROI)
    meanReg = cv2.mean(regROI)
    regROI = cv2.multiply(regROI, meanIR[0] / meanReg[0])

    diff = cv2.subtract(irROI, regROI)

    # TODO: can we resample down instead of blur? Is that faster?
    diff = cv2.GaussianBlur(diff, (55,55),0)
    _, diff = cv2.threshold(diff, 10, 255, cv2.THRESH_BINARY)
    _, contours, hierarchy = cv2.findContours(diff, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)

    contourAreas = []
    for contour in contours:
      contourAreas.append(cv2.contourArea(contour))
    n = min(len(contours), 3)
    topIndicies = np.argpartition(contourAreas, -n)[-n:]

    polygons = []
    for idx in topIndicies:
      contour = contours[idx]
      contourArea = contourAreas[idx]

      if contourArea < MIN_CONTOUR_AREA:
        continue

      epsilon = 0.001*cv2.arcLength(contour,True)
      approx = cv2.approxPolyDP(contour,epsilon,True)

      if draw:
        drawApprox = approx + np.array([100,100])
        cv2.drawContours(ir,[drawApprox],0,(0,0,255),2)

      normalized = np.array(approx, np.float32) / np.array(diff.shape, np.float32)
      out = ' '.join([str(pt[0][0])+","+str(pt[0][1]) for pt in normalized])

      polygons.append(out)

    return polygons

  def update(self):
    while True:
      if self.stopped:
        return

      ir = self.irCam.undistort(gray=True)
      reg = self.regCam.undistort(gray=True)

      self.polygons = self.findPolygons(ir, reg)
      u.sendToClients(self.clients, "/ir", { 'polygons': self.polygons })

  def stop(self):
    self.stopped = True

if __name__ == "__main__":
  cam1 = MaproomCamera(0)
  cam1.load('./calibrations')
  cam2 = MaproomCamera(1)
  cam2.load('./calibrations', loadHeight=False)

  irFinder = IRFinder(None, None, clientIPs=['192.168.7.20'])

  cam1.start()
  cam2.start()

  draw = False

  fps = FPS()
  while True:
    cam1.update()
    cam2.update()

    ir = cam1.undistort(gray=True)
    reg = cam2.undistort(gray=True)
    polygons = irFinder.findPolygons(ir, reg, draw=draw)
    u.sendToClients(irFinder.clients, "/ir", { 'polygons': polygons })

    if draw:
      cv2.imshow('frame', ir)
      if cv2.waitKey(1) & 0xFF == ord('q'):
        break

    p, f, frameidx = fps.update()
    if p:
      print('FPS', f)
