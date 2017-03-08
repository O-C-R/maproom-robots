import math

import cv2
import numpy as np

imgPPI = 300
inToM = 0.0254

posterMaxWidthIn = 17
posterMaxHeightIn = 23
posterMaxWidthM = posterMaxWidthIn * inToM
posterMaxHeightM = posterMaxHeightIn * inToM

charucoNSqVert = 10
charucoSqSizeM = float(posterMaxHeightM) / float(charucoNSqVert)
charucoMarkerSizeM = charucoSqSizeM * 0.7
charucoNSqHoriz = int(posterMaxWidthM / charucoSqSizeM)

charucoDictionary = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_50)
charucoBoard = cv2.aruco.CharucoBoard_create(charucoNSqHoriz, charucoNSqVert, charucoSqSizeM, charucoMarkerSizeM, charucoDictionary)
charucoDimsM = charucoImgDims = (charucoNSqHoriz * charucoSqSizeM / inToM, charucoNSqVert * charucoSqSizeM / inToM)
charucoImgDims = (int(charucoDimsM[0] * imgPPI), int(charucoDimsM[1] * imgPPI))

markerDictionary = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_5X5_50)
markerSizeIn = 5
markerSizeM = markerSizeIn * inToM

detectorParams = cv2.aruco.DetectorParameters_create()
detectorParams.doCornerRefinement = True
detectorParams.cornerRefinementMaxIterations = 500
# detectorParams.cornerRefinementWinSize = 1
detectorParams.cornerRefinementMinAccuracy = 0.001
# detectorParams.minMarkerPerimeterRate = 0.05
# detectorParams.maxMarkerPerimeterRate = 0.2
detectorParams.adaptiveThreshWinSizeMin = 10
# detectorParams.adaptiveThreshWinSizeStep = 3
detectorParams.adaptiveThreshWinSizeMax = 10

resolution = (960, 720)
mappedImageDst = np.array([[ 100, 100 ],
                           [ 612, 100 ],
                           [ 612, 612 ],
                           [ 100, 612 ]])
mappedImageResolution = (712, 712)
mappedImageStart = (100,100)
mappedImageSize = (512, 512)
mappedImageCenter = (712.0 / 2.0, 712.0 / 2.0)
outImageMappedHeight = 512

timelapseCaptureTime = 5.0
timelapseProfile = 'stlmaproom-uploader'
timelapseBucket = 'stlmaproom-frames'

STATE_TRACKING = 0
STATE_FLASHLIGHT = 1
