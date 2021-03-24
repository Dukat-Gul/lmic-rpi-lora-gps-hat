/*
 * Copyright (c) 2014-2016 IBM Corporation.
 * All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of the <organization> nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "lmic.h"
#include "debug.h"

//////////////////////////////////////////////////
// CONFIGURATION (FOR APPLICATION CALLBACKS BELOW)
//////////////////////////////////////////////////

// application router ID (LSBF)
static const u1_t APPEUI[8]  = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x03, 0xFE, 0x74 };

// unique device ID (LSBF)
static const u1_t DEVEUI[8]  = { 0x00, 0x60, 0x59, 0xE0, 0xEF, 0xF9, 0x49, 0xA3 };

// device-specific AES key (derived from device EUI)
static const u1_t DEVKEY[16] = { 0x0C, 0x00, 0xF9, 0x0B, 0x50, 0x16, 0x07, 0xE0, 0xCB, 0x00, 0x90, 0x5C, 0x00, 0x10, 0xEE, 0x02 };


//////////////////////////////////////////////////
// APPLICATION CALLBACKS
//////////////////////////////////////////////////

// provide application router ID (8 bytes, LSBF)
void os_getArtEui (u1_t* buf) {
    memcpy(buf, APPEUI, 8);
}

// provide device ID (8 bytes, LSBF)
void os_getDevEui (u1_t* buf) {
    memcpy(buf, DEVEUI, 8);
}

// provide device key (16 bytes)
void os_getDevKey (u1_t* buf) {
    memcpy(buf, DEVKEY, 16);
}


//////////////////////////////////////////////////
// MAIN - INITIALIZATION AND STARTUP
//////////////////////////////////////////////////

// initial job
static void initfunc (osjob_t* j) {
    // reset MAC state
    LMIC_reset();
    // start joining
    LMIC_startJoining();
    // init done - onEvent() callback will be invoked...
}


// application entry point
int main () {
    osjob_t initjob;

    // initialize runtime env
    os_init();
    // initialize debug library
    debug_init();
    // setup initial job
    os_setCallback(&initjob, initfunc);
    // execute scheduled jobs and events
    os_runloop();
    // (not reached)
    return 0;
}


//////////////////////////////////////////////////
// LMIC EVENT CALLBACK
//////////////////////////////////////////////////

void onEvent (ev_t ev) {
    debug_event(ev);

    switch(ev) {
   
      // network joined, session established
      case EV_JOINED:
          // enable pinging mode, start scanning...
          // (set local ping interval configuration to 2^1 == 2 sec)
          LMIC_setPingable(1);
          debug_str("SCANNING...\r\n");
          break;

      // beacon found by scanning
      case EV_BEACON_FOUND:
          // send empty frame up to notify server of ping mode and interval!
          LMIC_sendAlive();
          break;

      // data frame received in ping slot
      case EV_RXCOMPLETE:
          // log frame data
          debug_buf(LMIC.frame+LMIC.dataBeg, LMIC.dataLen);
          if(LMIC.dataLen == 1) {
              // set LED state if exactly one byte is received
              debug_led(LMIC.frame[LMIC.dataBeg] & 0x01);
          }
          break;    
    }
}
