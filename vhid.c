#include "vhid.h"
#include <stdio.h>
#include <stdlib.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>

typedef struct __IOHIDUserDevice *IOHIDUserDeviceRef;

extern IOHIDUserDeviceRef IOHIDUserDeviceCreate(CFAllocatorRef allocator,
                                                CFDictionaryRef properties);
extern IOReturn IOHIDUserDeviceHandleReport(IOHIDUserDeviceRef device,
                                            const uint8_t *report,
                                            CFIndex reportLength);
extern void CFRelease(CFTypeRef cf);

struct vhid {
    IOHIDUserDeviceRef dev;
};

static const uint8_t hid_descriptor[] = {
    0x05, 0x01,
    0x09, 0x05,
    0xA1, 0x01,

    0x05, 0x09,
    0x19, 0x01,
    0x29, 0x10,
    0x15, 0x00,
    0x25, 0x01,
    0x75, 0x01,
    0x95, 0x10,
    0x81, 0x02,

    0x05, 0x01,
    0x09, 0x30,
    0x09, 0x31,
    0x09, 0x32,
    0x09, 0x35,
    0x16, 0x00, 0x80,
    0x26, 0xFF, 0x7F,
    0x75, 0x10,
    0x95, 0x04,
    0x81, 0x02,

    0x09, 0x39,
    0x15, 0x00,
    0x25, 0x07,
    0x35, 0x00,
    0x46, 0x3B, 0x01,
    0x65, 0x14,
    0x75, 0x04,
    0x95, 0x01,
    0x81, 0x42,

    0x75, 0x04, 0x95, 0x01, 0x81, 0x03,

    0xC0
};

static void dict_set_int(CFMutableDictionaryRef d, CFStringRef key, int value) {
    CFNumberRef n = CFNumberCreate(NULL, kCFNumberIntType, &value);
    CFDictionarySetValue(d, key, n);
    CFRelease(n);
}

vhid_t *vhid_create_slot(int slot) {
    CFMutableDictionaryRef props = CFDictionaryCreateMutable(
        NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    CFDataRef desc = CFDataCreate(NULL, hid_descriptor, sizeof(hid_descriptor));
    CFDictionarySetValue(props, CFSTR("ReportDescriptor"), desc);
    CFRelease(desc);

    dict_set_int(props, CFSTR("VendorID"),         0x1209);
    dict_set_int(props, CFSTR("ProductID"),        0x5860 + slot);
    dict_set_int(props, CFSTR("PrimaryUsagePage"), 0x01);
    dict_set_int(props, CFSTR("PrimaryUsage"),     0x05);
    dict_set_int(props, CFSTR("VersionNumber"),    0x0100);

    char product[64];
    char serial[64];
    snprintf(product, sizeof(product), "Xenon360 Virtual Guitar Slot %d", slot + 1);
    snprintf(serial,  sizeof(serial),  "xenon360-%04d", slot + 1);
    CFStringRef prod_cf = CFStringCreateWithCString(NULL, product, kCFStringEncodingUTF8);
    CFStringRef ser_cf  = CFStringCreateWithCString(NULL, serial,  kCFStringEncodingUTF8);
    CFDictionarySetValue(props, CFSTR("Product"),      prod_cf);
    CFDictionarySetValue(props, CFSTR("Manufacturer"), CFSTR("Vesanerie / Xenon360"));
    CFDictionarySetValue(props, CFSTR("SerialNumber"), ser_cf);
    CFRelease(prod_cf);
    CFRelease(ser_cf);

    IOHIDUserDeviceRef dev = IOHIDUserDeviceCreate(kCFAllocatorDefault, props);
    CFRelease(props);

    if (!dev) {
        fprintf(stderr, "vhid: IOHIDUserDeviceCreate a echoue (slot %d).\n", slot + 1);
        fprintf(stderr, "  Sur Apple Silicon il faut probablement :\n");
        fprintf(stderr, "    1. csrutil disable (depuis Recovery, Cmd-R au boot)\n");
        fprintf(stderr, "    2. sudo nvram boot-args=\"amfi_get_out_of_my_way=1\"\n");
        fprintf(stderr, "    3. reboot\n");
        return NULL;
    }

    vhid_t *v = calloc(1, sizeof(*v));
    v->dev = dev;
    return v;
}

vhid_t *vhid_create(void) {
    return vhid_create_slot(0);
}

void vhid_destroy(vhid_t *v) {
    if (!v) return;
    if (v->dev) CFRelease(v->dev);
    free(v);
}

int vhid_send(vhid_t *v, const vhid_report_t *r) {
    if (!v || !v->dev) return -1;
    IOReturn ret = IOHIDUserDeviceHandleReport(v->dev, (const uint8_t *)r, sizeof(*r));
    return (ret == kIOReturnSuccess) ? 0 : -1;
}
