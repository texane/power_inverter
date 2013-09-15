/* generate pwm duty values so that the average output
   voltage is a sine wave of frequency CONFIG_FSINE.
   the duty is changed at a frequency of CONFIG_FSAMPL.
   the maximum output voltage is CONFIG_VMAX.
   the pwm is expected to drive a hbridge. thus, half
   the time is spent driving one half according to the
   sine wave values while the other hbridge half is
   disabled. Then, the other hbridge half is driven
   with the cosine for half the time. Thus, only one
   pwm duty table needs to be generated.
 */

#include <stdint.h>
#include "sin_fix_u16.h"

/* the carrier sampling frequency, in hertz */
#define CONFIG_FSAMPL ((uint16_t)5000)

/* the sine frequency, in hertz */
#define CONFIG_FSINE ((uint16_t)50)

/* global pwm duty table */
static uint16_t pwm_duties[4096];

/* pwm duty table size */
static uint16_t nduties = 0;


#if 0

static volatile uint8_t br_side;

static void on_isr(void)
{
  pwm_set_duty(pwm_duty[i]);

  if (++i == nsampl)
  {
    /* toggle ctl bit */
    br_set_side(br_side);
    br_side ^= 1;
    i = 0;
  }
}
#endif

#if 0
static void on_button(void)
{
  read_adc();
  pwm_disable();
  nsampl = generate_duties(fsampl, fsine, duties);
}
#endif


static uint16_t gen_half_sine(uint16_t fsampl, uint16_t fsine, uint16_t* p)
{
  /* fsampl, the sampling frequency */
  /* fsine, the sine frequency */
  /* p the resulting table */

  uint16_t n = fsampl / fsine;
  const uint16_t d = 1 + (1UL << 16) / n;
  uint16_t i;
  uint16_t x;

  /* only interested in half wave */
  n = 1 + n / 2;

  for (i = 0, x = 0; i < n; ++i, x += d) p[i] = sin_fix_u16(x);

  return n;
}


#if (CONFIG_UNIT == 1) /* unit testing */

#include <stdio.h>
#include <math.h>

static void print_table(const uint16_t* p, uint16_t n)
{
  /* plot '/tmp/o' using 2 with lines lw 3, '' using 3 with lines */

  uint16_t i;

  for (i = 0; i < n; ++i)
  {
    const double y =
      65536.0 * sin(((double)i * 2.0 * M_PI * CONFIG_FSINE) / CONFIG_FSAMPL);
    printf("%d %u %lf\n", i, p[i], y);
  }
}

#endif /* unit testing */

int main(int ac, char** av)
{
  nduties = gen_half_sine(CONFIG_FSAMPL, CONFIG_FSINE, pwm_duties);
#if (CONFIG_UNIT == 1)
  print_table(pwm_duties, nduties);
#endif
  return 0;
}
