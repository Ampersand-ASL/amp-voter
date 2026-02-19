/**
 * Copyright (C) 2026, Bruce MacKinnon KC1FSZ
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <stdio.h>

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
#include "SignalGenerator.h"

#define LED_PIN (25)

#define LINE_ID_VOTER (24)
#define LINE_ID_GENERATOR (25)

using namespace std;
using namespace kc1fsz;

static const char* VERSION = "20260219.0";
const char ssid[] = "Gloucester Island Municipal WIFI";
const char wifiPwd[] = "xxx";

int main() {
    
    stdio_init_all();

    // Stock LED setup
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_put(LED_PIN, 1);
    sleep_ms(1000);
    gpio_put(LED_PIN, 0);
    sleep_ms(1000);

    PicoClock2 clock;
    Log log;

    log.info("KC1FSZ Ampersand VOTER");
    log.info("Powered by the Ampersand ASL Project https://github.com/Ampersand-ASL");
    log.info("Version %s", VERSION);

    if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA)) {
        log.error("Failed to initialize WIFI");
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
            if (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP) {
                if (!isUp) {
                    log.info("WIFI is connected %s", 
                        ipaddr_ntoa(&cyw43_state.netif[0].ip_addr));
                    isUp = true;        
                }
            }
        }
    );

    // Setup link to the VOTER server
    VoterClient client24(log, clock, LINE_ID_VOTER, router);
    router.addRoute(&client24, LINE_ID_VOTER);
    // #### TODO REMOVE HARD-CODING
    client24.setClientPassword("client0");
    client24.setServerPassword("parrot0");
    int rc = client24.open("52.8.247.112:1667");
    if (rc != 0) {
        log.error("Failed to open connection");
    }

    // Can be used in inject tones
    SignalGenerator generator25(log, clock, LINE_ID_GENERATOR, router, LINE_ID_VOTER);
    router.addRoute(&generator25, LINE_ID_GENERATOR);

    // Main loop        
    Runnable2* tasks2[] = { &cy34Task, &timer1, &client24, &generator25 };
    log.info("Entering event loop ...");
    PicoEventLoop::run(log, clock, 0, 0, tasks2, std::size(tasks2), nullptr, false);
}

