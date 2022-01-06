# TrainSpeedo
This project is a "real size" speedometer for miniature trains, cars and anything you wish to measure.
It can calculate the speed of the object between 2 measuring points along the track (or road) by counting milliseconds between passages at the start and end gates. If you enter the distance between those gates (in dm) and the used scale (ex. H0 = 1:87) in to the Arduino, it will calculate the actual "real" speed of the object if it were on a 1:1 scale in the real world. This is a useful tool for measuring and calibrating the speed of your model trains on the layout, in order to be able to set and control their speed in the PC or central station for a realistic run on the tracks.
TrainSpeedo_rev1_0.odt 06.01.2022 Page 1 of 7 TMO Version 1.0
  
2 detectors are set up along the track, at a known distance (in dm) from each other. When a train runs by one of the detectors, the timer is started, the switch is auto-disabled (to eliminate double counts or other forms of interference) and only when the object passes by the other detector, will the timer be stopped and will the module calculate the speed based on time , distance and scale factors.
Due to the auto-disable fuction of the detectors, the system can calculate either LTR or RTL (left to right or right to left) moving objects with the same accuracy.
The commands :
-- START button : use this one to trigger the circuit and put it in stand-by so it will start measuring when an boject enters the test track section.
-- ZERO button : to clear the display or to interrupt an ongoing measurement (bypass) -- MENU button : enters the 3 menu levels
* first push : enter the menu and go to the input screen for track length
* 2nd push : record the current setting and go to screen for scale input
* 3rd push : record the scale factor and go to the confirm / exit screen
* 4th push : confirm and store the new values or return to old ones and then go to main mode. (exit menu)
-- UP / DOWN buttons : used to increment / decrement or change the values in the menu screen. -- RESET button (optional) : for a full reset of the device (however, scale and length are stored in
EEPROM and will be remembered)
