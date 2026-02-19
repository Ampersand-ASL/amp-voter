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
#pragma once

#include <netinet/in.h>

#include <functional>

// amp-core
#include "Transcoder_G711_ULAW.h"
#include "Runnable2.h"
#include "IAX2Util.h"
#include "Message.h"
#include "MessageConsumer.h"
#include "VoterPeer.h"

namespace kc1fsz {

class Log;
class Clock;

class SignalGenerator : public Runnable2, public MessageConsumer {
public:

    /**
     * @param consumer This is the sink interface that received messages
     * will be sent to. VERY IMPORTANT: Audio frames will not have been 
     * de-jittered before they are passed to this sink. 
     */
    SignalGenerator(Log& log, Clock& clock, unsigned lineId, MessageConsumer& consumer,
        unsigned destLineId);
   
    // ----- Line/MessageConsumer-----------------------------------------------------

    virtual void consume(const Message& m);

    // ----- Runnable -------------------------------------------------------

    virtual void audioRateTick(uint32_t tickTimeMs);

private:

    Log& _log;
    Clock& _clock;
    const unsigned _lineId;
    MessageConsumer& _bus;
    const unsigned _destLineId;

    float _omega = 0;
    float _phi = 0;

    Transcoder_G711_ULAW _tc;
};

}
