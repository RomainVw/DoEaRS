//
//  ProcessPacket.c
//  DHCPRelay
//
//  Created by Ludovic Vannoorenberghe on 9/05/14.
//  Copyright (c) 2014 Ludovic Vannoorenberghe. All rights reserved.
//

#include <stdio.h>
#define __SMTPDEMO_C
#define __18F97J60
#define __SDCC__

#include <pic18f97j60.h> //ML

#include "Include/TCPIPConfig.h"



#include "Include/TCPIP_Stack/TCPIP.h"
#include "Include/MainDemo.h"

/*****************************************************************************
 Function:
 void SMTPDemo(void)
 
 Summary:
 Demonstrates use of the e-mail (SMTP) client.
 
 Description:
 This function demonstrates the use of the SMTP client.  The function is
 called periodically by the stack, and checks if BUTTON2 and BUTTON3 are
 pressed simultaneously.  If they are, it attempts to send an e-mail
 message using parameters hard coded in the function below.
 
 While the client is executing, LED1 will be used as a busy indicator.
 LED2 will light when the transmission has been completed successfully.
 If both LEDs extinguish, an error occurred.
 
 For an example of sending a longer message (one that does not exist in
 RAM all at once), see the commented secondary implementation of this
 function in this file (SMTPDemo.c) below.  For an example of sending
 a message using parameters gathered at run time, and/or a message with
 attachments, see the implementation of HTTPPostEmail in CustomHTTPApp.c.
 
 Precondition:
 The SMTP client is initialized.
 
 Parameters:
 None
 
 Returns:
 None
 ***************************************************************************/
void SMTPDemo(void)
{
	// Send an email once if someone pushes BUTTON2 and BUTTON3 at the same time
	// This is a simple message example, where the message
	// body must already be in RAM.
	// LED1 will be used as a busy indicator
	// LED2 will be used as a mail sent successfully indicator
	static enum
	{
		IDLE = 0,
		CHECKING_TYPE,
		PROCESS_DISCOVERY,
		PROCESS_OFFER,
		PROCESS_REQUEST,
		PROCESS_ACK,
		PROCESS_NACK,
		PROCESS_DECLINE,
		REQUEST_FROM_UNKNOW,
		REQUEST_FROM_KNOWN,
		ACK_FROM_UNKNOWN,
		ACK_FROM_KNOWN
	} PacketState = IDLE;
	
	
	switch(PacketState)
	{
		case IDLE:
			break;
			
		case CHECKING_TYPE:
			break;
			
		case PROCESS_DISCOVERY:
			break;
			
		case PROCESS_OFFER:
			break;
			
		case PROCESS_REQUEST:
			
		case PROCESS_ACK:
			break;
			
		case PROCESS_NACK:
			break;
			
		case PROCESS_DECLINE:
			break;
			
		case REQUEST_FROM_UNKNOW:
			break;
			
		case REQUEST_FROM_KNOWN:
			break;
			
		case ACK_FROM_UNKNOWN:
			break;
			
		case ACK_FROM_KNOWN:
			break;
	}
}