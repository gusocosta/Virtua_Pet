# Virtua_Pet
This is a tiny little project I made to get my own Virtual Pet. I hope you like it as much as I do. 
The code was written in the Arduino IDE and can receive some optimiztion, so feel free to change it as you like. 
All the bitmap sprites are on the bitmap.h file.
I would be glade to see your project too if you choose to make one pet by yourself. 
Thank you! 

![hardware](https://i.imgur.com/JO8UEco.jpeg)

HARDWARE: 
- STM32 F103C8T6 (Blue Pill); (It demands at least 3.5KB RAM and 32KB ROM)
- 1.3" OLED I2C Screen 128x64;
- 4 push buttons (and optional 1n4148 diode to prevent debounce);

WIREING:
I Used the pins PA9, PA10, PA11, PA12 for all the buttons, and I used a 1n4148 diode between the pins and the buttons to prevent debounce.
There is a buzzer at PB8, but I'm not using in this project in particular. 
The OLED display was wired at I2C1 (PB7 and PB8 for SDA and CLK).
Note: Remember to verify the firmware of your display. It can be either SSD1306 or Sh1106. You can change it at the beginning of the code. 

![schematics](https://raw.githubusercontent.com/gusocosta/Virtua_Pet/refs/heads/main/schematics.png) 

by: Gus Costa
(GitHub: gusocosta)
(Reddit: gu-ocosta)
