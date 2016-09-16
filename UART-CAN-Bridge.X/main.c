#include <xc.h>
#include <pic18f46k80.h>
#include "../PIC18F46K80_Drivers/Config_Word/src/configword.h"
#include "../PIC18F46K80_Drivers/UART/src/UART.h"

unsigned char incomingByte, plusCounter, incData[16], incDataCounter, endCounter, defaultSID = 1;

enum caseCounter_s {getData, ending1, ending2, done} caseCounter = done;



void CAN_Transmit(unsigned char data[8], unsigned char SID, unsigned char DLC) {
  int i;

  uartTransmitByte('[');
  uartTransmitByte(DLC);

  for(i=0; i < DLC; i++){
    uartTransmitByte(data[i]);
  }

  uartTransmitByte(SID);
  uartTransmitByte(')');
  uartTransmitByte(']');
  uartTransmitByte('\r');

  return;
}

void sendCANMessage(void) {
  if(incDataCounter == (incData[0] + 4)) {
    CAN_Transmit(&incData[1], 5, incData[0]);
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

unsigned char data1[8], data2[8];

int main() {

uartInitialisation();


while(1) {

    if (uartRecieveByte(&incomingByte)) {

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
}
}
