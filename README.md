# CY7C68013A
I have a blue chinese board of this. It must be so called FX2LP board.  
It is an interesting old technology based on ancient 8051 core and powerful USB 2.0 High Speed capability.  

## to build
first install the following packages  
  
sudo apt install sdcc cycfx2prog  
sudo apt install fxload  

then
* make
* sudo make run  
  to download into ram and run
* sudo make program  
  to flash to eeprom  
  
You might have to wrestle with EEPROM A0 jumper on the board.  
In my case, A0 is pulled up to 3V3 via 10K resistor by default, which makes sense since EEPOM is a large one.  
And the jumper actually connects A0 pin to GND.  
But most documents in the internet say the opposite.  
  
And SDCC is not that bad.  
