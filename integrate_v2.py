"""
Author: Yash Pathak [Github: vindicta07]
Email: yashpradeeppathak@gmail.com
Description: Advanced ArUco marker detection with keyboard control and navigation
"""

import cv2
from cv2 import aruco
import numpy as np
import math
import socket
from pynput import keyboard

# IP and Port of the UDP receiver (replace with your robot's IP and port)
UDP_IP = "192.168.1.XXX"  # Change this to the IP of your robot
UDP_PORT = 12345           # Change this to the port you set for UDP communication

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Function to send UDP commands
def send_command(command):
    message = command.encode()  # Encode the command into bytes
    sock.sendto(message, (UDP_IP, UDP_PORT))  # Send the command to the specified IP and port
    print(f"Sent command: {command}")

# Detect ArUco markers and retrieve details
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

            # Skip ArUco markers with IDs 24 and 48
            if id in [24, 48]:
                continue

            # Extract the corner coordinates of the detected marker
            corner_coordinates = corners[i][0]

            # Calculate the center of the marker by taking the mean of x and y coordinates
            center_x = int(np.mean(corner_coordinates[:, 0]))
            center_y = int(np.mean(corner_coordinates[:, 1]))

            # Store the details in dictionaries
            ArUco_details_dict[id] = [center_x, center_y, corner_coordinates]
            ArUco_corners[id] = corner_coordinates

    return ArUco_details_dict, ArUco_corners 

# Mark detected ArUco markers on the image
def mark_ArUco_image(image, ArUco_details_dict, ArUco_corners):
    for ids, details in ArUco_details_dict.items():
        center = details[0:2]
        cv2.circle(image, tuple(center), 5, (0, 0, 255), -1)

        corner = ArUco_corners[int(ids)]
        cv2.polylines(image, [np.int32(corner)], True, (0, 255, 0), 2)

        cv2.putText(image, str(ids), (center[0], center[1] - 10), cv2.FONT_HERSHEY_COMPLEX, 1, (255, 0, 0), 2)
    return image

# Handle ArUco-based robot control
def handle_robot_movement(ArUco_details_dict, frame_width, frame_height):
    # Only focus on marker 72
    if 72 in ArUco_details_dict:
        marker_id = 72
        center_x, center_y, corners = ArUco_details_dict[marker_id]

        # Calculate the size of the marker using the distance between the diagonal corners
        marker_size = math.sqrt((corners[0][0] - corners[2][0])**2 + (corners[0][1] - corners[2][1])**2)

        # Determine the threshold for stopping the robot (based on the size of the marker)
        threshold_size = 200  # Adjust this based on your needs

        # Check if the marker size exceeds the threshold
        if marker_size >= threshold_size:
            send_command("S")  # Stop if the marker is large enough (close)
            print(f"Stopping for marker {marker_id} (size: {marker_size})")
        else:
            # Determine the direction to move based on the marker's position in the frame
            if center_x < frame_width // 3:
                send_command("L")  # Turn left if the marker is on the left side
                print(f"Turning left towards marker {marker_id}")
            elif center_x > 2 * frame_width // 3:
                send_command("R")  # Turn right if the marker is on the right side
                print(f"Turning right towards marker {marker_id}")
            else:
                send_command("F")  # Move forward if the marker is centered
                print(f"Moving forward towards marker {marker_id} (size: {marker_size})")
    else:
        # Stop if marker 72 is not detected
        send_command("S")
        print("Marker 72 not detected, stopping.")

# Keyboard control functions
def on_press(key):
    global aruco_navigation
    try:
        if key.char == 'a':  # Start ArUco navigation
            aruco_navigation = True
            print("ArUco navigation started.")
        elif key.char == 's':  # Stop ArUco navigation
            aruco_navigation = False
            send_command("S")  # Stop any movement
            print("ArUco navigation stopped.")
    except AttributeError:
        # Handle arrow keys and other special keys regardless of navigation state
        if key == keyboard.Key.up:
            send_command("F")  # Move forward
            print("Moving forward")
        elif key == keyboard.Key.down:
            send_command("B")  # Move backward
            print("Moving backward")
        elif key == keyboard.Key.left:
            send_command("L")  # Turn left
            print("Turning left")
        elif key == keyboard.Key.right:
            send_command("R")  # Turn right
            print("Turning right")

def on_release(key):
    # Stop movement when arrow keys are released
    if key in [keyboard.Key.up, keyboard.Key.down, keyboard.Key.left, keyboard.Key.right]:
        send_command("S")  # Send stop command

    # Exit the listener when 'esc' is pressed
    if key == keyboard.Key.esc:
        return False

# Real-time video capture
if __name__ == "__main__":
    cap = cv2.VideoCapture(0)  # Capture from webcam

    if not cap.isOpened():
        print("Error: Could not open video stream.")
        exit()

    # Start listening for keyboard inputs in a separate thread
    listener = keyboard.Listener(on_press=on_press, on_release=on_release)
    listener.start()

    # Global variable for ArUco navigation
    aruco_navigation = False

    while True:
        ret, frame = cap.read()

        if not ret:
            print("Failed to grab frame")
            break

        # Get frame dimensions
        frame_height, frame_width, _ = frame.shape

        # Detect ArUco markers
        ArUco_details_dict, ArUco_corners = detect_ArUco_details(frame)

        # Handle robot movement based on detected ArUco markers
        if aruco_navigation:
            handle_robot_movement(ArUco_details_dict, frame_width, frame_height)

        # Mark the ArUco markers on the frame
        frame = mark_ArUco_image(frame, ArUco_details_dict, ArUco_corners)

        # Display the frame
        cv2.imshow("Aruco Marker Detection with Bounding Box", frame)

        # Exit on 'q' key press
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # Release video capture object and close windows
    cap.release()
    cv2.destroyAllWindows()
