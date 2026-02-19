A software simulation of the VOTER board.

[Most of the Ampersand project documentation is here](https://mackinnon.info/ampersand/).

# Building

    export PICO_SDK_FETCH_FROM_GIT=1
    mkdir build
    cd build
    # Debug build:
    cmake .. -DPICO_BOARD=pico_w -DCMAKE_BUILD_TYPE=Debug
    make voter

# Flashing

    ~/git/openocd/src/openocd -s ~/git/openocd/tcl -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program voter.elf verify reset exit"

# Serial Console

    minicom -b 115200 -o -D /dev/ttyACM1
