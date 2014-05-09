//
//  DHCPRelay.h
//  DHCPRelay
//
//  Created by Ludovic Vannoorenberghe on 9/05/14.
//  Copyright (c) 2014 Ludovic Vannoorenberghe. All rights reserved.
//

#ifndef DHCPRelay_DHCPRelay_h
#define DHCPRelay_DHCPRelay_h

// Helpers functions to print IP ADRESS on the LCD
#if defined(__SDCC__)
void DisplayIPValue(DWORD IPVal);
void DisplayString(BYTE pos, char* text);
void DisplayWORD(BYTE pos, WORD w);
#else
void DisplayIPValue(IP_ADDR IPVal);
#endif

#endif // _DHCPRelay_H
size_t  strlcpy(char *dst, const char *src, size_t siz);

