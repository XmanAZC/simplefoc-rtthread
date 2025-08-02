
#include "SimpleFOCDebug.h"

#ifndef SIMPLEFOC_DISABLE_DEBUG

void SimpleFOCDebug::enable() {
}

void SimpleFOCDebug::println(int val) {
    rt_kprintf("%d\n", val);
}

void SimpleFOCDebug::println(float val) {
    rt_kprintf("%f\n", val);
}

void SimpleFOCDebug::println(const char* str) {
    rt_kprintf("%s\n", str);
}

// void SimpleFOCDebug::println(const __FlashStringHelper* str) {
//     rt_kprintf("%s\n", str);
// }


void SimpleFOCDebug::println(const char* str, float val) {
    rt_kprintf("%s %f\n", str, val);
}

// void SimpleFOCDebug::println(const __FlashStringHelper* str, float val) {
//     rt_kprintf("%s %f\n", str, val);
// }

void SimpleFOCDebug::println(const char* str, int val) {
    rt_kprintf("%s %d\n", str, val);
}

void SimpleFOCDebug::println(const char* str, char val) {
    rt_kprintf("%s %c\n", str, val);
}

// void SimpleFOCDebug::println(const __FlashStringHelper* str, int val) {
//     rt_kprintf("%s %d\n", str, val);
// }


void SimpleFOCDebug::print(const char* str) {
    rt_kprintf("%s", str);
}


// void SimpleFOCDebug::print(const __FlashStringHelper* str) {
//     rt_kprintf("%s", str);
// }

// void SimpleFOCDebug::print(const StringSumHelper str) {
//     rt_kprintf("%s", str.c_str());
// }


// void SimpleFOCDebug::println(const StringSumHelper str) {
//     rt_kprintf("%s\n", str.c_str());
// }

void SimpleFOCDebug::print(int val) {
    rt_kprintf("%d", val);
}


void SimpleFOCDebug::print(float val) {
    rt_kprintf("%f", val);
}


void SimpleFOCDebug::println() {
    rt_kprintf("\n");
}

#endif