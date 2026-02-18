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
#include <cmath> 

// amp-core
#include "Message.h"

#include "SignalGenerator.h"

namespace kc1fsz {

SignalGenerator::SignalGenerator(Log& log, Clock& clock, unsigned lineId, 
    MessageConsumer& bus, unsigned destLineId) 
:   _log(log),
    _clock(clock),
    _lineId(lineId),
    _bus(bus),
    _destLineId(destLineId) {
    _omega = 400.0f * 2.0f * 3.1415926f / 8000.0f;
}

void SignalGenerator::consume(const Message& m) {    
}

void SignalGenerator::audioRateTick(uint32_t tickTimeMs) {    

    int16_t pcm8[160];
    for (unsigned i = 0; i < 160; i++) {
        pcm8[i] = 32767.0f * 0.5 * std::cos(_phi);
        _phi += _omega;
        _phi = fmod(_phi, 2.0f * 3.1415926f);
    }

    uint8_t ulaw[160];
    _tc.encode(pcm8, 160, ulaw, 160);
    MessageWrapper msg(Message::Type::AUDIO, 0, 160, ulaw, 0, 0);
    msg.setDest(_destLineId, Message::UNKNOWN_CALL_ID);
    _bus.consume(msg);
}

}
