/*
 * Author: Divyanshu Modi (divyan.m05@gmail.com)
 * Co-Author: Yash Pathak [Github: vindicta07] (yashpradeeppathak@gmail.com)
 * Description: ESP32 PS4 Controller interface for robot control via Cytron MDDS 10 motor driver
 *              Configured for UART communication with dual motor control system
 * Original copyright (c) 2024, TEAM RAW
 * Date: 19th June 2024 12:30:00 
 */

#include <HardwareSerial.h>
#include <PS4Controller.h>

// Variables to store received data
ps4_t ps_data = {0};

// UART port definitions
#define UART_PORT_1_TX 16
#define UART_PORT_2_TX 17

#define DEBUG 0

#define SERIAL1 Serial1
#define SERIAL2 Serial2

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
  Serial.printf("uartWrite: Port: %d, cmd: %d\n", port, speed);

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
  PS4.begin("XX:XX:XX:XX:XX:XX");  // Replace with your PS4 controller MAC address
  Serial.println("Ready.");
  xTaskCreate(ps_handler, "PS_HANDLER", 2048, NULL, 1, NULL);
}

void ps_handler(void *data) {
  while (1) {
    ps_data = PS4.data;
    delay(20);
  }
}

// Main loop function
void loop() {
  delay(20);
  ps4_analog_t ps_analog = ps_data.analog;
  ps4_button_t ps_button = ps_data.button;
  int8_t LY = ps_analog.stick.ly;
  int8_t RX = ps_analog.stick.rx;
  int8_t LX = ps_analog.stick.lx;
  int8_t RY = ps_analog.stick.ry;
  bool L1 = ps_button.l1;
  bool R1 = ps_button.r1;

  Serial.printf(" LY: %d, RX: %d, L1:%d, R1: %d\n", LY, RX, L1, R1);

  // Control logic based on received inputs
  if (LY < -10) {
    if (RX > 10) {
      back_right();
    } else if (RX < -10) {
      back_left();
    } else {
      backwards();
    }
  } else if (LY > 10) {
    if (RX > 10) {
      forw_right();
    } else if (RX < -10) {
      forw_left();
    } else {
      forwards();
    }
  } else if (RX > 10) {
    turnright();
  } else if (RX < -10) {
    turnleft();
  } else {
    stop();
  }
  forwards();

  if(LY > 30)
  {
    backwards();
  }
  else if(LY < -30)
  {
    forwards();
  }

  else if(RX > 30)
  {
    forw_right();
  }

  else if(RX < -30)
  {
    forw_left();
  }

  else if (R1)
  {
    turnright();
  }

  else if(L1)
  {
    turnleft();
  }

  else
  {
    stop();
  }

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
  uart1Write(UART_SIG_LCHAN, UART_SIG_CCW, 60);
  uart2Write(UART_SIG_LCHAN, UART_SIG_CW, 60);
  uart1Write(UART_SIG_RCHAN, UART_SIG_CCW, 60);
  uart2Write(UART_SIG_RCHAN, UART_SIG_CW, 60);
}

void turnleft() {
  uart1Write(UART_SIG_LCHAN, UART_SIG_CW, 60);
  uart2Write(UART_SIG_LCHAN, UART_SIG_CCW, 60);
  uart1Write(UART_SIG_RCHAN, UART_SIG_CW, 60);
  uart2Write(UART_SIG_RCHAN, UART_SIG_CCW, 60);
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

void stop() {
  uart1Write(UART_SIG_LCHAN, UART_SIG_CCW, 0);
  uart2Write(UART_SIG_LCHAN, UART_SIG_CCW, 0);
  uart1Write(UART_SIG_RCHAN, UART_SIG_CCW, 0);
  uart2Write(UART_SIG_RCHAN, UART_SIG_CCW, 0);
}


void back_right() {
  uart1Write(UART_SIG_LCHAN, UART_SIG_CW, 60);
  uart2Write(UART_SIG_LCHAN, UART_SIG_CW, 60);
  uart1Write(UART_SIG_RCHAN, UART_SIG_CCW, 60);
  uart2Write(UART_SIG_RCHAN, UART_SIG_CCW, 60);
}

void back_left() {
  uart1Write(UART_SIG_LCHAN, UART_SIG_CW, 60);
  uart2Write(UART_SIG_LCHAN, UART_SIG_CW, 60);
  uart1Write(UART_SIG_RCHAN, UART_SIG_CW, 60);
  uart2Write(UART_SIG_RCHAN, UART_SIG_CW, 60);
}