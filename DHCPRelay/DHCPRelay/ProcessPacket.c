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
#define LEASE_TIME		300ul
typedef enum
{
	SM_IDLE = 0,
	SM_CHECKING_TYPE,
	SM_PROCESS_DISCOVER,
	SM_REQUEST_FROM_KNOWN,
	SM_REQUEST_FROM_UNKNOWN,
	SM_PROCESS_DECLINE
} C2SSTATE;

typedef enum
{
	SM_IDLE_S = 0,
	SM_CHECKING_TYPE_S,
	SM_PROCESS_OFFER,
	SM_PROCESS_ACK,
	SM_PROCESS_NACK,
	SM_ACK_FROM_UNKNOWN,
	SM_ACK_FROM_KNOWN,
	SM_SEND
} S2CSTATE;

typedef struct _DHCP_CONTROL_BLOCK
{
	TICK 		LeaseExpires;	// Expiration time for this lease
	MAC_ADDR	ClientMAC;		// Client's MAC address.  Multicase bit is used to determine if a lease is given out or not
	IP_ADDR		ClientIp;
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
	UINT nb_options;
	DHCP_OPTION *options;
} DHCP_MESSAGE ;


// Unique variables per interface
typedef struct
{
    UDP_SOCKET  s2cSocket;  // Handle to DHCP client socket
    UDP_SOCKET  c2sSocket;  // Handle to DHCP server socket
	
    S2CSTATE	s2cState;	  // DHCP client state machine variable
    C2SSTATE	c2sState;	  // DHCP server state machine variable
    //Some variables for relay

	DHCP_MESSAGE s2c_message;
	DHCP_MESSAGE c2s_message;
	
	IP_ADDR my_ip;
	IP_ADDR router_ip;
	IP_ADDR broadcast_adress;

	DHCP_CONTROL_BLOCK	DCB[DHCP_MAX_LEASES];
} DHCP_RELAY_VARS;

static DHCP_RELAY_VARS DHCPRelay;


void DHCPRelayInit(void)
{
			// Open a socket to send and receive broadcast messages on
	        DHCPRelay.s2cSocket = UDPOpen(DHCP_CLIENT_PORT, NULL, DHCP_SERVER_PORT);
	        if(DHCPRelay.s2cSocket == INVALID_UDP_SOCKET)
				return;
			
			DHCPRelay.c2sSocket = UDPOpen(DHCP_SERVER_PORT, NULL, DHCP_CLIENT_PORT);
	        if(DHCPRelay.c2sSocket == INVALID_UDP_SOCKET)
				return;
			
	        DHCPRelay.s2cState = SM_IDLE_S;
	        DHCPRelay.c2sState = SM_IDLE;
}


void ServerToClient(void)
{
	BOOTP_HEADER header;
	switch(DHCPRelay.s2cState)
	{

		case SM_IDLE_S:			
			if(UDPIsGetReady(DHCPRelay.s2cSocket) >= 241u)
			{
				DHCPRelay.s2cState = SM_CHECKING_TYPE_S;
			}
			else break;
			
		case SM_CHECKING_TYPE_S:
			
			// Retrieve the BOOTP header
			UDPGetArray((BYTE *)&header, sizeof(BOOTP_HEADER));
			//Must be a server to client message !
			if(header.MessageType != BOOT_REPLY)
				break;
			header.RelayAgentIP = DHCPRelay.my_ip;

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
			BOOL broadcastOptionPresent = FALSE;
			BYTE type;
			BYTE op, len;
			BYTE *cont;
			DHCP_OPTION *options;
			UINT allocated_option = 52;
			options = (DHCP_OPTION *) calloc(allocated_option, sizeof(DHCP_OPTION)); //Minimum 52
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
				if(op == DHCP_END_OPTION) {
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
						if ( len == 4u ) {
							//MODIFY the lease time for the client
							cont[3] = (LEASE_TIME >>24) & 0xFF;
							cont[2] = (LEASE_TIME >>16) & 0xFF;
							cont[1] = (LEASE_TIME >>8) & 0xFF;
							cont[0] = (LEASE_TIME) & 0xFF;
						}
						else
							UDPDiscard(); // Packet not correct
					case DHCP_ROUTER:
						len = 4;
						memcpy(cont, &DHCPRelay.router_ip, 4);
						
					case DHCP_BROADCAST_ADRESS:
						if (len == 4u) {
							memcpy(cont, &DHCPRelay.broadcast_adress, 4);
							broadcastOptionPresent = TRUE;
						}
						else
							UDPDiscard();
						
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
				if (allocated_option == i) {
					options = realloc(options, allocated_option + (5 * sizeof(DHCP_OPTION)));
					allocated_option += 5;
				}
			} while (!end);
			
			if (!broadcastOptionPresent) {
				DHCP_OPTION option;
				option.type = DHCP_BROADCAST_ADRESS;
				option.len = 4;
				cont = (BYTE *) calloc(option.len, sizeof(BYTE));
				memcpy(cont, &DHCPRelay.broadcast_adress, 4);
				options[++i] = option;
			}
			
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
					DHCPRelay.s2cState = SM_IDLE_S;
					break;
			}
			//Store the packet
			
			DHCPRelay.s2c_message.header = header;
			memcpy(DHCPRelay.s2c_message.mac_offset, macoffset, 10);
			memcpy(DHCPRelay.s2c_message.sname, sname, 64);
			memcpy(DHCPRelay.s2c_message.file, file, 128);
			DHCPRelay.s2c_message.nb_options = i + 1;
			DHCPRelay.s2c_message.options = options;
			//Done with the packet
			UDPDiscard();
			
			break;
			
		case SM_PROCESS_OFFER:
			DHCPRelay.s2c_message.header.Hops += 1;
			DHCPRelay.s2cState = SM_SEND;
			break;
			
		case SM_PROCESS_ACK:
			break;
			
		case SM_PROCESS_NACK:
			break;
			
		case SM_ACK_FROM_UNKNOWN:
			break;
			
		case SM_ACK_FROM_KNOWN:
			break;
		case SM_SEND:
			// Ensure transmitter is ready to accept data
			if(UDPIsPutReady(DHCPRelay.c2sSocket) < 300u) break;
			UDPPutArray((BYTE*)&DHCPRelay.s2c_message.header, sizeof(BOOTP_HEADER));
			UDPPutArray(DHCPRelay.s2c_message.mac_offset, 10);
			UDPPutArray(DHCPRelay.s2c_message.sname, sizeof(DHCPRelay.s2c_message.sname));
			UDPPutArray(DHCPRelay.s2c_message.file, sizeof(DHCPRelay.s2c_message.file));
			for(i = 0; i < DHCPRelay.s2c_message.nb_options; i++ ) {
				UDPPut(DHCPRelay.s2c_message.options[i].type);
				UDPPut(DHCPRelay.s2c_message.options[i].len);
				UDPPutArray(DHCPRelay.s2c_message.options[i].content,
							DHCPRelay.s2c_message.options[i].len);
			}
			UDPFlush();
			
			//FREEEEE
			
			for (i = 0; i < DHCPRelay.s2c_message.nb_options; i++) {
				free(DHCPRelay.s2c_message.options[i].content);
				free(DHCPRelay.s2c_message.options);
			}
			
			break;
	}
}