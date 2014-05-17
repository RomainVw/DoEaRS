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
	SM_ACK_FROM_UNKNOWN,
	SM_ACK_FROM_KNOWN,
	SM_SEND_S
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
			
			DHCPRelay.my_ip = AppConfig.MyIPAddr;
			DHCPRelay.router_ip = AppConfig.MyGateway;
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
			DHCPRelay.s2cState = SM_SEND_S;
			break;
			
		case SM_PROCESS_ACK:
			//Store information 
			break;
			
		case SM_PROCESS_NACK:
			break;
			
		case SM_ACK_FROM_UNKNOWN:
			break;
			
		case SM_ACK_FROM_KNOWN:
			break;
		case SM_SEND_S:
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


void ClientToServer(void)
{
    int clientIndex = 0;
    int isKnown = 0;
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
				
                
                if (op == DHCP_MESSAGE_TYPE) {
                    // Len must be 1.
                    if ( len == 1u )
                        type = *cont;
                    else
                        UDPDiscard(); // Packet not correct
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
            // We forward the message
			DHCPRelay.c2s_message.header.Hops += 1;
			DHCPRelay.c2sState = SM_SEND;
			break;
			
		case SM_PROCESS_REQUEST:
            
            for (clientIndex = 0; clientIndex < DHCP_MAX_LEASES && isKnown == 0; i++) {
                isKnown = 1;
                for(int j = 0; j < 6 && isKnown == 1; j++)
                {
                    if (DHCPRelay.c2s_message.header.ClientMAC.v[j] != DHCPRelay.DCB[clientIndex].ClientMAC.v[j])
                        isKnown = 0;
                }
            }
            
            if (isKnown){
                DHCP_MESSAGE message_to_send;
                
                // The remaining lease time is not sufficient
                if (DHCPRelay.DCB[clientIndex].RealLeaseTime < 20){
                    DHCPRelay.c2sState = SM_IDLE;
                }
                
                // Sends ACK message
                message_to_send.header.MessageType = BOOT_REPLY;
                // htype ? hlen ?
                message_to_send.header.HardwareType = 1;
                message_to_send.header.HardwareLen = 6;
                
                
                message_to_send.header.Hops = 0;
                message_to_send.header.TransactionID = header.TransactionID;
                message_to_send.header.BootpFlags = header.BootpFlags;
                message_to_send.header.SecondsElapsed = 0;
                message_to_send.header.ClientIP = header.ClientIP;
                message_to_send.header.YourIP = DHCPRelay.DCB[clientIndex].ClientIp;
                message_to_send.header.NextServerIP = DHCPRelay.router_ip;
                message_to_send.header.RelayAgentIP =  DHCPRelay.my_ip;
                
                memcpy(message_to_send.mac_offset, macoffset, 10);
                memcpy(message_to_send.sname, sname, 64);
                memcpy(message_to_send.file, file, 128);
                
                // ch addr same as client
                
                
                if(UDPIsPutReady(DHCPRelay.c2sSocket) < 300u) break;
                UDPPutArray((BYTE*)&message_to_send.header, sizeof(BOOTP_HEADER));
                UDPPutArray(message_to_send.mac_offset, 10);
                UDPPutArray(message_to_send.sname, sizeof(message_to_send.sname));
                UDPPutArray(message_to_send.file, sizeof(message_to_send.file));
                UDPPutArray(DHCPRelay.c2s_message.magic_cookie, 4 * sizeof(BYTE));
                
                
                
                int isRouter = 0, isBroadcast = 0 ;
                
                
                for(i = 0; i < DHCPRelay.c2s_message.nb_options; i++ ) {
                    switch (options[i].type) {
                        case DHCP_MESSAGE_TYPE:
                            *(options[i].content) = DHCP_ACK_MESSAGE;
                            break;
                        case DHCP_BROADCAST_ADRESS:
                            isBroadcast = 1;
                            memcopy(options[i].content,&DHCPRelay.broadcast_address, sizeof(BYTE) * 4);
                            options[i].len = 4u;
                            break;
                        case DHCPRouter:
                            isRouter = 1;
                            memcopy(options[i].content, &DHCPRelay.router_ip, sizeof(BYTE) * 4);
                            options[i].len = 4u;
                        case DHCP_IP_LEASE_TIME:
                            options[i].content[3] = (min(LEASE_TIME, DHCPRelay.DCB[clientIndex].LeaseExpires) >>24) & 0xFF;
							options[i].content[2] = (min(LEASE_TIME, DHCPRelay.DCB[clientIndex].LeaseExpires) >>16) & 0xFF;
							options[i].content[1] = (min(LEASE_TIME, DHCPRelay.DCB[clientIndex].LeaseExpires) >>8) & 0xFF;
							options[i].content[0] = (min(LEASE_TIME, DHCPRelay.DCB[clientIndex].LeaseExpires)) & 0xFF;
                        default:
                            break;
                    }
                    
                    UDPPut(options[i].type);
                    UDPPut(options[i].len);
                    UDPPutArray(options[i].content,
                                options[i].len);
                }

                
                for(i = 0; i < message_to_send.nb_options; i++ ) {
                    UDPPut(options[i].type);
                    UDPPut(options[i].len);
                    UDPPutArray(options[i].content,
                                options[i].len);
                }
                
                
                if (!isRouter) {
                    DHCP_OPTION option_added;
                    option_added.type = DHCPRouter;
                    memcopy(option_added.content, &DHCPRelay.router_ip, sizeof(BYTE) * 4);
                    option_added.len = 4u;
                    
                    UDPPut(option_added.type);
                    UDPPut(option_added.len);
                    UDPPutArray(option_added.content,
                                option_added.len);
                    
                }
                
                if (!isBroadcast) {
                    DHCP_OPTION option_added;
                    option_added.type = DHCP_BROADCAST_ADRESS;
                    memcopy(option_added.content,&DHCPRelay.broadcast_address, sizeof(BYTE) * 4);
                    option_added.len = 4u;
                    
                    UDPPut(option_added.type);
                    UDPPut(option_added.len);
                    UDPPutArray(option_added.content,
                                option_added.len);
                }
                
                
                
                
                UDPFlush();
                
                //FREEEEE
           
                for (i = 0; i < message_to_send.nb_options; i++) {
                    free(options[i].content);
                    free(options);
                }

            
                
                
            } else { // We forward the message
                DHCPRelay.c2s_message.header.Hops += 1;
                DHCPRelay.c2sState = SM_SEND;
            }
			break;
			

	/*	case SM_REQUEST_FROM_UNKNOWN:
			break;
			
		case SM_REQUEST_FROM_KNOWN:
			break;*/
		
		case SM_PROCESS_DECLINE:
            // We forward the message
            DHCPRelay.c2s_message.header.Hops += 1;
			DHCPRelay.c2sState = SM_SEND;
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