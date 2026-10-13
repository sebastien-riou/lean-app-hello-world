# lean-app-hello-world

The minimal [lean-bsp](https://github.com/sebastien-riou/lean-bsp) application, and the check every new BSP goes
through: after printing `hello world`, it reports what each mandatory primitive of the BSP returned.

````
lbsp build-load-run                       # on linux-host, the default_bsp in app.toml
lbsp build-load-run --bsp nucleo-l552     # on a board
````

On linux-host the report looks like this:

````
hello world
board: linux-host, 1 UART(s), 0 LED(s), 0 button(s), cycle counter: yes, 0 NVM region(s)
cpu frequency: bsp_cpu_freq_hz() = 1000000000 Hz
uart: this line was sent with bsp_uart_tx(0, ...), the others with printf
delay: bsp_delay_ms(100) took 100373224 cycles, 100 ms at bsp_cpu_freq_hz()
cache clear: bsp_clear_caches() took 90 cycles
heap: with 1000 bytes allocated, bsp_heap_usage() = 1000 bytes
led: blinked each of the 0 LED(s) 3 times
button: none
reset: within 3000 ms, send 'r' on UART 0 to test bsp_reset()
reset: not requested
hello-world done
````

Reset is tested only on request, otherwise the report would start over forever: within 3 s, press button 0 (on a
board that has one) or send `r` on UART 0 — on linux-host, type `r` then Enter. `--define HELLO_RESET_WAIT_MS=<ms>`
changes the wait, and `--define HELLO_RESET_WAIT_MS=0` skips it.
