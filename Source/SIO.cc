/* Base structure and authentic idea PSeudo (Credits: Dennis Koluris) */

#include "Global.h"


#define status \
    *(uw *)&mem.hwr.ptr[0x1044]

#define control \
    *(uh *)&mem.hwr.ptr[0x104a]

// Check for pushed button
#define btnCheck(btn) \
    if (pushed) { \
        btnState &= ( (0xffff ^ (1 << btn))); \
    } \
    else { \
        btnState |= (~(0xffff ^ (1 << btn))); \
    }


CstrSerial sio;

void CstrSerial::reset() {
    status = SIO_STAT_TX_READY | SIO_STAT_TX_EMPTY;
    index  = 0;
    step   = 0;
    btnState = 0xffff;
    
    // Default pad buffer
    bfr[0] = 0x00;
    bfr[1] = 0x41;
    bfr[2] = 0x5a;
    bfr[3] = 0xff;
    bfr[4] = 0xff;
}

void CstrSerial::padListener(int code, bool pushed) {
#ifdef APPLE_MACOS
    if (code ==  19) { // Select
        btnCheck(PAD_BTN_SELECT);
    }
    
    if (code ==  18) { // Start
        btnCheck(PAD_BTN_START);
    }
    
    if (code == 126) { // Up
        btnCheck(PAD_BTN_UP);
    }
    
    if (code == 124) { // R
        btnCheck(PAD_BTN_RIGHT);
    }
    
    if (code == 125) { // Down
        btnCheck(PAD_BTN_DOWN);
    }
    
    if (code == 123) { // Left
        btnCheck(PAD_BTN_LEFT);
    }
    
    if (code ==   7) { // X
        btnCheck(PAD_BTN_CROSS);
    }
    
    if (code ==   6) { // Z
        btnCheck(PAD_BTN_CIRCLE);
    }
#elif defined(DREAMCAST) && defined(__KOS__)
    // Map Dreamcast controller (KOS CONT_* bitmasks) to PS1 buttons.
    // padListener is called once per changed bit, so code holds a single bit.
    if (code & CONT_START)       { btnCheck(PAD_BTN_START);    }
    if (code & CONT_A)           { btnCheck(PAD_BTN_CROSS);    }
    if (code & CONT_B)           { btnCheck(PAD_BTN_CIRCLE);   }
    if (code & CONT_X)           { btnCheck(PAD_BTN_SQUARE);   }
    if (code & CONT_Y)           { btnCheck(PAD_BTN_TRIANGLE); }
    if (code & CONT_DPAD_UP)     { btnCheck(PAD_BTN_UP);       }
    if (code & CONT_DPAD_DOWN)   { btnCheck(PAD_BTN_DOWN);     }
    if (code & CONT_DPAD_LEFT)   { btnCheck(PAD_BTN_LEFT);     }
    if (code & CONT_DPAD_RIGHT)  { btnCheck(PAD_BTN_RIGHT);    }
    if (code & CONT_Z)           { btnCheck(PAD_BTN_L2);       }
    if (code & CONT_C)           { btnCheck(PAD_BTN_R2);       }
#endif
}

void CstrSerial::write16(uw addr, uh data) {
    switch(LOW_BITS(addr)) {
        case 0x104a:
            control = data & (~(SIO_CTRL_RESET_ERROR));
            
            if (control & SIO_CTRL_RESET || !control) {
                status  = SIO_STAT_TX_READY | SIO_STAT_TX_EMPTY;
                
                index = 0;
                step  = 0;
            }
            return;
    }
    
    accessMem(mem.hwr, uh) = data;
}

void CstrSerial::write08(uw addr, ub data) {
    switch(LOW_BITS(addr)) {
        case 0x1040:
            switch(step) {
                case 1:
                    if (data & 0x40) {
                        index = 1;
                        step  = 2;
                        
                        if (data  == 0x42) {
                            bfr[1] = 0x41;
                        }
                        else
                        if (data  == 0x43) {
                            bfr[1] = 0x43;
                        }
                        else {
                            printx("/// PSeudo SIO: Data == 0x%x", data);
                        }
                    }
                    else {
                        step = 0;
                    }
                    
                    bus.interruptSet(CstrBus::INT_SIO0);
                    return;
                    
                case 2:
                    if (++index == sizeof(bfr) - 1) {
                        step = 0;
                        return;
                    }
                    
                    bus.interruptSet(CstrBus::INT_SIO0);
                    return;
            }
            
            if (data == 1) {
                status &= (~(SIO_STAT_TX_EMPTY));
                status |= ( (SIO_STAT_RX_READY));
                
                index = 0;
                step  = 1;
                
                if (control & SIO_CTRL_DTR) {
                    if (control & 0x2000) { // Controller 2
                        bfr[3] = 0xff;
                        bfr[4] = 0xff;
                    } else { // Controller 1
                        bfr[3] = (ub)(btnState);
                        bfr[4] = (ub)(btnState >> 8);
                    }
                    bus.interruptSet(CstrBus::INT_SIO0);
                }
            }
            return;
    }
    
    accessMem(mem.hwr, ub) = data;
}

uh CstrSerial::read16(uw addr) {
    return accessMem(mem.hwr, uh);
}

ub CstrSerial::read08(uw addr) {
    switch(LOW_BITS(addr)) {
        case 0x1040:
            if (!(status & SIO_STAT_RX_READY)) {
                return 0;
            }
            
            if (index == sizeof(bfr) - 1) {
                status &= (~(SIO_STAT_RX_READY));
                status |= ( (SIO_STAT_TX_EMPTY));
                
                if (step == 2) {
                    step  = 0;
                }
            }
            return bfr[index];
    }
    
    return accessMem(mem.hwr, ub);
}
