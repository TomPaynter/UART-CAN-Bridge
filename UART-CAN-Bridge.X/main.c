#include <xc.h>
#include <pic18f46k80.h>
#include "../PIC18F46K80_Drivers/Config_Word/src/configword.h"
#include "../PIC18F46K80_Drivers/UART/src/UART.h"
#include "../PIC18F46K80_Drivers/CAN/src/CAN.h"

unsigned char incomingByte, plusCounter, incData[16], incDataCounter,
    endCounter, defaultSID = 1, filter[2], mask[2];

unsigned int canDefaultID, canMask, canFilter, CAN_SID = 0;

volatile enum caseCounter_s {getData, ending1, ending2, done} caseCounter = done;
volatile enum ATTimeout_s {ready, standby, wait, inATMODE} ATTimeout = standby;
volatile enum ATStatus_s {plus1, plus2, plus3, ATMODE, normal} ATStatus = normal;
volatile enum ATCommandStatus_s {A, T, plus, mask1, mask2, filter1, filter2,
    ATstandby} ATCommandStatus = ATstandby;

volatile unsigned char CAN_Data[8], CAN_DLC;

void UART_TransmitPacket(unsigned char data[8], unsigned int SID, unsigned char DLC) {
  int i;

  uart1TransmitByte('[');
  uart1TransmitByte(DLC);

  for(i=0; i < DLC; i++){
    uart1TransmitByte(data[i]);
  }

  if (SID < 256) {
    uart1TransmitByte(SID);
  }
  else {
    uart1TransmitByte((SID / 256) & 255);
    uart1TransmitByte(SID & 255);
  }

  uart1TransmitByte(')');
  uart1TransmitByte(']');
  uart1TransmitByte('\r');

  return;
}


void ATTimeoutReset(void) {
  TMR0L = 0;
  TMR0H = 0;
}


void interrupt low_priority LPISR(void){
    ///Timer0
    if(INTCONbits.T0IF && INTCONbits.T0IE) {
        INTCONbits.T0IF = 0;            // Clear the interrupt flag
            
        if(ATTimeout == standby) {
          ATTimeout = ready;
        }

        if(ATTimeout == wait) {
          ATTimeout = inATMODE;
          ATStatus = ATMODE;

            uart1TransmitByte('O');
            uart1TransmitByte('K');

          ATTimeoutReset();
        }

        else if(ATTimeout == inATMODE) {
          ATTimeout = standby;
          ATStatus = normal;

        }

    }

    //CAN RXB0
    if ((RXB0CONbits.RXFUL == 1) && (PIE5bits.RXB0IE == 1) && (PIR5bits.RXB0IF)) {

        CAN_DLC = RXB0DLCbits.DLC;
        CAN_Data[0] = RXB0D0;
        CAN_Data[1] = RXB0D1;
        CAN_Data[2] = RXB0D2;
        CAN_Data[3] = RXB0D3;
        CAN_Data[4] = RXB0D4;
        CAN_Data[5] = RXB0D5;
        CAN_Data[6] = RXB0D6;
        CAN_Data[7] = RXB0D7;

        CAN_SID = ((RXB0SIDL / 32) & 0b111 | (RXB0SIDH * 8)) & 0b11111111111;

        UART_TransmitPacket(CAN_Data, CAN_SID, CAN_DLC);

        RXB0CONbits.RXFUL = 0;   // Mark the buffer as read, and no longer contains a new message
        PIR5bits.RXB0IF = 0;     // reset receive buffer 0 interrupt flag
        }

}

void sendCANMessage(void) {
    
  if(incDataCounter == (incData[0] + 4)) {
    CAN_Transmit(&incData[1], defaultSID, incData[0]);
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

  uart1Initialisation();

  CAN_Initialise();

  //Setting up Timer0
    T0CON = 0b10000101;
    INTCON = 0b11100000;
    RCONbits.IPEN = 1;

  while(1){

    if (uart1RecieveByte_POLL(&incomingByte)) {
        enterATMode();

          if(ATStatus == ATMODE) {
              runATParser();
          }

          else {
              runNormalParser();
          }

    }
  }
}
