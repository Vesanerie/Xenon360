#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include "vhid.h"

typedef struct {
    uint16_t vid;
    uint16_t pid;
    const char *name;
    bool is_guitar;
} known_device_t;

static const known_device_t known[] = {
    {0x045E, 0x028E, "Microsoft Xbox 360 Controller (wired)",         false},
    {0x045E, 0x028F, "Microsoft Xbox 360 Controller v2 (wired)",      false},

    {0x046D, 0xC21D, "Logitech Gamepad F310",                         false},
    {0x046D, 0xC21E, "Logitech Gamepad F510",                         false},
    {0x046D, 0xC21F, "Logitech Gamepad F710",                         false},
    {0x046D, 0xC242, "Logitech Chillstream Controller",               false},

    {0x056E, 0x2004, "Elecom JC-U3613M",                              false},
    {0x06A3, 0xF51A, "Saitek P3600",                                  false},

    {0x0738, 0x4716, "Mad Catz Wired Xbox 360 Controller",            false},
    {0x0738, 0x4718, "Mad Catz Street Fighter IV FightStick SE",      false},
    {0x0738, 0x4726, "Mad Catz Xbox 360 Controller",                  false},
    {0x0738, 0x4728, "Mad Catz Street Fighter IV FightPad",           false},
    {0x0738, 0x4736, "Mad Catz MicroCon Gamepad",                     false},
    {0x0738, 0x4738, "Mad Catz Wired Controller (SFIV)",              false},
    {0x0738, 0x4740, "Mad Catz Beat Pad",                             false},
    {0x0738, 0x4540, "Mad Catz Guitar Hero (wired)",                  true},
    {0x0738, 0xB726, "Mad Catz Xbox controller MW2",                  false},
    {0x0738, 0xBEEF, "Mad Catz JOYTECH NEO SE Advanced GamePad",      false},
    {0x0738, 0xCB29, "Saitek Aviator Stick AV8R02",                   false},
    {0x0738, 0xF738, "Super SFIV FightStick TE S",                    false},

    {0x07FF, 0xFFFF, "Mad Catz GamePad",                              false},
    {0x0C12, 0x0E30, "Excalibur Storm Pad",                           false},

    {0x0E6F, 0x0105, "HSM3 Xbox 360 dancepad",                        true},
    {0x0E6F, 0x0113, "Afterglow AX.1 Gamepad",                        false},
    {0x0E6F, 0x011F, "Rock Candy Gamepad Wired",                      false},
    {0x0E6F, 0x0131, "PDP EA Sports Controller",                      false},
    {0x0E6F, 0x0133, "Xbox 360 Wired Controller",                     false},
    {0x0E6F, 0x0201, "Pelican PL-3601 Wired Xbox 360 Controller",     false},
    {0x0E6F, 0x0213, "Afterglow Gamepad",                             false},
    {0x0E6F, 0x021F, "Rock Candy Gamepad",                            false},
    {0x0E6F, 0x0301, "Logic3 Controller",                             false},
    {0x0E6F, 0x0401, "Logic3 Controller",                             false},
    {0x0E6F, 0x0413, "Afterglow AX.1 Gamepad",                        false},
    {0x0E6F, 0x0501, "PDP Xbox 360 Controller",                       false},
    {0x0E6F, 0xF900, "PDP Afterglow AX.1",                            false},

    {0x0E8F, 0x0201, "SmartJoy Frag Xpad/PS2 adaptor",                false},
    {0x0E8F, 0x3008, "Generic Xbox control (dealextreme)",            false},

    {0x0F0D, 0x000A, "Hori Co. DOA4 FightStick",                      false},
    {0x0F0D, 0x000C, "Hori PadEX Turbo",                              false},
    {0x0F0D, 0x000D, "Hori Fighting Stick EX2",                       false},
    {0x0F0D, 0x0016, "Hori Real Arcade Pro EX",                       false},
    {0x0F0D, 0x001B, "Hori Real Arcade Pro VX",                       false},

    {0x11C9, 0x55F0, "Nacon GC-100XF",                                false},

    {0x12AB, 0x0004, "Honey Bee Xbox 360 dancepad",                   true},
    {0x12AB, 0x0301, "PDP AFTERGLOW AX.1",                            false},
    {0x12AB, 0x0303, "Mortal Kombat Klassic FightStick",              false},

    {0x12BA, 0x0100, "RedOctane Guitar Hero PS3 (Xbox compat)",       true},

    {0x1430, 0x4734, "RedOctane Guitar Hero (wired)",                 true},
    {0x1430, 0x4748, "RedOctane Guitar Hero X-plorer",                true},
    {0x1430, 0x474B, "RedOctane Guitar Hero (wired)",                 true},
    {0x1430, 0x474C, "RedOctane Guitar Hero (wired)",                 true},
    {0x1430, 0xF801, "RedOctane Controller",                          true},

    {0x146B, 0x0601, "BigBen Interactive XBOX 360 Controller",        false},

    {0x1532, 0x0037, "Razer Sabertooth",                              false},

    {0x15E4, 0x3F00, "Power A Mini Pro Elite",                        false},
    {0x15E4, 0x3F0A, "Xbox Airflo wired controller",                  false},
    {0x15E4, 0x3F10, "Batarang Xbox 360 controller",                  false},

    {0x162E, 0xBEEF, "Joytech Neo-Se Take2",                          false},

    {0x1689, 0xFD00, "Razer Onza Tournament Edition",                 false},
    {0x1689, 0xFD01, "Razer Onza Classic Edition",                    false},
    {0x1689, 0xFE00, "Razer Sabertooth",                              false},

    {0x1949, 0x041A, "Amazon Game Controller",                        false},

    {0x1BAD, 0x0002, "Harmonix Rock Band Guitar",                     true},
    {0x1BAD, 0x0003, "Harmonix Rock Band Drumkit",                    true},
    {0x1BAD, 0x0130, "Ion Drum Rocker",                               true},
    {0x1BAD, 0x074B, "Mad Catz Wireless Guitar",                      true},
    {0x1BAD, 0xF016, "Mad Catz Xbox 360 Controller",                  false},
    {0x1BAD, 0xF018, "Mad Catz Street Fighter IV SE Fighting Stick",  false},
    {0x1BAD, 0xF019, "Mad Catz Brawlstick for Xbox 360",              false},
    {0x1BAD, 0xF021, "Mad Catz Ghost Recon FS GamePad",               false},
    {0x1BAD, 0xF023, "MLG Pro Circuit Controller (Xbox)",             false},
    {0x1BAD, 0xF025, "Mad Catz Call Of Duty",                         false},
    {0x1BAD, 0xF027, "Mad Catz FPS Pro",                              false},
    {0x1BAD, 0xF028, "Street Fighter IV FightPad",                    false},
    {0x1BAD, 0xF02E, "Mad Catz Fightpad",                             false},
    {0x1BAD, 0xF030, "Mad Catz MC2 MicroCON Racing Wheel",            false},
    {0x1BAD, 0xF036, "Mad Catz MicroCon GamePad Pro",                 false},
    {0x1BAD, 0xF038, "Street Fighter IV FightStick TE",               false},
    {0x1BAD, 0xF039, "Mad Catz MvC2 TE",                              false},
    {0x1BAD, 0xF03A, "Mad Catz SFxT Fightstick Pro",                  false},
    {0x1BAD, 0xF03D, "Street Fighter IV Arcade Stick TE Chun Li",     false},
    {0x1BAD, 0xF03E, "Mad Catz MLG FightStick TE",                    false},
    {0x1BAD, 0xF03F, "Mad Catz FightStick SoulCaliber",               false},
    {0x1BAD, 0xF042, "Mad Catz FightStick TES+",                      false},
    {0x1BAD, 0xF080, "Mad Catz FightStick TE2",                       false},
    {0x1BAD, 0xF501, "HoriPad EX2 Turbo",                             false},
    {0x1BAD, 0xF502, "Hori Real Arcade Pro VX SA",                    false},
    {0x1BAD, 0xF506, "Hori Real Arcade Pro EX Premium VLX",           false},
    {0x1BAD, 0xF900, "Harmonix Xbox 360 Controller",                  false},
    {0x1BAD, 0xF901, "Gamestop Xbox 360 Controller",                  false},
    {0x1BAD, 0xF903, "Tron Xbox 360 controller",                      false},
    {0x1BAD, 0xF904, "PDP Versus Fighting Pad",                       false},
    {0x1BAD, 0xF906, "Mortal Kombat FightStick",                      false},
    {0x1BAD, 0xFA01, "MadCatz GamePad",                               false},
    {0x1BAD, 0xFD00, "Razer Onza TE",                                 false},
    {0x1BAD, 0xFD01, "Razer Onza",                                    false},

    {0x24C6, 0x5000, "Razer Atrox Arcade Stick",                      false},
    {0x24C6, 0x5300, "PowerA MINI PROEX Controller",                  false},
    {0x24C6, 0x5303, "Xbox Airflo wired controller",                  false},
    {0x24C6, 0x530A, "Xbox 360 Pro EX Controller",                    false},
    {0x24C6, 0x531A, "PowerA Pro Ex",                                 false},
    {0x24C6, 0x5397, "FUS1ON Tournament Controller",                  false},
    {0x24C6, 0x5500, "Hori XBOX 360 EX 2 with Turbo",                 false},
    {0x24C6, 0x5501, "Hori Real Arcade Pro VX-SA",                    false},
    {0x24C6, 0x5502, "Hori Fighting Stick VX Alt",                    false},
    {0x24C6, 0x5503, "Hori Fighting Edge",                            false},
    {0x24C6, 0x5506, "Hori SOULCALIBUR V Stick",                      false},
    {0x24C6, 0x550D, "Hori GEM Xbox controller",                      false},
    {0x24C6, 0x550E, "Hori Real Arcade Pro V Kai 360",                false},
    {0x24C6, 0x551A, "PowerA FUSION Pro Controller",                  false},
    {0x24C6, 0x561A, "PowerA FUSION Controller",                      false},
    {0x24C6, 0x5B00, "ThrustMaster Ferrari 458 Racing Wheel",         false},
    {0x24C6, 0x5B02, "Thrustmaster GPX Controller",                   false},
    {0x24C6, 0x5B03, "Thrustmaster Ferrari 458 Racing Wheel",         false},
    {0x24C6, 0x5D04, "Razer Sabertooth",                              false},
    {0x24C6, 0xFAFE, "Rock Candy Gamepad",                            false},

    {0x2563, 0x058D, "OneXPlayer Gamepad",                            false},
    {0x260D, 0x0103, "GameSir Wired",                                 false},
    {0x294B, 0x3303, "Snakebyte GAMEPAD BASE X",                      false},
    {0x294B, 0x3404, "Snakebyte GAMEPAD RGB X",                       false},
    {0x2C22, 0x2503, "PXN V900",                                      false},

    {0x044F, 0xB326, "Thrustmaster Gamepad GP XID",                   false},
    {0x0566, 0x1014, "Hori Wireless Switch Pad",                      false},
    {0x0079, 0x18D4, "GPD Win 2 X-Box Controller",                    false},
    {0x03EB, 0xFF01, "Wooting One (Legacy)",                          false},
    {0x03EB, 0xFF02, "Wooting Two (Legacy)",                          false},

    {0,      0,      NULL,                                            false}
};

typedef struct {
    uint16_t vid;
    uint16_t pid;
    const char *name;
} wireless_receiver_t;

static const wireless_receiver_t wireless_receivers[] = {
    {0x045E, 0x0291, "Microsoft Xbox 360 Wireless Receiver"},
    {0x045E, 0x0719, "Microsoft Xbox 360 Wireless Receiver for Windows"},
    {0x045E, 0x02A1, "Microsoft Xbox 360 Wireless Receiver (variant)"},
    {0x1BAD, 0x0719, "Mad Catz Xbox 360 Wireless Receiver clone"},
    {0,      0,      NULL},
};

static const wireless_receiver_t *find_wireless_match(uint16_t vid, uint16_t pid) {
    for (const wireless_receiver_t *w = wireless_receivers; w->vid; w++) {
        if (vid == w->vid && pid == w->pid) return w;
    }
    return NULL;
}

#define KEY_A      0x00
#define KEY_S      0x01
#define KEY_J      0x26
#define KEY_K      0x28
#define KEY_L      0x25
#define KEY_UP     0x7E
#define KEY_DOWN   0x7D
#define KEY_RETURN 0x24
#define KEY_ESCAPE 0x35
#define KEY_SPACE  0x31

typedef struct {
    bool green, red, yellow, blue, orange;
    bool strum_up, strum_down;
    bool start, back;
    bool star_power;
    uint8_t  b4_last;
    uint8_t  b5_last;
    int16_t  lx_last, ly_last, rx_last, ry_last;
} state_t;

static volatile int keep_running = 1;
static void sigint_handler(int sig) { (void)sig; keep_running = 0; }

static void inject_key(CGKeyCode key, bool down) {
    CGEventRef event = CGEventCreateKeyboardEvent(NULL, key, down);
    if (event) {
        CGEventPost(kCGHIDEventTap, event);
        CFRelease(event);
    }
}

static bool check_accessibility_permission(void) {
    // Prompt only when stdout is a tty (user is watching). Under launchd or
    // when bundled inside Xenon360.app via NSTask, the prompt would either be
    // suppressed silently or stack up across restarts; emit a plain warning
    // line instead and let the launching app surface it.
    bool prompt = isatty(STDOUT_FILENO);
    const void *keys[] = { kAXTrustedCheckOptionPrompt };
    const void *vals[] = { prompt ? kCFBooleanTrue : kCFBooleanFalse };
    CFDictionaryRef opts = CFDictionaryCreate(NULL, keys, vals, 1,
                                              &kCFTypeDictionaryKeyCallBacks,
                                              &kCFTypeDictionaryValueCallBacks);
    bool trusted = AXIsProcessTrustedWithOptions(opts);
    CFRelease(opts);
    return trusted;
}

static void update_key(bool *prev, bool current, CGKeyCode key) {
    if (current != *prev) {
        inject_key(key, current);
        *prev = current;
    }
}

static void process_packet(const uint8_t *data, int len, state_t *state, bool verbose, vhid_t *vhid) {
    if (len < 14) return;
    if (data[0] != 0x00) return;

    uint8_t btn2 = data[2];
    uint8_t btn3 = data[3];

    bool dpad_up    = (btn2 & 0x01) != 0;
    bool dpad_down  = (btn2 & 0x02) != 0;
    bool dpad_left  = (btn2 & 0x04) != 0;
    bool dpad_right = (btn2 & 0x08) != 0;
    bool btn_start  = (btn2 & 0x10) != 0;
    bool btn_back   = (btn2 & 0x20) != 0;

    bool btn_lb = (btn3 & 0x01) != 0;
    bool btn_a  = (btn3 & 0x10) != 0;
    bool btn_b  = (btn3 & 0x20) != 0;
    bool btn_x  = (btn3 & 0x40) != 0;
    bool btn_y  = (btn3 & 0x80) != 0;

    int16_t lx = (int16_t)(data[6]  | (data[7]  << 8));
    int16_t ly = (int16_t)(data[8]  | (data[9]  << 8));
    int16_t rx = (int16_t)(data[10] | (data[11] << 8));
    int16_t ry = (int16_t)(data[12] | (data[13] << 8));
    uint8_t b4 = data[4];
    uint8_t b5 = data[5];

    bool tilt = ry > 22000;

    bool digital_change =
        (state->green != btn_a) || (state->red != btn_b) ||
        (state->yellow != btn_y) || (state->blue != btn_x) ||
        (state->orange != btn_lb) ||
        (state->strum_up != dpad_up) || (state->strum_down != dpad_down) ||
        (state->start != btn_start) || (state->back != btn_back) ||
        (state->star_power != tilt);

    int analog_threshold_byte = 20;
    int analog_threshold_i16  = 2000;
    bool analog_change =
        (abs((int)b4 - (int)state->b4_last) > analog_threshold_byte) ||
        (abs((int)b5 - (int)state->b5_last) > analog_threshold_byte) ||
        (abs((int)lx - (int)state->lx_last) > analog_threshold_i16) ||
        (abs((int)ly - (int)state->ly_last) > analog_threshold_i16) ||
        (abs((int)rx - (int)state->rx_last) > analog_threshold_i16) ||
        (abs((int)ry - (int)state->ry_last) > analog_threshold_i16);

    if (digital_change || analog_change) {
        state->b4_last = b4;
        state->b5_last = b5;
        state->lx_last = lx;
        state->ly_last = ly;
        state->rx_last = rx;
        state->ry_last = ry;
    }

    if (vhid) {
        vhid_report_t r = {0};
        if (btn_a)     r.buttons |= VHID_BTN_GREEN;
        if (btn_b)     r.buttons |= VHID_BTN_RED;
        if (btn_y)     r.buttons |= VHID_BTN_YELLOW;
        if (btn_x)     r.buttons |= VHID_BTN_BLUE;
        if (btn_lb)    r.buttons |= VHID_BTN_ORANGE;
        if (btn_start) r.buttons |= VHID_BTN_START;
        if (btn_back)  r.buttons |= VHID_BTN_BACK;
        r.lx = lx;
        r.ly = ly;
        r.rx = rx;
        r.ry = ry;
        if (dpad_up && dpad_left)        r.hat = 7;
        else if (dpad_up && dpad_right)  r.hat = 1;
        else if (dpad_down && dpad_left) r.hat = 5;
        else if (dpad_down && dpad_right)r.hat = 3;
        else if (dpad_up)                r.hat = 0;
        else if (dpad_right)             r.hat = 2;
        else if (dpad_down)              r.hat = 4;
        else if (dpad_left)              r.hat = 6;
        else                             r.hat = 8;
        vhid_send(vhid, &r);
    } else {
        update_key(&state->green,     btn_a,            KEY_A);
        update_key(&state->red,       btn_b,            KEY_S);
        update_key(&state->yellow,    btn_y,            KEY_J);
        update_key(&state->blue,      btn_x,            KEY_K);
        update_key(&state->orange,    btn_lb,           KEY_L);
        update_key(&state->strum_up,  dpad_up,          KEY_UP);
        update_key(&state->strum_down,dpad_down,        KEY_DOWN);
        update_key(&state->start,     btn_start,        KEY_RETURN);
        update_key(&state->back,      btn_back,         KEY_ESCAPE);
        update_key(&state->star_power,tilt,             KEY_SPACE);
    }

    if (verbose && (digital_change || analog_change)) {
        printf("G%d R%d Y%d B%d O%d strum:%c%c start:%d back:%d | b4=%3u b5=%3u | LX=%6d LY=%6d RX=%6d RY=%6d\n",
               btn_a, btn_b, btn_y, btn_x, btn_lb,
               dpad_up ? 'U' : '-', dpad_down ? 'D' : '-',
               btn_start, btn_back,
               b4, b5, lx, ly, rx, ry);
        fflush(stdout);
    }
}

static void release_all_keys(state_t *state) {
    if (state->green)      { inject_key(KEY_A,      false); state->green = false; }
    if (state->red)        { inject_key(KEY_S,      false); state->red = false; }
    if (state->yellow)     { inject_key(KEY_J,      false); state->yellow = false; }
    if (state->blue)       { inject_key(KEY_K,      false); state->blue = false; }
    if (state->orange)     { inject_key(KEY_L,      false); state->orange = false; }
    if (state->strum_up)   { inject_key(KEY_UP,     false); state->strum_up = false; }
    if (state->strum_down) { inject_key(KEY_DOWN,   false); state->strum_down = false; }
    if (state->start)      { inject_key(KEY_RETURN, false); state->start = false; }
    if (state->back)       { inject_key(KEY_ESCAPE, false); state->back = false; }
    if (state->star_power) { inject_key(KEY_SPACE,  false); state->star_power = false; }
}

typedef enum {
    DEV_NONE = 0,
    DEV_WIRED,
    DEV_WIRELESS_RECEIVER,
} dev_type_t;

static dev_type_t find_and_open(libusb_context *ctx,
                                libusb_device_handle **out_handle,
                                const known_device_t **out_wired,
                                const wireless_receiver_t **out_wireless) {
    libusb_device **list;
    ssize_t cnt = libusb_get_device_list(ctx, &list);
    if (cnt < 0) return DEV_NONE;

    for (ssize_t i = 0; i < cnt; i++) {
        struct libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(list[i], &desc) < 0) continue;

        const wireless_receiver_t *w = find_wireless_match(desc.idVendor, desc.idProduct);
        if (w) {
            libusb_device_handle *h = NULL;
            int r = libusb_open(list[i], &h);
            if (r == 0) {
                *out_handle = h;
                *out_wireless = w;
                libusb_free_device_list(list, 1);
                return DEV_WIRELESS_RECEIVER;
            }
            continue;
        }

        for (const known_device_t *k = known; k->vid; k++) {
            if (desc.idVendor == k->vid && desc.idProduct == k->pid) {
                libusb_device_handle *h = NULL;
                int r = libusb_open(list[i], &h);
                if (r == 0) {
                    *out_handle = h;
                    *out_wired = k;
                    libusb_free_device_list(list, 1);
                    return DEV_WIRED;
                }
            }
        }
    }

    fprintf(stderr, "No known device found. Vendor-specific (class 0xFF) USB devices:\n");
    for (ssize_t i = 0; i < cnt; i++) {
        struct libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(list[i], &desc) < 0) continue;
        if (desc.bDeviceClass == 0xFF) {
            fprintf(stderr, "  VID 0x%04X PID 0x%04X\n", desc.idVendor, desc.idProduct);
        } else {
            struct libusb_config_descriptor *cfg;
            if (libusb_get_active_config_descriptor(list[i], &cfg) == 0) {
                for (int j = 0; j < cfg->bNumInterfaces; j++) {
                    const struct libusb_interface_descriptor *intf = &cfg->interface[j].altsetting[0];
                    if (intf->bInterfaceClass == 0xFF) {
                        fprintf(stderr, "  VID 0x%04X PID 0x%04X (vendor-specific interface)\n",
                                desc.idVendor, desc.idProduct);
                        break;
                    }
                }
                libusb_free_config_descriptor(cfg);
            }
        }
    }
    libusb_free_device_list(list, 1);
    return DEV_NONE;
}

typedef struct {
    int slot;
    libusb_device_handle *handle;
    struct libusb_transfer *xfer;
    uint8_t buf[32];
    state_t state;
    bool present;
    vhid_t *vhid;
    bool verbose;
    bool keyboard_mode;
    uint8_t in_ep;
    uint8_t out_ep;
} wireless_slot_t;

static void wireless_send_led(wireless_slot_t *s, uint8_t pattern) {
    if (!s->handle) return;
    uint8_t cmd[12] = {0x00, 0x00, 0x08, (uint8_t)(0x40 | pattern),
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int transferred = 0;
    int r = libusb_interrupt_transfer(s->handle, s->out_ep, cmd, sizeof(cmd),
                                      &transferred, 100);
    if (r < 0 && s->verbose) {
        fprintf(stderr, "Slot %d : LED send failed (%s)\n",
                s->slot + 1, libusb_error_name(r));
    }
}

static void wireless_handle_packet(wireless_slot_t *s, const uint8_t *data, int len) {
    if (len < 2) return;

    if (data[0] & 0x08) {
        bool now_present = (data[1] & 0x80) != 0;
        if (now_present != s->present) {
            s->present = now_present;
            if (now_present) {
                printf("Slot %d : manette connectee\n", s->slot + 1);
                wireless_send_led(s, 0x02 + s->slot);
            } else {
                printf("Slot %d : manette deconnectee\n", s->slot + 1);
                if (s->keyboard_mode) release_all_keys(&s->state);
            }
            fflush(stdout);
        }
        return;
    }

    if (data[1] == 0x01 && len >= 18) {
        if (s->keyboard_mode && s->slot != 0) {
            if (s->verbose) {
                printf("Slot %d input (ignore : clavier sur slot 1 uniquement)\n", s->slot + 1);
                fflush(stdout);
            }
            return;
        }
        process_packet(data + 4, len - 4, &s->state, s->verbose, s->vhid);
    }
}

static void LIBUSB_CALL wireless_cb(struct libusb_transfer *xfer) {
    wireless_slot_t *s = (wireless_slot_t *)xfer->user_data;

    if (xfer->status == LIBUSB_TRANSFER_COMPLETED) {
        wireless_handle_packet(s, xfer->buffer, xfer->actual_length);
    } else if (xfer->status == LIBUSB_TRANSFER_NO_DEVICE) {
        fprintf(stderr, "\nReceiver debranche.\n");
        keep_running = 0;
        return;
    } else if (xfer->status != LIBUSB_TRANSFER_TIMED_OUT &&
               xfer->status != LIBUSB_TRANSFER_CANCELLED) {
        if (s->verbose) {
            fprintf(stderr, "Slot %d transfer status %d\n", s->slot + 1, xfer->status);
        }
    }

    if (xfer->status == LIBUSB_TRANSFER_CANCELLED) return;
    if (!keep_running) return;

    int r = libusb_submit_transfer(xfer);
    if (r < 0 && r != LIBUSB_ERROR_NO_DEVICE) {
        fprintf(stderr, "submit_transfer slot %d echec: %s\n",
                s->slot + 1, libusb_error_name(r));
        keep_running = 0;
    }
}

static int run_wireless(libusb_context *ctx, libusb_device_handle *handle,
                        const wireless_receiver_t *match, bool gamepad_mode, bool verbose) {
    printf("Detecte : %s (VID 0x%04X PID 0x%04X)\n",
           match->name, match->vid, match->pid);

    static const int interfaces[4] = {0, 2, 4, 6};
    static const uint8_t in_eps[4] = {0x81, 0x83, 0x85, 0x87};
    static const uint8_t out_eps[4]= {0x01, 0x03, 0x05, 0x07};

    int claimed[4] = {0};
    for (int i = 0; i < 4; i++) {
        int r = libusb_claim_interface(handle, interfaces[i]);
        if (r < 0) {
            fprintf(stderr, "claim_interface %d echec : %s\n",
                    interfaces[i], libusb_error_name(r));
            fprintf(stderr, "Le receiver est peut-etre deja claim par le driver natif Apple.\n");
            for (int j = 0; j < i; j++) {
                if (claimed[j]) libusb_release_interface(handle, interfaces[j]);
            }
            return 1;
        }
        claimed[i] = 1;
    }

    vhid_t *vhids[4] = {0};
    if (gamepad_mode) {
        for (int i = 0; i < 4; i++) {
            vhids[i] = vhid_create_slot(i);
            if (!vhids[i]) {
                fprintf(stderr, "Slot %d : vhid non cree (SIP/AMFI requis).\n", i + 1);
            }
        }
    } else {
        if (check_accessibility_permission()) {
            printf("OK : permission Accessibilite accordee.\n");
        } else {
            printf("ATTENTION : permission Accessibilite NON accordee.\n");
            printf("  Va dans Reglages > Confidentialite > Accessibilite, active Terminal, relance.\n");
        }
    }

    wireless_slot_t slots[4] = {0};
    struct libusb_transfer *xfers[4] = {0};
    int rc = 0;

    for (int i = 0; i < 4; i++) {
        slots[i].slot = i;
        slots[i].handle = handle;
        slots[i].present = false;
        slots[i].vhid = vhids[i];
        slots[i].verbose = verbose;
        slots[i].keyboard_mode = !gamepad_mode;
        slots[i].in_ep = in_eps[i];
        slots[i].out_ep = out_eps[i];

        xfers[i] = libusb_alloc_transfer(0);
        if (!xfers[i]) {
            fprintf(stderr, "alloc_transfer echec\n");
            rc = 1;
            goto cleanup;
        }
        libusb_fill_interrupt_transfer(xfers[i], handle, in_eps[i],
                                       slots[i].buf, sizeof(slots[i].buf),
                                       wireless_cb, &slots[i], 0);
        slots[i].xfer = xfers[i];

        int r = libusb_submit_transfer(xfers[i]);
        if (r < 0) {
            fprintf(stderr, "submit_transfer slot %d echec : %s\n",
                    i + 1, libusb_error_name(r));
            rc = 1;
            goto cleanup;
        }
    }

    printf("\nMode WIRELESS RECEIVER : 4 slots a l'ecoute.\n");
    printf("Allume tes manettes (bouton Guide) et attribue chaque slot.\n");
    if (gamepad_mode) {
        printf("Gamepad virtuel par slot : 'Xenon360 Virtual Guitar Slot N'\n");
    } else {
        printf("Mode CLAVIER : seul le slot 1 injecte au clavier (sinon collision).\n");
        printf("Pour multi-joueur, utilise --gamepad (necessite SIP/AMFI off).\n");
    }
    printf("Ctrl-C pour quitter.\n\n");

    while (keep_running) {
        struct timeval tv = {0, 100000};
        libusb_handle_events_timeout(ctx, &tv);
    }

cleanup:
    for (int i = 0; i < 4; i++) {
        if (xfers[i]) libusb_cancel_transfer(xfers[i]);
    }
    for (int drain = 0; drain < 10; drain++) {
        struct timeval tv = {0, 50000};
        libusb_handle_events_timeout(ctx, &tv);
    }
    for (int i = 0; i < 4; i++) {
        if (xfers[i]) libusb_free_transfer(xfers[i]);
        if (!gamepad_mode) release_all_keys(&slots[i].state);
        if (vhids[i]) vhid_destroy(vhids[i]);
        if (claimed[i]) libusb_release_interface(handle, interfaces[i]);
    }
    return rc;
}

#ifndef XENON360_NO_MAIN
int main(int argc, char **argv) {
    bool verbose = false;
    bool gamepad_mode = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) verbose = true;
        else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--gamepad") == 0) gamepad_mode = true;
    }

    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    libusb_context *ctx;
    if (libusb_init(&ctx) < 0) {
        fprintf(stderr, "libusb_init failed\n");
        return 1;
    }

    libusb_device_handle *handle = NULL;
    const known_device_t *wired_match = NULL;
    const wireless_receiver_t *wireless_match = NULL;
    dev_type_t devtype = find_and_open(ctx, &handle, &wired_match, &wireless_match);
    if (devtype == DEV_NONE) {
        fprintf(stderr, "Branche ta guitare/receiver et relance.\n");
        libusb_exit(ctx);
        return 1;
    }

    if (devtype == DEV_WIRELESS_RECEIVER) {
        int rc = run_wireless(ctx, handle, wireless_match, gamepad_mode, verbose);
        libusb_close(handle);
        libusb_exit(ctx);
        return rc;
    }

    printf("Detecte : %s (VID 0x%04X PID 0x%04X)\n", wired_match->name, wired_match->vid, wired_match->pid);

    int r = libusb_claim_interface(handle, 0);
    if (r < 0) {
        fprintf(stderr, "claim_interface failed: %s\n", libusb_error_name(r));
        fprintf(stderr, "Sur macOS Tahoe le driver natif Apple peut avoir pris la main.\n");
        fprintf(stderr, "Essaie de debrancher/rebrancher, ou redemarre.\n");
        libusb_close(handle);
        libusb_exit(ctx);
        return 1;
    }

    printf("Connecte. Ctrl-C pour quitter.\n");

    vhid_t *vhid = NULL;
    if (gamepad_mode) {
        printf("Mode GAMEPAD (HID virtuel, whammy/tilt analogiques)\n");
        vhid = vhid_create();
        if (!vhid) {
            fprintf(stderr, "\nImpossible de creer le gamepad virtuel.\n");
            fprintf(stderr, "Sur Apple Silicon il faut desactiver SIP + AMFI :\n");
            fprintf(stderr, "  1. Reboot en Recovery (Eteindre puis tenir bouton power)\n");
            fprintf(stderr, "  2. Utilitaires > Terminal > csrutil disable\n");
            fprintf(stderr, "  3. Reboot normal\n");
            fprintf(stderr, "  4. sudo nvram boot-args=\"amfi_get_out_of_my_way=1\"\n");
            fprintf(stderr, "  5. Reboot\n");
            fprintf(stderr, "Sinon utilise le mode clavier par defaut (sans -g).\n");
            libusb_release_interface(handle, 0);
            libusb_close(handle);
            libusb_exit(ctx);
            return 1;
        }
        printf("Gamepad virtuel cree : \"Xenon360 Virtual Guitar\"\n");
        printf("Mapping HID :\n");
        printf("  Boutons 1-5 = frettes  |  Bouton 6=Start, 7=Back\n");
        printf("  Axe Z = whammy (analogique)  |  Axe Rz = tilt (analogique)\n");
        printf("  Hat switch = strum (haut/bas)\n");
        printf("\nDans Clone Hero : Settings > Controls > nouveau device, remap boutons.\n");
    } else {
        printf("Mode CLAVIER (Clone Hero default keymap):\n");
        printf("  Vert=A  Rouge=S  Jaune=J  Bleu=K  Orange=L\n");
        printf("  Strum Up/Down = fleches  |  Tilt (star power) = Espace\n");
        printf("  Start=Entree  Back=Echap\n");
        printf("  (Whammy analogique non supporte en mode clavier, lance avec -g)\n\n");

        bool trusted = check_accessibility_permission();
        if (trusted) {
            printf("OK : permission Accessibilite accordee, injection clavier active.\n");
        } else {
            printf("ATTENTION : permission Accessibilite NON accordee.\n");
            printf("  Les touches ne seront PAS envoyees aux jeux.\n");
            printf("  macOS a affiche une popup. Va dans :\n");
            printf("  Reglages > Confidentialite et securite > Accessibilite\n");
            printf("  Active Terminal (ou l'app depuis laquelle tu lances ce binaire).\n");
            printf("  Puis relance ce binaire.\n");
        }
    }
    printf("\n");
    if (verbose) printf("Mode verbose actif (affiche chaque changement d'etat).\n\n");

    state_t state = {0};
    uint8_t buf[64];

    while (keep_running) {
        int transferred = 0;
        r = libusb_interrupt_transfer(handle, 0x81, buf, sizeof(buf), &transferred, 100);
        if (r == LIBUSB_ERROR_TIMEOUT) continue;
        if (r == LIBUSB_ERROR_NO_DEVICE) {
            fprintf(stderr, "\nDevice debranche.\n");
            break;
        }
        if (r < 0) {
            fprintf(stderr, "\ntransfer error: %s\n", libusb_error_name(r));
            break;
        }
        process_packet(buf, transferred, &state, verbose, vhid);
    }

    if (!gamepad_mode) release_all_keys(&state);
    printf("\nLiberation...\n");
    if (vhid) vhid_destroy(vhid);
    libusb_release_interface(handle, 0);
    libusb_close(handle);
    libusb_exit(ctx);
    return 0;
}
#endif
