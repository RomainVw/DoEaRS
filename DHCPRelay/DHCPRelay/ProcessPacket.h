//
//  ProcessPacket.h
//  DHCPRelay
//
//  Created by Ludovic Vannoorenberghe on 17/05/14.
//  Copyright (c) 2014 Ludovic Vannoorenberghe. All rights reserved.
//

#ifndef DHCPRelay_ProcessPacket_h
#define DHCPRelay_ProcessPacket_h

#define DHCP_MAX_LEASES					10
#define LEASE_TIME		300ul

#define SERVER_IP_LB 192
#define SERVER_IP_HB 168
#define SERVER_IP_UB 2
#define SERVER_IP_MB 1

#define MAX_OPTION 52
#define MAX_CONTENT 20


typedef enum
{
	SM_SEND_ARP = 0,
	SM_GET_ARP,
	SM_IDLE,
	SM_CHECKING_TYPE,
	SM_PROCESS_DISCOVER,
	SM_PROCESS_REQUEST,
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

typedef enum
{
	LEASE_UNUSED = 0,
	LEASE_REQUESTED,
	LEASE_GRANTED
} LEASE_STATE;

typedef struct _DHCP_CONTROL_BLOCK
{
	LEASE_STATE smLease;
	TICK 		LeaseExpires;	// Expiration time for this lease
	TICK		RealLeaseTime;
	MAC_ADDR	ClientMAC;		// Client's MAC address.  Multicase bit is used to determine if a lease is given out or not
	IP_ADDR		ClientIp;
	UINT		nbLeaseMissed; // Status of this lease
} DHCP_CONTROL_BLOCK;


typedef struct {
	BYTE type;
	BYTE len;
	BYTE content[MAX_CONTENT];
} DHCP_OPTION;

typedef struct
{
	BOOTP_HEADER header;
	BYTE mac_offset[10];
	BYTE sname[64];
	BYTE file[128];
	BYTE magic_cookie[4];
	UINT nb_options;
	DHCP_OPTION options[MAX_OPTION];
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
	
	NODE_INFO server_info;
	
	DHCP_CONTROL_BLOCK	DCB[DHCP_MAX_LEASES];
} DHCP_RELAY_VARS;


void DHCPRelayInit(void);

void ServerToClient(void);

void ClientToServer(void);

void TimerTask(void);

#endif
