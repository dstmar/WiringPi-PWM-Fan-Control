# WiringPi-PWM-Fan-Control
Use WiringPi C library to control a PWM fan on Raspberry Pi

Depends on WiringPi - to install on Pi4B, I needed to install v2.52 as v2.50 on the apt repos wasn't compatible :
wget https://project-downloads.drogon.net/wiringpi-latest.deb
sudo dpkg -i wiringpi-latest.deb

If you're going to use the unofficial mirror (https://github.com/WiringPi/WiringPi) on a Pi4B,
some code changes might need to be done to adjust for the new way it handles the 54MHz clock with this commit :
https://github.com/WiringPi/WiringPi/commit/651136a110d1a63320193d4f7d39e9399762847e

I'm running this on a Pi4B with a Delta ASB0305HP-00CP4 PWM fan.

Helpful guide for the hardware side of wiring a PWM 4-pin fan (also check fan datasheet for required values and resistor for tachometer) :
https://blog.driftking.tw/en/2019/11/Using-Raspberry-Pi-to-Control-a-PWM-Fan-and-Monitor-its-Speed/

Configuring :
This is configured to use the WiringPi pin numbering, to see a list of pins, this command will about a little table :
gpio readall

I haven't built in any kind of config file or arguments for this tool, so all the settings must be changed in the source.
It includes both PWM speed control, as well as RPM tachometer settings

const int refresh_time = 5;      /* Seconds to wait between updates */
const int debug = 1;             /* Set to 1 to print debug message or 0 to run silently */

const int pwm_pin = 1;           /* GPIO 1 as per WiringPi, GPIO 18 as per BCM */
const int pi_freq = 54000000;    /* Base frequency of PI - 54 MHz for Pi4B (19.2MHz for older models) */
const int pwm_freq = 25000;      /* Fan PWM Frequency in Hz (see fan datasheet for this value) */
const int speed_start = 35;      /* Fan speed in % to start fan from dead stop (see fan datasheet for this value) */
const int speed_min = 10;        /* Minimum fan speed in % (my fan couldn't reliably run under 10% speed) */
const int speed_max = 80;        /* Maximum fan speed in % (depending on your temp_max, this speed may never get reached) */
const int temp_min = 35;         /* Temperature in °C to stop fan */
const int temp_max = 75;         /* Temperature in °C to set fan to speed_max */

const int tach_pin = 3;          /* GPIO 3 as per WiringPi, GPIO 22 as per BCM */
const int tach_pulse = 2;        /* Number of pulses per fan revolution (see datasheet for this value) */

Compile with :
gcc -o wpi wpi.c -lwiringPi
chmod +x wpi

Run with (must run as root to get access to hardware PWM on Pi) :
sudo ./wpi

You can also create a systemd service to launch this as a service on startup.
