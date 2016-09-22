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
    
                if (RXB0CONbits.RXFUL == 1) {
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

 RCSTA2bits.SPEN = 1;
  
  //Setting TRIS to output
  TRISBbits.TRISB6 = 0;
  TRISBbits.TRISB7 = 0;

  //Setting Asynchronous
  TXSTA2 = 0b00100000;
  RCSTA2 = 0b10010000;
  BAUDCON2 = 0b00001000;
  
  //Setting Baudrate for 9600 @ Fosc = 8 MHz
  //SPBRGH1 = 0;
  //SPBRG1 = 51;
  
 //Setting Baudrate for 9600 @ Fosc = 16 MHz
  SPBRGH2 = 0;
  SPBRG2 = 103;
  

        
T0CON = 0b10000101;
INTCON = 0b11100000;
RCONbits.IPEN = 1;

TRISBbits.TRISB2 = 1;
TRISBbits.TRISB3 = 1;

/*CAN needs to be in CONFIG mode to edit the following settings.
    * We do that by editing CANCONbits.REQOP (Bits 7-5).*/
    CANCON = 0b10000000;

    /*To ensure we don't proceed before we have changed to CONFIG mode, we check
    * CANSTATbits.OPMODE and wait for it to change to 0b100*/
    while(!(CANSTATbits.OPMODE == 0b100));


    /* Set the module to mode 1, enhanced legacy mode.*/
    ECANCON = 0b01000000;

    // Initialize CAN Timing
    //  Setting Baud Rate
    //  1 MBps Kbps @ 16MHz

    //BRGCON1

    BRGCON1bits.SJW = 0b00; // Sync Jump Width is 1 TQ
    BRGCON1bits.BRP = 0; // Baud Rate Prescaler is Tq = 2*(BRP + 1)/Fosc = 0.125 us


    //BRGCON2
    BRGCON2bits.SEG2PHTS = 1; // Makes the SEG2 fully programmable
    BRGCON2bits.SAM = 1; // On reception the bit is sampled once at the intersection of SEG1 and SEG2
    BRGCON2bits.SEG1PH = 0b011; // SEG1 is 4*TQ
    BRGCON2bits.PRSEG = 0b000; // Propogation time is 1*TQ

    //BREGCON3
    BRGCON3bits.WAKDIS = 0; //Enable wakeup from CAN
    BRGCON3bits.WAKFIL = 0; //Disable filters for wakeup
    BRGCON3bits.SEG2PH = 0b001; //SEG2 is 2*TQ

    // Thus the timeline for the CAN bit is:
    //  1           1     4       2   = 8 TQ = 1 MBits/s
    // SyncSeg  PropSeg SEG1    SEG2


    BSEL0 = 0b00000000;                   //1=TX, 0=RX


    /* Initialize Receive Masks
    The first mask is used that accepts all SIDs */
    RXM0SIDH = 0b00000000; // Standard ID MASK all 0s, meaning the filters are irrelevant!
    RXM0SIDL = 0b00000000;

    // Enable Filters
    // Only using first three filters
    RXFCON0 = 0b00000001;          //Enable Filter 0
    RXFCON1 = 0b00000000;          //Disable all others

    // Assign Filters to Masks
    MSEL0 = 0b11111100;            //Assign Filters 0 to Mask 0
    MSEL1 = 0b11111111;            //Assign Filters 4-7 No mask
    MSEL2 = 0b11111111;            //Assign Filters 8-11 to No mask
    MSEL3 = 0b11111111;            //Assign Filters 12-15 to No mask

    // Initialize Receive Filters
    /* In this case, we are only passing messages with ID=1
    The SID has been split between  two registers,
    bits SID<10:3> lie in RXFnSIDH and SID<2:0> are in MSBs of RXFnSIDL.
    Thus the whole SID  is 0000000000100000.*/

    RXF0SIDH = 0b00000000; // Set to ID1
    RXF0SIDL = 0b01100000;

    // Assign Filters to Buffers
    RXFBCON0 = 0b11110000;         //Assign Filter 0 to RXB0
    RXFBCON1 = 0b11111111;         //Assign the rest of the buffers with no filter
    RXFBCON2 = 0b11111111;
    RXFBCON3 = 0b11111111;
    RXFBCON4 = 0b11111111;
    RXFBCON5 = 0b11111111;
    RXFBCON6 = 0b11111111;
    RXFBCON7 = 0b11111111;

    //Setting up the recieve buffer.
    RXB0CON = 0b00000000;

    // Now we can set CAN back to normal mode and this allows it to actually use it.
    CANCON = 0b0000000;

    //Another while loop to check CANSTATbits.OPMODE,
    //and wait for it to change to 0b000 before continuing.
    while(!(CANSTATbits.OPMODE==0b000));
    
       PIE5bits.RXB0IE = 0;     // reset receive buffer 0 interrupt flag

unsigned char i=0;

    while(1){
        
        if(1 == 1) {
            
            unsigned int i;
            
            for(i=0; i < 500; i++);
            
                       // uartTransmitByte('.');



            }
        }
    }
