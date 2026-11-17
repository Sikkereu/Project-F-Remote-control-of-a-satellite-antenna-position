#include <math.h>
#include <stdio.h>
#include "30010_io.h"
#include "delay.h"
#include "LSM9DS1.h"
#include "servo.h"

// Provisional calibration from the unloaded servo test.
#define ARM_PULSE_0_US 600u
#define ARM_PULSE_45_US 1000u
#define ARM_PULSE_90_US 1500u
#define ARM_NEUTRAL_DEG 90u
#define SAMPLE_MS 25u
#define NEUTRAL_SAMPLES 32u
// Reverse this sign if the arm should move the other way.
#define JOYSTICK_SIGN 1.0f

static uint16_t arm_pulse_us(unsigned angle_deg)
{
    if (angle_deg <= 45u)
    {
        unsigned span = ARM_PULSE_45_US - ARM_PULSE_0_US;
        return ARM_PULSE_0_US + (angle_deg * span + 22u) / 45u;
    }

    unsigned span = ARM_PULSE_90_US - ARM_PULSE_45_US;
    return ARM_PULSE_45_US + ((angle_deg - 45u) * span + 22u) / 45u;
}

int main(void)
{
    uart_init(9600);
    delay_init();
    init_SPI3_9DOF();
    delay_ms(50);

    uint8_t id = SPI3_read_byte(READ | 0x0F, GyroAccTemp_SEL);
    if (id != WHO_AM_I_AG_VALUE)
    {
        printf("LSM9DS1 sensor-ID 0x%02X, forventet 0x%02X.\n",
               (unsigned)id, (unsigned)WHO_AM_I_AG_VALUE);
        for (;;) { }
    }

    // Accelerometer only, +/-2 g at 50 Hz. BDU keeps each sample's bytes together.
    SPI3_write_reg(CTRL_REG1_G, 0x00, GyroAccTemp_SEL);
    SPI3_write_reg(CTRL_REG8, 0x44, GyroAccTemp_SEL);
    SPI3_write_reg(CTRL_REG6_XL, 0x40, GyroAccTemp_SEL);
    printf("Hold det hjemmelavede joystick stille i neutral.\n");
    delay_ms(1000);

    int32_t neutral_x = 0;
    int32_t neutral_z = 0;
    for (unsigned i = 0; i < NEUTRAL_SAMPLES; ++i)
    {
        int16_t ax, ay, az;
        readAccData(&ax, &ay, &az);
        neutral_x += ax;
        neutral_z += az;
        delay_ms(SAMPLE_MS);
    }

    float ref_x = (float)neutral_x / NEUTRAL_SAMPLES;
    float ref_z = (float)neutral_z / NEUTRAL_SAMPLES;
    if (ref_x * ref_x + ref_z * ref_z < 8000.0f * 8000.0f)
    {
        printf("Neutral maaling ugyldig. Hold joysticket lodret og tryk Reset.\n");
        for (;;) { }
    }

    servo_init();
    uint16_t pulse_us = arm_pulse_us(ARM_NEUTRAL_DEG);
    servo_set_pulse_us(SERVO_1, pulse_us);  // PA6 / D12
    float filtered_target = (float)ARM_NEUTRAL_DEG;
    uint32_t last_report = millis();

    for (;;)
    {
        int16_t ax, ay, az;
        readAccData(&ax, &ay, &az);

        // Signed tilt from neutral in the X/Z plane (rotation about sensor Y).
        float cross = ref_x * az - ref_z * ax;
        float dot = ref_x * ax + ref_z * az;
        float tilt = JOYSTICK_SIGN * atan2f(cross, dot) * 57.2957795f;
        if (fabsf(tilt) < 2.0f)
        {
            tilt = 0.0f;
        }

        float target = fminf(90.0f, fmaxf(0.0f, (float)ARM_NEUTRAL_DEG - tilt));
        filtered_target += 0.25f * (target - filtered_target);
        unsigned target_deg = (unsigned)(filtered_target + 0.5f);
        uint16_t next_pulse = arm_pulse_us(target_deg);
        if (next_pulse != pulse_us)
        {
            pulse_us = next_pulse;
            servo_set_pulse_us(SERVO_1, pulse_us);
        }

        uint32_t now = millis();
        if (now - last_report >= 200u)
        {
            printf("Joystickvip %+d grader | Armmaal %u grader | PWM %u us\n",
                   (int)lroundf(tilt), target_deg, (unsigned)pulse_us);
            last_report = now;
        }
        delay_ms(SAMPLE_MS);
    }
}
