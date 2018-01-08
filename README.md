# UART-CAN Bridge
Using a PIC18F46K80 as a UART-CAN Bridge to give the PC CAN capabilities

#Concepts of Operation:
- Output or Input CAN data of any ID
- User definable Masks and filters for reception
- Default transmission address or on-th-fly changes
- AT Command control

# Write to CAN Network
In order to place a message onto the can network on the default transmit ID, as defined previously by the AT commands. In the most extended form the data packet takes the form:

```
{ DLC Data0 Data1* Data2* Data3* Data4* Data5* Data6* Data7* ) } <CR>
```

Note the extra round bracket and carrige return!

Any fields marked with a * indicate they are optional.

In order to send a message on another CAN ID it is simply a case of add one or two extra bytes to the end of the string, noting that it is BIG ENDIAN such that:

```
{ DLC Data0 Data1* Data2* Data3* Data4* Data5* Data6* Data7* ID<15:8>* ID<7:0> )}<CR>
```

The device will know if they are IDs because the DLC will tell how many bytes of the packet are actual data. Also note that only the lower 8 bits of the address are required, if the upper byte is not there it will assume that it was 0.

At this stage the CAN driver is not written to accommodate EIDs but it wouldnt be hard.

#AT Command Set

## To Enter AT Command Mode
Similar to X-Bee, must read "+++" with nothing else before, during or after for at least 1 second. It will then return an "OK". Once this has occurred it will enter into the AT Command mode.

## Exiting the AT Command Mode
After recieving no information for 3 seconds it will automatically revert to normal mode.

## AT-Commands

Example transmission:

```
+++
-OK
AT+SM+123
-SM+123
```


| AT Command        | Description           | Parameters  |
| :-------------: |-------------| :-----:|
| **SM** | Set recieve mask | unsigned char |
| **SF** | Set recieve filter | unsigned char |
| **DI** | Set default transmit ID | unsigned char |

Future Plans
 - ASCII ID and DLC options
 - Change bus rate
