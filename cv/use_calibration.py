import time
import cv2
import numpy as np
import json
import sys
import argparse
from threading import Thread
from datetime import datetime

from pythonosc import osc_message_builder
from pythonosc import udp_client

ap = argparse.ArgumentParser()
ap.add_argument("-c", "--camera", type=int, default=0,
  help="Which camera to use")
ap.add_argument("-d", "--display", type=bool, default=False,
  help="Whether or not frames should be displayed")
ap.add_argument("-r", "--remotes", type=str, default='192.168.1.20,192.168.1.21',
  help="Host IPs for OSC messages, string delimited")
ap.add_argument("-p", "--port", type=int, default=5100,
  help="Host port for OSC messages")
ap.add_argument("-x", "--width", type=int, default=960,
  help="Width of image")
ap.add_argument("-y", "--height", type=int, default=720,
  help="Height of image")
args = vars(ap.parse_args())

camera_id = args['camera']
remotes = args['remotes'].split(',')

filename = 'calibrations/video'+str(camera_id)+'.json'
print('Loading calibration from file:', filename)
with open(filename, 'r') as f:
  data = json.loads(f.read())

  cameraMatrix = np.array(data['cameraMatrix'])
  distCoeffs = np.array(data['distCoeffs'])

class CVVideoStream:
  def __init__(self, src=0, resolution=(800,600)):
    # initialize the video camera stream and read the first frame
    # from the stream
    self.stream = cv2.VideoCapture(src)
    self.stream.set(cv2.CAP_PROP_FRAME_WIDTH,resolution[0])
    self.stream.set(cv2.CAP_PROP_FRAME_HEIGHT,resolution[1])
    (self.grabbed, self.frame) = self.stream.read()

    # initialize the variable used to indicate if the thread should
    # be stopped
    self.stopped = False

  def start(self):
    # start the thread to read frames from the video stream
    t = Thread(target=self.update, args=())
    t.daemon = True
    t.start()
    return self

  def update(self):
    # keep looping infinitely until the thread is stopped
    while True:
      # if the thread indicator variable is set, stop the thread
      if self.stopped:
        return

      # otherwise, read the next frame from the stream
      (self.grabbed, self.frame) = self.stream.read()

  def read(self):
    # return the frame most recently read
    return self.frame

  def stop(self):
    # indicate that the thread should be stopped
    self.stopped = True

dictionary = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_5X5_50)

marker_size_in = 8
marker_size_m = 0.2032
ppi = 72

robot01 = cv2.aruco.drawMarker(dictionary, 23, marker_size_in * ppi)
cv2.imwrite('markers/robot01.png', robot01)

robot02 = cv2.aruco.drawMarker(dictionary, 24, int(marker_size_in * ppi / 2.0))
cv2.imwrite('markers/robot02.png', robot02)

detector_params = cv2.aruco.DetectorParameters_create()
detector_params.doCornerRefinement = True
detector_params.cornerRefinementMaxIterations = 500
detector_params.cornerRefinementWinSize = 3
detector_params.cornerRefinementMinAccuracy = 0.001
detector_params.minMarkerPerimeterRate = 0.1
detector_params.adaptiveThreshWinSizeMin = 10
detector_params.adaptiveThreshWinSizeMax = 10

clients = [udp_client.SimpleUDPClient(remote, args['port']) for remote in remotes]
resolution = (args['width'], args['height'])
vs = CVVideoStream(src=camera_id, resolution=resolution).start()
time.sleep(1.0)

fps = 30.0
last_time = datetime.now()
frameidx = 0
while True:
  frame = vs.read()
  gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
  marker_corners, marker_ids, rejected = cv2.aruco.detectMarkers(gray, dictionary, parameters=detector_params)
  rvecs, tvecs = cv2.aruco.estimatePoseSingleMarkers(marker_corners, marker_size_m, cameraMatrix, distCoeffs)

  if args["display"]:
    cv2.aruco.drawDetectedMarkers(gray, marker_corners, marker_ids)
    cv2.imshow('frame',gray)

  if not marker_ids is None:
    marker_id_list = [int(x[0]) for x in marker_ids]
    data = {
      'ids': marker_id_list,
      'rvecs': [x[0].tolist() for x in rvecs],
      'tvecs': [x[0].tolist() for x in tvecs]
    }
    oscmsg = json.dumps(data)
    for client in clients:
      client.send_message("/cv", oscmsg)
  else:
    marker_id_list = None

  if cv2.waitKey(1) & 0xFF == ord('q'):
    break

  now = datetime.now()
  delta = now - last_time
  last_time = now
  curr_fps = 1.0 / delta.total_seconds()
  fps = fps + (curr_fps - fps) * 0.1
  frameidx = frameidx + 1
  if frameidx % 30 == 0:
    print("fps", fps, "with markers", marker_id_list)

vs.stop()
cv2.destroyAllWindows()
