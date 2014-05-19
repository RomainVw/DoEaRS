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
    
	switch(DHCPRelay.s2cState)
	{
            
		case SM_IDLE_S: {
			if(UDPIsGetReady(DHCPRelay.s2cSocket) >= 241u && UDPIsPutReady(DHCPRelay.c2sSocket) >= 300u)
			{
				DHCPRelay.s2cState = SM_CHECKING_TYPE_S;
			}
			
			
			break;
		}
			
			
		case SM_CHECKING_TYPE_S: {
			DWORD dw;
			BOOL end = FALSE;
			BOOL broadcastOptionPresent = FALSE;
			BOOL routerOptionPresent = FALSE;
			BYTE type;
			DHCP_MESSAGE message;
			DWORD leaseTime = 0;
			DHCP_OPTION option;
            
			UDPIsGetReady(DHCPRelay.s2cSocket);
			UDPGetArray((BYTE *)&(message.header), sizeof(BOOTP_HEADER));
            
			//Must be a server to client message !
			if(message.header.MessageType != BOOT_REPLY) break;
			
			//Modify the header
			message.header.RelayAgentIP.Val = DHCPRelay.my_ip.Val;
			message.header.Hops += 1;
            
			UDPGetArray(message.mac_offset, 10);
			UDPGetArray(message.sname, 64);
			UDPGetArray(message.file, 128);
			// Obtain Magic Cookie and verify
			UDPGetArray((BYTE*)&dw, sizeof(DWORD));
			if(dw != 0x63538263ul)
				break;
			
			UDPIsPutReady(DHCPRelay.c2sSocket);
			//Forward header
			UDPPutArray((BYTE *)&(message.header), sizeof(BOOTP_HEADER));
			UDPPutArray(message.mac_offset, sizeof(BYTE)*10);
			UDPPutArray(message.sname, sizeof(BYTE)*10);
			UDPPutArray(message.file, sizeof(BYTE)*10);
			UDPPutArray((BYTE*)&dw, sizeof(DWORD));
			
			
			//CHECKING TYPE !
			type = DHCP_UNKNOWN_MESSAGE;
			do {
				UDPIsGetReady(DHCPRelay.s2cSocket);
				// Get the Option number
				// Break out eventually in case if this is a malformed
				// DHCP message, ie: missing DHCP_END_OPTION marker
				if(!UDPGet(&(option.type))) {
					end = TRUE;
					break;
				}
				if(option.type == DHCP_END_OPTION) {
					end = TRUE;
					break;
				}
				UDPGet(&(option.len));       // Get option len
				BYTE content[option.len];
				UDPGetArray(content, option.len);
				option.content = content;
				switch(option.type)
				{
					case DHCP_MESSAGE_TYPE:
						// Len must be 1.
						if (option.len == 1u ) {
							type = *(option.content);
						}
						else
							UDPDiscard(); // Packet not correct
						break;
					case DHCP_ROUTER:
						option.len = 4;
						memcpy(option.content, &DHCPRelay.router_ip, sizeof(IP_ADDR));
						routerOptionPresent = TRUE;
						break;
					case DHCP_BROADCAST_ADRESS:
						if (option.len == 4u) {
							memcpy(option.content, &DHCPRelay.broadcast_adress, sizeof(IP_ADDR));
							broadcastOptionPresent = TRUE;
						}
						else
							UDPDiscard();
						break;
					case DHCP_IP_LEASE_TIME:
						//MODIFY the lease time for the client
						(&(leaseTime))[3] = option.content[3];
						(&(leaseTime))[2] = option.content[2];
						(&(leaseTime))[1] = option.content[1];
						(&(leaseTime))[0] = option.content[0];
						option.content[3] = (LEASE_TIME >>24) & 0xFF;
						option.content[2] = (LEASE_TIME >>16) & 0xFF;
						option.content[1] = (LEASE_TIME >>8) & 0xFF;
						option.content[0] = (LEASE_TIME) & 0xFF;
						
					default:
						break;
						
				}
                
				UDPIsPutReady(DHCPRelay.c2sSocket);
				UDPPut(option.type);
				UDPPut(option.len);
				UDPPutArray(option.content, sizeof(BYTE) * option.len);
				
                
			} while (!end);
			
			switch (type) {
				case DHCP_ACK_MESSAGE:
					record_in_table(&(message.header), DHCPRelay.DCB, leaseTime);
					break;
				case DHCP_NAK_MESSAGE:
					delete_of_table(&(message.header), DHCPRelay.DCB);
					break;
				default:
					break;
			}
            
			UDPIsPutReady(DHCPRelay.c2sSocket);
			if (!broadcastOptionPresent) {
				UDPPut(DHCP_BROADCAST_ADRESS);
				UDPPut(sizeof(IP_ADDR));
				UDPPutArray(DHCPRelay.broadcast_adress.v, sizeof(IP_ADDR));
			}
			if (!routerOptionPresent) {
				UDPPut(DHCP_ROUTER);
				UDPPut(sizeof(IP_ADDR));
				UDPPutArray(DHCPRelay.router_ip.v, sizeof(IP_ADDR));
                
			}
			UDPPut(DHCP_END_OPTION);
			UDPFlush();
            
			UDPIsGetReady(DHCPRelay.s2cSocket);
			UDPDiscard();
			DHCPRelay.s2cState = SM_IDLE_S;
			
			break;
		}
			
	}
}


void ClientToServer(void)
{
    int clientIndex = 0;
    int isKnown = 0;
    int isRouter = 0, isBroadcast = 0 ;
    int ReplyAck= 0;
    DHCP_OPTION option;
    BOOL end;
    BYTE content[256];
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
            DWORD dw;
            
            BYTE type;
            
            
			// Retrieve the BOOTP header
			UDPGetArray((BYTE *)&DHCPRelay.c2s_message.header, sizeof(BOOTP_HEADER));
            
			//Must be a client to server message !
			if(DHCPRelay.c2s_message.header.MessageType != BOOT_REQUEST)
				break;
            
			DHCPRelay.c2s_message.header.RelayAgentIP.Val = DHCPRelay.my_ip.Val;
			
            
			UDPGetArray(DHCPRelay.c2s_message.mac_offset, 10);
			UDPGetArray(DHCPRelay.c2s_message.sname, 64);
			UDPGetArray(DHCPRelay.c2s_message.file, 128);
			
			
			// Obtain Magic Cookie and verify
			UDPGetArray((BYTE*)&dw, sizeof(DWORD));
			if(dw != 0x63538263ul)
				break;
			
			DHCPRelay.c2s_message.header.Hops += 1;
			
            
            // write on serveur side
			if(UDPIsPutReady(DHCPRelay.s2cSocket) < 300u) break;
			UDPPutArray((BYTE*)&DHCPRelay.c2s_message.header, sizeof(BOOTP_HEADER));
			UDPPutArray(DHCPRelay.c2s_message.mac_offset, 10);
			UDPPutArray(DHCPRelay.c2s_message.sname, sizeof(DHCPRelay.c2s_message.sname));
			UDPPutArray(DHCPRelay.c2s_message.file, sizeof(DHCPRelay.c2s_message.file));
			UDPPutArray(DHCPRelay.c2s_message.magic_cookie, sizeof(DHCPRelay.c2s_message.magic_cookie));
            
			
			end = FALSE;
			type = DHCP_UNKNOWN_MESSAGE;
			//i = 0;
			do {
                // listen on client side
                UDPIsPutReady(DHCPRelay.c2sSocket);
                
				if(!UDPGet(&option.type)) {
					end = TRUE;
					break;
				}
				if(option.type == DHCP_END_OPTION) {
					end = TRUE;
					break;
				}
				UDPGet(&option.len);       // Get option len
                
                
                UDPGetArray(content, option.len);
                
                if (option.type == DHCP_MESSAGE_TYPE) {
                    // Len must be 1.
                    if ( option.len == 1u ){
                        if(*content == DHCP_REQUEST_MESSAGE){
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
                                
                                
                                
                                ReplyAck = 1;
                                // The remaining lease time is not sufficient
                                if (DHCPRelay.DCB[clientIndex].RealLeaseTime < 20){
                                    DHCPRelay.c2sState = SM_IDLE;
                                }
                                
                                // Sends ACK message
                                DHCPRelay.c2s_message.header.MessageType = BOOT_REPLY;
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
                                
                                
                                
                                if(UDPIsPutReady(DHCPRelay.c2sSocket) < 300u) break;
                                UDPPutArray((BYTE*)&DHCPRelay.c2s_message.header, sizeof(BOOTP_HEADER));
                                UDPPutArray(DHCPRelay.c2s_message.mac_offset, 10);
                                UDPPutArray(DHCPRelay.c2s_message.sname, sizeof(DHCPRelay.c2s_message.sname));
                                UDPPutArray(DHCPRelay.c2s_message.file, sizeof(DHCPRelay.c2s_message.file));
                                UDPPutArray(DHCPRelay.c2s_message.magic_cookie, 4 * sizeof(BYTE));
                                
                                
                                UDPPut(DHCP_MESSAGE_TYPE);
                                UDPPut(sizeof(BYTE));
                                UDPPut(DHCP_ACK_MESSAGE);
                            }
                            
                        }
                        
                        
                    } else{
                        UDPDiscard(); // Packet not correct
                    }
                    
                }
                
                
                switch (option.type){
                    case DHCP_BROADCAST_ADRESS:
                        isBroadcast = 1;
                        memcpy(content,&DHCPRelay.broadcast_adress, sizeof(BYTE) * 4);
                        option.len = 4u;
                        break;
                        
                    case DHCP_ROUTER:
                        isRouter = 1;
                        memcpy(content, &DHCPRelay.router_ip, sizeof(BYTE) * 4);
                        option.len = 4u;
                        break;
                        
                    case DHCP_IP_LEASE_TIME:
                        content[3] = (MIN(LEASE_TIME, DHCPRelay.DCB[clientIndex].LeaseExpires) >>24) & 0xFF;
                        content[2] = (MIN(LEASE_TIME, DHCPRelay.DCB[clientIndex].LeaseExpires) >>16) & 0xFF;
                        content[1] = (MIN(LEASE_TIME, DHCPRelay.DCB[clientIndex].LeaseExpires) >>8) & 0xFF;
                        content[0] = (MIN(LEASE_TIME, DHCPRelay.DCB[clientIndex].LeaseExpires)) & 0xFF;
                    default:
                        break;
                }
                
                UDPPut(option.type);
                UDPPut(option.len);
                UDPPutArray(content,option.len);
                
                if (!ReplyAck) {
                    UDPIsPutReady(DHCPRelay.s2cSocket);
                    UDPPut(option.type);
                    UDPPut(option.len);
                    UDPPutArray(content,option.len);
                }
                
                
                
                
            } while (!end);
			
            
            if (!ReplyAck) {
                UDPDiscard();
                UDPIsPutReady(DHCPRelay.s2cSocket);
            }
            if (!isBroadcast) {
                option.type = DHCP_BROADCAST_ADRESS;
                option.len = 4;
                memcpy(content, &DHCPRelay.broadcast_adress, 4);
                
                
                UDPPut(option.type);
                UDPPut(option.len);
                UDPPutArray(content, option.len);
            }
            if (!isRouter) {
                option.type = DHCP_ROUTER;
                memcpy(content, &DHCPRelay.router_ip, sizeof(BYTE) * 4);
                option.len = 4u;
                
                UDPPut(option.type);
                UDPPut(option.len);
                UDPPutArray(content,option.len);
                
            }
            UDPPut(DHCP_END_OPTION);
            UDPFlush();
			
            if (ReplyAck) {
                UDPIsPutReady(DHCPRelay.s2cSocket);
                UDPDiscard();
            }
			
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

BOOL delete_of_table(BOOTP_HEADER * header, DHCP_CONTROL_BLOCK* dcb) {
	int i;
	//forward it for the moment
	for (i = 0; i < DHCP_MAX_LEASES; i++) {
		if (dcb[i].smLease == LEASE_GRANTED) {
			int j;
			BOOL same = TRUE;
			for (j = 0; j < 6; j++) {
				if(dcb[i].ClientMAC.v[j] != header->ClientMAC.v[j])
					same = FALSE;
			}
			if (same) dcb[i].smLease = LEASE_UNUSED;
		}
	}
	return TRUE;
}

BOOL record_in_table(BOOTP_HEADER * header, DHCP_CONTROL_BLOCK* dcb, DWORD leaseTime) {
	
	int i;
	//Store information
	int clientIndex = -1;
	int freeSlotIndex = -1;
	DHCP_CONTROL_BLOCK* client;
	BOOL known = FALSE;
	for (i = 0; i < DHCP_MAX_LEASES; i++) {
		int j;
		known = TRUE;
		if (DHCPRelay.DCB[i].smLease == LEASE_UNUSED) {
			freeSlotIndex = i;
		}
		else {
			for (j = 0; j < 6; j++) {
				if(DHCPRelay.DCB[i].ClientMAC.v[j] != header->ClientMAC.v[j]){
					known = FALSE;
					break;
				}
			}
			if(known) {
				clientIndex = i;
			}
		}
	}
	
	if(clientIndex >= 0) {//client is known
		client = dcb + clientIndex;
	}
	else if (freeSlotIndex >= 0) {// client not known
		client = dcb + freeSlotIndex;
	}
	else{ //no lease left
		return FALSE;
	}
	
	memcpy(client->ClientMAC.v, header->ClientMAC.v, sizeof(MAC_ADDR));
	client->ClientIp.Val = header->YourIP.Val;
	client->LeaseExpires = LEASE_TIME;
	client->RealLeaseTime = leaseTime;
	client->smLease = LEASE_GRANTED;
	client->nbLeaseMissed = 0;
	return TRUE;
}