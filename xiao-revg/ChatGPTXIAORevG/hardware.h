#pragma once
/* Rev G XIAO ESP32-C6. Native USB is on the XIAO.
 * UART1 uses a hardware-gated 3V3 push-pull TX. Radio/CPU idle policy keeps UART awake. */
#if !defined(CONFIG_IDF_TARGET_ESP32C6) || !CONFIG_IDF_TARGET_ESP32C6 || !defined(ARDUINO_XIAO_ESP32C6)
#error "Select XIAO_ESP32C6, 4MB flash. This firmware is for Rev G."
#endif
#include <driver/uart.h>
#include <driver/gpio.h>
#include "tx_lease.h"
#include "rx_idle_guard.h"
static const int RX_PIN=17,TX_PIN=16,TX_ARM_PIN=2,HEARTBEAT_PIN=1;
static bool boardUartReady=false;
static TxLease boardLease;
static RxIdleGuard boardRxGuard;
static void boardInitPins(){
 /* digitalWrite is ignored before GPIO ownership in Arduino core 3.3.0.
  * Preload the output latches through IDF before enabling either output. */
 gpio_set_level((gpio_num_t)TX_ARM_PIN,LOW);pinMode(TX_ARM_PIN,OUTPUT);
 gpio_set_level((gpio_num_t)HEARTBEAT_PIN,LOW);pinMode(HEARTBEAT_PIN,OUTPUT);
 pinMode(TX_PIN,INPUT);
}
static void boardPulse(){
 digitalWrite(TX_ARM_PIN,HIGH);digitalWrite(HEARTBEAT_PIN,HIGH);
 delayMicroseconds(5);digitalWrite(HEARTBEAT_PIN,LOW);boardLease.pulsed(millis());
}
static void boardServiceLease(bool rxPending=false){
 if(boardUartReady&&boardLease.active&&boardRxGuard.lost(millis(),gpio_get_level((gpio_num_t)RX_PIN)!=0,rxPending,boardLease.lastFrame))boardLease.active=false;
 bool pulse=boardLease.due(millis());
 if(!boardUartReady||!boardLease.active){digitalWrite(TX_ARM_PIN,LOW);digitalWrite(HEARTBEAT_PIN,LOW);return;}
 if(pulse)boardPulse();
}
static bool boardAcceptFrame(const WFrame &frame){
 bool opening=!boardLease.active||boardLease.expired(millis());
 if(!boardUartReady||!boardLease.accept(frame.type,frame.len,millis()))return false;
 boardPulse();
 /* Let the newly enabled push-pull line settle HIGH before the first start bit. */
 if(opening)delayMicroseconds(200);
 return true;
}
