# tello-remote
A remote controller for the Tello drone based on the ESP32. A video on this can be found on [YouTube](https://youtu.be/NZ7PIbjot90).

## The Hardware

The remote controller case is 3D printed. You can find the STL as well as the Fusion360 files in the 3D folder. 

Everything is controlled by a NODEMCU ESP32 development board. I've attached 10 LED to show various information. The toggle switches and joy sticks are also attached to the ESP32. It is fairly straight forward and should become clear from the code for the ESP32. If you need more information, just let me know. 
Three inputs come from a [PicoPlanet by Bleeptrack](https://www.tindie.com/products/bleeptrack/picoplanet/). But you could also use buttons or switches instead. 

I'm powering the device with four AA batteries connected to GND and 5V on the ESP. The board should be able to regulate any voltage up to 12V. 

## The Software

Check out the ESP32 sketch. It is flashed to the controller through the Arduino IDE. 

## Resources

During the creation of this project, I mainly used the forum and wiki from https://tellopilots.com/ as well as the [Tello SDK 1.3 documentation](https://terra-1-g.djicdn.com/2d4dce68897a46b19fc717f3576b7c6a/Tello%20%E7%BC%96%E7%A8%8B%E7%9B%B8%E5%85%B3/For%20Tello/Tello%20SDK%20Documentation%20EN_1.3_1122.pdf). 

There's also already projects online that implement Tello commands on an ESP32, such as https://github.com/anoochit/tello-esp32 and https://github.com/flexlab-ruc/ESP32-TelloDrone. 
