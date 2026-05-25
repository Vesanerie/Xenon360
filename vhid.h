#ifndef XENON360_VHID_H
#define XENON360_VHID_H

#include <stdint.h>
#include <stdbool.h>

typedef struct vhid vhid_t;

typedef struct __attribute__((packed)) {
    uint16_t buttons;
    int16_t  lx, ly, rx, ry;
    uint8_t  hat;
} vhid_report_t;

#define VHID_BTN_GREEN    (1 << 0)
#define VHID_BTN_RED      (1 << 1)
#define VHID_BTN_YELLOW   (1 << 2)
#define VHID_BTN_BLUE     (1 << 3)
#define VHID_BTN_ORANGE   (1 << 4)
#define VHID_BTN_START    (1 << 5)
#define VHID_BTN_BACK     (1 << 6)
#define VHID_BTN_GUIDE    (1 << 7)

#define VHID_HAT_N        0
#define VHID_HAT_E        2
#define VHID_HAT_S        4
#define VHID_HAT_W        6
#define VHID_HAT_NULL     8

vhid_t *vhid_create(void);
vhid_t *vhid_create_slot(int slot);
void    vhid_destroy(vhid_t *v);
int     vhid_send(vhid_t *v, const vhid_report_t *r);

#endif
