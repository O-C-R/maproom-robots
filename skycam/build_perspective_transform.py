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
ap.add_argument("--refimage", type=str, required=True,
  help="Reference image")
ap.add_argument("--refimagepx", type=str, required=True,
  help="Reference image pixels to match (format x,y,x,y,x,y,x,y)")
args = vars(ap.parse_args())

cameraID = args['camera']
resolution = (args['width'], args['height'])
refimgFilename = args['refimage']
rawReferencePixels = u.parsePixels(args['refimagepx'])

refimageOrig, refimageFinal, refPixels = u.loadReferenceImage(refimgFilename, c.mappedImageDst, resolution, c.mappedImageResolution, rawReferencePixels)

perspectiveTransform = None
savedCorners = []

camera = MaproomCamera(cameraID, resolution)
camera.load(args["calibrations"], loadCameraMatrix=True, loadPerspective=False, loadHeight=False)

camera.start()
fps = FPS()
while True:
  camera.update()

  if len(savedCorners) == 4 and perspectiveTransform is None:
    perspectiveTransform = cv2.getPerspectiveTransform(np.array(savedCorners, np.float32), np.array(refPixels, np.float32))
    camera.setPerspective(perspectiveTransform)

  frame = camera.undistort()

  markerCorners, markerIds = camera.detectAruco()
  markerCenterPx = None

  if len(markerCorners) > 0:
    markerCornerSet = markerCorners[0]
    markerCornerSet = cv2.undistortPoints(markerCornerSet, camera.cameraMatrix, camera.distCoeffs, P=camera.newCameraMatrix)
    markerCenterPx = (markerCornerSet[0][0] + markerCornerSet[0][1] + markerCornerSet[0][2] + markerCornerSet[0][3]) / 4.0

  if perspectiveTransform is not None:
    cv2.addWeighted(refimageFinal, 0.25, frame, 1 - 0.25, 0, frame)
  else:
    cv2.addWeighted(refimageOrig, 0.25, frame, 1 - 0.25, 0, frame)
    if markerCenterPx is not None:
      cv2.circle(frame, u.tup(markerCenterPx), 2, (255, 255, 0))
    cv2.circle(frame, u.tup(refPixels[len(savedCorners)]), 2, (255, 0, 0))

  cv2.imshow('frame', frame)
  key = cv2.waitKey(1)

  if key & 0xFF == ord('s') and perspectiveTransform is not None:
    u.saveJSON(args['calibrations'], u.videoFilename(cameraID, '-perspective'), {
      'perspectiveTransform': perspectiveTransform.tolist()
    })

  if key & 0xFF == ord('q'):
    break

  if key & 0xFF == ord(' ') and markerCenterPx is not None:
    print('Saved corner', len(savedCorners))
    savedCorners.append(markerCenterPx)

  if key & 0xFF == ord('c'):
    perspectiveTransform = None
    savedCorners = []

  p, f, frameidx = fps.update()
  if p:
    print("fps", f, "with any perspective?", perspectiveTransform is not None)

camera.stop()
cv2.destroyAllWindows()
