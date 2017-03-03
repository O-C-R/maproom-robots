import cv2
import constants

print('wrote image with dims', constants.charuco_img_dims, 'maps to size', (constants.charuco_sq_size_m * constants.charuco_n_sq_horiz, constants.charuco_sq_size_m * constants.charuco_n_sq_vert), 'meters')
print('or', (constants.charuco_sq_size_m * constants.charuco_n_sq_horiz / constants.in_to_m, constants.charuco_sq_size_m * constants.charuco_n_sq_vert / constants.in_to_m), 'inches')

charuco_img = constants.charuco_board.draw(constants.charuco_img_dims)
cv2.imwrite('markers/charuco-calibration.png', charuco_img)

pixel_size = constants.marker_size_in * constants.img_ppi

robot01 = cv2.aruco.drawMarker(constants.marker_dictionary, 23, pixel_size)
cv2.imwrite('markers/robot01.png', robot01)

robot02 = cv2.aruco.drawMarker(constants.marker_dictionary, 24, pixel_size)
cv2.imwrite('markers/robot02.png', robot02)

robot03 = cv2.aruco.drawMarker(constants.marker_dictionary, 25, pixel_size)
cv2.imwrite('markers/robot03.png', robot03)

robot04 = cv2.aruco.drawMarker(constants.marker_dictionary, 26, pixel_size)
cv2.imwrite('markers/robot04.png', robot04)

robot05 = cv2.aruco.drawMarker(constants.marker_dictionary, 27, pixel_size)
cv2.imwrite('markers/robot05.png', robot05)

robot06 = cv2.aruco.drawMarker(constants.marker_dictionary, 28, pixel_size)
cv2.imwrite('markers/robot06.png', robot06)

robot07 = cv2.aruco.drawMarker(constants.marker_dictionary, 29, pixel_size)
cv2.imwrite('markers/robot07.png', robot07)

robot08 = cv2.aruco.drawMarker(constants.marker_dictionary, 30, pixel_size)
cv2.imwrite('markers/robot08.png', robot08)
