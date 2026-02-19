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

include(pico_sdk_import.cmake)

project(amp-voter C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

pico_sdk_init()

add_compile_options(-g)

# ----- voter ---------------------------------------------------------------

add_executable(voter
  src/main.cpp
  src/VoterClient.cpp
  amp-core/src/VoterUtil.cpp
  amp-core/src/VoterPeer.cpp
  amp-core/src/pico/PicoEventLoop.cpp
  kc1fsz-tools-cpp/src/Common.cpp
  kc1fsz-tools-cpp/src/NetUtils.cpp
  kc1fsz-tools-cpp/src/StdPollTimer.cpp
  micro-ip/impl-pico/main.c
  micro-ip/impl-pico/main2.c
)

pico_enable_stdio_usb(voter 1)
pico_enable_stdio_uart(voter 0)
pico_add_extra_outputs(voter)

target_include_directories(voter PRIVATE src)
target_include_directories(voter PRIVATE amp-core/src)
target_include_directories(voter PRIVATE amp-core/include)
target_include_directories(voter PRIVATE kc1fsz-tools-cpp/include)
target_include_directories(voter PRIVATE micro-ip)

target_link_libraries(voter pico_cyw43_arch_lwip_poll pico_stdlib)





