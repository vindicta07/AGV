/*
 * Author: Yash Pathak [Github: vindicta07]
 * Email: yashpradeeppathak@gmail.com
 * Description: ESP32 Arduino code for robot motor control via UDP commands
 */

#include <HardwareSerial.h>
#include <PS4Controller.h>
#include <WiFi.h>
#include <AsyncUDP.h>
#include "DataParser.h"

// Variables to store received data
ps4_t ps_data = { 0 };

// UART port definitions
#define UART_PORT_1_TX 16
#define UART_PORT_2_TX 17

#define DEBUG 0

#define SERIAL1 Serial1
#define SERIAL2 Serial2

AsyncUDP udp;
DataParser dataParser;

// Time tracking variables
unsigned long lastCommandTime = 0;  // Stores the last time a command was received
const unsigned long COMMAND_TIMEOUT = 2000;  // 2 seconds timeout (adjust if needed)
const unsigned long COMMAND_DURATION = 500;   // 0.5 seconds duration for each command

// Enum definitions for UART control
typedef enum {
  UART_SIG_CW = 0,
  UART_SIG_CCW = 1,
  UART_SIG_DIR_MAX,
} uart_ctrl_dir_t;

typedef enum {
  UART_SIG_LCHAN = 0,
  UART_SIG_RCHAN = 1,
  UART_SIG_CHAN_MAX,
} uart_ctrl_chan_t;

typedef enum {
  SERIAL_PORT_1 = 0,
  SERIAL_PORT_2 = 1,
  SERIAL_PORT_MAX,
} uart_custom_port_t;

// Function to write UART data
void __uartWrite(uart_custom_port_t port, uart_ctrl_chan_t chan, uart_ctrl_dir_t dir, uint8_t speed) {
  uint8_t udata = 0;

  // Ensure valid port, channel, and direction
  if (port >= SERIAL_PORT_MAX || chan >= UART_SIG_CHAN_MAX || dir >= UART_SIG_DIR_MAX) {
    return;
  }

  // Mask speed to 6 bits
  speed &= 0x3F;

  // Form the data byte
  udata = (chan << 7) | (dir << 6) | speed;

  // Debug print
  // Serial.printf("uartWrite: Port: %d, cmd: %d\n", port, speed);

  // Send data through the appropriate serial port
  if (port == SERIAL_PORT_1) {
    SERIAL1.write(udata);
  } else {
    SERIAL2.write(udata);
  }
}

// Wrapper functions for each UART port
void uart1Write(uart_ctrl_chan_t chan, uart_ctrl_dir_t dir, uint8_t speed) {
  __uartWrite(SERIAL_PORT_1, chan, dir, speed);
}

void uart2Write(uart_ctrl_chan_t chan, uart_ctrl_dir_t dir, uint8_t speed) {
  __uartWrite(SERIAL_PORT_2, chan, dir, speed);
}

// Setup function
void setup() {
  // Initialize serial ports
  SERIAL1.begin(115200, SERIAL_8N1, -1, UART_PORT_1_TX);
  SERIAL1.flush();
  SERIAL2.begin(115200, SERIAL_8N1, -1, UART_PORT_2_TX);
  SERIAL2.flush();
  Serial.begin(115200);
  // PS4.begin("78:2b:46:91:ef:af");
  // Serial.println("Ready.");
  xTaskCreate(ps_handler, "PS_HANDLER", 2048, NULL, 0, NULL);

  // Setup UDP and WiFi
  WiFi.begin("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  if (udp.listen(12345)) {
    Serial.print("UDP Listening on IP: ");
    Serial.println(WiFi.localIP());
    udp.onPacket([](AsyncUDPPacket packet) {
      String incomingData = (char*)packet.data();
      dataParser.parseData(incomingData, ',');
      handleParsedData(); // Function to handle the parsed data
      lastCommandTime = millis();  // Update the last command time when data is received
    });
  }
}

void ps_handler(void *data) {
  while (1) {
    ps_data = PS4.data;
    delay(20);
  }
}

// Main loop function
void loop() {
  // Check if the timeout has been exceeded
  if (millis() - lastCommandTime > COMMAND_TIMEOUT) {
    stop();  // Stop the motors if no command received within the timeout period
  }
  delay(20);  // Small delay for stability
}

// Motor control functions
void forwards() {
  uart1Write(UART_SIG_LCHAN, UART_SIG_CW, 60);
  uart2Write(UART_SIG_LCHAN, UART_SIG_CW, 60);
  uart1Write(UART_SIG_RCHAN, UART_SIG_CCW, 60);
  uart2Write(UART_SIG_RCHAN, UART_SIG_CCW, 60);
}

void backwards() {
  uart1Write(UART_SIG_LCHAN, UART_SIG_CCW, 60);
  uart2Write(UART_SIG_LCHAN, UART_SIG_CCW, 60);
  uart1Write(UART_SIG_RCHAN, UART_SIG_CW, 60);
  uart2Write(UART_SIG_RCHAN, UART_SIG_CW, 60);
}

void turnright() {
  uart1Write(UART_SIG_LCHAN, UART_SIG_CW, 19);
  uart2Write(UART_SIG_LCHAN, UART_SIG_CCW, 19);
  uart1Write(UART_SIG_RCHAN, UART_SIG_CW, 38);
  uart2Write(UART_SIG_RCHAN, UART_SIG_CW, 38);
}

void turnleft() {
  uart1Write(UART_SIG_LCHAN, UART_SIG_CCW, 19);
  uart2Write(UART_SIG_LCHAN, UART_SIG_CW, 19);
  uart1Write(UART_SIG_RCHAN, UART_SIG_CCW, 38);
  uart2Write(UART_SIG_RCHAN, UART_SIG_CCW, 38);
}

void rotateclk() {
  uart1Write(UART_SIG_LCHAN, UART_SIG_CCW, 38);
  uart2Write(UART_SIG_LCHAN, UART_SIG_CCW, 38);
  uart1Write(UART_SIG_RCHAN, UART_SIG_CCW, 38);
  uart2Write(UART_SIG_RCHAN, UART_SIG_CCW, 38);
}

void rotate_anticlk() {
  uart1Write(UART_SIG_LCHAN, UART_SIG_CW, 38);
  uart2Write(UART_SIG_LCHAN, UART_SIG_CW, 38);
  uart1Write(UART_SIG_RCHAN, UART_SIG_CW, 38);
  uart2Write(UART_SIG_RCHAN, UART_SIG_CW, 38);
}

void stop() {
  uart1Write(UART_SIG_LCHAN, UART_SIG_CCW, 0);
  uart2Write(UART_SIG_LCHAN, UART_SIG_CCW, 0);
  uart1Write(UART_SIG_RCHAN, UART_SIG_CCW, 0);
  uart2Write(UART_SIG_RCHAN, UART_SIG_CCW, 0);
}

void forw_right() {                    // Perfect Right
  uart1Write(UART_SIG_LCHAN, UART_SIG_CCW, 60);
  uart2Write(UART_SIG_LCHAN, UART_SIG_CCW, 60);
  uart1Write(UART_SIG_RCHAN, UART_SIG_CCW, 60);
  uart2Write(UART_SIG_RCHAN, UART_SIG_CCW, 60);
}

void forw_left() {               // Perfect Left
  uart1Write(UART_SIG_LCHAN, UART_SIG_CW, 60);
  uart2Write(UART_SIG_LCHAN, UART_SIG_CW, 60);
  uart1Write(UART_SIG_RCHAN, UART_SIG_CW, 60);
  uart2Write(UART_SIG_RCHAN, UART_SIG_CW, 60);
}

void back_right() {
  uart1Write(UART_SIG_LCHAN, UART_SIG_CW, 0);
  uart2Write(UART_SIG_LCHAN, UART_SIG_CW, 0);
  uart1Write(UART_SIG_RCHAN, UART_SIG_CCW, 38);
  uart2Write(UART_SIG_RCHAN, UART_SIG_CCW, 38);
}

void back_left() {
  uart1Write(UART_SIG_LCHAN, UART_SIG_CW, 38);
  uart2Write(UART_SIG_LCHAN, UART_SIG_CW, 38);
  uart1Write(UART_SIG_RCHAN, UART_SIG_CW, 0);
  uart2Write(UART_SIG_RCHAN, UART_SIG_CW, 0);
}

// Function to handle parsed data
void handleParsedData() {
  String command = dataParser.getField(0);
  int speed = dataParser.getField(1).toInt();

  // Stop the previous command if it was active
  stop();

  if (command == "F") {
    forwards();
    Serial.println("FORWARD");
  } else if (command == "B") {
    backwards();
    Serial.println("BACKWARD");
  } else if (command == "L") {
    forw_left();
    Serial.println("LEFT");
  } else if (command == "R") {
    forw_right();
    Serial.println("RIGHT");
  } else if (command == "S") {
    stop();
    Serial.println("STOP");
  }

  // Set a timer to stop the motors after COMMAND_DURATION milliseconds
  lastCommandTime = millis();  // Update the last command time when data is received
  delay(COMMAND_DURATION);      // Wait for 0.5 seconds
  stop();                       // Stop the motors after 0.5 seconds
}
