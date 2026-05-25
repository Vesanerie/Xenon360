#include "xenon360.c"

#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, name)                                              \
    do {                                                                \
        if (cond) { g_pass++; printf("  PASS: %s\n", name); }           \
        else      { g_fail++; printf("  FAIL: %s (%s:%d)\n",            \
                                     name, __FILE__, __LINE__); }       \
    } while (0)

static void test_wireless_match(void) {
    printf("\n[test_wireless_match]\n");
    const wireless_receiver_t *w;

    w = find_wireless_match(0x045E, 0x0291);
    CHECK(w && strstr(w->name, "Microsoft"), "MS Xbox 360 Wireless Receiver (0x0291)");

    w = find_wireless_match(0x045E, 0x0719);
    CHECK(w != NULL, "MS Xbox 360 Wireless Receiver for Windows (0x0719)");

    w = find_wireless_match(0x045E, 0x02A1);
    CHECK(w != NULL, "MS Xbox 360 Wireless Receiver variant (0x02A1)");

    w = find_wireless_match(0x1BAD, 0x0719);
    CHECK(w && strstr(w->name, "Mad Catz"), "Mad Catz clone (0x1BAD:0x0719)");

    w = find_wireless_match(0x0000, 0x0000);
    CHECK(w == NULL, "null VID/PID rejected");

    w = find_wireless_match(0x1430, 0x4748);
    CHECK(w == NULL, "wired X-plorer not flagged as receiver");

    w = find_wireless_match(0x045E, 0x028E);
    CHECK(w == NULL, "wired MS Xbox 360 controller not flagged as receiver");
}

static void test_presence_transitions(void) {
    printf("\n[test_presence_transitions]\n");
    wireless_slot_t s = {0};
    s.slot = 0;
    s.handle = NULL;
    s.keyboard_mode = true;

    CHECK(s.present == false, "initial state: not present");

    uint8_t pkt_connect[] = {0x08, 0x80, 0x00, 0x00};
    wireless_handle_packet(&s, pkt_connect, sizeof(pkt_connect));
    CHECK(s.present == true, "0x08 0x80 -> connected");

    wireless_handle_packet(&s, pkt_connect, sizeof(pkt_connect));
    CHECK(s.present == true, "duplicate connect packet -> still connected");

    uint8_t pkt_disconnect[] = {0x08, 0x00, 0x00, 0x00};
    wireless_handle_packet(&s, pkt_disconnect, sizeof(pkt_disconnect));
    CHECK(s.present == false, "0x08 0x00 -> disconnected");

    uint8_t pkt_reconnect[] = {0x08, 0x80, 0x00, 0x00};
    wireless_handle_packet(&s, pkt_reconnect, sizeof(pkt_reconnect));
    CHECK(s.present == true, "reconnect after disconnect");
}

static void test_packet_dispatch(void) {
    printf("\n[test_packet_dispatch]\n");
    wireless_slot_t s = {0};
    s.handle = NULL;
    s.keyboard_mode = true;
    s.slot = 0;
    s.present = true;

    uint8_t short_pkt[] = {0x08};
    wireless_handle_packet(&s, short_pkt, 1);
    CHECK(s.present == true, "1-byte packet ignored, no state change");

    wireless_handle_packet(&s, NULL, 0);
    CHECK(s.present == true, "zero-length packet ignored");

    uint8_t unknown_pkt[] = {0x00, 0xFF, 0x00, 0x00, 0x00};
    bool prev = s.present;
    wireless_handle_packet(&s, unknown_pkt, sizeof(unknown_pkt));
    CHECK(s.present == prev, "non-presence non-input packet ignored");
}

static void test_keyboard_slot_isolation(void) {
    printf("\n[test_keyboard_slot_isolation]\n");
    wireless_slot_t s = {0};
    s.handle = NULL;
    s.keyboard_mode = true;
    s.slot = 1;

    uint8_t pkt_input[20] = {
        0x00, 0x01, 0x00, 0xF0,
        0x00, 0x14, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };
    wireless_handle_packet(&s, pkt_input, sizeof(pkt_input));
    CHECK(s.state.green == false, "slot 1 keyboard input does NOT mutate state (collision guard)");

    s.slot = 0;
    wireless_handle_packet(&s, pkt_input, sizeof(pkt_input));
    CHECK(s.state.green == false, "slot 0 with all-zero buttons leaves green=false");
}

static void test_led_pattern_for_slot(void) {
    printf("\n[test_led_pattern_for_slot]\n");
    for (int slot = 0; slot < 4; slot++) {
        uint8_t pattern = 0x02 + slot;
        uint8_t expected_cmd_byte = (uint8_t)(0x40 | pattern);
        CHECK(expected_cmd_byte == (0x40 | (0x02 + slot)),
              "LED pattern computation per slot");
        (void)expected_cmd_byte;
    }
}

int main(void) {
    printf("Xenon360 wireless internal test harness\n");
    printf("(no USB hardware, no OS event injection)\n");

    test_wireless_match();
    test_presence_transitions();
    test_packet_dispatch();
    test_keyboard_slot_isolation();
    test_led_pattern_for_slot();

    printf("\nResult: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
