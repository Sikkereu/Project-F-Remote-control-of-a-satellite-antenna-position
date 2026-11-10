#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

// SERVO_1 -> PA6, TIM16_CH1
// SERVO_2 -> PA7, TIM17_CH1

typedef enum { SERVO_1, SERVO_2 } servo_id_t;

void servo_init(void); // pins + timers, both servos centered
void servo_set_angle(servo_id_t servo, uint8_t deg); // 0..180 deg, clamped
void servo_set_pulse_us(servo_id_t servo, uint16_t us); // raw pulse width, clamped

#endif
