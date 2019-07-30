# Fluke_Z8_Emulator

This is a attempt at a Z8 emulator using a PSoC processor that can be used in a Fluke 8840A multimeter, and possibly a Fluke 8842A multimeter (although I have not tested the latter.)

I'm releasing this in the hope that someone will be able to help fix the remaining issue, the code is currently in a messy state as I've not had a chance to clean it up prior to releasing it.

The pcb and schematic (KiCad format) are available in the pcb directory.

The PSoC Creator workspace is in the PSoC_Workspace directory.

** THIS IS FAR FROM PRODUCTION WORTHY **

Some known issues:

1. There is a mistake on the PCB where all the P5 pins are offset by one, this doesn't cause a problem as it can be fixed by configuration, I'll fix this on the next version.

2. The GPIB board doesn't work properly. It works ok with external trigger set, but you get intermittent strange behaviour if not using the external trigger (more info below.)

3. I'm not entirely sure everything else is ok as I've hacked around with just about everything trying to fix the GPIB problem.

I'm fairly sure the outstanding issue is one of three things:

- A mis-implemented instruction
- Still something wrong with interrupts and masking
- A timing thing

Basic Description:

** I will spend some time documenting this properly, but for now here is a very brief overview **

Port 0 and 1 -- there is a verilog module that recreates the Z8 bus protocol which controls most of port 0 and 1.

Port 2 -- port 2 is a completely normal port, handled by firmware.

Port 3 -- port 3 is made up of incoming interrupts, one input sense line, and serial and timer.

Timer 1 -- the timer is implemented using a PSoC timer, but with a verilog module attached to the output to toggle the output at the end of each timing cycle. There is also the ability to set the output to 1 to copy the behaviour of the Z8 timer.

Serial -- currently there are three different serial implementations (all attempting to fix the GPIB problem), however the behaviour is the same with all of them, so I don't think it's related. The standard PSoC UART module is used for tx, I then have two verilog RX modules. SerialDRX is implemented using a datapath component so is lightweight and needs to be used if the timer block is also used. (I'm quite pleased with this as it's super efficient as it uses the datapath for both timer and shift register!)

TODO's for the next version...

1. Redo the whole bus access bit ... there are probably quite a lot of efficiencies to be had to give us more headroom for other things.

2. New board ... different output for the clock signal. Re-look at the P3 pins ... why did they stick the debug pins in the middle of a useful port???

