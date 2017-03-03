import time
import cv2
import numpy as np
import json
import sys
import argparse
from threading import Thread
from datetime import datetime
import constants
import math

from pythonosc import osc_message_builder
from pythonosc import udp_client

ap = argparse.ArgumentParser()
ap.add_argument("-c", "--camera", type=int, default=0,
  help="Which camera to use")
ap.add_argument("-d", "--display", type=bool, default=False,
  help="Whether or not frames should be displayed")
ap.add_argument("-r", "--remotes", type=str, default='192.168.7.20,192.168.7.21,192.168.7.22,192.168.7.23',
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

detector_params = cv2.aruco.DetectorParameters_create()
detector_params.doCornerRefinement = True
detector_params.cornerRefinementMaxIterations = 500
# detector_params.cornerRefinementWinSize = 1
detector_params.cornerRefinementMinAccuracy = 0.001
# detector_params.minMarkerPerimeterRate = 0.05
# detector_params.maxMarkerPerimeterRate = 0.2
detector_params.adaptiveThreshWinSizeMin = 10
# detector_params.adaptiveThreshWinSizeStep = 3
detector_params.adaptiveThreshWinSizeMax = 10

resolution = (args['width'], args['height'])
newCameraMatrix, validPixROI = cv2.getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, resolution, 0)

clients = [udp_client.SimpleUDPClient(remote, args['port']) for remote in remotes]

vs = CVVideoStream(src=camera_id, resolution=resolution).start()
time.sleep(1.0)

fps = 30.0
last_time = datetime.now()
frameidx = 0
while True:
  frame = vs.read()
  gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

  marker_corners, marker_ids, rejected = cv2.aruco.detectMarkers(gray, constants.marker_dictionary, parameters=detector_params)

  marker_id_list = []
  pos_list = []
  up_list = []
  if not marker_ids is None:
    marker_id_list = [int(x[0]) for x in marker_ids]

    img_center_px = np.divide(np.array([gray.shape[1], gray.shape[0]]), 2.0)
    img_half_height = gray.shape[0] / 2.0
    for i in range(len(marker_corners)):
      marker_corner_set = marker_corners[i]
      marker_id = marker_ids[i]

      # undistort the points, in case they were...
      marker_corner_set = cv2.undistortPoints(marker_corner_set, cameraMatrix, distCoeffs, P=newCameraMatrix)

      # calculate the center and up direction for the markers. this assume minimal perspective transformation
      marker_center_px = (marker_corner_set[0][0] + marker_corner_set[0][1] + marker_corner_set[0][2] + marker_corner_set[0][3]) / 4.0
      dist_from_center_px = marker_center_px - img_center_px
      dist_from_center_norm = dist_from_center_px / img_half_height

      marker_up_px = (marker_corner_set[0][0] + marker_corner_set[0][1]) / 2.0
      up_dir_px = marker_up_px - marker_center_px
      up_dir = up_dir_px / np.sqrt(up_dir_px.dot(up_dir_px))

      pos_list.append(dist_from_center_norm.tolist())
      up_list.append(up_dir.tolist())

      # debug output
      if args["display"]:
        cv2.circle(gray, tuple([int(x) for x in marker_center_px]), 5, 255, -1)
        cv2.line(gray, tuple([int(x) for x in marker_center_px]), tuple([int(x) for x in marker_up_px]), 255)

  if args["display"]:
    cv2.aruco.drawDetectedMarkers(gray, marker_corners, marker_ids)
    cv2.imshow('frame',gray)

  data = {
    'ids': marker_id_list,
    'pos': pos_list,
    'up': up_list
  }
  oscmsg = json.dumps(data)
  for client in clients:
    try:
      client.send_message("/cv", oscmsg)
    except:
      pass

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
