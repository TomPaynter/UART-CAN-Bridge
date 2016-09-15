# UART-CAN Bridge
Using a PIC18F46K80 as a UART-CAN Bridge to give the PC CAN capabilities

#Concepts of Operation:
- Output or Input CAN data of any ID
- User definable Masks and filters for reception
- Default transmission address or on-th-fly changes
- AT Command control


#AT Command Set

## To Enter AT Command Mode
Similar to X-Bee, must read "+++" with nothing else before, during or after for at least 1 second. It will then return an "OK". Once this has occurred it will enter into the AT Command mode.

## Exiting the AT Command Mode
After recieving no information for 3 seconds it will automatically revert to normal mode.

## AT-Commands

| AT Command        | Description           | Parameters  |
| :-------------: |:-------------:| :-----:|
| **SM** | Set recieve mask | unsigned char |
| **SF** | Set recieve filter | unsigned char |
| **DI** | Set default transmit ID | unsigned char |
| **BF* ** | Set bus frequency | unsigned int |

* = Future Plans
 



