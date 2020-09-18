# WiringPi-PWM-Fan-Control
Use WiringPi C library to control a 4-pin PWM fan on Raspberry Pi

After attempting software PWM using python, I quickly realized that it runs the CPU at around 20%.  Using C, and the hardware PWM provided on the Pi drops that to virtually 0% CPU usage.

Depends on WiringPi - to install on Pi4B, I needed to install v2.52 as v2.50 on the apt repos wasn't compatible :<br/>
wget https://project-downloads.drogon.net/wiringpi-latest.deb<br/>
sudo dpkg -i wiringpi-latest.deb

If you're going to use the unofficial mirror (https://github.com/WiringPi/WiringPi) on a Pi4B, some code changes might need to be done to adjust for the new way it handles the 54MHz clock with this commit :<br/>
https://github.com/WiringPi/WiringPi/commit/651136a110d1a63320193d4f7d39e9399762847e

I'm running this on a Pi4B with a Delta ASB0305HP-00CP4 PWM fan.

Helpful guide for the hardware side of wiring a PWM 4-pin fan (also check fan datasheet for required values and resistor for tachometer) :<br/>
https://blog.driftking.tw/en/2019/11/Using-Raspberry-Pi-to-Control-a-PWM-Fan-and-Monitor-its-Speed/

I don't actually do anything with the RPM tachometer readings, other than display them in debug mode, but feel free to add functionality, or remove the feature entirely if you don't need it.

This is my first attempt at coding using C, so I make no guarantees with this code, simply wanted to share for anyone that wants to build upon this themselves.

<h2>Configuring :</h2>
This is configured to use the WiringPi pin numbering, to see a list of pins, this command will about a little table :<br/>
gpio readall

I haven't built in any kind of config file or arguments for this, so all the settings must be changed in the source.

<b>pwm_pin :</b> GPIO pin (WiringPi numbering) which connects to fan's PWM wire<br/>
<b>pi_freq :</b> Base frequency in Hz of Pi - 54 MHz for Pi4B (19.2MHz for older models)<br/>
<b>pwm_freq :</b> Fan PWM frequency in Hz (see fan datasheet for this value)<br/>
<b>speed_start :</b> Fan speed in % to start fan from dead stop (see fan datasheet for this value)<br/>
<b>speed_min :</b> Minimum fan speed in % (my fan couldn't reliably run under 10% speed)<br/>
<b>speed_max :</b> Maximum fan speed in % (depending on your temp_max, this speed may never get reached)<br/>
<b>temp_min :</b> Temperature in °C to stop fan<br/>
<b>temp_max :</b> Temperature in °C to set fan to speed_max<br/>
<b>temp_start :</b> Don't start fan until this temperature in °C is reached<br/>
<b>tach_pin :</b> GPIO pin (WiringPi numbering) which connects to the tach wire on fan, along with 3.3V with resistor<br/>
<b>tach_pulse :</b> Number of tachometer pulses per fan revolution (see datasheet for this value)<br/>
<b>refresh_time :</b> Seconds to wait between updates<br/>
<b>debug :</b> Set to 1 to print debug message or 0 to run silently

<h2>Compile with :</h2>
gcc -o wpi wpi.c -lwiringPi<br/>
chmod +x wpi

<h2>Run with :</h2>
sudo ./wpi

You can also create a systemd service to launch this as a service on startup.
