#include <xc.h>
#include <pic18f46k80.h>
#include "../PIC18F46K80_Drivers/Config_Word/src/configword.h"
#include "../PIC18F46K80_Drivers/UART/src/UART.h"
#include "../PIC18F46K80_Drivers/CAN/src/CAN.h"


unsigned char data1[8], data2[8];
int main()
{

uartInitialisation();

unsigned char incomingByte, plusCounter, incData[12], incDataCounter, endCounter, defaultSID = 1;

enum caseCounter_s {getData, ending1, ending2, done} caseCounter = done;



while(1) {
    
    if (uartRecieveByte(&incomingByte)) {
         
        //uartTransmitByte(incomingByte);
        
        if (incomingByte == '+') {
            plusCounter = plusCounter + 1;
            
            //if (plusCounter == 3 && ATIntroTimer == VALID) {
              //  enterATMode();
                //plusCounter = 0;
        //    }
        }
        
        else {
           // resetATIntroTimer();
            
            if ((incomingByte == '{') && (caseCounter != getData)) {
                caseCounter = getData;
                incDataCounter = 0;
                uartTransmitByte('A');
            }
            
            else if ((caseCounter != done)) {
                incData[incDataCounter] = incomingByte;
                incDataCounter = incDataCounter + 1;
                                uartTransmitByte('B');

            }
            
            if ((incomingByte == ')') && (caseCounter != done)) {
                caseCounter = ending1;
                                uartTransmitByte('C');

            }
            
            if ((incomingByte == '}') && (caseCounter == ending1)) {
                caseCounter = ending2;
                                uartTransmitByte('D');

            }
            else {
                caseCounter == getData;
            }
            
            if ((incomingByte == '\r') && (caseCounter == ending2)) {
                caseCounter == done;
                                uartTransmitByte('E');

                
                if(incData[0] == incDataCounter - 3 + '0') {
                   //CAN_Transmit(&incData[1], defaultSID, incData[0]);
                   }
                
                else if(incData[0] == incDataCounter - 4  + '0') {
                    //CAN_Transmit(&incData[1], incData[incDataCounter + 1], incData[0]);
                    }
                
                else if(incData[0] == incDataCounter - 5 + '0') {
                    
                    unsigned int tempID;
                    
                    tempID = incData[incDataCounter - 3] * 256 & incData[incDataCounter - 2 + '0'];
                    
                    //CAN_Transmit(&incData[1], tempID, incData[0]);
                    
                }
                                
                else {
                    
                }
                
            }
            
            else {
                caseCounter == getData;
            }
            
        }
        
    }
}
}

