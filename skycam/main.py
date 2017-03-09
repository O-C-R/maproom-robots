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
from maproom.oscreceiver import OSCReceiver
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
  help="Host port to send OSC messages to")
ap.add_argument("--port2", type=int, default=5201,
  help="Host port to send IR camera OSC messages to")
ap.add_argument("--localhost", type=str, default='0.0.0.0',
  help="Local host to receive OSC messages")
ap.add_argument("--localport", type=int, default=5300,
  help="Local port to receive OSC messages")
ap.add_argument("--timelapsecam", type=int, default=-1,
  help="Which camera for timelapse upload to S3, either 0 or 1")
ap.add_argument("--ir", type=bool, default=False,
  help="Enable IR post to Maproom")
ap.add_argument("--fixedfocus", type=bool, default=False,
  help="Fix the focus of the second camera")
ap.add_argument("--calibrations", type=str, default="./calibrations",
  help="Calibrations path")
ap.add_argument("--refimage", type=str, required=False,
  help="Reference image")
ap.add_argument("--profile", type=bool, default=False,
  help="Do profiling")
args = vars(ap.parse_args())

state = c.STATE_TRACKING

def updateState(newState):
  global state
  global camera2
  global irfinder
  global s3uploader

  if newState == c.STATE_TRACKING:
    if camera2 is not None and camera2.isRunning():
      camera2.stop()
    else:
      print("No camera2 to stop")

    if irfinder is not None and irfinder.isRunning():
      irfinder.stop()
    else:
      print("No irfinder to stop")

    if s3uploader is not None and s3uploader.isRunning():
      s3uploader.stop()
    else:
      print("No s3uploader to stop")

    print("State is now STATE_TRACKING")
  elif newState == c.STATE_FLASHLIGHT:
    if camera2 is not None and not camera2.isRunning():
      camera2.start()
    else:
      print("No camera2 to start")

    if irfinder is not None and not irfinder.isRunning():
      irfinder.start()
    else:
      print("No irfinder to start")

    if s3uploader is not None and not s3uploader.isRunning():
      s3uploader.start()
    else:
      print("No s3uploader to start")

    print("State is now STATE_FLASHLIGHT")

  state = newState

def detectMarkers(camera, frame):
  global args
  global running
  global refimageScaledFinal
  global clients

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
  u.sendToClients(clients, "/cv", data)

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

# SET UP GLOBAL STATE
cameraID = args['camera']
cameraID2 = args['camera2']
remotes = args['remotes'].split(',')
resolution = (args['width'], args['height'])

clients = u.clientsFromIPs(remotes, args['port'])
oscReceiver = OSCReceiver(host=args['localhost'], port=args['localport'])

camera = MaproomCamera(cameraID, resolution)
camera.load(args["calibrations"])

camera2 = None
if cameraID2 > 0:
  if args['fixedfocus']:
    u.setFixedFocus(cameraID2, 1)

  camera2 = MaproomCamera(cameraID2, resolution)
  camera2.load(args["calibrations"], loadHeight=False)

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
    irfinder = IRFinder(camera, camera2, clientIPs=remotes, clientPort=args['port2'])
  else:
    print('Could not start IR finder, no second camera enabled')

refimageScaledFinal = None
if args["refimage"]:
  refimageScaledOrig, refimageScaledFinal = u.loadReferenceImage(args["refimage"], c.mappedImageDst, resolution, c.mappedImageResolution)

# Start camera 1, camera 2 boots up only when asked for
camera.start()

# Wait for cameras to warm up
time.sleep(2.0)

# Start OSC listener
oscReceiver.start()

if args['profile']:
  import yappi
  yappi.start()

try:
  # Start main loop
  running = True
  fps = FPS()
  while running is True:
    newState = oscReceiver.getState()
    if newState >= 0 and newState != state:
      updateState(newState)

    frame, gray, grayUndistorted = camera.update()
    frame2, gray2, grayUndistorted2 = None, None, None

    if state == c.STATE_FLASHLIGHT and camera2 is not None:
      frame2, gray2, grayUndistorted = camera2.update()

    markerIdList = None
    if state == c.STATE_TRACKING:
      markerIdList = detectMarkers(camera, frame)

    p, f, frameidx = fps.update()
    if p:
      u.sendToClients(clients, "/state", { 'state': state })
      print("fps", f, "in state", state, "with markers", markerIdList)
finally:
  if args['profile']:
    yappi.get_func_stats().sort('tsub').print_all()

  # Stop all gracefully
  camera.stop()
  if camera2 is not None:
    camera2.stop()
  if s3uploader is not None:
    s3uploader.stop()
  if irfinder is not None:
    irfinder.stop()

  cv2.destroyAllWindows()
