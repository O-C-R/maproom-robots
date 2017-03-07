import time
import json
import argparse
from datetime import datetime
import math

import cv2
import numpy as np

from maproom.camera import MaproomCamera
from maproom.fps import FPS
import maproom.constants as c
import maproom.util as u

ap = argparse.ArgumentParser()
ap.add_argument("-c", "--camera", type=int, default=0,
  help="Which camera to use")
ap.add_argument("-x", "--width", type=int, default=c.resolution[0],
  help="Width of image")
ap.add_argument("-y", "--height", type=int, default=c.resolution[1],
  help="Height of image")
ap.add_argument("--calibrations", type=str, default="./calibrations",
  help="Calibrations path")
ap.add_argument("-m", "--mouse", type=bool, default=False,
  help="Enable mouse clicking on top of Aruco")
ap.add_argument("--refimage", type=str, required=False,
  help="Reference image")
args = vars(ap.parse_args())

cameraID = args['camera']
resolution = (args['width'], args['height'])

refimage = None

heightTransform = None
markerCenterPx = None
savedLow = []
savedHigh = []

camera = MaproomCamera(cameraID, resolution)
camera.load(args["calibrations"], loadCameraMatrix=True, loadPerspective=False, loadHeight=False)

def addPt(pt):
  if len(savedLow) == len(savedHigh):
    savedLow.append(pt)
  else:
    savedHigh.append(pt)
  print('Saved corner', len(savedLow), len(savedHigh))

def onmouse(event,x,y,flags,param):
  if event == cv2.EVENT_LBUTTONDOWN:
    addPt([x, y])

cv2.namedWindow('frame')
if args['mouse']:
  cv2.setMouseCallback('frame', onmouse)

camera.start()
fps = FPS()
while True:
  camera.update()
  frame = camera.undistort()

  markerCorners, markerIds = camera.detectAruco()

  if len(markerCorners) > 0:
    markerCornerSet = markerCorners[0]
    markerCornerSet = cv2.undistortPoints(markerCornerSet, camera.cameraMatrix, camera.distCoeffs, P=camera.newCameraMatrix)
    markerCenterPx = (markerCornerSet[0][0] + markerCornerSet[0][1] + markerCornerSet[0][2] + markerCornerSet[0][3]) / 4.0

  if heightTransform is not None:
    transformedHigh = u.transformPixels(savedHigh, heightTransform)
    for i in range(len(transformedHigh)):
      o = savedHigh[i]
      t = transformedHigh[i]
      cv2.line(frame, u.tup(o), u.tup(t), (200,200,200))
      cv2.circle(frame, u.tup(o), 2, (200,200,200), -1)
      cv2.circle(frame, u.tup(t), 2, (255,255,0), -1)

    if markerCenterPx is not None:
      markerCenterPxT = u.transformPixels([markerCenterPx], heightTransform)[0]
      cv2.line(frame, u.tup(markerCenterPx), u.tup(markerCenterPxT), (255,255,255))
      cv2.circle(frame, u.tup(markerCenterPx), 2, (255,255,255), -1)
      cv2.circle(frame, u.tup(markerCenterPxT), 2, (255,0,255), -1)
  else:
    for pt in savedLow:
      cv2.circle(frame, u.tup(pt), 2, (0,255,255), -1)
    for pt in savedHigh:
      cv2.circle(frame, u.tup(pt), 2, (255,255,255), -1)
    if markerCenterPx is not None:
      cv2.circle(frame, u.tup(markerCenterPx), 2, (255, 255, 0))

  if refimage is not None:
    cv2.addWeighted(refimage, 0.25, frame, 1 - 0.25, 0, frame)

  status = str(len(savedLow)) + " low / " + str(len(savedHigh)) + " high"
  cv2.putText(frame, status, (20, 20), cv2.FONT_HERSHEY_PLAIN, 1, (255, 255, 255))
  status = "space: save, g: compute, s: save, c: clear, q: quit"
  cv2.putText(frame, status, (20, 40), cv2.FONT_HERSHEY_PLAIN, 1, (255, 255, 255))

  cv2.imshow('frame', frame)
  key = cv2.waitKey(1)

  if key & 0xFF == ord('s') and heightTransform is not None:
    u.saveJSON(args['calibrations'], u.videoFilename(cameraID, '-height'), {
      'heightTransform': heightTransform.tolist()
    })

  if key & 0xFF == ord('q'):
    break

  if key & 0xFF == ord(' ') and markerCenterPx is not None:
    addPt(markerCenterPx)

  if key & 0xFF == ord('g'):
    heightTransform, _ = cv2.findHomography(np.array(savedHigh, np.float32), np.array(savedLow, np.float32), cv2.LMEDS)
    camera.setHeight(heightTransform)

  if key & 0xFF == ord('c'):
    heightTransform = None
    camera.setPerspective(None)
    savedLow = []
    savedHigh = []

  p, f, frameidx = fps.update()
  if p:
    print("fps", f, "with any height?", heightTransform is not None)

camera.stop()
cv2.destroyAllWindows()
