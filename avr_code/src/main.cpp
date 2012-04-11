/*
 */

#include <RF12.h>
#include <Ports.h>
#include <stdlib.h>
#include <stdio.h>
#include <IRremote.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>   
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <WProgram.h>

#define NODE_ID 1
#define NETGROUP 42

#define SOCKET_GROUPS 1
#define SOCKET_IDS 4

#define TRANSMITTER_PIN 9

#define PULSE_LENGTH_0 521
#define PULSE_LENGTH_1 1224
#define PULSE_PAUSE_LENGTH 3265

#define NUM_REPEATS 25
#define REPEAT_PAUSE_MS 22

#define NUM_COUNTERS 5

typedef struct {
	char counter[NUM_COUNTERS];
	uint32_t duration[NUM_COUNTERS];
} Powermeter;
Powermeter powermeter;

typedef struct {
	int pir;
	int tC;
	int t_awake;
} Thermometer;
Thermometer thermometer;

char serialString[256];
char inChar = -1;
int strIdx = 0;
uint32_t msg_id;

bool socketState[SOCKET_IDS][SOCKET_GROUPS];
bool sequences[8][26] = {

	{1,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,1,1,0,1,1,1},
	{1,0,0,1,1,0,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1,1,0,0,1,1},

	{1,0,0,1,1,0,0,0,0,0,0,0,1,1,0,1,1,0,1,0,1,1,1,1,1,0},
	{1,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,1,1,1,1,1,0,1,0},

	{1,0,0,1,1,0,0,0,0,0,0,0,1,1,0,1,0,1,1,0,1,1,1,1,0,1},
	{1,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,0,1,1,1,1,1,1,0,0,1},

	{1,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,1,0,1,1,0,1,0,0},
	{1,0,0,1,1,0,0,0,0,0,0,0,1,1,0,1,0,0,1,1,1,1,0,0,0,0}
};

IRsend irsend;


void pulseOut(int pin, int us) {
	digitalWrite(pin, HIGH);
	us = max(us - 20, 1);
	delayMicroseconds(us);
	digitalWrite(pin, LOW);
} 

void transmitBit(bool value) {
	if(value) {
		pulseOut(TRANSMITTER_PIN, PULSE_LENGTH_1);
	} else {
		pulseOut(TRANSMITTER_PIN, PULSE_LENGTH_0);
	}

	delayMicroseconds(PULSE_PAUSE_LENGTH);
}


void printHeader() {
	Serial.print("hdr: 0x");
	Serial.print(rf12_hdr, HEX);
	Serial.print(", seq: ");
	Serial.print(rf12_seq, DEC);
	Serial.print(", data =");
}

void printPower() {
	powermeter = *(Powermeter*) rf12_data;
	printHeader();
	for (int i = 0; i < NUM_COUNTERS; i++) {
		Serial.print(' ');
		Serial.print(powermeter.counter[i], DEC);
		Serial.print(' ');
		Serial.print(powermeter.duration[i], DEC);
	}
	Serial.println();
}

void printTemp() {
 int tC, tFrac;
	thermometer = *(Thermometer*) rf12_data;
 tC = thermometer.tC;                             // read high-resolution temperature

	printHeader();
	Serial.print(' ');

 if (tC < 0) {
		tC = -tC;                                   // fix for integer division
		Serial.print("-");                          // indicate negative
 }

 tFrac = tC % 100;                             // extract fractional part
 tC /= 100;                                    // extract whole part

 Serial.print(tC);
 Serial.print(".");
 if (tFrac < 10)
		Serial.print("0");
 Serial.print(tFrac);
	Serial.print(" degrees C, awake: ");
	Serial.print(thermometer.t_awake);
	Serial.print("ms, PIR: ");
	Serial.print(thermometer.pir);
	Serial.println();
}

void printUnknown() {
	int rf12_node_id;
	rf12_node_id = 0x1F & rf12_hdr;
	Serial.print("Unknown node: id: ");
	Serial.print(rf12_node_id, DEC);
	Serial.print(", hdr: 0b");
	Serial.print(rf12_hdr, BIN);
	Serial.print(" (0x");
	Serial.print(rf12_hdr, HEX);
	Serial.print(")");
	Serial.print(", seq: ");
	Serial.println(rf12_seq, DEC);
}

void receiveRF12() {
	int rf12_node_id;
	if (rf12_recvDone() && (rf12_crc == 0)) {
		rf12_node_id = 0x1F & rf12_hdr;
		if ((rf12_node_id == 5) && (rf12_len == sizeof(Powermeter))) {
			printPower();
		} else if ((rf12_node_id == 6)  && (rf12_len == sizeof(Thermometer))) {
			printTemp();
		} else { // unknown node ID
			printUnknown();
		}
		
		// this must happen AFTER all processing of rf12_data, etc.
		if (RF12_WANTS_ACK) {
			rf12_sendStart(RF12_ACK_REPLY, 0, 0);
		}
	}
}


void transmitSequence(bool* sequence, byte seqLen, byte numRepeat) {
	for(byte n=0; n<numRepeat; n++) {
		for(byte i=0; i<seqLen; i++) {
			transmitBit(sequence[i]);
		}
		int start = millis();
		receiveRF12();
		int end = millis();
		if ((end - start) > 0) {
			Serial.print("spent ");
			Serial.print(end - start);
			Serial.println("ms processing RF12 Packets");
		}
		delay(REPEAT_PAUSE_MS - (end - start));
	}
}

void printSocketState() {
	int group, id;
	Serial.print("MSG ID: 0x");
	Serial.print(msg_id, HEX);
	Serial.print(": {\"socket_state\": [ ");
	for (id = 0 ; id < SOCKET_IDS ; id++) {
		Serial.print("{");
		for (group = 0 ; group < SOCKET_GROUPS ; group++) {
			bool onoff = socketState[id][group];
			Serial.print("\"group\": \"");
			Serial.print(group +1);
			Serial.print("\", \"id\": \"");
			Serial.print(id +1);
			Serial.print("\", \"state\": \"");
			Serial.print(onoff);
			Serial.print("\"");
		}
		Serial.print("}");
		if (id < SOCKET_IDS - 1){
			Serial.print(", ");
		}
	}
	Serial.println(" ] };");
}

//void transmitOOK() {
void transmitOOK(int group, int id, int onoff) {

	//int group, id;
	//for (id = 0 ; id < SOCKET_IDS ; id++) {
		//for (group = 0 ; group < SOCKET_GROUPS ; group++) {
			//bool onoff = socketState[id][group];
			Serial.print("switching socket: ");
			Serial.print("Group: ");
			Serial.print(group +1);
			Serial.print(", id: ");
			Serial.print(id +1);
			Serial.print(", onoff: ");
			Serial.print(onoff);
			Serial.println();

			transmitSequence(sequences[id * 2 + onoff], 26, NUM_REPEATS);
		//}
	//}
}

void transmitIR(int proto, int length, uint32_t code1, uint32_t code2) {
					//unsigned long long code =  ((unsigned long long)code1 << 32) | (unsigned long long)code2;
					uint64_t code =  ((uint64_t)0xaabbccdd << 32) | (uint64_t)0x11223344;
	Serial.print("MSG ID: 0x");
	Serial.print(msg_id, HEX);
	Serial.print(": {transmitting IR:");
	Serial.print(" proto: ");
	Serial.print(proto);
	Serial.print(", length: ");
	Serial.print(length);
	Serial.print(", code: ");
	//code = 0xaabbccddeeff1122LL;
	//uint32_t code1 = code >> 32;
	//uint32_t code2 = code;
	Serial.print((uint32_t)code >> 32, HEX);
	Serial.print((uint32_t)code, HEX);
	Serial.println("}");
/*
	Serial.print(code >> 32, HEX);
	Serial.print(code, HEX);
	uint32_t code1 = (uint64_t)code >> 32;
	uint32_t code2 = (uint64_t)code;
	Serial.print(code1, HEX);
	Serial.print(code2, HEX);
	Serial.println();
*/
/*
						switch (irProtocol) {
							case "rc5" //RC5
									for (int j = 0; j < 3; j++) {
										irsend.sendRC5(code, 20);
										delay(500);
									}
								break;
							case 0x02: // RC6
									for (int j = 0; j < 3; j++) {
										irsend.sendRC6(0x10011, 20);
*/
}


void readSerialString () {

	int socketGroup;
	int socketId;
	int socketOnOff;

	int irProtocol;
	int irLength;
	uint32_t irCode[2];

	while (Serial.available() > 0) {
		if(strIdx < 255) {
			inChar = Serial.read();
			if ((inChar == '\n') || (inChar == '\r')) {

				// Parse input
				if (sscanf_P(serialString, PSTR("MSG ID: 0x%04x: command=socket.%d.%d.%d"), &msg_id, &socketGroup, &socketId, &socketOnOff) == 4) {
					socketState[socketId - 1][socketGroup - 1] = socketOnOff;
					printSocketState();
					transmitOOK(socketGroup - 1, socketId - 1, socketOnOff);
					//transmitOOK();
				} else if (sscanf_P(serialString, PSTR("MSG ID: 0x%04x: command=socketstate"), &msg_id) == 1) {
					printSocketState();
				} else if (sscanf_P(serialString, PSTR("MSG ID: 0x%04x: command=ir.%d.%d.%lx.%lx"), &msg_id, &irProtocol, &irLength, &irCode[0], &irCode[1]) == 5) {
					transmitIR(irProtocol, irLength, irCode[0], irCode[1]);
				} else {
					Serial.print("Unrecognized String (length: ");
					Serial.print(strIdx);
					Serial.print("): ");
					Serial.println(serialString);
				}

				memset(&serialString[0], 0, sizeof(serialString));
				strIdx = 0;
			} else {
				serialString[strIdx++] = inChar;
			}
		} else {
			memset(&serialString[0], 0, sizeof(serialString));
			strIdx = 0;
		}
	}
}
void setup() {                
	Serial.begin(57600);
	Serial.println("[Alis Home Automation Network - Master Node]");
	byte cryptArray[16];
	eeprom_read_block(cryptArray, RF12_EEPROM_EKEY, 16);
	Serial.print("key:");
	for(int i=0; i<16; i++) {
		Serial.print(cryptArray[i]);
		Serial.print(","); 
	}
	Serial.println();

	pinMode(TRANSMITTER_PIN, OUTPUT);
	rf12_initialize(NODE_ID, RF12_868MHZ, NETGROUP);
	rf12_encrypt(RF12_EEPROM_EKEY);
}

void loop() {
	readSerialString();
	receiveRF12();
}
