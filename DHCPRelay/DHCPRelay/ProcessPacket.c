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
#include "Include/TCPIPConfig.h/DHCP.h"


#include "../Include/TCPIPConfig.h"
#include "Include/TCPIP_Stack/TCPIP.h"
#include "Include/MainDemo.h"

#define DHCP_MAX_LEASES					2
#define LEASE_TIME		300
typedef enum
{
	SM_GET_SOCKET = 0,
	SM_IDLE,
	SM_CHECKING_TYPE,
	SM_PROCESS_OFFER,
	SM_PROCESS_ACK,
	SM_PROCESS_NACK,
	SM_ACK_FROM_UNKNOWN,
	SM_ACK_FROM_KNOWN
} S2CSTATE;

typedef struct _DHCP_CONTROL_BLOCK
{
	TICK 		LeaseExpires;	// Expiration time for this lease
	MAC_ADDR	ClientMAC;		// Client's MAC address.  Multicase bit is used to determine if a lease is given out or not
	enum
	{
		LEASE_UNUSED = 0,
		LEASE_REQUESTED,
		LEASE_GRANTED
	} smLease;					// Status of this lease
} DHCP_CONTROL_BLOCK;


typedef struct {
	BYTE type;
	BYTE len;
	BYTE *content;
} DHCP_OPTION;

typedef struct
{
	BOOTP_HEADER header;
	BYTE mac_offset[10];
	BYTE sname[64];
	BYTE file[128];
	BYTE magic_cookie[4];
	DHCP_OPTION *options;
} DHCP_MESSAGE ;


// Unique variables per interface
typedef struct
{
    UDP_SOCKET  s2cSocket;  // Handle to DHCP client socket
	
    S2CSTATE	s2cState;	  // DHCP client state machine variable
    //Some variables for relay
	DHCP_MESSAGE message;
	DHCP_CONTROL_BLOCK	DCB[DHCP_MAX_LEASES];
} DHCP_RELAY_VARS;

static DHCP_RELAY_VARS DHCPRelay;

void ServerToClient(void)
{
	BOOTP_HEADER header;
	switch(DHCPRelay.s2cState)
	{
		case SM_GET_SOCKET:
	        // Open a socket to send and receive broadcast messages on
	        DHCPRelay.s2cSocket = UDPOpen(DHCP_CLIENT_PORT, NULL, DHCP_SERVER_PORT);
	        if(DHCPRelay.s2cSocket == INVALID_UDP_SOCKET) break;
			
	        DHCPRelay.s2cState = SM_IDLE;
	        // No break
		case SM_IDLE:
			
			if(UDPIsGetReady(DHCPRelay.s2cSocket))
			{
				DHCPRelay.s2cState = SM_CHECKING_TYPE;
			}
			else break;
			
		case SM_CHECKING_TYPE:
			
			// Retrieve the BOOTP header
			UDPGetArray((BYTE *)&header, sizeof(BOOTP_HEADER));
			//Must be a server to client message !
			if(header.MessageType != BOOT_REPLY)
				break;
			

			int i;
			// Throw away part of client hardware address,
			BYTE macoffset[10];
			UDPGetArray(macoffset, 10);
			// server host name, and boot file name -- unsupported/not needed.
			BYTE sname[64];
			UDPGetArray(sname, 64);
			BYTE file[128];
			UDPGetArray(file, 128);
			
			DWORD dw;
			
			// Obtain Magic Cookie and verify
			UDPGetArray((BYTE*)&dw, sizeof(DWORD));
			if(dw != 0x63538263ul)
				break;
			
			//CHECKING TYPE !
			BOOL end = FALSE;
			
			BYTE type;
			BYTE op, len;
			BYTE *cont;
			DHCP_OPTION *options;
			options = (DHCP_OPTION *) calloc(52, sizeof(DHCP_OPTION)); //Minimum 52
			type = DHCP_UNKNOWN_MESSAGE;
			i = 0;
			do {
				// Get the Option number
				// Break out eventually in case if this is a malformed
				// DHCP message, ie: missing DHCP_END_OPTION marker
				if(!UDPGet(&op)) {
					end = TRUE;
					break;
				}
				UDPGet(&len);       // Get option len
				cont = (BYTE *) calloc(len, sizeof(BYTE));
				UDPGetArray(cont, len);
				switch(op)
				{
					case DHCP_MESSAGE_TYPE:
						// Len must be 1.
						if ( len == 1u )
							type = *cont;
						else
							UDPDiscard(); // Packet not correct
						break;
					case DHCP_IP_LEASE_TIME:
						
					case DHCP_END_OPTION:
						end = TRUE;
						break;
						
					default:
						break;
						
				}
				//Store it to forward it !
				DHCP_OPTION option;
				option.type = op;
				option.len = len;
				option.content = cont;
				options[i] = option;
 				i++;
			} while (end);
			
			switch (type) {
				case DHCP_OFFER_MESSAGE:
					DHCPRelay.s2cState = SM_PROCESS_OFFER;
					break;
				case DHCP_ACK_MESSAGE:
					DHCPRelay.s2cState = SM_PROCESS_ACK;
				case DHCP_NAK_MESSAGE:
					DHCPRelay.s2cState = SM_PROCESS_NACK;
					break;
				default:
					//Ignore it
					DHCPRelay.s2cState = SM_IDLE;
					break;
			}
			//Store the packet
			
			DHCPRelay.message.header = header;
			memcpy(DHCPRelay.message.mac_offset, macoffset, 10);
			memcpy(DHCPRelay.message.sname, sname, 64);
			memcpy(DHCPRelay.message.file, file, 128);
			DHCPRelay.message.options = options;
			//Done with the packet
			UDPDiscard();
			
			break;
			
		case SM_PROCESS_OFFER:
			DHCPRelay.message.header.Hops += 1;
			
			break;
			
		case SM_PROCESS_ACK:
			break;
			
		case SM_PROCESS_NACK:
			break;
			
		case SM_ACK_FROM_UNKNOWN:
			break;
			
		case SM_ACK_FROM_KNOWN:
			break;
	}
}