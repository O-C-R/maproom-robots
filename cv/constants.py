import cv2

img_ppi = 72
in_to_m = 0.0254

charuco_n_sq_horiz = 3
charuco_n_sq_vert = 4
charuco_sq_size_m = 0.065
charuco_marker_size_m = 0.045

marker_size_in = 5
marker_size_m = marker_size_in * in_to_m

charuco_dictionary = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_50)
charuco_board = cv2.aruco.CharucoBoard_create(charuco_n_sq_horiz, charuco_n_sq_vert, charuco_sq_size_m, charuco_marker_size_m, charuco_dictionary)

marker_dictionary = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_5X5_50)

robot_01_marker_img = cv2.aruco.drawMarker(marker_dictionary, 23, marker_size_in * img_ppi)
robot_02_marker_img = cv2.aruco.drawMarker(marker_dictionary, 24, marker_size_in * img_ppi)
