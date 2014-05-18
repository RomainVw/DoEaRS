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
#include "ProcessPacket.h"

#define MIN(a,b) (((a)<(b))?(a):(b))



static DHCP_RELAY_VARS DHCPRelay;


void DHCPRelayInit(void)
{
			int i;
			// Open a socket to send and receive broadcast messages on
	        DHCPRelay.s2cSocket = UDPOpen(DHCP_CLIENT_PORT, NULL, DHCP_SERVER_PORT);
	        if(DHCPRelay.s2cSocket == INVALID_UDP_SOCKET)
				return;
			
				
			DHCPRelay.my_ip.Val = AppConfig.MyIPAddr.Val;
			DHCPRelay.router_ip.Val = AppConfig.MyGateway.Val;
	        DHCPRelay.s2cState = SM_IDLE_S;
	        DHCPRelay.c2sState = SM_IDLE;
	
			DHCPRelay.server_info.IPAddr.byte.LB = SERVER_IP_LB;
			DHCPRelay.server_info.IPAddr.byte.HB = SERVER_IP_HB;
			DHCPRelay.server_info.IPAddr.byte.UB = SERVER_IP_UB;
			DHCPRelay.server_info.IPAddr.byte.MB = SERVER_IP_MB;

			for( i = 0; i < DHCP_MAX_LEASES; i++) {
				DHCPRelay.DCB[i].smLease = LEASE_UNUSED;
			}
	
}


void ServerToClient(void)
{
	
	int i;
	switch(DHCPRelay.s2cState)
	{

		case SM_IDLE_S: {
				
			if(UDPIsGetReady(DHCPRelay.s2cSocket) >= 241u)
			{
				DHCPRelay.s2cState = SM_CHECKING_TYPE_S;
			}
			
			
			break;
		}
			
			
		case SM_CHECKING_TYPE_S: {
			DWORD dw;
			BOOL end = FALSE;
			BOOL broadcastOptionPresent = FALSE;
			BYTE type;
			//BYTE op, len;
			//BYTE *cont;


			// Retrieve the BOOTP header
            
			UDPGetArray((BYTE *)&DHCPRelay.s2c_message.header, sizeof(BOOTP_HEADER));
            
			//Must be a server to client message !
			if(DHCPRelay.s2c_message.header.MessageType != BOOT_REPLY)
				break;
			DHCPRelay.s2c_message.header.RelayAgentIP.Val = DHCPRelay.my_ip.Val;

			
			// Throw away part of client hardware address,
			
			UDPGetArray(DHCPRelay.s2c_message.mac_offset, 10);
			// server host name, and boot file name -- unsupported/not needed.
			UDPGetArray(DHCPRelay.s2c_message.sname, 64);
			UDPGetArray(DHCPRelay.s2c_message.file, 128);
						
            
			// Obtain Magic Cookie and verify
			UDPGetArray((BYTE*)&dw, sizeof(DWORD));
			if(dw != 0x63538263ul)
				break;
			
			//CHECKING TYPE !
			type = DHCP_UNKNOWN_MESSAGE;
			i = 0;
			do {
				// Get the Option number
				// Break out eventually in case if this is a malformed
				// DHCP message, ie: missing DHCP_END_OPTION marker
				if(!UDPGet(&DHCPRelay.s2c_message.options[i].type)) {
					end = TRUE;
					break;
				}
				if(DHCPRelay.s2c_message.options[i].type == DHCP_END_OPTION) {
					end = TRUE;
					break;
				}
				UDPGet(&DHCPRelay.s2c_message.options[i].len);       // Get option len
				UDPGetArray(DHCPRelay.s2c_message.options[i].content, DHCPRelay.s2c_message.options[i].len);
				switch(DHCPRelay.s2c_message.options[i].type)
				{
					case DHCP_MESSAGE_TYPE:
						// Len must be 1.
						if ( DHCPRelay.s2c_message.options[i].len == 1u )
							type = *DHCPRelay.s2c_message.options[i].content;
						else
							UDPDiscard(); // Packet not correct
						break;
					case DHCP_ROUTER:
						DHCPRelay.s2c_message.options[i].len = 4;
						memcpy(DHCPRelay.s2c_message.options[i].content, &DHCPRelay.router_ip, 4);
						
					case DHCP_BROADCAST_ADRESS:
						if (DHCPRelay.s2c_message.options[i].len == 4u) {
							memcpy(DHCPRelay.s2c_message.options[i].content, &DHCPRelay.broadcast_adress, 4);
							broadcastOptionPresent = TRUE;
						}
						else
							UDPDiscard();
						
					default:
						break;
						
				}
				//Store it to forward it !
               // options[i].type = op;
				//options[i].len = len;
				//options[i].content = cont;
 				i++;
				/*if (allocated_option == i) {
					options = (DHCP_OPTION *) realloc(options, allocated_option + (5 * sizeof(DHCP_OPTION)));
					allocated_option += 5;
				}*/
			} while (!end);
			
			if (!broadcastOptionPresent) {
				DHCPRelay.s2c_message.options[i].type = DHCP_BROADCAST_ADRESS;
				DHCPRelay.s2c_message.options[i].len = 4;
                memcpy(DHCPRelay.s2c_message.options[i].content, &DHCPRelay.broadcast_adress, 4);
				i++;
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
			
			//memcpy(&(DHCPRelay.s2c_message.header), &(DHCPRelay.s2c_message.header), sizeof(BOOTP_HEADER));
			//memcpy(&(DHCPRelay.s2c_message.mac_offset), &(macoffset), sizeof(BYTE) *10);
			//memcpy(&(DHCPRelay.s2c_message.sname), &(sname), sizeof(BYTE)*64);
			//memcpy(&(DHCPRelay.s2c_message.file), &(file), sizeof(BYTE)*128);
			//memcpy(&(DHCPRelay.s2c_message.magic_cookie), &dw, sizeof(BYTE)*4);
			DHCPRelay.s2c_message.nb_options = i;
			//DHCPRelay.s2c_message.options = options;
			//Done with the packet
			DHCPRelay.s2c_message.header.Hops += 1;
			UDPDiscard();
			
			break;
		}
			
		case SM_PROCESS_OFFER:
			//just forward it
			DHCPRelay.s2cState = SM_SEND_S;
			break;
			
		case SM_PROCESS_ACK: {
			//Store information
			int clientIndex = -1;
			int freeSlotIndex = -1;
			DHCP_CONTROL_BLOCK* client;
			DHCP_OPTION* option;
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
			
			if(clientIndex >= 0) {//client is known
				client = DHCPRelay.DCB + clientIndex ;
			}
			else if (freeSlotIndex >= 0) {// client not known
				client = DHCPRelay.DCB + freeSlotIndex;
            }
			else{ //no lease left
				DHCPRelay.s2cState = SM_IDLE_S;
                break;
            }
            
			memcpy(&(client->ClientMAC), &(DHCPRelay.s2c_message.header.ClientMAC), sizeof(MAC_ADDR));
			client->ClientIp.Val = DHCPRelay.s2c_message.header.YourIP.Val;
			client->LeaseExpires = LEASE_TIME;
			for(i = 0; i < DHCPRelay.s2c_message.nb_options; i++ ) {
				option = DHCPRelay.s2c_message.options+i;
				if (option->type == DHCP_IP_LEASE_TIME)
				{
					(&(client->RealLeaseTime))[3] = option->content[3];
					(&(client->RealLeaseTime))[2] = option->content[2];
					(&(client->RealLeaseTime))[1] = option->content[1];
					(&(client->RealLeaseTime))[0] = option->content[0];
						
					//MODIFY the lease time for the client
					option->content[3] = (LEASE_TIME >>24) & 0xFF;
					option->content[2] = (LEASE_TIME >>16) & 0xFF;
					option->content[1] = (LEASE_TIME >>8) & 0xFF;
					option->content[0] = (LEASE_TIME) & 0xFF;
				}
			}
			client->smLease = LEASE_GRANTED;
			client->nbLeaseMissed = 0;
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
			

			DHCPRelay.s2cState = SM_IDLE_S;

			break;
	}
}


void ClientToServer(void)
{
    int clientIndex = 0;
    int isKnown = 0;
	//BOOTP_HEADER header;
	switch(DHCPRelay.c2sState)
	{
		case SM_SEND_ARP:
			ARPResolve(&DHCPRelay.server_info.IPAddr);
			DHCPRelay.c2sState = SM_GET_ARP;
			break;
			
		case SM_GET_ARP:
			if(ARPIsResolved(&DHCPRelay.server_info.IPAddr, &DHCPRelay.server_info.MACAddr)) {
				DHCPRelay.c2sSocket = UDPOpen(DHCP_SERVER_PORT, &DHCPRelay.server_info, DHCP_CLIENT_PORT);
				if(DHCPRelay.c2sSocket != INVALID_UDP_SOCKET)
					DHCPRelay.c2sState = SM_IDLE;
				else DHCPRelay.c2sState = SM_SEND_ARP;
			}
			
			break;
			
		case SM_IDLE:			
			if(UDPIsGetReady(DHCPRelay.c2sSocket) >= 241u)
			{
				DHCPRelay.c2sState = SM_CHECKING_TYPE;
			}
			else break;
			
		case SM_CHECKING_TYPE:{
			int i;
            //BYTE macoffset[10];
            //BYTE sname[64];
            //BYTE file[128];
            DWORD dw;
            BOOL end;
            BOOL broadcastOptionPresent;
            BYTE type;
			//BYTE op, len;
			//BYTE *cont;
			//DHCP_OPTION *options;
            
			// Retrieve the BOOTP header
			UDPGetArray((BYTE *)&DHCPRelay.c2s_message.header, sizeof(BOOTP_HEADER));
			//Must be a client to server message !
			if(DHCPRelay.c2s_message.header.MessageType != BOOT_REQUEST)
				break;
			DHCPRelay.c2s_message.header.RelayAgentIP.Val = DHCPRelay.my_ip.Val;

			
			// Throw away part of client hardware address,
			
			UDPGetArray(DHCPRelay.c2s_message.mac_offset, 10);
			// server host name, and boot file name -- unsupported/not needed.
			UDPGetArray(DHCPRelay.c2s_message.sname, 64);
			UDPGetArray(DHCPRelay.c2s_message.file, 128);
			
			
			
			// Obtain Magic Cookie and verify
			UDPGetArray((BYTE*)&dw, sizeof(DWORD));
			if(dw != 0x63538263ul)
				break;
			
			//CHECKING TYPE !
			end = FALSE;
			broadcastOptionPresent = FALSE;


			type = DHCP_UNKNOWN_MESSAGE;
			i = 0;
			do {
				// Get the Option number
				// Break out eventually in case if this is a malformed
				// DHCP message, ie: missing DHCP_END_OPTION marker
				if(!UDPGet(&DHCPRelay.c2s_message.options[i].type)) {
					end = TRUE;
					break;
				}
				if(DHCPRelay.c2s_message.options[i].type == DHCP_END_OPTION) {
					end = TRUE;
					break;
				}
				UDPGet(&DHCPRelay.c2s_message.options[i].len);       // Get option len

				UDPGetArray(DHCPRelay.c2s_message.options[i].content, DHCPRelay.c2s_message.options[i].len);
				
                
                if (DHCPRelay.c2s_message.options[i].type == DHCP_MESSAGE_TYPE) {
                    // Len must be 1.
                    if ( DHCPRelay.c2s_message.options[i].len == 1u )
                        type = *DHCPRelay.c2s_message.options[i].content;
                    else
                        UDPDiscard(); // Packet not correct
                }
				//Store it to forward it !
				//options[i].type = op;
				//options[i].len = len;
				//options[i].content = cont;
 				i++;
			} while (!end);
			
			if (!broadcastOptionPresent) {
                DHCPRelay.c2s_message.options[i].type = DHCP_BROADCAST_ADRESS;
				DHCPRelay.c2s_message.options[i].len = 4;
				memcpy(DHCPRelay.c2s_message.options[i].content, &DHCPRelay.broadcast_adress, 4);
				i++;
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
			
			//memcpy(&(DHCPRelay.c2s_message.header),&header,sizeof(header));
			//memcpy(DHCPRelay.c2s_message.mac_offset, macoffset, 10);
			//memcpy(DHCPRelay.c2s_message.sname, sname, 64);
			//memcpy(DHCPRelay.c2s_message.file, file, 128);
			DHCPRelay.c2s_message.nb_options = i;
			//DHCPRelay.c2s_message.options = options;
			//Done with the packet
			UDPDiscard();
			
			break;
        }
			
		case SM_PROCESS_DISCOVER:
            // We forward the message
			DHCPRelay.c2s_message.header.Hops += 1;
			DHCPRelay.c2sState = SM_SEND;
			break;
			
		case SM_PROCESS_REQUEST:{
            int j;
            
            for (clientIndex = 0; clientIndex < DHCP_MAX_LEASES && isKnown == 0; clientIndex++) {
                isKnown = 1;
                for(j = 0; j < 6 && isKnown == 1; j++)
                {
                    if (DHCPRelay.c2s_message.header.ClientMAC.v[j] != DHCPRelay.DCB[clientIndex].ClientMAC.v[j])
                        isKnown = 0;
                }
            }
            
            if (isKnown){
               // DHCP_MESSAGE DHCPRelay.c2s_message;
                int i;
                int isRouter = 0, isBroadcast = 0 ;
                
                
                // The remaining lease time is not sufficient
                if (DHCPRelay.DCB[clientIndex].RealLeaseTime < 20){
                    DHCPRelay.c2sState = SM_IDLE;
                }
                
                // Sends ACK message
                DHCPRelay.c2s_message.header.MessageType = BOOT_REPLY;
                // htype ? hlen ?
                DHCPRelay.c2s_message.header.HardwareType = 1;
                DHCPRelay.c2s_message.header.HardwareLen = 6;
                
                
                DHCPRelay.c2s_message.header.Hops = 0;
                //DHCPRelay.c2s_message.header.TransactionID = DHCPRelay.c2s_message.header.TransactionID;
                //DHCPRelay.c2s_message.header.BootpFlags = DHCPRelay.c2s_message.header.BootpFlags;
                DHCPRelay.c2s_message.header.SecondsElapsed = 0;
                //DHCPRelay.c2s_message.header.ClientIP.Val = DHCPRelay.c2s_message.header.ClientIP.Val;
                DHCPRelay.c2s_message.header.YourIP.Val = DHCPRelay.DCB[clientIndex].ClientIp.Val;
                DHCPRelay.c2s_message.header.NextServerIP.Val = DHCPRelay.router_ip.Val;
                DHCPRelay.c2s_message.header.RelayAgentIP.Val =  DHCPRelay.my_ip.Val;
                
                //memcpy(DHCPRelay.c2s_message.mac_offset, DHCPRelay.c2s_message.mac_offset, 10);
                //memcpy(DHCPRelay.c2s_message.sname, DHCPRelay.c2s_message.sname, 64);
                //memcpy(DHCPRelay.c2s_message.file, DHCPRelay.c2s_message.file, 128);
                
                // ch addr same as client
                
                
                if(UDPIsPutReady(DHCPRelay.c2sSocket) < 300u) break;
                UDPPutArray((BYTE*)&DHCPRelay.c2s_message.header, sizeof(BOOTP_HEADER));
                UDPPutArray(DHCPRelay.c2s_message.mac_offset, 10);
                UDPPutArray(DHCPRelay.c2s_message.sname, sizeof(DHCPRelay.c2s_message.sname));
                UDPPutArray(DHCPRelay.c2s_message.file, sizeof(DHCPRelay.c2s_message.file));
                UDPPutArray(DHCPRelay.c2s_message.magic_cookie, 4 * sizeof(BYTE));
                
                
                
                
                
                
                for(i = 0; i < DHCPRelay.c2s_message.nb_options; i++ ) {
                    switch (DHCPRelay.c2s_message.options[i].type) {
                        case DHCP_MESSAGE_TYPE:
                            *(DHCPRelay.c2s_message.options[i].content) = DHCP_ACK_MESSAGE;
                            break;
                        case DHCP_BROADCAST_ADRESS:
                            isBroadcast = 1;
                            memcpy(DHCPRelay.c2s_message.options[i].content,&DHCPRelay.broadcast_adress, sizeof(BYTE) * 4);
                            DHCPRelay.c2s_message.options[i].len = 4u;
                            break;
                        case DHCP_ROUTER:
                            isRouter = 1;
                            memcpy(DHCPRelay.c2s_message.options[i].content, &DHCPRelay.router_ip, sizeof(BYTE) * 4);
                            DHCPRelay.c2s_message.options[i].len = 4u;
                        case DHCP_IP_LEASE_TIME:
                            DHCPRelay.c2s_message.options[i].content[3] = (MIN(LEASE_TIME, DHCPRelay.DCB[clientIndex].LeaseExpires) >>24) & 0xFF;
							DHCPRelay.c2s_message.options[i].content[2] = (MIN(LEASE_TIME, DHCPRelay.DCB[clientIndex].LeaseExpires) >>16) & 0xFF;
							DHCPRelay.c2s_message.options[i].content[1] = (MIN(LEASE_TIME, DHCPRelay.DCB[clientIndex].LeaseExpires) >>8) & 0xFF;
							DHCPRelay.c2s_message.options[i].content[0] = (MIN(LEASE_TIME, DHCPRelay.DCB[clientIndex].LeaseExpires)) & 0xFF;
                        default:
                            break;
                    }
                    
                    UDPPut(DHCPRelay.c2s_message.options[i].type);
                    UDPPut(DHCPRelay.c2s_message.options[i].len);
                    UDPPutArray(DHCPRelay.c2s_message.options[i].content,
                                DHCPRelay.c2s_message.options[i].len);
                }

                
                
                
                if (!isRouter) {
                    DHCP_OPTION option_added;
                    option_added.type = DHCP_ROUTER;
                    memcpy(option_added.content, &DHCPRelay.router_ip, sizeof(BYTE) * 4);
                    option_added.len = 4u;
                    
                    UDPPut(option_added.type);
                    UDPPut(option_added.len);
                    UDPPutArray(option_added.content,
                                option_added.len);
                    
                }
                
                if (!isBroadcast) {
                    DHCP_OPTION option_added;
                    option_added.type = DHCP_BROADCAST_ADRESS;
                    memcpy(option_added.content,&DHCPRelay.broadcast_adress, sizeof(BYTE) * 4);
                    option_added.len = 4u;
                    
                    UDPPut(option_added.type);
                    UDPPut(option_added.len);
                    UDPPutArray(option_added.content,
                                option_added.len);
                }
                
                
                
                
                UDPFlush();

            
                
                
            } else { // We forward the message
                DHCPRelay.c2s_message.header.Hops += 1;
                DHCPRelay.c2sState = SM_SEND;
            }
			break;
        }
		case SM_PROCESS_DECLINE:
            // We forward the message
            DHCPRelay.c2s_message.header.Hops += 1;
			DHCPRelay.c2sState = SM_SEND;
			break;
			
		case SM_SEND:{
            int i;
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
			

			
			break;
        }
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