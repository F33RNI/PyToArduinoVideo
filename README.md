# PyToArduinoVideo
## Python to Arduino video converter

![Photo](PHOTO.png "Photo")

[![Twitter](https://img.shields.io/twitter/url?style=social&url=https%3A%2F%2Ftwitter.com%2Ffern_hertz)](https://twitter.com/fern_hertz)
[![YouTube](https://img.shields.io/badge/YouTube-Bad%20Apple%20but%20it%20is%20on%20HD44780-red)](https://youtu.be/uPZFti0UoGw)
[![GitHub license](https://img.shields.io/github/license/XxOinvizioNxX/PyToArduinoVideo)](https://github.com/XxOinvizioNxX/PyToArduinoVideo/blob/main/LICENSE)

----------

This project allows you to run video from a computer on a 20x4 LCD display with an HD44780 controller.

The original video is captured by OpenCV (Python), divided into blocks of pixels and sent to the serial port. Each character = 40 bits. Arduino takes 8 characters (one line) and writes them to the display memory. As soon as a new line arrives, the screen is cleared.

The project has no real meaning and is made just for fun :)

----------

## Feedback
- Developer: Fern H.
- E-Mail: xxoinvizionxx@gmail.com
- GitHub: https://github.com/XxOinvizioNxX
- Twitter: https://twitter.com/fern_hertz
