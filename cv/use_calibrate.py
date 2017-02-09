import time
import cv2
import cv2.aruco as A
import numpy as np
import json
import sys
from datetime import datetime

from pythonosc import osc_message_builder
from pythonosc import udp_client

camera_id = int(sys.argv[1])

with open('calibrations/video'+str(camera_id)+'.json', 'r') as f:
  data = json.loads(f.read())

  cameraMatrix = np.array(data['cameraMatrix'])
  distCoeffs = np.array(data['distCoeffs'])

dictionary = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_5X5_50)

marker_size_in = 8
marker_size_m = 0.2032
ppi = 72

robot01 = cv2.aruco.drawMarker(dictionary, 23, marker_size_in * ppi)
cv2.imwrite('markers/robot01.png', robot01)

detector_params = cv2.aruco.DetectorParameters_create()
detector_params.doCornerRefinement = True
detector_params.cornerRefinementMaxIterations = 500
detector_params.cornerRefinementWinSize = 3
detector_params.cornerRefinementMinAccuracy = 0.001

print(detector_params)

cap = cv2.VideoCapture(camera_id)
client = udp_client.SimpleUDPClient('192.168.1.20', 5100)

cap.set(cv2.CAP_PROP_FRAME_WIDTH,800)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT,600)

fps = 30.0
last_time = datetime.now()
while True:
  ret,frame = cap.read()

  gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
  marker_corners, marker_ids, rejected = cv2.aruco.detectMarkers(gray, dictionary, parameters=detector_params)  
  rvecs, tvecs = cv2.aruco.estimatePoseSingleMarkers(marker_corners, marker_size_m, cameraMatrix, distCoeffs)

  cv2.aruco.drawDetectedMarkers(gray, marker_corners, marker_ids)
  cv2.imshow('frame',gray)

  if not marker_ids is None:
    data = {
      'ids': marker_ids[0].tolist(),
      'rvecs': [x.tolist() for x in rvecs[0]],
      'tvecs': [x.tolist() for x in tvecs[0]]
    }
    oscmsg = json.dumps(data)
    print(oscmsg)
    client.send_message("/cv", oscmsg)

  if cv2.waitKey(1) & 0xFF == ord('q'):
    break

  now = datetime.now()
  delta = now - last_time
  last_time = now
  curr_fps = 1.0 / delta.total_seconds()
  fps = fps + (curr_fps - fps) * 0.1
  print("fps", fps)
