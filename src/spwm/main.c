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
#include <stdio.h>
#include "sin_fix_u16.h"

/* the carrier sampling frequency, in hertz */
#define CONFIG_FSAMPL ((uint16_t)1000)

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
  const uint16_t d = (1UL << 16) / n;
  uint16_t i;
  uint16_t x;

  /* only interested in half wave */
  n = 1 + n / 2;

  for (i = 0, x = 0; i < n; ++i, x += d) p[i] = sin_fix_u16(x);

  return n;
}

static void print_table(const uint16_t* p, uint16_t n)
{
  uint16_t i;
  for (i = 0; i < n; ++i) printf("%d %u\n", i, p[i]);
}

int main(int ac, char** av)
{
  nduties = gen_half_sine(CONFIG_FSAMPL, CONFIG_FSINE, pwm_duties);
  print_table(pwm_duties, nduties);
  return 0;
}
