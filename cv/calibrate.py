import time
import cv2
import cv2.aruco as A
import numpy as np
import json
import sys
import argparse

import constants

ap = argparse.ArgumentParser()
ap.add_argument("-c", "--camera", type=int, default=0,
  help="Which camera to use")
ap.add_argument("-d", "--display", type=bool, default=False,
  help="Whether or not frames should be displayed")
ap.add_argument("-x", "--width", type=int, default=960,
  help="Width of image")
ap.add_argument("-y", "--height", type=int, default=720,
  help="Height of image")
args = vars(ap.parse_args())

#Start capturing images for calibration
camera_id = args['camera']
cap = cv2.VideoCapture(camera_id)

cap.set(cv2.CAP_PROP_FRAME_WIDTH, args['width'])
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, args['height'])

count = 25
allCorners = []
allIds = []
frameIdx = 0
while True:
  ret,frame = cap.read()
  gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
  marker_corners, marker_ids, rejected = cv2.aruco.detectMarkers(gray, constants.charuco_dictionary)

  if len(marker_corners) > 0:
    # Only update every 10 frames so we get more diverse data
    if frameIdx % 10 == 0:
      ret, charuco_corners, charuco_ids = cv2.aruco.interpolateCornersCharuco(marker_corners, marker_ids, gray, constants.charuco_board)
      if charuco_corners is not None and charuco_ids is not None and len(charuco_corners)>3:
        allCorners.append(charuco_corners)
        allIds.append(charuco_ids)

    cv2.aruco.drawDetectedMarkers(gray, marker_corners, marker_ids)

  if args['display']:
    cv2.imshow('frame',gray)

  if cv2.waitKey(1) & 0xFF == ord('q'):
    break

  frameIdx += 1
  print("Found: " + str(len(allIds)) + " / " + str(count))

  if (len(allIds) >= count):
    break

imsize = gray.shape

print('Done collecting data...')

# InputOutputArray    cameraMatrix,
# InputOutputArray    distCoeffs,
# OutputArrayOfArrays     rvecs = noArray(),
# OutputArrayOfArrays     tvecs = noArray(),
# int     flags = 0,
# TermCriteria    criteria = TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 30, DBL_EPSILON)

#Calibration fails for lots of reasons. Release the video if we do
try:
  err, cameraMatrix, distCoeffs, rvecs, tvecs = cv2.aruco.calibrateCameraCharuco(allCorners,allIds,constants.charuco_board,imsize,None,None)
  print('Calibrated with err', err)

  print('writing json')
  out = json.dumps({
    'cameraMatrix': cameraMatrix.tolist(),
    'distCoeffs': distCoeffs.tolist(),
    'err': err
  })
  filename = 'calibrations/video'+str(camera_id)+'.json'
  print('Saving calibration to file:', filename)
  with open(filename, 'w') as f:
    f.write(out)
except Exception as err:
  print(err)

while True:
  ret,frame = cap.read()
  gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

  undistorted = cv2.undistort(gray, cameraMatrix, distCoeffs)

  cv2.imshow('frame',undistorted)
  if cv2.waitKey(1) & 0xFF == ord('q'):
    break

cap.release()
cv2.destroyAllWindows()
