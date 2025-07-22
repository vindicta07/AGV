"""
Author: Yash Pathak [Github: vindicta07]
Email: yashpradeeppathak@gmail.com
Description: ArUco marker detection and processing utilities
"""

import cv2
from cv2 import aruco
import numpy as np
import math

def detect_ArUco_details(image):
    ArUco_details_dict = {}
    ArUco_corners = {}
    
    # Define the ArUco dictionary (DICT_4X4_250 in this case)
    aruco_dict = cv2.aruco.getPredefinedDictionary(aruco.DICT_4X4_250)

    # Detect ArUco markers in the input image
    corners, ids, _ = aruco.detectMarkers(image, aruco_dict)

    # Check if ArUco markers were detected
    if ids is not None:
        for i in range(len(ids)):
            # Extract the ArUco marker ID (cast to an integer)
            id = int(ids[i][0])

            # Extract the corner coordinates of the detected marker
            corner_coordinates = corners[i][0]

            # Calculate the center of the marker by taking the mean of x and y coordinates
            center_x = int(np.mean(corner_coordinates[:, 0]))
            center_y = int(np.mean(corner_coordinates[:, 1]))

            # Calculate the angle from the vertical axis
            angle = int(math.degrees(math.atan2(corner_coordinates[1][1] - corner_coordinates[0][1],
                                                corner_coordinates[1][0] - corner_coordinates[0][0])))

            # Store the details in dictionaries
            ArUco_details_dict[id] = [[center_x, center_y], angle]
            ArUco_corners[id] = corner_coordinates

    return ArUco_details_dict, ArUco_corners 

def mark_ArUco_image(image, ArUco_details_dict, ArUco_corners):
    for ids, details in ArUco_details_dict.items():
        center = details[0]
        cv2.circle(image, center, 5, (0,0,255), -1)

        corner = ArUco_corners[int(ids)]
        cv2.circle(image, (int(corner[0][0]), int(corner[0][1])), 5, (50, 50, 50), -1)
        cv2.circle(image, (int(corner[1][0]), int(corner[1][1])), 5, (0, 255, 0), -1)
        cv2.circle(image, (int(corner[2][0]), int(corner[2][1])), 5, (128, 0, 255), -1)
        cv2.circle(image, (int(corner[3][0]), int(corner[3][1])), 5, (25, 255, 255), -1)

        tl_tr_center_x = int((corner[0][0] + corner[1][0]) / 2)
        tl_tr_center_y = int((corner[0][1] + corner[1][1]) / 2) 

        cv2.line(image,center,(tl_tr_center_x, tl_tr_center_y),(255,0,0),5)
        display_offset = int(math.sqrt((tl_tr_center_x - center[0])**2+(tl_tr_center_y - center[1])**2))
        cv2.putText(image,str(ids),(center[0]+int(display_offset/2),center[1]),cv2.FONT_HERSHEY_COMPLEX, 1, (0, 0, 255), 2)
        angle = details[1]
        cv2.putText(image,str(angle),(center[0]-display_offset,center[1]),cv2.FONT_HERSHEY_COMPLEX, 1, (0, 255, 0), 2)
    return image

# Real-time video capture
if __name__ == "__main__":
    cap = cv2.VideoCapture(0)  # Capture from webcam

    if not cap.isOpened():
        print("Error: Could not open video stream.")
        exit()

    while True:
        ret, frame = cap.read()

        if not ret:
            print("Failed to grab frame")
            break

        # Detect ArUco markers
        ArUco_details_dict, ArUco_corners = detect_ArUco_details(frame)
        
        # Mark the ArUco markers on the frame
        frame = mark_ArUco_image(frame, ArUco_details_dict, ArUco_corners)
        
        # Display the frame
        cv2.imshow("Aruco Marker Detection", frame)

        # Exit on 'q' key press
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # Release video capture object and close windows
    cap.release()
    cv2.destroyAllWindows()
