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
#include <avr/wdt.h>
#include <WProgram.h>

#define NODE_ID 1
#define NETGROUP 42

#define SOCKET_GROUPS 2
#define SOCKET_IDS 8

#define TRANSMITTER_PIN 9

#define NUM_COUNTERS 3

IRsend irsend;

MilliTimer sendTimer;
MilliTimer powerTimer;

bool pending;
uint32_t doorbell_seqnum;
uint32_t last_smartmeter_seq;

typedef struct {
	uint32_t counter_millis;
	uint32_t active_counter;
} smartmeter_pulse_t;
smartmeter_pulse_t smartmeter_pulse;

uint32_t smartmeter_counter[NUM_COUNTERS + 2];

typedef struct {
	int pir;
	int tC;
	int t_awake;
} thermometer_t;
thermometer_t thermometer;

typedef struct {
	byte start;
	byte later;
	byte buzzer;
	byte ringer;
	int voltage;
} doorbell_t;
doorbell_t doorbell;

typedef struct {
 int dummy;
 int group;
 int id;
 int state;
} rf12socket_t;
rf12socket_t rf12socket;
rf12socket_t rf12socket_answer;

typedef struct {
	int switch_id;
	int key_id;
} rf12switch_t;
rf12switch_t rf12switch;


byte buzzer;

char serialString[256];
char inChar = -1;
int strIdx = 0;
uint32_t msg_id;



void printHeader() {
	int rf12_node_id;
	rf12_node_id = 0x1F & rf12_hdr;
	Serial.print("Node ID: ");
	Serial.print(rf12_node_id, DEC);
	Serial.print(", seq: ");
	Serial.print(rf12_seq, DEC);
	Serial.print(", data =");
}

uint32_t last_smartmeter_millis = 0;
void printSmartmeterPulse() {
	smartmeter_pulse = *(smartmeter_pulse_t*) rf12_data;
	int seq_diff;
    int milli_diff;

	if (last_smartmeter_seq) {
		seq_diff = rf12_seq - last_smartmeter_seq;
		if (seq_diff > 1) {
			Serial.print("ALARM: Smartmeter LOST ");
			Serial.print(seq_diff - 1, DEC);
			Serial.print(" PULSE(S)!");
			Serial.println();
			smartmeter_counter[NUM_COUNTERS] += seq_diff - 1;
		} else if (seq_diff < 0) {
			Serial.print("ALARM: Counter overflow or controller reset - seq_diff = ");
            Serial.println(seq_diff);
		}
	}
	last_smartmeter_seq = rf12_seq;
    milli_diff = smartmeter_pulse.counter_millis - last_smartmeter_millis;
    last_smartmeter_millis = smartmeter_pulse.counter_millis;

    printHeader();
	Serial.print(" Smartmeter counted pulse on meter: ");
	Serial.print(smartmeter_pulse.active_counter + 1, DEC);
    Serial.print(", millis: ");
	Serial.print(smartmeter_pulse.counter_millis);
    Serial.print(", diff: ");
    Serial.println(milli_diff);

	smartmeter_counter[smartmeter_pulse.active_counter]++;
	smartmeter_counter[NUM_COUNTERS + 1]++;
}


void printSmartmeterAggregated() {
	Serial.print("Smartmeter aggregated:");
	for (int i = 0; i < NUM_COUNTERS + 2; i++) {
		Serial.print(' ');
		Serial.print(smartmeter_counter[i], DEC);
		smartmeter_counter[i] = 0;
	}
	Serial.println();
}

void printTemp() {
 int tC, tFrac, tCR, tFracR;
	thermometer = *(thermometer_t*) rf12_data;
 tC = tCR = thermometer.tC;                             // read high-resolution temperature

	printHeader();
	Serial.print(' ');

 if (tC < 0) {
		tC = -tC;                                   // fix for integer division
		Serial.print("-");                          // indicate negative
 }

 tFrac = tFracR = tC % 100;                             // extract fractional part
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

	Serial.print("REST PUT: URL{http://yavdr:8081/rest/items/Temperature_FF_Office/state}, DATA{");
 if (tCR < 0) {
		tCR = -tCR;                                   // fix for integer division
		Serial.print("-");                          // indicate negative
 }

 tFracR = tCR % 100;                             // extract fractional part
 tCR /= 100;                                    // extract whole part

 Serial.print(tCR);
 Serial.print(".");
 if (tFracR < 10)
		Serial.print("0");
 Serial.print(tFracR);
	Serial.println("}");
}

void printDoorbell() {
	doorbell = *(doorbell_t*) rf12_data;

	int milliVolts;
	milliVolts = (doorbell.voltage * (330000/511)) / 100;

/*
	printHeader();
	Serial.print(" #");
	Serial.print(doorbell_seqnum);
	Serial.print(" start: ");
	Serial.print(doorbell.start, DEC);
	Serial.print(" recvd: ");
	Serial.print(doorbell.later, DEC);
*/
	Serial.print("Doorbell battery voltage: ");
	Serial.print(doorbell.voltage / (5120/33), DEC);
	Serial.print(".");
	Serial.print(doorbell.voltage % (5120/33), DEC);
	Serial.println("V");

	if (milliVolts < 4000) {
		Serial.print("WARNING: DOORBELL BATTERY LOW: ");
		Serial.print(doorbell.voltage / (5120/33), DEC);
		Serial.print(".");
		Serial.print(doorbell.voltage % (5120/33), DEC);
		Serial.println("V");
	}

	if (doorbell.ringer == 1) {
		printHeader();
		Serial.println(" DOORBELL!");
		Serial.println("REST PUT: URL{http://yavdr:8081/rest/items/doorbell/state}, DATA{ON}");
		doorbell.ringer == 0;
	}
}

void printSocket() {
	rf12socket_answer = *(rf12socket_t*) rf12_data;
	Serial.print("MSG ID: 0x");
	Serial.print(msg_id, HEX);
	Serial.print(": ");
	printHeader();
	Serial.print(" Socket State: Group: ");
	Serial.print(rf12socket_answer.group);
	Serial.print(", ID: ");
	Serial.print(rf12socket_answer.id);
	Serial.print(", State: ");
	Serial.println(rf12socket_answer.state);

	Serial.print("REST PUT: URL{http://yavdr:8081/rest/items/socket_");
	Serial.print(rf12socket_answer.group);
	Serial.print("_");
	Serial.print(rf12socket_answer.id);
	Serial.print("/state}, DATA{");
	if (rf12socket_answer.state) {
		Serial.println("ON}");
	} else {
		Serial.println("OFF}");
	}
}

void printSocketAck() {
	rf12socket_answer = *(rf12socket_t*) rf12_data;
	Serial.print("MSG ID: 0x");
	Serial.print(msg_id, HEX);
	Serial.print(": {\"socket_state\": [ ");
	Serial.print("\"group\": \"");
	Serial.print(rf12socket_answer.group);
	Serial.print("\", \"id\": \"");
Serial.print(rf12socket_answer.id);
	Serial.print("\", \"state\": \"");
	Serial.print(rf12socket_answer.state);
	Serial.print("\"");
	Serial.println(" ] };");
}

void printSwitch() {
	rf12switch = *(rf12switch_t*) rf12_data;
	printHeader();
	Serial.print("Switch ID: ");
	Serial.print(rf12switch.switch_id);
	Serial.print(", Key ID: ");
	Serial.println(rf12switch.key_id);

	Serial.print("REST PUT: URL{http://yavdr:8081/rest/items/switch_");
	Serial.print(rf12switch.switch_id);
	Serial.print("/state}, DATA{");
	Serial.print(rf12switch.key_id);
	Serial.println("}");
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
	Serial.print(rf12_seq, DEC);
	Serial.print(", length: ");
	Serial.println(rf12_len, DEC);
}

void receiveRF12() {
	int rf12_node_id;
	if (rf12_recvDone() && (rf12_crc == 0)) {
		rf12_node_id = 0x1F & rf12_hdr;
		if ((rf12_node_id == 5) && (rf12_len == sizeof(smartmeter_pulse_t))) {
			printSmartmeterPulse();
		} else if ((rf12_node_id == 6)  && (rf12_len == sizeof(thermometer_t))) {
			printTemp();
		} else if ((rf12_node_id == 7) && (rf12_len == sizeof(doorbell_t))) {
			sendTimer.set(0);
			printDoorbell();
		} else if ((rf12_node_id == 8) && (rf12_len == sizeof(rf12socket_t))) {
			printSocket();
		} else if ((rf12_node_id == 1) && (rf12_len == sizeof(rf12socket_t))) {
			printSocket();
		} else if ((rf12_node_id == 3) && (rf12_len == sizeof(rf12switch_t))) {
			printSwitch();
		} else { // unknown node ID
			printUnknown();
		}
		
		// this must happen AFTER all processing of rf12_data, etc.
		if (RF12_WANTS_ACK) {
			rf12_sendStart(RF12_ACK_REPLY, 0, 0);
		}
	}

 // timedSend: 
	if (sendTimer.poll(2096))
		pending = 1;

	if (pending && rf12_canSend()) {
		pending = 0;
		//Serial.println("Polling Doorbell");
		doorbell.buzzer = buzzer;
		rf12_sendStart(7, &doorbell, sizeof(doorbell_t));
		buzzer = 0;
		++doorbell_seqnum;
	}
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

static void invalidCommand (char *buf) {
	Serial.print("MSG ID: 0x");
	Serial.print(msg_id, HEX);
	Serial.print(": ");
	printHeader();
	Serial.println("INVALID COMMAND");
}


static void pollRF12socket (int group, int id) {
	memset(&rf12socket_answer, 0, sizeof rf12socket_answer);
	Serial.print("Polling RF12 Socket: Group: ");
	Serial.print(group);
	Serial.print(", ID: ");
	Serial.println(id);

	rf12socket.dummy = 'S';
	rf12socket.group = group;
	rf12socket.id = id;
	rf12socket.state = '?';
	receiveRF12();
	rf12_sendStart(RF12_HDR_ACK, &rf12socket, sizeof(rf12socket));
}

static void switchRF12socket (int group, int id, int state) {
	memset(&rf12socket_answer, 0, sizeof rf12socket_answer);
	Serial.print("Switching RF12 Socket: Group: ");
	Serial.print(group);
	Serial.print(", ID: ");
	Serial.print(id);
	Serial.print(", State: ");
	Serial.println(state);

	rf12socket.dummy = 'S';
	rf12socket.group = group;
	rf12socket.id = id;
	rf12socket.state = state;
	int max_retries = 10;
	bool rf12socket_ack_pending = 1;
	while (rf12socket_ack_pending == 1) {
		Serial.println(max_retries);
		if (max_retries-- > 0) {
			while (!rf12_canSend()) {
				receiveRF12();
			}
			rf12_sendStart(RF12_HDR_ACK, &rf12socket, sizeof(rf12socket));
			rf12_sendWait(1);
			Serial.println("!");
			for (int i=0; i <= 5000; i++) {
				receiveRF12();
			}
			if ((rf12socket_answer.group == group) && (rf12socket_answer.id == id) && (rf12socket_answer.state == state)) {
				Serial.println("SUCCESS!");
				rf12socket_ack_pending = 0;
			}
			Serial.println(".");
		} else {
			Serial.println("FAILED!");
			rf12socket_ack_pending = 0;
		}
	}
/*
*/
}

void readSerialString () {

	int dummy;
	int socketGroup;
	int socketId;
	char socketCmd[32];

	int irProtocol;
	int irLength;
	uint32_t irCode[2];

	while (Serial.available() > 0) {
		if(strIdx < 255) {
			inChar = Serial.read();
			if ((inChar == '\n') || (inChar == '\r')) {

				// Parse input
				if (sscanf_P(serialString, PSTR("MSG ID: 0x%04x: /socket/%d/%d/%s"), &msg_id, &socketGroup, &socketId, &socketCmd) == 4) {
					if (0 == strncmp(socketCmd, "state", 6)) {
						pollRF12socket(socketGroup, socketId);
					} else if (0 == strncmp(socketCmd, "OFF", 3)) {
						switchRF12socket(socketGroup, socketId, 0);
					} else if (0 == strncmp(socketCmd, "ON", 2)) {
						switchRF12socket(socketGroup, socketId, 1);
					} else {
						invalidCommand(serialString);
					}
				} else if (sscanf_P(serialString, PSTR("MSG ID: 0x%04x: command=ir.%d.%d.%lx.%lx"), &msg_id, &irProtocol, &irLength, &irCode[0], &irCode[1]) == 5) {
					transmitIR(irProtocol, irLength, irCode[0], irCode[1]);
				} else if (sscanf_P(serialString, PSTR("MSG ID: 0x%04x: command=buzzer.%d"), &msg_id, dummy) == 2) {
					Serial.println("Setting buzzer to 1");
					buzzer = 1;
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
	//init_ook_values();
	wdt_reset();
	wdt_enable(WDTO_2S);

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
	rf12_easyInit(0);
}

void loop() {
	if(powerTimer.poll(60000))
    printSmartmeterAggregated();
	readSerialString();
	receiveRF12();
	wdt_reset();
}




// vim: expandtab sw=4 ts=4
