# tello-remote
A remote controller for the Tello drone based on the ESP32. A video on this can be found on [YouTube](https://youtu.be/NZ7PIbjot90).

## The Hardware

The remote controller case is 3D printed. You can find the STL as well as the Fusion360 files in the 3D folder. 

Everything is controlled by a NODEMCU ESP32 development board. I've attached 10 LED to show various information. The toggle switches and joy sticks are also attached to the ESP32. It is fairly straight forward and should become clear from the code for the ESP32. If you need more information, just let me know. 
Three inputs come from a [PicoPlanet by Bleeptrack](https://www.tindie.com/products/bleeptrack/picoplanet/). But you could also use buttons or switches instead. 

I'm powering the device with four AA batteries connected to GND and 5V on the ESP. The board should be able to regulate any voltage up to 12V. 

## The Software

Check out the ESP32 sketch. It is flashed to the controller through the Arduino IDE. 
