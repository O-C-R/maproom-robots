import time
import cv2
import cv2.aruco as A
import numpy as np
import json
import sys
import argparse

img_ppi = 72
in_to_m = 0.0254

n_sq_horiz = 3
n_sq_vert = 4

sq_size_m = 0.065
marker_size_m = 0.045

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

dictionary = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_50)
board = cv2.aruco.CharucoBoard_create(n_sq_horiz, n_sq_vert, sq_size_m,marker_size_m, dictionary)
img = board.draw((img_ppi*n_sq_horiz,img_ppi*n_sq_vert))

#Dump the calibration board to a file
cv2.imwrite('charuco.png',img)
print('wrote image with dims', (img_ppi*n_sq_horiz,img_ppi*n_sq_vert), 'maps to size', (sq_size_m * n_sq_horiz, sq_size_m * n_sq_vert), 'meters')
print('or', (sq_size_m * n_sq_horiz / in_to_m, sq_size_m * n_sq_vert / in_to_m), 'inches')

#Start capturing images for calibration
camera_id = args['camera']
cap = cv2.VideoCapture(camera_id)

cap.set(cv2.CAP_PROP_FRAME_WIDTH, args['width'])
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, args['height'])

allCorners = []
allIds = []
decimator = 0
for i in range(300):
  ret,frame = cap.read()
  gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
  res = cv2.aruco.detectMarkers(gray,dictionary)

  if len(res[0])>0:
    res2 = cv2.aruco.interpolateCornersCharuco(res[0],res[1],gray,board)
    if res2[1] is not None and res2[2] is not None and len(res2[1])>3 and decimator%3==0:
      allCorners.append(res2[1])
      allIds.append(res2[2])

    cv2.aruco.drawDetectedMarkers(gray,res[0],res[1])

  if args['display']:
    cv2.imshow('frame',gray)

  if cv2.waitKey(1) & 0xFF == ord('q'):
    break
  decimator+=1
  print(str(i) + ": " + str(len(allIds)))

  if (len(allIds) > 15):
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
  err, cameraMatrix, distCoeffs, rvecs, tvecs = cv2.aruco.calibrateCameraCharuco(allCorners,allIds,board,imsize,None,None)
  print('Calibrated with err', err)

  print('writing json')
  out = json.dumps({
    'cameraMatrix': cameraMatrix.tolist(),
    'distCoeffs': distCoeffs.tolist(),
    'err': err
  })
  with open('calibrations/video'+str(camera_id)+'.json', 'w') as f:
    f.write(out)
except Exception as err:
  print(err)

cap.release()
cv2.destroyAllWindows()
