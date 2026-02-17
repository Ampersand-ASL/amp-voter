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
#include <stdlib.h>
#include <unistd.h>
#include <execinfo.h>
#include <signal.h>
#include <pthread.h>
#include <sys/syscall.h> 
#include <cstring>

#include <iostream>
#include <cmath> 
#include <queue>
#include <thread>

// KC1FSZ
#include "kc1fsz-tools/Log.h"
#include "kc1fsz-tools/MTLog2.h"
#include "kc1fsz-tools/linux/StdClock.h"
#include "kc1fsz-tools/fixedqueue.h"
#include "kc1fsz-tools/threadsafequeue.h"
#include "kc1fsz-tools/threadsafequeue2.h"
#include "kc1fsz-tools/linux/MTLog.h"
#include "kc1fsz-tools/copyableatomic.h"

// amp-core
#include "NullLog.h"
#include "EventLoop.h"
#include "MultiRouter.h"
#include "ThreadUtil.h"
#include "TimerTask.h"

#include "VoterClient.h"
#include "SignalGenerator.h"

#define LINE_ID_VOTER (24)
#define LINE_ID_GENERATOR (25)

using namespace std;
using namespace kc1fsz;

static const char* VERSION = "20260217.0";

static void sigHandler(int sig);

int main(int argc, const char** argv) {

    amp::setThreadName("Voter");

    signal(SIGSEGV, sigHandler);
    MTLog2 log;

    log.info("KC1FSZ Ampersand Voter Simulator");
    log.info("Powered by the Ampersand ASL Project https://github.com/Ampersand-ASL");
    log.info("Version %s", VERSION);

    StdClock clock;
    NullLog traceLog;

    // Setup the message router
    threadsafequeue2<MessageCarrier> respQueue;
    MultiRouter router(respQueue);

    // Setup link to the voter server
    VoterClient client24(log, clock, LINE_ID_VOTER, router);
    router.addRoute(&client24, LINE_ID_VOTER);
    client24.setClientPassword(getenv("AMP_VOTER_CLIENT_PASSWORD"));
    client24.setServerPassword(getenv("AMP_VOTER_SERVER_PASSWORD"));
    int rc = client24.open(getenv("AMP_VOTER_SERVER_ADDR"));
    if (rc != 0) {
        log.error("Failed to open connection");
        std::exit(-1);
    }

    // Can be used in inject tones
    SignalGenerator generator25(log, clock, LINE_ID_GENERATOR, router, LINE_ID_VOTER);
    router.addRoute(&generator25, LINE_ID_GENERATOR);

    // Can be used to generate period activity
    TimerTask timer1(log, clock, 10, 
        [&log, &client24]() {
        }
    );

    // Main loop        
    Runnable2* tasks2[] = { &client24, &generator25, &router, &timer1 };
    EventLoop::run(log, clock, 0, 0, tasks2, std::size(tasks2), nullptr, false);

    return 0;
}

// A crash signal handler that displays stack information
static void sigHandler(int sig) {
    void *array[32];
    // get void*'s for all entries on the stack
    size_t size = backtrace(array, 32);
    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    // Now do the regular thing
    signal(sig, SIG_DFL); 
    raise(sig);
}
