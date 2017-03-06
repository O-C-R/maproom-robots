import sys
sys.path.append('..')

import cv2
from os import path

import maproom.constants as c

def markerPath(fname):
  return path.join('..', 'markers', fname)

print('wrote image with dims', c.charucoImgDims, 'maps to size', (c.charucoSqSizeM * c.charucoNSqHoriz, c.charucoSqSizeM * c.charucoNSqVert), 'meters')
print('or', (c.charucoSqSizeM * c.charucoNSqHoriz / c.inToM, c.charucoSqSizeM * c.charucoNSqVert / c.inToM), 'inches')

charucoImg = c.charucoBoard.draw(c.charucoImgDims)
cv2.imwrite(markerPath('charuco-calibration.png'), charucoImg)

pixelSize = c.markerSizeIn * c.imgPPI

robot01 = cv2.aruco.drawMarker(c.markerDictionary, 23, pixelSize)
cv2.imwrite(markerPath('robot01.png'), robot01)

robot02 = cv2.aruco.drawMarker(c.markerDictionary, 24, pixelSize)
cv2.imwrite(markerPath('robot02.png'), robot02)

robot03 = cv2.aruco.drawMarker(c.markerDictionary, 25, pixelSize)
cv2.imwrite(markerPath('robot03.png'), robot03)

robot04 = cv2.aruco.drawMarker(c.markerDictionary, 26, pixelSize)
cv2.imwrite(markerPath('robot04.png'), robot04)

robot05 = cv2.aruco.drawMarker(c.markerDictionary, 27, pixelSize)
cv2.imwrite(markerPath('robot05.png'), robot05)

robot06 = cv2.aruco.drawMarker(c.markerDictionary, 28, pixelSize)
cv2.imwrite(markerPath('robot06.png'), robot06)

robot07 = cv2.aruco.drawMarker(c.markerDictionary, 29, pixelSize)
cv2.imwrite(markerPath('robot07.png'), robot07)

robot08 = cv2.aruco.drawMarker(c.markerDictionary, 30, pixelSize)
cv2.imwrite(markerPath('robot08.png'), robot08)
