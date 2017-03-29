import time
import json
import argparse
import math

import cv2
import numpy as np

def tup(arr):
  return tuple([int(x) for x in arr])

ap = argparse.ArgumentParser()
ap.add_argument("input")
ap.add_argument("output")
args = vars(ap.parse_args())

origImage = cv2.imread(args["input"], cv2.IMREAD_COLOR)
sqSize = 1024
lgFactor = 2
refPixels = [[0,0], [sqSize,0], [sqSize,sqSize], [0, sqSize]]

displayWidth = sqSize
scaleFactor = float(origImage.shape[1]) / float(displayWidth)
finalSize = (int(origImage.shape[1] / scaleFactor), int(origImage.shape[0] / scaleFactor))

refImage = cv2.resize(origImage, finalSize)

pTransform = None
pTransformLarge = None
savedCorners = []

def onmouse(event,x,y,flags,param):
  global savedCorners
  if event == cv2.EVENT_LBUTTONDOWN:
    savedCorners.append([x, y])

cv2.namedWindow('frame')
cv2.setMouseCallback('frame', onmouse)

while True:
  if pTransform is None:
    frame = refImage.copy()
  else:
    frame = cv2.warpPerspective(origImage, pTransform, (sqSize, sqSize))

  if len(savedCorners) == 4 and pTransform is None:
    pTransform = cv2.getPerspectiveTransform(np.array(savedCorners, np.float32) * scaleFactor, np.array(refPixels, np.float32))
    pTransformLg = cv2.getPerspectiveTransform(np.array(savedCorners, np.float32) * scaleFactor, np.array(refPixels, np.float32) * lgFactor)

  if pTransform is None:
    for pt in savedCorners:
      cv2.circle(frame, tup(pt), 2, (0,255,255), -1)

  cv2.imshow('frame', frame)
  key = cv2.waitKey(1)

  if key & 0xFF == ord('s') and pTransform is not None:
    lg = cv2.warpPerspective(origImage, pTransformLg, (sqSize * lgFactor, sqSize * lgFactor))
    cv2.imwrite(args["output"], lg)

  if key & 0xFF == ord('q'):
    break

  if key & 0xFF == ord('c'):
    pTransform = None
    savedCorners = []

cv2.destroyAllWindows()
