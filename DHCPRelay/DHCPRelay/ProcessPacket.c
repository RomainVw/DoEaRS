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
	SM_PROCESS_REQUEST,
	SM_REQUEST_FROM_KNOWN,
	SM_REQUEST_FROM_UNKNOWN,
	SM_PROCESS_DECLINE,
	SM_SEND
} C2SSTATE;

typedef enum
{
	SM_IDLE_S = 0,
	SM_CHECKING_TYPE_S,
	SM_PROCESS_OFFER,
	SM_PROCESS_ACK,
	SM_PROCESS_NACK,
	SM_SEND_S
} S2CSTATE;

typedef struct _DHCP_CONTROL_BLOCK
{
	TICK 		LeaseExpires;	// Expiration time for this lease
	TICK		RealLeaseTime;
	MAC_ADDR	ClientMAC;		// Client's MAC address.  Multicase bit is used to determine if a lease is given out or not
	IP_ADDR		ClientIp;
	UINT		nbLeaseMissed;
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
	
	TICK dwTimer;
	
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
			
			DHCPRelay.my_ip = AppConfig.MyIPAddr;
			DHCPRelay.router_ip = AppConfig.MyGateway;
	        DHCPRelay.s2cState = SM_IDLE_S;
	        DHCPRelay.c2sState = SM_IDLE;
			int i;
			for( i = 0; i < DHCP_MAX_LEASES; i++) {
				DHCPRelay.DCB[i].smLease = LEASE_UNUSED;
			}
			DHCPRelay.dwTimer = TickGet();
}


void ServerToClient(void)
{
	BOOTP_HEADER header;
	int i;
	switch(DHCPRelay.s2cState)
	{

		case SM_IDLE_S:			
			if(UDPIsGetReady(DHCPRelay.s2cSocket) >= 241u)
			{
				DHCPRelay.s2cState = SM_CHECKING_TYPE_S;
			}
			
			
			break;
			
		case SM_CHECKING_TYPE_S:
			
			// Retrieve the BOOTP header
			UDPGetArray((BYTE *)&header, sizeof(BOOTP_HEADER));
			//Must be a server to client message !
			if(header.MessageType != BOOT_REPLY)
				break;
			header.RelayAgentIP = DHCPRelay.my_ip;

			
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
			memcpy(DHCPRelay.s2c_message.mac_offset, macoffset, sizeof(BYTE) *10);
			memcpy(DHCPRelay.s2c_message.sname, sname, sizeof(BYTE)*64);
			memcpy(DHCPRelay.s2c_message.file, file, sizeof(BYTE)*128);
			memcpy(DHCPRelay.s2c_message.magic_cookie, &dw, sizeof(BYTE)*4);
			DHCPRelay.s2c_message.nb_options = i + 1;
			DHCPRelay.s2c_message.options = options;
			//Done with the packet
			DHCPRelay.s2c_message.header.Hops += 1;
			UDPDiscard();
			
			break;
			
		case SM_PROCESS_OFFER:
			//just forward it
			DHCPRelay.s2cState = SM_SEND_S;
			break;
			
		case SM_PROCESS_ACK: {
			//Store information
			int clientIndex = -1;
			int freeSlotIndex = -1;
			BOOL known = FALSE;
			for (i = 0; i < DHCP_MAX_LEASES; i++) {
				int j;
				known = TRUE;
				if (DHCPRelay.DCB[i].smLease == LEASE_UNUSED) {
					freeSlotIndex = i;
				}
				else {
					for (j = 0; j < 6; j++) {
						if(DHCPRelay.DCB[i].ClientMAC.v[j] != DHCPRelay.s2c_message.header.ClientMAC.v[j]){
							known = FALSE;
							break;
						}
					}
				}
				if(known) {
					clientIndex = i;
				}
			}
			DHCP_CONTROL_BLOCK client;
			if(clientIndex >= 0)//client is known
				client = DHCPRelay.DCB[clientIndex];
			
			else if (freeSlotIndex >= 0)// client not known
				client = DHCPRelay.DCB[freeSlotIndex];
			else //no lease left
				DHCPRelay.s2cState = SM_IDLE_S;
			client.ClientMAC = DHCPRelay.s2c_message.header.ClientMAC;
			client.ClientIp = DHCPRelay.s2c_message.header.YourIP;
			client.LeaseExpires = LEASE_TIME;
			for(i = 0; i < DHCPRelay.s2c_message.nb_options; i++ ) {
				DHCP_OPTION option = DHCPRelay.s2c_message.options[i];
				if (option.type == DHCP_IP_LEASE_TIME)
				{
					(&client.RealLeaseTime)[3] = option.content[3];
					(&client.RealLeaseTime)[2] = option.content[2];
					(&client.RealLeaseTime)[1] = option.content[1];
					(&client.RealLeaseTime)[0] = option.content[0];
						
					//MODIFY the lease time for the client
					option.content[3] = (LEASE_TIME >>24) & 0xFF;
					option.content[2] = (LEASE_TIME >>16) & 0xFF;
					option.content[1] = (LEASE_TIME >>8) & 0xFF;
					option.content[0] = (LEASE_TIME) & 0xFF;
				}
			}
			client.smLease = LEASE_GRANTED;
			client.nbLeaseMissed = 0;
			DHCPRelay.s2cState = SM_SEND_S;
			break;
		}
			
		case SM_PROCESS_NACK:
			//forward it for the moment
			for (i = 0; i < DHCP_MAX_LEASES; i++) {
				if (DHCPRelay.DCB[i].smLease == LEASE_GRANTED) {
					int j;
					BOOL same = TRUE;
					for (j = 0; j < 6; j++) {
						if(DHCPRelay.DCB[i].ClientMAC.v[j] != DHCPRelay.s2c_message.header.ClientMAC.v[j])
							same = FALSE;
					}
					if (same) DHCPRelay.DCB[i].smLease = LEASE_UNUSED;
				}
			}
			DHCPRelay.s2cState = SM_SEND_S;
			break;

		case SM_SEND_S:

			// Ensure transmitter is ready to accept data
			if(UDPIsPutReady(DHCPRelay.c2sSocket) < 300u) break;
			UDPPutArray((BYTE*)&DHCPRelay.s2c_message.header, sizeof(BOOTP_HEADER));
			UDPPutArray(DHCPRelay.s2c_message.mac_offset, 10);
			UDPPutArray(DHCPRelay.s2c_message.sname, sizeof(DHCPRelay.s2c_message.sname));
			UDPPutArray(DHCPRelay.s2c_message.file, sizeof(DHCPRelay.s2c_message.file));
			UDPPutArray(DHCPRelay.s2c_message.magic_cookie, sizeof(DHCPRelay.s2c_message.magic_cookie));
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
			
			DHCPRelay.s2cState = SM_IDLE_S;

			break;
	}
}


void ClientToServer(void)
{
	BOOTP_HEADER header;
	switch(DHCPRelay.c2sState)
	{

		case SM_IDLE:			
			if(UDPIsGetReady(DHCPRelay.c2sSocket) >= 241u)
			{
				DHCPRelay.c2sState = SM_CHECKING_TYPE;
			}
			else break;
			
		case SM_CHECKING_TYPE:
			
			// Retrieve the BOOTP header
			UDPGetArray((BYTE *)&header, sizeof(BOOTP_HEADER));
			//Must be a client to server message !
			if(header.MessageType != BOOT_REQUEST)
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
				if(op == DHCP_END_OPTION) {
					end = TRUE;
					break;
				}
				UDPGet(&len);       // Get option len
				cont = (BYTE *) calloc(len, sizeof(BYTE));
				UDPGetArray(cont, len);
				switch(op)
				{/*
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
						break;*/
						
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
				case DHCP_DISCOVER_MESSAGE :
					DHCPRelay.c2sState = SM_PROCESS_DISCOVER;
					break;
					
				case DHCP_REQUEST_MESSAGE:
					DHCPRelay.c2sState = SM_PROCESS_REQUEST;
					
				case DHCP_DECLINE_MESSAGE:
					DHCPRelay.c2sState = SM_PROCESS_DECLINE;
					break;
					
				default:
					//Ignore it
					DHCPRelay.c2sState = SM_IDLE;
					break;
			}
			//Store the packet
			
			DHCPRelay.c2s_message.header = header;
			memcpy(DHCPRelay.c2s_message.mac_offset, macoffset, 10);
			memcpy(DHCPRelay.c2s_message.sname, sname, 64);
			memcpy(DHCPRelay.c2s_message.file, file, 128);
			DHCPRelay.c2s_message.nb_options = i + 1;
			DHCPRelay.c2s_message.options = options;
			//Done with the packet
			UDPDiscard();
			
			break;
			
		case SM_PROCESS_DISCOVER:
			DHCPRelay.c2s_message.header.Hops += 1;
			DHCPRelay.c2sState = SM_SEND;
			break;
			
		case SM_PROCESS_REQUEST:
			break;
			

		case SM_REQUEST_FROM_UNKNOWN:
			break;
			
		case SM_REQUEST_FROM_KNOWN:
			break;
		
		case SM_PROCESS_DECLINE:
			break;
			
		case SM_SEND:
			// Ensure transmitter is ready to accept data
			if(UDPIsPutReady(DHCPRelay.s2cSocket) < 300u) break;
			UDPPutArray((BYTE*)&DHCPRelay.c2s_message.header, sizeof(BOOTP_HEADER));
			UDPPutArray(DHCPRelay.c2s_message.mac_offset, 10);
			UDPPutArray(DHCPRelay.c2s_message.sname, sizeof(DHCPRelay.c2s_message.sname));
			UDPPutArray(DHCPRelay.c2s_message.file, sizeof(DHCPRelay.c2s_message.file));
			for(i = 0; i < DHCPRelay.c2s_message.nb_options; i++ ) {
				UDPPut(DHCPRelay.c2s_message.options[i].type);
				UDPPut(DHCPRelay.c2s_message.options[i].len);
				UDPPutArray(DHCPRelay.c2s_message.options[i].content,
							DHCPRelay.c2s_message.options[i].len);
			}
			UDPFlush();
			
			//FREEEEE
			
			for (i = 0; i < DHCPRelay.c2s_message.nb_options; i++) {
				free(DHCPRelay.c2s_message.options[i].content);
				free(DHCPRelay.c2s_message.options);
			}
			
			break;
	}
	
}

void TimerTask() {
	static enum
	{
		SM_INIT = 0,
		SM_MONITOR_TIME,
		SM_RENEWING,
		SM_REMOVING
	} TimerState = SM_INIT;
	static TICK Timer;
	
	static int i;
	switch (TimerState) {
		case SM_INIT:
			Timer = TickGet();
			TimerState = SM_MONITOR_TIME;
			break;
		case SM_MONITOR_TIME:
			if(TickGet() - Timer < TICK_SECOND) break;
			
			// Check to see if our lease is still valid, if so, decrement
			// lease time
			
			Timer += TICK_SECOND;
			for (i = 0; i < DHCP_MAX_LEASES; i++) {
				if (DHCPRelay.DCB[i].smLease != LEASE_GRANTED) {
					break;
				}
				if(DHCPRelay.DCB[i].LeaseExpires >= 2ul) {
					DHCPRelay.DCB[i].LeaseExpires--;
				} else {
					if(++DHCPRelay.DCB[i].nbLeaseMissed >= 5){
						TimerState = SM_REMOVING;
						break;
					}
					DHCPRelay.DCB[i].LeaseExpires = LEASE_TIME;
				}
				if (DHCPRelay.DCB[i].RealLeaseTime >= 2ul) {
					DHCPRelay.DCB[i].RealLeaseTime--;
				}
				else {
					TimerState = SM_RENEWING;
					break;
				}
			}
			break;
			
		case SM_RENEWING:
			DHCPRelay.DCB[i].smLease = LEASE_REQUESTED;
			//send to server
			break;
			
		case SM_REMOVING:
			DHCPRelay.DCB[i].smLease = LEASE_UNUSED; //mark as unused
			TimerState = SM_MONITOR_TIME;
			break;
	}
	
}