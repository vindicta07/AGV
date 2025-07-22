"""
Author: Yash Pathak [Github: vindicta07]
Email: yashpradeeppathak@gmail.com
Description: Simple UDP command interface for robot control using keyboard
"""

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

# Define action on key press
def on_press(key):
    try:
        # Check for arrow key presses and send corresponding commands
        if key == keyboard.Key.up:
            send_command("F")  # Move forward
        elif key == keyboard.Key.down:
            send_command("B")  # Move backward
        elif key == keyboard.Key.left:
            send_command("L")  # Turn left
        elif key == keyboard.Key.right:
            send_command("R")  # Turn right
    except AttributeError:
        pass

# Define action on key release (for stop)
def on_release(key):
    # Stop movement when arrow keys are released
    if key in [keyboard.Key.up, keyboard.Key.down, keyboard.Key.left, keyboard.Key.right]:
        send_command("S")  # Send stop command

    # Exit the listener when 'esc' is pressed
    if key == keyboard.Key.esc:
        return False

# Start listening for key presses
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()
