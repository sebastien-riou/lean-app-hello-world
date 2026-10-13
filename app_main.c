/* hello-world: prints "hello world", then a report of what every mandatory lean-bsp primitive
 * returned, so a new BSP can be checked end to end with a single `lbsp build-load-run`. */
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "bsp_api.h"

#ifndef HELLO_RESET_WAIT_MS
#define HELLO_RESET_WAIT_MS 3000  /* how long to wait for a reset request; 0 skips the reset test */
#endif

static uint32_t cycles_since(uint32_t start) { return bsp_cycle_count() - start; }

static void report_board(const bsp_capabilities_t *caps) {
    printf("board: %s, %u UART(s), %u LED(s), %u button(s), cycle counter: %s, %u NVM region(s)\n",
           caps->board_name, caps->n_uarts, caps->n_leds, caps->n_buttons,
           caps->has_cycle_counter ? "yes" : "no", caps->n_nvm_regions);
    printf("cpu frequency: bsp_cpu_freq_hz() = %" PRIu32 " Hz\n", bsp_cpu_freq_hz());
}

static void report_uart(void) {
    static const char line[] = "uart: this line was sent with bsp_uart_tx(0, ...), the others with printf\n";
    bsp_uart_tx(0, line, sizeof line - 1);
}

static void report_delay(void) {
    uint32_t start = bsp_cycle_count();
    bsp_delay_ms(100);
    uint32_t cycles = cycles_since(start);
    uint32_t freq = bsp_cpu_freq_hz();
    printf("delay: bsp_delay_ms(100) took %" PRIu32 " cycles", cycles);
    if (freq) printf(", %" PRIu32 " ms at bsp_cpu_freq_hz()", (uint32_t)((uint64_t)cycles * 1000u / freq));
    printf("\n");
}

static void report_cache_clear(void) {
    uint32_t start = bsp_cycle_count();
    bsp_clear_caches();
    printf("cache clear: bsp_clear_caches() took %" PRIu32 " cycles\n", cycles_since(start));
}

static void report_heap(void) {
    bsp_heap_usage_reset();
    volatile char *block = malloc(1000);
    if (!block) {
        printf("heap: malloc(1000) failed\n");
        return;
    }
    block[0] = 1;  /* volatile accesses: the compiler may not drop the allocation */
    block[999] = 2;
    size_t used = bsp_heap_usage();
    free((void *)block);
    printf("heap: with 1000 bytes allocated, bsp_heap_usage() = %lu bytes\n", (unsigned long)used);
}

static void report_leds(const bsp_capabilities_t *caps) {
    for (unsigned led = 0; led < caps->n_leds; led++) {
        for (int i = 0; i < 3; i++) {
            bsp_led_set(led, true);
            bsp_delay_ms(150);
            bsp_led_set(led, false);
            bsp_delay_ms(150);
        }
    }
    printf("led: blinked each of the %u LED(s) 3 times\n", caps->n_leds);
}

static void report_buttons(const bsp_capabilities_t *caps) {
    printf("button:");
    for (unsigned b = 0; b < caps->n_buttons; b++) printf(" %u is %s", b, bsp_button_get(b) ? "pressed" : "released");
    printf(caps->n_buttons ? "\n" : " none\n");
}

/* Reset needs a human to ask for it, otherwise the report would start over forever. */
static void offer_reset(const bsp_capabilities_t *caps) {
#if HELLO_RESET_WAIT_MS == 0
    (void)caps;
    printf("reset: not tested (HELLO_RESET_WAIT_MS=0)\n");
#else
    printf("reset: within %u ms, %ssend 'r' on UART 0 to test bsp_reset()\n", (unsigned)HELLO_RESET_WAIT_MS,
           caps->n_buttons ? "press button 0 or " : "");
    for (uint32_t waited = 0; waited < HELLO_RESET_WAIT_MS; waited += 10) {
        char c;
        bool asked = (caps->n_buttons && bsp_button_get(0)) || (bsp_uart_rx(0, &c, 1) == 1 && (c == 'r' || c == 'R'));
        if (asked) {
            while (caps->n_buttons && bsp_button_get(0)) bsp_delay_ms(10);  /* don't reset again on the way up */
            printf("reset: calling bsp_reset(), the report should start over\n");
            bsp_reset();
        }
        bsp_delay_ms(10);
    }
    printf("reset: not requested\n");
#endif
}

int app_main(void) {
    const bsp_capabilities_t *caps = bsp_get_capabilities();
    printf("hello world\n");
    report_board(caps);
    report_uart();
    report_delay();
    report_cache_clear();
    report_heap();
    report_leds(caps);
    report_buttons(caps);
    offer_reset(caps);
    printf("hello-world done\n");
    return 0;
}
