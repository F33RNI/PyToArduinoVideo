/*
 * Copyright (C) 2021 Fern H., Python to Arduino video converter
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// I2C LCD Library
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Serial port speed
#define SERIAL_BAUD_RATE        115200

// Hello world message
#define START_MESSAGE_LINE_1    F("     BAD  APPLE     ")
#define START_MESSAGE_LINE_2    F("       BUT ON       ")
#define START_MESSAGE_LINE_3    F("       20 x 4       ")
#define START_MESSAGE_LINE_4    F("    LCD  DISPLAY    ")

// Delay between each start message
#define START_MESSAGE_TIMEOUT   1000

// Indent (in characters) from the left edge of the display to the picture
#define CURSOR_LEFT_MARGIN      6

// Define custom chars write function
#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

// Define 20x4 display
LiquidCrystal_I2C lcd(0x27, 20, 4);

// System variables
uint8_t i;
uint64_t serial_buffer;
uint8_t char_buffer[8];
uint8_t serial_bytes_buffer[40], serial_bytes_buffer_position;
uint8_t row_counter;
uint8_t row_states[8];

void setup()
{
    // Initialize serial port and lcd
    Serial.begin(SERIAL_BAUD_RATE);
    lcd.init();

    // Enable backlight
    lcd.backlight();

    // Print hello world message
    lcd.setCursor(0, 0);
    delay(START_MESSAGE_TIMEOUT);
    lcd.print(START_MESSAGE_LINE_1);
    lcd.setCursor(0, 1);
    delay(START_MESSAGE_TIMEOUT);
    lcd.print(START_MESSAGE_LINE_2);
    lcd.setCursor(0, 2);
    delay(START_MESSAGE_TIMEOUT);
    lcd.print(START_MESSAGE_LINE_3);
    lcd.setCursor(0, 3);
    delay(START_MESSAGE_TIMEOUT);
    lcd.print(START_MESSAGE_LINE_4);
    delay(START_MESSAGE_TIMEOUT);
    delay(START_MESSAGE_TIMEOUT);
    lcd.noBacklight();
    delay(START_MESSAGE_TIMEOUT);


    // Print start message (line number = 0)
    Serial.write(0);
}

void loop()
{
    // New serial data
    if (Serial.available() >= 40) {
        // Enable backlight on new data
        lcd.backlight();

        // Read all bytes to the buffer
        while (Serial.available()) {
            serial_bytes_buffer[serial_bytes_buffer_position] = Serial.read();
            serial_bytes_buffer_position++;
            if (serial_bytes_buffer_position >= 40)
                break;
        }
        serial_bytes_buffer_position = 0;

        // Clear previous lines
        lcd.clear();

        for (i = 0; i < 8; i++) {
            // Parse 5 bytes into 40 bits
            serial_buffer = (uint64_t)serial_bytes_buffer[i * 5] << 32
                | (uint64_t)serial_bytes_buffer[i * 5 + 1] << 24
                | (uint64_t)serial_bytes_buffer[i * 5 + 2] << 16
                | (uint64_t)serial_bytes_buffer[i * 5 + 3] << 8
                | (uint64_t)serial_bytes_buffer[i * 5 + 4];

            // Create 2D custom char buffer
            char_buffer[0] = (serial_buffer >> 35) & 0x1F;
            char_buffer[1] = (serial_buffer >> 30) & 0x1F;
            char_buffer[2] = (serial_buffer >> 25) & 0x1F;
            char_buffer[3] = (serial_buffer >> 20) & 0x1F;
            char_buffer[4] = (serial_buffer >> 15) & 0x1F;
            char_buffer[5] = (serial_buffer >> 10) & 0x1F;
            char_buffer[6] = (serial_buffer >> 5) & 0x1F;
            char_buffer[7] = serial_buffer & 0x1F;

            // If all bytes are 0 or 1 no need to create custom symbols
            row_states[i] = 0;
            if (!serial_buffer)
                row_states[i] = 1;
            else if (serial_buffer == 1099511627775LL)
                row_states[i] = 2;

            // Create custom symbols
            if (!row_states[i])
                lcd.createChar(i, char_buffer);
        }

        // Set cursor at the begging of new line
        lcd.setCursor(CURSOR_LEFT_MARGIN, row_counter);
        
        // Fill the whole 8 chars line
        for (i = 0; i < 8; i++) {
            if (!row_states[i]) {
                lcd.printByte(i);
            }
            else if (row_states[i] == 2) {
                lcd.printByte(255);
            }
            else {
                lcd.printByte(32);
            }
        }

        // Increment row counter
        row_counter++;
        if (row_counter >= 4)
            row_counter = 0;


        // Send row number as ready symbol
        Serial.write(row_counter);
    }
}
