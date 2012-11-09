###################################################################
#          This file is machine-generated - Do NOT edit!          #
#                    Edit scripts.make.rb instead!                #
# Written by: Paulo H. "Taka" Torrens <paulo_torrens@hotmail.com> #
###################################################################

CC = avr-gcc $(CPP_FLAGS)
LD = avr-gcc $(LD_FLAGS)
AR = avr-ar $(AR_FLAGS)
OBJCP = avr-objcopy
SIZE = avr-size -A --mcu=$(MCU)
AVRDUDE = ./avrdude.sh

LIBRARIES = IRremote.a Ports.a RF12.a default.a
OBJECTS = src/main.o
INCLUDES = -I"/usr/lib/avr/include/avr" -I"./include" -I"src" -I"lib/IRremote" -I"lib/IRremote/examples" -I"lib/IRremote/examples/IRsendDemo" -I"lib/IRremote/examples/IRrelay" -I"lib/IRremote/examples/IRrecvDump" -I"lib/IRremote/examples/IRrecord" -I"lib/IRremote/examples/IRrecvDemo" -I"lib/IRremote/examples/IRtest" -I"lib/Ports" -I"lib/Ports/examples" -I"lib/Ports/examples/rtc_demo" -I"lib/Ports/examples/toneGen" -I"lib/Ports/examples/glcdClock" -I"lib/Ports/examples/h48c_demo" -I"lib/Ports/examples/recv433demo" -I"lib/Ports/examples/SMDdemo" -I"lib/Ports/examples/dimmer_demo" -I"lib/Ports/examples/lux_demo" -I"lib/Ports/examples/pir_demo" -I"lib/Ports/examples/bmp085recv" -I"lib/Ports/examples/powerdown_demo" -I"lib/Ports/examples/opto_demo" -I"lib/Ports/examples/isp_repair" -I"lib/Ports/examples/nunchuk_demo" -I"lib/Ports/examples/eemem" -I"lib/Ports/examples/glcdScope" -I"lib/Ports/examples/expander" -I"lib/Ports/examples/uart_demo" -I"lib/Ports/examples/indoor" -I"lib/Ports/examples/lcd_demo" -I"lib/Ports/examples/relay_demo" -I"lib/Ports/examples/glcdSpectrum" -I"lib/Ports/examples/blink_demo" -I"lib/Ports/examples/blink_ports" -I"lib/Ports/examples/heading_demo" -I"lib/Ports/examples/output_stepper" -I"lib/Ports/examples/button_demo" -I"lib/Ports/examples/ir_recv_irq" -I"lib/Ports/examples/dcmotor_demo" -I"lib/Ports/examples/glcd_demo" -I"lib/Ports/examples/dcf77demo" -I"lib/Ports/examples/memory_demo" -I"lib/Ports/examples/schedule" -I"lib/Ports/examples/bmp085demo" -I"lib/Ports/examples/highside" -I"lib/Ports/examples/parser_demo" -I"lib/Ports/examples/thermo_demo" -I"lib/Ports/examples/isp_repair2" -I"lib/Ports/examples/isp_capture" -I"lib/Ports/examples/blink_recv" -I"lib/Ports/examples/sht11demo" -I"lib/Ports/examples/analog_demo" -I"lib/Ports/examples/isp_flash" -I"lib/Ports/examples/qti_demo" -I"lib/Ports/examples/glcdTracer" -I"lib/Ports/examples/snapNikon" -I"lib/Ports/examples/ir_recv" -I"lib/Ports/examples/input_demo" -I"lib/Ports/examples/combi_demo" -I"lib/Ports/examples/hm55b_demo" -I"lib/Ports/examples/tsl230demo" -I"lib/Ports/examples/ir_repeater" -I"lib/Ports/examples/accel2125_demo" -I"lib/Ports/examples/isp_prepare" -I"lib/Ports/examples/proximity_demo" -I"lib/Ports/examples/blink_xmit" -I"lib/Ports/examples/gravity_demo" -I"lib/RF12" -I"lib/RF12/examples" -I"lib/RF12/examples/rf12serial" -I"lib/RF12/examples/pingPong" -I"lib/RF12/examples/timedRecv" -I"lib/RF12/examples/kaku_demo" -I"lib/RF12/examples/rf12stream" -I"lib/RF12/examples/rfRangeRX" -I"lib/RF12/examples/groupRelay" -I"lib/RF12/examples/RFM12B_OOK" -I"lib/RF12/examples/fs20demo" -I"lib/RF12/examples/packetBuf" -I"lib/RF12/examples/timedSend" -I"lib/RF12/examples/rgbRemote" -I"lib/RF12/examples/crypRecv" -I"lib/RF12/examples/radioBlip" -I"lib/RF12/examples/rfRangeTX" -I"lib/RF12/examples/roomNode" -I"lib/RF12/examples/crypSend" -I"lib/RF12/examples/ookRelay2" -I"lib/RF12/examples/RF12demo" -I"lib/default"
DEFINES = -DF_CPU=$(F_CPU)L -DARDUINO=18

CPP_FLAGS = -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -mmcu=$(MCU) $(DEFINES) $(INCLUDES)
LD_FLAGS = -Os -Wl,--gc-sections -mmcu=$(MCU)
AR_FLAGS = rcs

USB=0
PORT = /dev/ttyUSB$(USB)
MCU = atmega328p
F_CPU = 16000000
FORMAT = ihex
UPLOAD_RATE = 115200
MAX_SIZE = 30720

OUTPUT = wl500

all: $(LIBRARIES) $(OUTPUT).hex

IRremote.a: lib/IRremote/IRremote.o
	$(AR) IRremote.a lib/IRremote/IRremote.o

lib/IRremote/IRremote.o: lib/IRremote/IRremote.cpp
	$(CC) -c lib/IRremote/IRremote.cpp -o lib/IRremote/IRremote.o

Ports.a: lib/Ports/PortsSHT11.o lib/Ports/PortsRF12.o lib/Ports/Ports.o lib/Ports/PortsBMP085.o lib/Ports/examples/glcdSpectrum/fix_fft.o lib/Ports/PortsLCD.o
	$(AR) Ports.a lib/Ports/PortsSHT11.o
	$(AR) Ports.a lib/Ports/PortsRF12.o
	$(AR) Ports.a lib/Ports/Ports.o
	$(AR) Ports.a lib/Ports/PortsBMP085.o
	$(AR) Ports.a lib/Ports/examples/glcdSpectrum/fix_fft.o
	$(AR) Ports.a lib/Ports/PortsLCD.o

lib/Ports/PortsSHT11.o: lib/Ports/PortsSHT11.cpp
	$(CC) -c lib/Ports/PortsSHT11.cpp -o lib/Ports/PortsSHT11.o

lib/Ports/PortsRF12.o: lib/Ports/PortsRF12.cpp
	$(CC) -c lib/Ports/PortsRF12.cpp -o lib/Ports/PortsRF12.o

lib/Ports/Ports.o: lib/Ports/Ports.cpp
	$(CC) -c lib/Ports/Ports.cpp -o lib/Ports/Ports.o

lib/Ports/PortsBMP085.o: lib/Ports/PortsBMP085.cpp
	$(CC) -c lib/Ports/PortsBMP085.cpp -o lib/Ports/PortsBMP085.o

lib/Ports/examples/glcdSpectrum/fix_fft.o: lib/Ports/examples/glcdSpectrum/fix_fft.cpp
	$(CC) -c lib/Ports/examples/glcdSpectrum/fix_fft.cpp -o lib/Ports/examples/glcdSpectrum/fix_fft.o

lib/Ports/PortsLCD.o: lib/Ports/PortsLCD.cpp
	$(CC) -c lib/Ports/PortsLCD.cpp -o lib/Ports/PortsLCD.o

RF12.a: lib/RF12/RF12sio.o lib/RF12/RF12.o
	$(AR) RF12.a lib/RF12/RF12sio.o
	$(AR) RF12.a lib/RF12/RF12.o

lib/RF12/RF12sio.o: lib/RF12/RF12sio.cpp
	$(CC) -c lib/RF12/RF12sio.cpp -o lib/RF12/RF12sio.o

lib/RF12/RF12.o: lib/RF12/RF12.cpp
	$(CC) -c lib/RF12/RF12.cpp -o lib/RF12/RF12.o

default.a: lib/default/main.o lib/default/wiring_digital.o lib/default/wiring_analog.o lib/default/WMath.o lib/default/WInterrupts.o lib/default/Tone.o lib/default/HardwareSerial.o lib/default/wiring_pulse.o lib/default/wiring_shift.o lib/default/pins_arduino.o lib/default/wiring.o lib/default/Print.o
	$(AR) default.a lib/default/main.o
	$(AR) default.a lib/default/wiring_digital.o
	$(AR) default.a lib/default/wiring_analog.o
	$(AR) default.a lib/default/WMath.o
	$(AR) default.a lib/default/WInterrupts.o
	$(AR) default.a lib/default/Tone.o
	$(AR) default.a lib/default/HardwareSerial.o
	$(AR) default.a lib/default/wiring_pulse.o
	$(AR) default.a lib/default/wiring_shift.o
	$(AR) default.a lib/default/pins_arduino.o
	$(AR) default.a lib/default/wiring.o
	$(AR) default.a lib/default/Print.o

lib/default/main.o: lib/default/main.cpp
	$(CC) -c lib/default/main.cpp -o lib/default/main.o

lib/default/wiring_digital.o: lib/default/wiring_digital.c
	$(CC) -c lib/default/wiring_digital.c -o lib/default/wiring_digital.o

lib/default/wiring_analog.o: lib/default/wiring_analog.c
	$(CC) -c lib/default/wiring_analog.c -o lib/default/wiring_analog.o

lib/default/WMath.o: lib/default/WMath.cpp
	$(CC) -c lib/default/WMath.cpp -o lib/default/WMath.o

lib/default/WInterrupts.o: lib/default/WInterrupts.c
	$(CC) -c lib/default/WInterrupts.c -o lib/default/WInterrupts.o

lib/default/Tone.o: lib/default/Tone.cpp
	$(CC) -c lib/default/Tone.cpp -o lib/default/Tone.o

lib/default/HardwareSerial.o: lib/default/HardwareSerial.cpp
	$(CC) -c lib/default/HardwareSerial.cpp -o lib/default/HardwareSerial.o

lib/default/wiring_pulse.o: lib/default/wiring_pulse.c
	$(CC) -c lib/default/wiring_pulse.c -o lib/default/wiring_pulse.o

lib/default/wiring_shift.o: lib/default/wiring_shift.c
	$(CC) -c lib/default/wiring_shift.c -o lib/default/wiring_shift.o

lib/default/pins_arduino.o: lib/default/pins_arduino.c
	$(CC) -c lib/default/pins_arduino.c -o lib/default/pins_arduino.o

lib/default/wiring.o: lib/default/wiring.c
	$(CC) -c lib/default/wiring.c -o lib/default/wiring.o

lib/default/Print.o: lib/default/Print.cpp
	$(CC) -c lib/default/Print.cpp -o lib/default/Print.o

src/main.o: src/main.cpp
	$(CC) -c src/main.cpp -o src/main.o

$(OUTPUT).elf: $(OBJECTS) $(LIBRARIES)
	$(LD) $(OBJECTS) $(LIBRARIES) -lm -o $(OUTPUT).elf

$(OUTPUT).hex: $(OUTPUT).elf
	$(OBJCP) -O ihex -R .eeprom $(OUTPUT).elf $(OUTPUT).hex
	$(SIZE) $(OUTPUT).hex | scripts/size.rb $(MAX_SIZE) "$(OUTPUT).hex"
  
.PHONY: upload clean

upload: all
	scp $(OUTPUT).hex wrt:/tmp/ ; ssh wrt "$(AVRDUDE) -Uflash:w:/tmp/$(OUTPUT).hex:i"

upload_local: all
	avrdude -p $(MCU) -c arduino -P $(PORT) -v -Uflash:w:$(OUTPUT).hex:i

clean:
	@rm -f $(LIBRARIES) $(OUTPUT).elf $(OUTPUT).hex $(shell find . -follow -name "*.o")
  
Makefile: scripts/make.rb $(shell find src -follow -not -type f -newer Makefile)
	@scripts/make.rb
