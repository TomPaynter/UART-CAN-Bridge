#include <xc.h>
#include <pic18f46k80.h>
#include "../PIC18F46K80_Drivers/Config_Word/src/configword.h"
#include "../PIC18F46K80_Drivers/UART/src/UART.h"

unsigned char incomingByte, plusCounter, incData[16], incDataCounter, endCounter, defaultSID = 1;

enum caseCounter_s {getData, ending1, ending2, done} caseCounter = done;

enum ATTimeout_s {ready, standby, wait, inATMODE} ATTimeout = standby;
enum ATStatus_s {plus1, plus2, plus3, ATMODE, normal} ATStatus = normal;

void CAN_Transmit(unsigned char data[8], unsigned int SID, unsigned char DLC) {
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
                uartTransmitByte('.');

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

        if(ATTimeout == inATMODE) {
          ATTimeout = standby;
        }

    }
}

int main() {

uartInitialisation();

T0CON = 0b10000101;
INTCON = 0b11100000;
RCONbits.IPEN = 1;

while(1) {

    if (uartRecieveByte(&incomingByte)) {
      if((incomingByte == '+') && (ATTimeout == ready)) {
        if(ATStatus == normal) {
          ATStatus = plus1;
                  uartTransmitByte('A');

        }

        else if(ATStatus == plus1) {
          ATStatus = plus2;
                  uartTransmitByte('B');

        }

        else if(ATStatus == plus2) {
          ATStatus = plus3;
          ATStatus = wait;
          ATTimeoutReset();
          uartTransmitByte('C');

        }
      }

      else {
        ATTimeout = standby;
        ATStatus = normal;
        ATTimeoutReset();
                uartTransmitByte('D');

      }

      if(ATStatus == ATMODE) {
        //runATParser();
                uartTransmitByte('e');

      }

      else {
        runNormalParser();

      }

}
}
}
