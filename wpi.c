/* Hardware PWM fan control */
/* https://github.com/dstmar/WiringPi-PWM-Fan-Control */

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

const int pwm_pin = 1;           /* GPIO 1 as per WiringPi, GPIO18 as per BCM */
const int pi_freq = 54000000;    /* Base frequency of PI - 54 MHz for Pi4B (19.2MHz for older models) */
const int pwm_freq = 25000;      /* Fan PWM Frequency in Hz */
const int speed_start = 35;      /* Fan speed in % to start fan from dead stop */
const int speed_min = 10;        /* Minimum fan speed in % */
const int speed_max = 80;        /* Maximum fan speed in % */
const int temp_min = 35;         /* Temperature in C to stop fan */
const int temp_max = 75;         /* Temperature in C to set fan to speed_max */
const int temp_start = 42;       /* Don't start fan until this temperature in C is reached */
const int tach_pin = 3;          /* GPIO 3 as per WiringPi, GPIO22 as per BCM */
const int tach_pulse = 2;        /* Number of pulses per fan revolution */
const int refresh_time = 5;      /* Seconds to wait between updates */
const int debug = 1;             /* Set to 1 to print debug messages or 0 to run silently */

int range = 0;
int current_speed = 0;
int rpm = 0;
struct timeval tach_time;

/* Sets fan % speed */
void set_speed(int speed)
{
  if (current_speed != speed)
  {
    speed = (speed > 100) ? 100 : (speed < 0) ? 0 : speed; // make sure speed is in range 0-100
    if (current_speed == 0 && speed > 0 && speed < speed_start)
    {
      int duty_start = range * speed_start / 100;
      pwmWrite(pwm_pin, duty_start);
      if (debug) printf("speed : %d%%\n",speed_start);
      sleep(2);
    }
    int duty = range * speed / 100;
    pwmWrite(pwm_pin, duty);
    current_speed = speed;
    if (debug) printf("speed : %d%%\n",speed);
  }
}

/* Get Pi temperature in C */
int get_temp(void)
{
  int fd, temp;
  char buf[10];

  fd = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
  read(fd, buf, sizeof(buf));
  close(fd);
  sscanf(buf, "%d", &temp);
  temp = temp/1000; 
  if (debug) printf("temp : %d C\n",temp);

  return temp;
}

/* Get fan RPM */
void get_rpm(void)
{
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  int dt = 1000000 * (current_time.tv_sec - tach_time.tv_sec) + current_time.tv_usec - tach_time.tv_usec;
  tach_time = current_time;
  if (dt > 0) rpm =  1000000 / dt / tach_pulse * 60;
}

/* Print error message and exit */
void error(char *message)
{
  printf("Error : %s\n", message);
  exit(1);
}

/* Get clock and range for PWM frequency */
int get_clock(void)
{
  int clock;
  int ratio = pi_freq / pwm_freq;
  int max_range = pi_freq / 2;
  if (pi_freq % pwm_freq == 0) // try to find an exact match for pwm_freq
  {
    for (int r = (ratio < max_range) ? ratio - 1 : max_range; r > 0; r--)
    {
      if (ratio % r == 0 && ratio / r >= 2)
      {
        if (ratio / r <= 4095)
        {
          range = r;
          clock = ratio / range;
        }
        break;
      }
    }
  }
  if (range == 0) // no exact match, get similar frequency
  {
    clock = ratio / max_range; // estimate clock for maximum range
    clock = (clock < 2) ? 2 : (clock > 4095) ? 4095 : clock; // make sure clock is in range 2-4095
    range = ratio / clock;
    if (range > max_range || range < 1) error("can't achieve this PWM frequency");
    if (debug) printf("No exact clock and range found - using %d Hz\n", pi_freq / clock / range);
  }
  if (debug) printf("clock div : %d, range : %d\n", clock, range);

  return clock;
}

/* Graceful shutdown */
void end(int signum)
{
  printf("Shutting down...\n");
  pwmWrite(pwm_pin, 0);
  sleep(1);
  pinMode(pwm_pin, INPUT);
  exit(0);
}

/* Setup GPIO for tachometer and PWM */
void setup(void)
{
  if (wiringPiSetup() == -1) error("wiringPiSetup failed");
  if (refresh_time <= 0) error ("refresh_time must be at least 1");

  // setup rpm tachometer
  if (tach_pulse <= 0) error ("tach_pulse must be at least 1");

  pinMode(tach_pin, INPUT);
  pullUpDnControl(tach_pin, PUD_UP);
  gettimeofday(&tach_time, NULL);
  wiringPiISR(tach_pin, INT_EDGE_FALLING, &get_rpm);

  // setup pwm fan speed control
  if (speed_max > 100 || speed_max < speed_min) error("speed_max must be between speed_min and 100");
  if (speed_min < 0) error ("speed_min must be at least 0");
  if (speed_start < 0 || speed_start > 100) error("speed_start must between 0 and 100");
  if (temp_min >= temp_max) error("temp_min must be less than temp_max");
  if (temp_min > temp_start) error("temp_start can't be less than temp_min");
  if (2 * pwm_freq > pi_freq) error("pwm_freq can't be over pi_freq / 2");

  int clock = get_clock();
  pinMode(pwm_pin, PWM_OUTPUT);
  pwmSetMode(PWM_MODE_MS);
  pwmSetRange(range);
  pwmSetClock(clock);
  pwmWrite(pwm_pin, 0);

  // shutdown interrupts
  signal(SIGINT, end);
  signal(SIGTERM, end);
}

int main (void)
{
  setup();

  int temp;
  int prev_rpm = 0;
  struct timeval prev_tach = tach_time;
  double ratio = (speed_max - speed_min) / (temp_max - temp_min);

  while (1)
  {
    temp = get_temp();

    if (temp < temp_min)
      set_speed(0);
    else if (current_speed > 0 || temp >= temp_start)
      set_speed((temp - temp_min) * ratio + speed_min);

    if (!timercmp(&prev_tach, &tach_time, !=))
      rpm = 0;

    if (debug && prev_rpm != rpm)
    {
      prev_rpm = rpm;
      prev_tach = tach_time;
      printf("rpm : %d\n", rpm);
    }

    sleep(refresh_time);
  }
}
