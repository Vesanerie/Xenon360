#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include <ApplicationServices/ApplicationServices.h>

typedef struct {
    uint16_t vid;
    uint16_t pid;
    const char *name;
    bool is_guitar;
} known_device_t;

static const known_device_t known[] = {
    {0x045E, 0x028E, "Microsoft Xbox 360 Controller (wired)",    false},
    {0x045E, 0x028F, "Microsoft Xbox 360 Wireless Controller",   false},
    {0x1430, 0x4748, "RedOctane Guitar Hero 6 / Warriors",       true},
    {0x1430, 0x474B, "RedOctane Guitar Hero (wired)",            true},
    {0x1430, 0x474C, "RedOctane Guitar Hero (wired)",            true},
    {0x1430, 0x4734, "RedOctane Guitar Hero (wired)",            true},
    {0x12BA, 0x0100, "RedOctane Guitar Hero PS3 (Xbox compat)",  true},
    {0x1BAD, 0x0002, "Harmonix Rock Band Guitar",                true},
    {0x1BAD, 0x0003, "Harmonix Rock Band Drumkit",               true},
    {0x1BAD, 0x0130, "Ion Drum Rocker",                          true},
    {0x1BAD, 0x074B, "Mad Catz Wireless Guitar",                 true},
    {0x0738, 0x4540, "Mad Catz Guitar Hero (wired)",             true},
    {0,      0,      NULL,                                       false}
};

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

static void update_key(bool *prev, bool current, CGKeyCode key) {
    if (current != *prev) {
        inject_key(key, current);
        *prev = current;
    }
}

static void process_packet(const uint8_t *data, int len, state_t *state, bool verbose) {
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

    bool tilt = ly > 16000;

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

    (void)dpad_left; (void)dpad_right; (void)lx; (void)rx;

    if (verbose) {
        printf("\rG%d R%d Y%d B%d O%d | strum:%c%c | start:%d back:%d | LY:%6d (tilt:%d) | RX(whammy):%6d   ",
               btn_a, btn_b, btn_y, btn_x, btn_lb,
               dpad_up ? 'U' : '-', dpad_down ? 'D' : '-',
               btn_start, btn_back, ly, tilt, rx);
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

static int find_and_open(libusb_context *ctx, libusb_device_handle **out_handle,
                         const known_device_t **out_match) {
    libusb_device **list;
    ssize_t cnt = libusb_get_device_list(ctx, &list);
    if (cnt < 0) return -1;

    for (ssize_t i = 0; i < cnt; i++) {
        struct libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(list[i], &desc) < 0) continue;
        for (const known_device_t *k = known; k->vid; k++) {
            if (desc.idVendor == k->vid && desc.idProduct == k->pid) {
                libusb_device_handle *h = NULL;
                int r = libusb_open(list[i], &h);
                if (r == 0) {
                    *out_handle = h;
                    *out_match = k;
                    libusb_free_device_list(list, 1);
                    return 0;
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
    return -1;
}

int main(int argc, char **argv) {
    bool verbose = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) verbose = true;
    }

    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);

    libusb_context *ctx;
    if (libusb_init(&ctx) < 0) {
        fprintf(stderr, "libusb_init failed\n");
        return 1;
    }

    libusb_device_handle *handle = NULL;
    const known_device_t *match = NULL;
    if (find_and_open(ctx, &handle, &match) < 0) {
        fprintf(stderr, "Branche ta guitare et relance.\n");
        libusb_exit(ctx);
        return 1;
    }

    printf("Detecte : %s (VID 0x%04X PID 0x%04X)\n", match->name, match->vid, match->pid);

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
    printf("Mapping Clone Hero (clavier):\n");
    printf("  Vert=A  Rouge=S  Jaune=J  Bleu=K  Orange=L\n");
    printf("  Strum Up/Down = fleches  |  Tilt (star power) = Espace\n");
    printf("  Start=Entree  Back=Echap\n\n");

    if (verbose) printf("Mode verbose actif.\n\n");

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
        process_packet(buf, transferred, &state, verbose);
    }

    release_all_keys(&state);
    printf("\nLiberation...\n");
    libusb_release_interface(handle, 0);
    libusb_close(handle);
    libusb_exit(ctx);
    return 0;
}
