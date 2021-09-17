"""
 Copyright (C) 2021 Fern H., Python to Arduino video converter

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
"""

import glob
import sys
import time

import cv2
import numpy as np
import serial

MIN_TIME_BTW_FRAMES = 0.033
MIN_TIME_BTW_LINES = 0.005

# Capture form file
cap = cv2.VideoCapture('Touhou - Bad Apple.mp4')

# Capture from video
# cap = cv2.VideoCapture(2, cv2.CAP_DSHOW)

resized_frame = np.zeros((32, 40), dtype=np.uint8)
ports = []
available_ports = []
serial_buffer = [0] * 5 * 8 * 8


def get_bytes(bits):
    """
    Converts bits array to bytes
    :param bits: bits 1D array
    :return: bytes 8D array of size bits / 8
    """
    bits = iter(bits)
    done = False
    while not done:
        byte = 0
        for _ in range(0, 8):
            try:
                bit = next(bits)
            except StopIteration:
                bit = 0
                done = True
            byte = (byte << 1) | bit
        yield byte


# Get list of serial ports
if sys.platform.startswith('win'):
    ports = ['COM%s' % (i + 1) for i in range(256)]
elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
    ports = glob.glob('/dev/tty[A-Za-z]*')
elif sys.platform.startswith('darwin'):
    ports = glob.glob('/dev/tty.*')
else:
    raise EnvironmentError('Unsupported platform')
for port in ports:
    try:
        s = serial.Serial(port)
        s.close()
        available_ports.append(s.name)
    except (OSError, serial.SerialException):
        pass
print('Available serial ports:', available_ports)

# Open first available serial port
lcd_port = serial.Serial(available_ports[0], 115200)
lcd_port.close()
lcd_port.open()

# Check if capture is opened
if not cap.isOpened():
    print('Error opening video stream or file')

while cap.isOpened():
    ret, frame = cap.read()
    if ret:
        # Store frame start time
        time_start = time.time()

        # Copy frame to show the user
        original_frame = frame.copy()

        # Resize to LCD
        frame = cv2.resize(frame, (40, 30), interpolation=cv2.INTER_AREA)

        # Convert to grayscale
        frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        # Binarize image
        _, frame = cv2.threshold(frame, 127, 255, cv2.THRESH_BINARY)

        # Resize and keep aspect ration
        resized_frame = np.zeros((32, 40), dtype=np.uint8)
        resized_frame[1: 31, 0: 40] = frame[0: 30, 0: 40]

        current_row = 0
        while current_row < 3:
            # Store current line start time
            line_time_start = time.time()

            # Wait and read line index
            current_row = lcd_port.read(1)[0]
            index = current_row * 8

            # Convert 1 row of image
            serial_buffer[0: 40] = np.array(resized_frame[index: index + 8, 0: 5] // 255).flatten()
            serial_buffer[40: 80] = np.array(resized_frame[index: index + 8, 5: 10] // 255).flatten()
            serial_buffer[80: 120] = np.array(resized_frame[index: index + 8, 10: 15] // 255).flatten()
            serial_buffer[120: 160] = np.array(resized_frame[index: index + 8, 15: 20] // 255).flatten()
            serial_buffer[160: 200] = np.array(resized_frame[index: index + 8, 20: 25] // 255).flatten()
            serial_buffer[200: 240] = np.array(resized_frame[index: index + 8, 25: 30] // 255).flatten()
            serial_buffer[240: 280] = np.array(resized_frame[index: index + 8, 30: 35] // 255).flatten()
            serial_buffer[280: 320] = np.array(resized_frame[index: index + 8, 35: 40] // 255).flatten()

            # Convert bits to bytes
            serial_bytes = []
            for serial_byte in get_bytes(serial_buffer):
                serial_bytes.append(serial_byte)
            serial_bytes.pop()

            # Write to serial port
            lcd_port.write(serial_bytes)

            # Check line time
            while time.time() - line_time_start < MIN_TIME_BTW_LINES:
                pass

        # Show current frame
        cv2.imshow('Frame', original_frame)

        # Check frame time
        while time.time() - time_start < MIN_TIME_BTW_FRAMES:
            pass

        # Press Q on keyboard to exit
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    else:
        break

cap.release()
cv2.destroyAllWindows()
