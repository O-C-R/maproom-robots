import cv2
import constants

charuco_img_dims = (constants.img_ppi * constants.charuco_n_sq_horiz, constants.img_ppi * constants.charuco_n_sq_vert)
charuco_img = constants.charuco_board.draw(charuco_img_dims)
cv2.imwrite('markers/charuco-calibration.png', charuco_img)
print('wrote image with dims', charuco_img_dims, 'maps to size', (constants.charuco_sq_size_m * constants.charuco_n_sq_horiz, constants.charuco_sq_size_m * constants.charuco_n_sq_vert), 'meters')
print('or', (constants.charuco_sq_size_m * constants.charuco_n_sq_horiz / constants.in_to_m, constants.charuco_sq_size_m * constants.charuco_n_sq_vert / constants.in_to_m), 'inches')

robot01 = cv2.aruco.drawMarker(constants.marker_dictionary, 23, constants.marker_size_in * constants.img_ppi)
cv2.imwrite('markers/robot01.png', robot01)

robot02 = cv2.aruco.drawMarker(constants.marker_dictionary, 24, constants.marker_size_in * constants.img_ppi)
cv2.imwrite('markers/robot02.png', robot02)
