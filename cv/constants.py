import cv2
import math

img_ppi = 300
in_to_m = 0.0254

poster_max_width_in = 17
poster_max_height_in = 23
poster_max_width_m = poster_max_width_in * in_to_m
poster_max_height_m = poster_max_height_in * in_to_m

charuco_n_sq_vert = 10

charuco_sq_size_m = float(poster_max_height_m) / float(charuco_n_sq_vert)
charuco_marker_size_m = charuco_sq_size_m * 0.7

charuco_n_sq_horiz = int(poster_max_width_m / charuco_sq_size_m)

charuco_dictionary = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_50)
charuco_board = cv2.aruco.CharucoBoard_create(charuco_n_sq_horiz, charuco_n_sq_vert, charuco_sq_size_m, charuco_marker_size_m, charuco_dictionary)

charuco_dims_m = charuco_img_dims = (charuco_n_sq_horiz * charuco_sq_size_m / in_to_m, charuco_n_sq_vert * charuco_sq_size_m / in_to_m)
charuco_img_dims = (int(charuco_dims_m[0] * img_ppi), int(charuco_dims_m[1] * img_ppi))

marker_dictionary = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_5X5_50)

marker_size_in = 5
marker_size_m = marker_size_in * in_to_m
marker_sm_px = 64
