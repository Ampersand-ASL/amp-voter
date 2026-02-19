/*
Building:

    export PICO_SDK_FETCH_FROM_GIT=1
    mkdir build
    cd build
    # Debug build:
    cmake .. -DPICO_BOARD=pico_w -DCMAKE_BUILD_TYPE=Debug
    make voter

Flashing:

    ~/git/openocd/src/openocd -s ~/git/openocd/tcl -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program voter.elf verify reset exit"

Serial console:

    minicom -b 115200 -o -D /dev/ttyACM1
*/

#include <stdio.h>
#include <iostream>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"

#include "lwip/pbuf.h"
#include "lwip/udp.h"

#include "kc1fsz-tools/Log.h"
#include "kc1fsz-tools/rp2040/PicoClock2.h"

#include "amp/CYW43Task.h"
#include "TimerTask.h"
#include "pico/PicoEventLoop.h"
#include "SimpleRouter.h"

#include "VoterClient.h"
//#include "SignalGenerator.h"

#define LED_PIN (25)

#define LINE_ID_VOTER (24)
#define LINE_ID_GENERATOR (25)

using namespace std;
using namespace kc1fsz;

const char ssid[] = "Gloucester Island Municipal WIFI";
const char wifiPwd[] = "xxx";

static void myRecv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    printf("Got %c\n", pbuf_get_at(p, 0));
    pbuf_free(p);
}

int main() {
    
    stdio_init_all();

    // Stock LED setup
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_put(LED_PIN, 1);
    sleep_ms(1000);
    gpio_put(LED_PIN, 0);
    sleep_ms(1000);

    cout << "Starting ..." << endl;

    PicoClock2 clock;
    Log log;

    if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA)) {
        printf("failed to initialise\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    cyw43_arch_wifi_connect_async(ssid, wifiPwd, CYW43_AUTH_WPA2_AES_PSK);

    bool isUp = false;    
    struct udp_pcb* u = 0;

    SimpleRouter router;

    // This task is required to keep the WIFI events flowing
    amp::CYW43Task cy34Task;

    // This task monitors for WIFI connect/disconnect events
    TimerTask timer1(log, clock, 5, 
        [&log, &isUp, &u]() {
            log.info("Tick");
            if (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP) {
                if (!isUp) {

                    cout << "Good" << endl;
                    cout << ipaddr_ntoa(&cyw43_state.netif[0].ip_addr) << endl;
                    isUp = true;
        
                    // Setup a UDP port
                    u = udp_new_ip_type(IPADDR_TYPE_ANY);
                    if (!u) {
                        cout << "Failed to create pcb" << endl;
                    } else {
                        udp_bind(u, IP_ADDR_ANY, 5190);

                        // Install a receive
                        udp_recv(u, myRecv, 0);
                    }
                }
            }
        }
    );

    // Setup link to the voter server
    VoterClient client24(log, clock, LINE_ID_VOTER, router);
    router.addRoute(&client24, LINE_ID_VOTER);
    client24.setClientPassword("client0");
    client24.setServerPassword("parrot0");
    log.info("Opening VOTER");
    int rc = client24.open("52.8.247.112:1667");
    if (rc != 0) {
        log.error("Failed to open connection");
    }

    // Main loop        
    Runnable2* tasks2[] = { &cy34Task, &timer1, &client24 };
    log.info("Entering loop ...");
    PicoEventLoop::run(log, clock, 0, 0, tasks2, std::size(tasks2), nullptr, false);
}

