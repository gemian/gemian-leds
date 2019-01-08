# Gemian LEDs

Daemon to control the 20 LED's available on the Gemini PDA.

Support should include:

* CapsLock
* Charging status
* WiFi/BlueTooth/Cellular status
* Low battery indicator - using sneaky extra LED next to power LED.
* A range of animations for the array of 5 for use by notifications.

The general idea is that we monitor dbus for any related messages to changes in state that we wish to display, then update all the LEDs at once. This allows us to use PWM control that is simplest if we program all the LEDs at once.

We also need to add a settings UI so that users can choose not to display some things or to disable when device is open or to set as extra dim/off at night time etc.

To enable tests to run you will probably have to install the dbus config file:
```
sudo cp src/org.thinkglobally.GemianLEDs.conf /etc/dbus-1/system.d/
```

## Block Animations

The block of 5 indicator LEDs can be configured with animations, see example python scripts in scripts folder. The general idea is to clear the existing animation steps then add a new set of steps and then push it to the controller.

The step addition parameters are:
1. LED number 1-5
2. Colour component to effect: 0 = Red, 1 = Green, 2 = Blue
3. Type of step: 0 = Specifig PWM value, 1 = Fade in with delay, 2 = Fade out  with delay, 3 = Next animation frame with a delay
4. Value: 0-255 for PWM or fade delays, 0-1023 for next frame delay

The chip only supports either a specific pwm value or a fade to/from fully on for each colour component. A second setting of the same LED colour component in the same animation frame will overwrite the first before it has been noticed.

There are only a total of 255 steps that can be added, about 8 are already used leaving about 247 for custom animations.
