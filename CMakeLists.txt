# Ampersand Project
# Copyright (C) 2025, Bruce MacKinnon KC1FSZ
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# ( at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
cmake_minimum_required(VERSION 3.13)
project(ampersand C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

# ----- Voter Simulator -----------------------------------------------------------

add_executable(voter-sim
  src/sim-main.cpp
  src/VoterClient.cpp
  src/SignalGenerator.cpp
  amp-core/src/EventLoop.cpp
  amp-core/src/VoterUtil.cpp
  amp-core/src/VoterPeer.cpp
  amp-core/src/MultiRouter.cpp
  amp-core/src/Message.cpp
  amp-core/src/ThreadUtil.cpp
  amp-core/src/Transcoder_G711_ULAW.cpp
  kc1fsz-tools-cpp/src/Common.cpp
  kc1fsz-tools-cpp/src/NetUtils.cpp
  kc1fsz-tools-cpp/src/StdPollTimer.cpp
  kc1fsz-tools-cpp/src/fixed_math.cpp
  kc1fsz-tools-cpp/src/linux/StdClock.cpp
  kc1fsz-tools-cpp/src/linux/MTLog.cpp
  itu-g711-codec/src/codec.cpp
)
target_compile_options(voter-sim PRIVATE -Wall -Wpedantic -g -O3 -mtune=native -ffast-math)

target_include_directories(voter-sim PRIVATE src)
target_include_directories(voter-sim PRIVATE include)
target_include_directories(voter-sim PRIVATE kc1fsz-tools-cpp/include)
target_include_directories(voter-sim PRIVATE amp-core/include)
target_include_directories(voter-sim PRIVATE amp-core/src)
target_include_directories(voter-sim PRIVATE itu-g711-codec/src)

