# Gemian LEDs

Daemon to control the 20 LED's available on the Gemini PDA.

Support should include:

* CapsLock
* Charging status
* WiFi/BlueTooth/Cellular status
* Low battery indicator - using sneaky extra LED next to power LED.
* A range of animations for the array of 5 for use by notifications.

The general idea is that we monitor dbus for any related messages to changes in state that we wish to display, then update all the LEDs at once. This is not strictly necessary with the current kernel interface but if we want to use PWM control in the future it looks like you have to program all the LEDs at once.

We also need to add a settings UI so that users can choose not to display some things or to disable when device is open or to set as extra dim/off at night time etc.

To enable tests to run you will probably have to install the dbus config file:
```
sudo cp src/org.thinkglobally.GemianLEDs.conf /etc/dbus-1/system.d/
```