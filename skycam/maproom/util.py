import json
import subprocess
from os import path

import cv2
import numpy as np

from pythonosc import udp_client

def loadCalibration(calibrationsPath, cameraID):
  data = loadJSON(calibrationsPath, videoFilename(cameraID, ''))
  cameraMatrix = np.array(data['cameraMatrix'])
  distCoeffs = np.array(data['distCoeffs'])
  return cameraMatrix, distCoeffs

def loadPerspective(calibrationsPath, cameraID):
  data = loadJSON(calibrationsPath, videoFilename(cameraID, '-perspective'))
  perspectiveTransform = np.array(data['perspectiveTransform'])
  return perspectiveTransform

def loadHeight(calibrationsPath, cameraID):
  data = loadJSON(calibrationsPath, videoFilename(cameraID, '-height'))
  transform = np.array(data['heightTransform'])
  return transform

def videoFilename(cameraID, postfix):
  return 'video'+str(cameraID)+postfix+'.json'

def loadJSON(dir, filename):
  filename = path.join(dir, filename)
  print('Loading from file:', filename)
  with open(filename, 'r') as f:
    data = json.loads(f.read())
    return data

def saveJSON(dir, filename, data):
  filename = path.join(dir, filename)
  print('Saving to file:', filename)
  out = json.dumps(data)
  with open(filename, 'w') as f:
    f.write(out)

def tup(arr):
  return tuple([int(x) for x in arr])

def dist(a, b):
  return np.linalg.norm(a-b)

def parsePixels(pixStr):
  x = -1
  y = -1
  pixels = []
  for val in pixStr.split(","):
    if x < 0:
      x = int(val)
    else:
      y = int(val)
      pixels.append([x,y])
      x = -1
      y = -1
  return pixels

def centerAndUp(corners):
  center = (corners[0][0] + corners[0][1] + corners[0][2] + corners[0][3]) / 4.0
  up = (corners[0][0] + corners[0][1]) / 2.0

  return center, up

def normalizePtUp(posPx, upPx, centerPx, scale):
  distPx = posPx - centerPx
  distNorm = distPx / scale
  upPx = upPx - posPx
  upNorm = upPx / np.sqrt(upPx.dot(upPx))

  return distPx, distNorm, upPx, upNorm

def transformPixels(pixels, perspectiveTransform):
  z = np.reshape(np.array(pixels, np.float32), (len(pixels),1,2))
  z = cv2.perspectiveTransform(z, perspectiveTransform)
  return [x[0] for x in z]

def loadReferenceImage(filename, dstPx, resolution, outresolution, points=None):
  refimage = cv2.imread(filename, cv2.IMREAD_COLOR)

  h,w,c = refimage.shape
  refsrc = [[0, 0], [w, 0], [w, h], [0, h]]
  refPerspectiveTransform = cv2.getPerspectiveTransform(np.array(refsrc, np.float32), np.array(dstPx, np.float32))

  refimageScaledOrig = cv2.warpPerspective(refimage, refPerspectiveTransform, resolution)
  refimageScaledFinal = cv2.warpPerspective(refimage, refPerspectiveTransform, outresolution)

  if points is not None:
    pointsScaled = transformPixels(points, refPerspectiveTransform)
    return refimageScaledOrig, refimageScaledFinal, pointsScaled

  return refimageScaledOrig, refimageScaledFinal

def clientsFromIPs(remotes, port):
  return [udp_client.SimpleUDPClient(remote, port) for remote in remotes]

def sendToClients(clients, endpoint, data):
  oscmsg = json.dumps(data)
  for client in clients:
    try:
      client.send_message(endpoint, oscmsg)
    except Exception as e:
      pass

def setFixedFocus(cameraID, focus):
  commands = [
    "v4l2-ctl -d " + str(cameraID) + " -c focus_auto=0",
    "v4l2-ctl -d " + str(cameraID) + " -c focus_absolute=" + str(focus)
  ]

  for command in commands:
    print(command)
    try:
      process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
      output, error = process.communicate()
      print(output, error)
    except Exception as e:
      print(e)
