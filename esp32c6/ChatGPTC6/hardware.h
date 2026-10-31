#pragma once
/* CG50 Rev E, factory ESP32-C6-MINI-1-H4. USB programming uses UART0/CH340C.
 * Casio uses a separate, gated, open-drain UART1 transmitter. No sleep here. */
#if !defined(CONFIG_IDF_TARGET_ESP32C6) || !CONFIG_IDF_TARGET_ESP32C6
#error "Select ESP32C6 Dev Module, 4MB flash. This firmware is for the Rev E PCB."
#endif
#include <driver/uart.h>
#include "tx_lease.h"
static const int RX_PIN=20,TX_PIN=21,TX_ARM_PIN=2,HEARTBEAT_PIN=1;
static bool boardUartReady=false;
static TxLease boardLease;
static void boardInitPins(){
 digitalWrite(TX_ARM_PIN,LOW);pinMode(TX_ARM_PIN,OUTPUT);
 digitalWrite(HEARTBEAT_PIN,LOW);pinMode(HEARTBEAT_PIN,OUTPUT);
 pinMode(TX_PIN,INPUT);
}
static void boardPulse(){
 digitalWrite(TX_ARM_PIN,HIGH);digitalWrite(HEARTBEAT_PIN,HIGH);
 delayMicroseconds(5);digitalWrite(HEARTBEAT_PIN,LOW);boardLease.pulsed(millis());
}
static void boardServiceLease(){
 bool pulse=boardLease.due(millis());
 if(!boardUartReady||!boardLease.active){digitalWrite(TX_ARM_PIN,LOW);digitalWrite(HEARTBEAT_PIN,LOW);return;}
 if(pulse)boardPulse();
}
static bool boardAcceptFrame(const WFrame &frame){
 if(!boardUartReady||!boardLease.accept(frame.type,frame.len,millis()))return false;
 boardPulse();return true;
}
