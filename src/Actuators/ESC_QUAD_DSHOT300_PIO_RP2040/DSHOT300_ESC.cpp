#include "DSHOT300_ESC.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

// PIO state machines
static PIO pio = pio0;
static uint offset;
static int sm[4];

// PIO program for DSHOT300
static const uint16_t dshot_pio_program[] = {
    (uint16_t)pio_encode_set(pio_y, 15),
    (uint16_t)pio_encode_out(pio_x, 1),
    (uint16_t)pio_encode_jmp_not_x(20),
    
    // Bit 0: HIGH 10, LOW 12
    (uint16_t)pio_encode_set(pio_pins, 1),
    (uint16_t)pio_encode_set(pio_x, 9),
    (uint16_t)pio_encode_nop(),
    (uint16_t)pio_encode_jmp_x_dec(5),
    (uint16_t)pio_encode_set(pio_pins, 0),
    (uint16_t)pio_encode_set(pio_x, 11),
    (uint16_t)pio_encode_nop(),
    (uint16_t)pio_encode_jmp_x_dec(9),
    (uint16_t)pio_encode_jmp_y_dec(1),
    (uint16_t)pio_encode_jmp(40),
    
    // Padding
    (uint16_t)pio_encode_nop(),
    (uint16_t)pio_encode_nop(),
    (uint16_t)pio_encode_nop(),
    (uint16_t)pio_encode_nop(),
    (uint16_t)pio_encode_nop(),
    (uint16_t)pio_encode_nop(),
    (uint16_t)pio_encode_nop(),
    
    // Bit 1: HIGH 16, LOW 5+1
    (uint16_t)pio_encode_set(pio_pins, 1),
    (uint16_t)pio_encode_set(pio_x, 15),
    (uint16_t)pio_encode_nop(),
    (uint16_t)pio_encode_jmp_x_dec(22),
    (uint16_t)pio_encode_set(pio_pins, 0),
    (uint16_t)pio_encode_set(pio_x, 4),
    (uint16_t)pio_encode_nop(),
    (uint16_t)pio_encode_jmp_x_dec(26),
    (uint16_t)pio_encode_nop(),
    (uint16_t)pio_encode_jmp_y_dec(1),
    (uint16_t)pio_encode_jmp(40),
    
    (uint16_t)pio_encode_nop()
};

static const pio_program_t dshot_program = {
    .instructions = dshot_pio_program,
    .length = sizeof(dshot_pio_program) / sizeof(dshot_pio_program[0]),
    .origin = -1
};

static uint16_t buildFrame(float thrust) {
    if (thrust < THRUST_MIN) thrust = THRUST_MIN;
    if (thrust > THRUST_MAX) thrust = THRUST_MAX;
    
    uint16_t throttle;
    if (thrust == 0.0f) {
        throttle = 0;
    } else {
        throttle = 48 + (uint16_t)((2047 - 48) * (1.0f - thrust));
        if (throttle > 2047) throttle = 2047;
    }
    
    uint16_t value = (throttle << 1);
    uint8_t crc = (value ^ (value >> 4) ^ (value >> 8)) & 0x0F;
    uint16_t frame = (throttle << 5) | crc;
    
    // Reverse bits
    uint16_t reversed = 0;
    for (int i = 0; i < 16; i++) {
        reversed <<= 1;
        reversed |= (frame & 1);
        frame >>= 1;
    }
    
    return reversed;
}

static void initStateMachine(int sm_idx, uint pin) {
    pio_gpio_init(pio, pin);
    
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_out_shift(&c, true, true, 16);
    sm_config_set_wrap(&c, offset, offset + dshot_program.length - 1);
    sm_config_set_clkdiv(&c, 8.7f);
    sm_config_set_set_pins(&c, pin, 1);
    
    pio_sm_set_consecutive_pindirs(pio, sm_idx, pin, 1, true);
    pio_sm_init(pio, sm_idx, offset, &c);
    pio_sm_set_enabled(pio, sm_idx, true);
}

void escInit() {
    sm[0] = pio_claim_unused_sm(pio, true);
    sm[1] = pio_claim_unused_sm(pio, true);
    sm[2] = pio_claim_unused_sm(pio, true);
    sm[3] = pio_claim_unused_sm(pio, true);
    
    offset = pio_add_program(pio, &dshot_program);
    
    initStateMachine(sm[0], ESC_PIN_1);
    initStateMachine(sm[1], ESC_PIN_2);
    initStateMachine(sm[2], ESC_PIN_3);
    initStateMachine(sm[3], ESC_PIN_4);
}

void escArm() {
    uint16_t arm_frame = 0x0000;
    
    for (int i = 0; i < 4000; i++) {
        for (int j = 0; j < 4; j++) {
            pio_sm_clear_fifos(pio, sm[j]);
            pio_sm_put_blocking(pio, sm[j], arm_frame);
        }
        sleep_us(106);
    }
}

void escSetThrust(float thrust) {
    uint16_t frame = buildFrame(thrust);
    
    for (int i = 0; i < 4; i++) {
        pio_sm_clear_fifos(pio, sm[i]);
        pio_sm_put_blocking(pio, sm[i], frame);
    }
}

void escSetIndividual(float t1, float t2, float t3, float t4) {
    uint16_t frames[4] = {
        buildFrame(t1),
        buildFrame(t2),
        buildFrame(t3),
        buildFrame(t4)
    };
    
    for (int i = 0; i < 4; i++) {
        pio_sm_clear_fifos(pio, sm[i]);
        pio_sm_put_blocking(pio, sm[i], frames[i]);
    }
}
