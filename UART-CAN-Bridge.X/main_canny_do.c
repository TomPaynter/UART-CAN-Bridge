#include <xc.h>
#include <pic18f46k80.h>
#include "../PIC18F46K80_Drivers/Config_Word/src/configword.h"
#include "../PIC18F46K80_Drivers/UART/src/UART.h"
#include "../PIC18F46K80_Drivers/CAN/src/can.h"

unsigned char incomingByte, plusCounter, incData[16], incDataCounter, endCounter, defaultSID = 1, filter[2], mask[2];
unsigned int canDefaultID, canMask, canFilter;
enum caseCounter_s {getData, ending1, ending2, done} caseCounter = done;

volatile enum ATTimeout_s {ready, standby, wait, inATMODE} ATTimeout = standby;
volatile enum ATStatus_s {plus1, plus2, plus3, ATMODE, normal} ATStatus = normal;
volatile enum ATCommandStatus_s {A, T, plus, mask1, mask2, filter1, filter2, ATstandby} ATCommandStatus = ATstandby;

        unsigned char bob[3] = {'C', 'W', '7'};

        
unsigned char CAN_Data[8], CAN_DLC;

        unsigned int i;

void UART_TransmitPacket(unsigned char data[8], unsigned int SID, unsigned char DLC) {
  int i;

  uartTransmitByte('[');
  uartTransmitByte(DLC);

  for(i=0; i < DLC; i++){
    uartTransmitByte(data[i]);
  }
  if (SID < 256) {
    uartTransmitByte(SID);
  }

  else {
    uartTransmitByte((SID / 256) & 255);
    uartTransmitByte(SID & 255);
  }

  uartTransmitByte(')');
  uartTransmitByte(']');
  uartTransmitByte('\r');

  return;
}

void sendCANMessage(void) {
  if(incDataCounter == (incData[0] + 4)) {
    CAN_Transmit(&incData[1], incData[0], incData[0]);
  }

  else if(incDataCounter == (incData[0] + 5)) {
    CAN_Transmit(&incData[1], incData[incDataCounter - 4], incData[0]);
  }

  else if(incDataCounter == (incData[0] + 6)) {
      unsigned int tempID;
      tempID = incData[incDataCounter - 5] * 256 & incData[incDataCounter - 4];
      CAN_Transmit(&incData[1], tempID, incData[0]);
  }

  UART_TransmitPacket(&incData[1], '?', incData[0]);
  
  return;
}

void runNormalParser(void) {


      if ((incomingByte == '{') && (caseCounter == done)) {
          caseCounter = getData;
          incDataCounter = 0;
      }

      else if (caseCounter != done) {
          incData[incDataCounter] = incomingByte;
          incDataCounter = incDataCounter + 1;

          if (incDataCounter > 15) {
              caseCounter = done;
          }
      }

      if ((incomingByte == ')') && (caseCounter != done)) {
          caseCounter = ending1;
      }

      else if (caseCounter == ending1) {
          if (incomingByte == 125) {
            caseCounter = ending2;
          }

          else {
              caseCounter = getData;
          }
      }

      else if (caseCounter == ending2) {
          if (incomingByte == '\r') {
            caseCounter = done;
            sendCANMessage();
          }

      else {
          caseCounter = getData;
      }
      }
  }


void ATTimeoutReset(void) {
  TMR0L = 0;
  TMR0H = 0;
}

void interrupt low_priority LPISR(void)
{
    ///Timer0
    if(INTCONbits.T0IF && INTCONbits.T0IE) {
        INTCONbits.T0IF = 0;            // Clear the interrupt flag

        if(ATTimeout == standby) {
          ATTimeout = ready;
        }

        if(ATTimeout == wait) {
          ATTimeout = inATMODE;
          ATStatus = ATMODE;

            uartTransmitByte('O');
            uartTransmitByte('K');

          ATTimeoutReset();
        }

        else if(ATTimeout == inATMODE) {
          ATTimeout = standby;
          ATStatus = normal;

        }

    }
}

void enterATMode(void) {

      if((incomingByte == '+') && (ATTimeout == ready)) {
        if(ATStatus == normal) {
          ATStatus = plus1;

        }

        else if(ATStatus == plus1) {
          ATStatus = plus2;
        }

        else if(ATStatus == plus2) {
          ATStatus = plus3;
          ATTimeout = wait;
          ATTimeoutReset();
        }
      }

      else {

        ATTimeoutReset();

      }

}

void runATParser(void) {


    if((incomingByte == 'A') && (ATCommandStatus == ATstandby)) {
        ATCommandStatus = A;
    }

    if((incomingByte == 'T') && (ATCommandStatus == A)) {
        ATCommandStatus = T;
    }

    if((incomingByte == '+') && (ATCommandStatus == T)) {
        ATCommandStatus = plus;
    }

    if((incomingByte == 'M') && (ATCommandStatus == plus)) {
        ATCommandStatus = mask1;
    }

    else if(ATCommandStatus == mask1) {
        mask[0] = incomingByte;
        ATCommandStatus = mask2;
    }
    else if(ATCommandStatus == mask2) {
        mask[1] = incomingByte;
        ATCommandStatus = ATstandby;
    }

    if((incomingByte == 'F') && (ATCommandStatus == plus)) {
        ATCommandStatus = filter1;
    }

    else if(ATCommandStatus == filter1) {
        filter[0] = incomingByte;
        ATCommandStatus = filter2;
    }
    else if(ATCommandStatus == filter2) {
        filter[1] = incomingByte;
        ATCommandStatus = ATstandby;
    }

}

int main() {
OSCCONbits.IRCF = 0b111; //16 MHz Internal Clock

uartInitialisation();
CAN_Initialise();
        
T0CON = 0b10000101;
INTCON = 0b11100000;
RCONbits.IPEN = 1;

    while(1) {

        if (uartRecieveByte(&incomingByte)) {

            enterATMode();

            if(ATStatus == ATMODE) {
                runATParser();
            }

            else {
                runNormalParser();
            }

        }
        
        /*
        
        if(RXB0CONbits.RXFUL) {
            UART_TransmitPacket(CAN_Data, '?', CAN_DLC);
              uartTransmitByte(RXB0D0);
              uartTransmitByte(RXB0D1);
RXB0CONbits.RXFUL=0;
                     uartTransmitByte('.');


        } */

    }
}
