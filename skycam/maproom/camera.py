import cv2
import numpy as np

from .videostream import VideoStream
from . import constants as c
from . import util as u

class MaproomCamera:
  def __init__(self, id, resolution=c.resolution):
    self.id = id
    self.resolution = resolution
    self.vs = VideoStream(src=id, resolution=resolution)

    self.cameraMatrix = None
    self.distCoeffs = None
    self.perspectiveTransform = None
    self.newCameraMatrix = None
    self.validPixROI = None
    self.mapx = None
    self.mapy = None
    self.heightTransform = None

    self.frame = None
    self.gray = None

  def load(self, calibrationsPath, loadCameraMatrix=True, loadPerspective=True, loadHeight=True):
    if loadCameraMatrix:
      cameraMatrix, distCoeffs = u.loadCalibration(calibrationsPath, self.id)
      self.setCameraMatrix(cameraMatrix, distCoeffs)

    if loadPerspective and self.cameraMatrix is not None and self.distCoeffs is not None:
      perspectiveTransform = u.loadPerspective(calibrationsPath, self.id)
      self.setPerspective(perspectiveTransform)

    if loadHeight:
      heightTransform = u.loadHeight(calibrationsPath, self.id)
      self.setHeight(heightTransform)

  def setCameraMatrix(self, cameraMatrix, distCoeffs):
    if cameraMatrix is None or distCoeffs is None:
      self.cameraMatrix = None
      self.distCoeffs = None
      self.newCameraMatrix = None
      self.validPixROI = None
      self.mapx = None
      self.mapy = None
      return

    self.cameraMatrix = cameraMatrix
    self.distCoeffs = distCoeffs

    self.newCameraMatrix, self.validPixROI = cv2.getOptimalNewCameraMatrix(self.cameraMatrix, self.distCoeffs, self.resolution, 0)
    self.mapx, self.mapy = cv2.initUndistortRectifyMap(self.cameraMatrix, self.distCoeffs, None, self.newCameraMatrix, self.resolution, 5)

  def setPerspective(self, perspectiveTransform):
    self.perspectiveTransform = perspectiveTransform

  def setHeight(self, heightTransform):
    self.heightTransform = heightTransform

  def start(self):
    self.vs.start()

  def update(self):
    self.frame = self.vs.read()
    self.gray = cv2.cvtColor(self.frame, cv2.COLOR_BGR2GRAY)

    return self.frame, self.gray

  def lastFrame(self):
    return self.frame, self.gray

  def stop(self):
    self.vs.stop()

  def undistort(self, gray=False):
    if self.frame is None:
      return None

    if gray:
      frame = self.gray
    else:
      frame = self.frame

    if self.mapx is not None and self.mapy is not None:
      frame = cv2.remap(frame, self.mapx, self.mapy, cv2.INTER_LINEAR)
    if self.perspectiveTransform is not None:
      frame = cv2.warpPerspective(frame, self.perspectiveTransform, c.mappedImageResolution)

    return frame

  def detectAruco(self):
    markerCorners, markerIds, rejected = cv2.aruco.detectMarkers(self.gray, c.markerDictionary, parameters=c.detectorParams)
    return markerCorners, markerIds

  def transformMarkers(self, markerCorners, markerIds, imgCenter=c.mappedImageCenter, imgScale=c.outImageMappedHeight):
    positions = {}
    imgCenterPx = np.divide(np.array(c.mappedImageResolution), 2.0)

    for i in range(len(markerCorners)):
      markerId = int(markerIds[i])
      markerCornerSet = markerCorners[i]

      # Undistort
      if self.cameraMatrix is not None and self.distCoeffs is not None:
        markerCornerSet = cv2.undistortPoints(markerCornerSet, self.cameraMatrix, self.distCoeffs, P=self.newCameraMatrix)

      # calculate the center and up direction for the markers
      rawMarkerCenterPx, rawMarkerUpPx = u.centerAndUp(markerCornerSet)
      if self.heightTransform is not None:
        markerCenterPx, markerUpPx = u.transformPixels([rawMarkerCenterPx, rawMarkerUpPx], self.heightTransform)
      else:
        markerCenterPx, markerUpPx = rawMarkerCenterPx, rawMarkerUpPx

      if self.perspectiveTransform is not None:
        markerCenterPx, markerUpPx = u.transformPixels([markerCenterPx, markerUpPx], self.perspectiveTransform)
        rawMarkerCenterPx, rawMarkerUpPx = u.transformPixels([rawMarkerCenterPx, rawMarkerUpPx], self.perspectiveTransform)

      rawDistPx, rawDistNorm, rawUpPx, rawUpNorm = u.normalizePtUp(rawMarkerCenterPx, rawMarkerUpPx, imgCenter, imgScale)
      distPx, distNorm, upPx, upNorm = u.normalizePtUp(markerCenterPx, markerUpPx, imgCenter, imgScale)

      positions[markerId] = {
        'pos': distNorm,
        'up': upNorm,
        'rawPos': rawDistNorm,
        'rawUp': rawUpNorm,

        'markerCenterPx': markerCenterPx,
        'markerUpPx': markerUpPx,
        'rawMarkerCenterPx': rawMarkerCenterPx,
        'rawMarkerUpPx': rawMarkerUpPx
      }

    return positions
