from threading import Thread

import cv2
import numpy as np

import requests

from . import constants as c
from . import util as u

class IRFinder:
  def __init__(self, irCam, regCam, clients=[], host=c.irCameraHost, path=c.irCameraPath):
    self.irCam = irCam
    self.regCam = regCam
    self.host = host
    self.path = path
    self.clients = clients

    self.stopped = False

  def start(self):
    t = Thread(target=self.update, args=())
    t.daemon = True
    t.start()
    return self

  def findPolygons(self, ir, reg, draw=False):
    diff = cv2.absdiff(ir, reg)
    diff = cv2.GaussianBlur(diff, (51,51),0)
    _, diff = cv2.threshold(diff, 0,255, cv2.THRESH_BINARY+cv2.THRESH_OTSU)

    _, contours, hierarchy = cv2.findContours(diff, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)

    contourAreas = []
    for contour in contours:
      contourAreas.append(cv2.contourArea(contour))
    n = min(len(contours), 3)
    topIndicies = np.argpartition(contourAreas, -n)[-n:]

    polygons = []
    for idx in topIndicies:
      contour = contours[idx]

      epsilon = 0.001*cv2.arcLength(contour,True)
      approx = cv2.approxPolyDP(contour,epsilon,True)

      if draw:
        cv2.drawContours(ir,[approx],0,(0,0,255),2)

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

      u.sendToClients(self.clients, "/ir", { 'polygons': polygons })

  def read(self):
    return self.frame

  def stop(self):
    self.stopped = True

if __name__ == "__main__":
  clients = u.clientsFromIPs(['127.0.0.1'], 5201)
  cam = IRFinder(None, None, clients=clients)

  ir = cv2.imread('/Users/anderson/Desktop/ir.png', cv2.IMREAD_COLOR)
  reg = cv2.imread('/Users/anderson/Desktop/reg.png', cv2.IMREAD_COLOR)

  ir = cv2.cvtColor(ir, cv2.COLOR_BGR2GRAY)
  reg = cv2.cvtColor(reg, cv2.COLOR_BGR2GRAY)

  polygons = cam.findPolygons(ir, reg, draw=True)

  while True:
    cv2.imshow('frame', ir)
    u.sendToClients(cam.clients, "/ir", { 'polygons': polygons })

    if cv2.waitKey(1) & 0xFF == ord('q'):
      break
