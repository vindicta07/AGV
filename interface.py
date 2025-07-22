"""
Author: Yash Pathak [Github: vindicta07]
Email: yashpradeeppathak@gmail.com
Description: UDP-based robot control interface using keyboard input
"""

import socket
import keyboard
import time
import threading

# IP and Port of the UDP receiver (replace with your robot's IP and port)
UDP_IP = "192.168.1.XXX"  # Change this to the IP of your robot
UDP_PORT = 12345           # Change this to the port you set for UDP communication

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Initialize last_command variable to keep track of the last command sent
last_command = "S"  # Default to stop

# Function to send commands over UDP to the robot
def send_command(command):
    message = command.encode()  # Encode the command into bytes
    sock.sendto(message, (UDP_IP, UDP_PORT))  # Send the command to the specified IP and port
    print(f"Sent command: {command}")  # For debugging

# Function to send a stop command after a specified duration
def send_stop_after_delay(delay):
    global last_command
    time.sleep(delay)  # Wait for the specified delay
    last_command = "S"  # Set command to stop
    send_command(last_command)  # Send stop command

# Function to set the last command based on key press
def on_key_press(event):
    global last_command
    if event.name == 'up':
        last_command = "F"  # Forward
        send_command(last_command)
        threading.Thread(target=send_stop_after_delay, args=(0.5,), daemon=True).start()
    elif event.name == 'down':
        last_command = "B"  # Backward
        send_command(last_command)
        threading.Thread(target=send_stop_after_delay, args=(0.5,), daemon=True).start()
    elif event.name == 'left':
        last_command = "A"  # Left
        send_command(last_command)
        threading.Thread(target=send_stop_after_delay, args=(0.5,), daemon=True).start()
    elif event.name == 'right':
        last_command = "C"  # Right
        send_command(last_command)
        threading.Thread(target=send_stop_after_delay, args=(0.5,), daemon=True).start()
    elif event.name == 's':  # Stop command
        last_command = "S"  # Stop
        send_command(last_command)

# Register the key press events
keyboard.on_press(on_key_press)

print("Press arrow keys to control the robot. Press 'S' to stop. Press 'Esc' to exit.")

# Keep the program running until the 'Esc' key is pressed
keyboard.wait('esc')

# Close the UDP socket when done
sock.close()
print("UDP socket closed.")
