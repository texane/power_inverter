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
#include <avr/io.h>
#include <avr/interrupt.h>
#include "sin_fix_u16.h"

/* the carrier sampling frequency, in hertz */
#define CONFIG_FSAMPL ((uint16_t)50000)

/* the sine frequency, in hertz */
#define CONFIG_FSINE ((uint16_t)50)

/* global pwm duty table */
static volatile uint16_t pwm_duties[512];

/* pwm duty table size */
static volatile uint16_t nduties = 0;


#if 0
static void on_button(void)
{
  read_adc();
  pwm_disable();
  nsampl = generate_duties(fsampl, fsine, duties);
}
#endif


static uint16_t gen_half_sine(void)
{
  /* p the resulting table */

  uint16_t n = CONFIG_FSAMPL / CONFIG_FSINE;
  const uint16_t d = 1 + (1UL << 16) / n;
  uint16_t i;
  uint16_t x;

  /* only interested in half wave */
  n = 1 + n / 2;

  for (i = 0, x = 0; i < n; ++i, x += d)
  {
    /* compare and match counter resolution is 10 bits */
    pwm_duties[i] = sin_fix_u16(x) >> 6;
  }

  return n;
}


/* 16 bits pwm, counter 1 */

static inline void pwm_set_duty(uint16_t x)
{
  OCR1A = x;
}

static void pwm_disable(void)
{
  TCCR1B = 0;
}

static void pwm_enable(void)
{
  /* disable pwm */
  pwm_disable();

  /* io port */
  DDRB |= 1 << 1;

  /* non inverting mode: set at bottom, clear on compare and match */
  /* use ICR1 as top register */
  TCCR1A = (0x2 << 6) | (0x2 << 0);

  /* counter */
  TCNT1 = 0;

  /* count from 0 to top */
  ICR1 = 0x3ff;

  /* output compare, ie. duty */
  OCR1A = 0x0;

  /* unused */
  TCCR1C = 0;

  /* interrupt disabled */
  TIMSK1 = 0;

  /* no prescaler, 16MHz */
  TCCR1B = (0x3 << 3) | (0x1 << 0);

  /* pwm started from here */
}


/* 8 bits timer, counter 0 */

static volatile uint16_t timer_pos = 0;
static volatile uint8_t timer_side = 0;

ISR(TIMER0_COMPA_vect)
{
#if 0 /* toremove */
  ++timer_pos;
  PORTB = (timer_pos & 1) << 1;
#endif /* toremove */

  pwm_set_duty(pwm_duties[timer_pos]);

  if ((++timer_pos) == nduties)
  {
    timer_pos = 0;
    timer_side ^= 1;
    /* todo: set_side(br_side); */
  }
}

static void timer_disable(void)
{
  TCCR0B = 0;
}

static void timer_enable(void)
{
  /* fcpu / (fsampl * prescal) = counter */
  /* fcpu = 16000000 */
  /* fsampl = 50000 */
  /* prescal = 8 */
  /* counter = 40 */

  /* disable timer */
  timer_disable();

  /* ctc mode, clear on OCRA0 */
  TCCR0A = 0x2 << 0;

  /* counter */
  TCNT0 = 0;

  /* timer top value */
  OCR0A = 40;

  /* interrupt on compare match */
  TIMSK0 = 0x1 << 1;

  /* prescaler = 8 */
  TCCR0B = 0x2 << 0;

  /* timer started from here */
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

int main(void)
{
  /* generate the half sine table */
  nduties = gen_half_sine();
#if (CONFIG_UNIT == 1)
  print_table(pwm_duties, nduties);
#endif

  /* enable timer */
  timer_enable();

  /* enable pwm */
  pwm_enable();

  /* enable interrupts */
  sei();

  while (1) ;

  return 0;
}
