import time
import json
import argparse
from datetime import datetime
import math

import cv2
import numpy as np

from pythonosc import osc_message_builder
from pythonosc import udp_client

from maproom.camera import MaproomCamera
from maproom.s3timelapse import S3Timelapse
from maproom.irfinder import IRFinder
from maproom.fps import FPS
import maproom.constants as c
import maproom.util as u

ap = argparse.ArgumentParser()
ap.add_argument("-c", "--camera", type=int, default=0,
  help="Which camera to use")
ap.add_argument("-c2", "--camera2", type=int, default=-1,
  help="Second camera, for subtraction from IR")
ap.add_argument("-x", "--width", type=int, default=c.resolution[0],
  help="Width of image")
ap.add_argument("-y", "--height", type=int, default=c.resolution[1],
  help="Height of image")
ap.add_argument("-d", "--display", type=bool, default=False,
  help="Whether or not frames should be displayed")
ap.add_argument("-r", "--remotes", type=str, default='192.168.7.20,192.168.7.21,192.168.7.22,192.168.7.23',
  help="Host IPs for OSC messages, string delimited")
ap.add_argument("-p", "--port", type=int, default=5100,
  help="Host port for OSC messages")
ap.add_argument("--timelapsecam", type=int, default=-1,
  help="Which camera for timelapse upload to S3, either 0 or 1")
ap.add_argument("--ir", type=bool, default=False,
  help="Enable IR post to Maproom")
ap.add_argument("--calibrations", type=str, default="./calibrations",
  help="Calibrations path")
ap.add_argument("--refimage", type=str, required=False,
  help="Reference image")
args = vars(ap.parse_args())

cameraID = args['camera']
cameraID2 = args['camera2']
remotes = args['remotes'].split(',')
resolution = (args['width'], args['height'])

clients = [udp_client.SimpleUDPClient(remote, args['port']) for remote in remotes]
def sendToClients(clients, data):
  oscmsg = json.dumps(data)
  for client in clients:
    try:
      client.send_message("/cv", oscmsg)
    except:
      pass

camera = MaproomCamera(cameraID, resolution)
camera.load(args["calibrations"], loadCameraMatrix=True, loadPerspective=True, loadHeight=True)

camera2 = None
if cameraID2 > 0:
  camera2 = MaproomCamera(cameraID2, resolution)
  camera2.load(args["calibrations"], loadCameraMatrix=True, loadPerspective=True)

s3uploader = None
if args['timelapsecam'] >= 0:
  if args['timelapsecam'] == 0:
    s3uploader = S3Timelapse(camera)
  elif args['timelapsecam'] == 1:
    if camera2 is not None:
      s3uploader = S3Timelapse(camera2)
    else:
      print("Could not start timelapse, no second camera enabled")
  else:
    print("Invalid timelapse cam option")

irfinder = None
if args['ir']:
  if camera2 is not None:
    irfinder = IRFinder(camera, camera2)
  else:
    print('Could not start IR finder, no second camera enabled')

refimageScaledFinal = None
if args["refimage"]:
  refimageScaledOrig, refimageScaledFinal = u.loadReferenceImage(args["refimage"], c.mappedImageDst, resolution, c.mappedImageResolution)

def detectMarkers(camera, frame):
  global args
  global running
  global refimageScaledFinal

  markerCorners, markerIds = camera.detectAruco()

  markerIdList = []
  positions = {}
  if markerIds is not None:
    markerIdList = [int(x[0]) for x in markerIds]
    positions = camera.transformMarkers(markerCorners, markerIds)

  data = { 'ids': [], 'pos': [], 'up': [], 'raw_pos': [], 'raw_up': [] }
  for markerID, position in positions.items():
    data['ids'].append(markerID)
    data['pos'].append(position['pos'].tolist())
    data['up'].append(position['up'].tolist())
    data['raw_pos'].append(position['rawPos'].tolist())
    data['raw_up'].append(position['rawUp'].tolist())
  sendToClients(clients, data)

  if args["display"]:
    frame = camera.undistort()
    if refimageScaledFinal is not None:
      cv2.addWeighted(refimageScaledFinal, 0.25, frame, 1 - 0.25, 0, frame)

    for markerID, position in positions.items():
      cv2.circle(frame, u.tup(position['rawMarkerCenterPx']), 2, (127,127,127), -1)
      cv2.line(frame, u.tup(position['rawMarkerCenterPx']), u.tup(position['rawMarkerUpPx']), (127,127,127))
      cv2.circle(frame, u.tup(position['markerCenterPx']), 2, (255, 255, 255), -1)
      cv2.line(frame, u.tup(position['markerCenterPx']), u.tup(position['markerUpPx']), (255,255,255))

    cv2.imshow('frame', frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
      running = False

  return markerIdList

# Start cameras
camera.start()
if camera2 is not None:
  camera2.start()

# Wait for cameras to warm up
time.sleep(1.0)

# Start uploader
if s3uploader is not None:
  s3uploader.start()

# Start IR finder
if irfinder is not None:
  irfinder.start()

try:
  # Start main loop
  running = True
  fps = FPS()
  while running is True:
    frame, gray = camera.update()

    frame2, gray2 = None, None
    if camera2 is not None:
      frame2, gray2 = camera2.update()

    markerIdList = detectMarkers(camera, frame)

    p, f, frameidx = fps.update()
    if p:
      print("fps", f, "with markers", markerIdList)
finally:
  # Stop all gracefully
  camera.stop()
  if camera2 is not None:
    camera2.stop()
  if s3uploader is not None:
    s3uploader.stop()
  if irfinder is not None:
    irfinder.stop()

  cv2.destroyAllWindows()
