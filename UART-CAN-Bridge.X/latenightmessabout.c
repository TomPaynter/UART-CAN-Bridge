#include <xc.h>
#include <pic18f46k80.h>
#include "../PIC18F46K80_Drivers/Config_Word/src/configword.h"
#include "../PIC18F46K80_Drivers/UART/src/UART.h"
//#include "../PIC18F46K80_Drivers/CAN/src/CAN.h"

unsigned char incomingByte, plusCounter, incData[16], incDataCounter, endCounter, defaultSID = 1, filter[2], mask[2];
unsigned int canDefaultID, canMask, canFilter;
enum caseCounter_s {getData, ending1, ending2, done} caseCounter = done;

volatile enum ATTimeout_s {ready, standby, wait, inATMODE} ATTimeout = standby;
volatile enum ATStatus_s {plus1, plus2, plus3, ATMODE, normal} ATStatus = normal;
volatile enum ATCommandStatus_s {A, T, plus, mask1, mask2, filter1, filter2, ATstandby} ATCommandStatus = ATstandby;

        unsigned char bob[3] = {'C', 'W', '7'}, data1[8], data2[8];

        
unsigned char CAN_Data[8], CAN_DLC;

        unsigned int i;

        
void interrupt low_priority LPISR(void)
{
   /*
     //Timer0
    if(INTCONbits.T0IF && INTCONbits.T0IE && 1 == 0) {
        INTCONbits.T0IF = 0;            // Clear the interrupt flag

       //TXB0CONbits.TXREQ = 0;

   // uartTransmitByte(TXB0CON);
      
  TXB0SIDH = 0;     //Thus the SID is 00110010 110 = 406
  TXB0SIDL = 0b00100000;

  TXB0DLC = 3;                //Thus DLC bytes will be sent! (This is the max)

  /*  This is where the meat of the CAN it, this is it!!!!!
  The TXBnDm register is the actual data you are to be sending over the
  network  
  TXB0D0 = 'C';
  TXB0D1 = 'W';
  TXB0D2 = '8';
  
  /*This bit literally says go for transmit!!*
  TXB0CONbits.TXREQ = 1;
    //  uartTransmitByte(TXB0CON);

              uartTransmitByte('O');
              
  //  uartTransmitByte(TXB0CON);

    int i = 0;
    
    for(i = 0; i <5000; i++);
    uartTransmitByte(TXB0CON);
    
               //   uartTransmitByte('B');
                       //         uartTransmitByte(TXERRCNT);



    } */
    
    if ((RXB0CONbits.RXFUL == 1) && (PIE5bits.RXB0IE == 1) && (PIR5bits.RXB0IF)) {
            uartTransmitByte('0' + RXB0DLCbits.DLC);
            uartTransmitByte(RXB0D0);
            uartTransmitByte(RXB0D1);
            uartTransmitByte(RXB0D2);
                
    RXB0CONbits.RXFUL = 0;   // Mark the buffer as read, and no longer contains a new message
   PIR5bits.RXB0IF = 0;     // reset receive buffer 0 interrupt flag
}
}


int main() {
OSCCONbits.IRCF = 0b111; //16 MHz Internal Clock

uart2Initialisation();
  
CAN_Initialise();

//Setting up Timer0 
T0CON = 0b10000101;
INTCON = 0b11100000;
RCONbits.IPEN = 1;

unsigned char i=0;

    while(1){
        
        if(1 == 1) {
            
            unsigned int i;
            
            for(i=0; i < 500; i++);
            
                       // uartTransmitByte('.');



            }
        }
    }
