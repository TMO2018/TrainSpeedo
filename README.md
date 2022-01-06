# TrainSpeedo
This project is a "real size" speedometer for miniature trains, cars and anything you wish to measure.
It can calculate the speed of the object between 2 measuring points along the track (or road) by counting milliseconds between passages at the start and end gates. If you enter the distance between those gates (in dm) and the used scale (ex. H0 = 1:87) in to the Arduino, it will calculate the actual "real" speed of the object if it were on a 1:1 scale in the real world. This is a useful tool for measuring and calibrating the speed of your model trains on the layout, in order to be able to set and control their speed in the PC or central station for a realistic run on the tracks.

2 detectors are set up along the track, at a known distance (in dm) from each other. When a train runs by one of the detectors, the timer is started, the switch is auto-disabled (to eliminate double counts or other forms of interference) and only when the object passes by the other detector, will the timer be stopped and will the module calculate the speed based on time , distance and scale factors.

Due to the auto-disable fuction of the detectors, the system can calculate either LTR or RTL (left to right or right to left) moving objects with the same accuracy.
