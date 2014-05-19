/*********************************************************************
 *
 
 * FileName:        DHCPRelay.c
 * Authors:			Vannoorenberghe Ludovic
 *					Vanwelde Romain
 * Date:			19 May 2014
 * Cours:			LINGI2315
 * Description:		Fichier contenant les différentes stuctures de données utilisées
 *					lors du stockage des messages DHCP ainsi que les différents états des State machine.
 *********************************************************************/
#ifndef DHCPRelay_ProcessPacket_h
#define DHCPRelay_ProcessPacket_h

#define DHCP_MAX_LEASES					10
#define LEASE_TIME		300ul

#define SERVER_IP_LB 192
#define SERVER_IP_HB 168
#define SERVER_IP_UB 2
#define SERVER_IP_MB 1

#define MAX_OPTION 52
#define MAX_CONTENT 254


typedef enum
{
	SM_IDLE = 0,
	SM_CHECKING_TYPE
} C2SSTATE;

typedef enum
{
	SM_SEND_ARP = 0,
	SM_GET_ARP,
	SM_IDLE_S,
	SM_CHECKING_TYPE_S,
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
} DHCP_MESSAGE ;


// Unique variables per interface
typedef struct
{
    UDP_SOCKET  client;  // Handle to DHCP client socket
    UDP_SOCKET  server;  // Handle to DHCP server socket
	
    S2CSTATE	s2cState;	  // DHCP client state machine variable
    C2SSTATE	c2sState;	  // DHCP server state machine variable
    //Some variables for relay
	
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

BOOL record_in_table(BOOTP_HEADER* header, DHCP_CONTROL_BLOCK* dcb, DWORD leaseTime);

BOOL delete_of_table(BOOTP_HEADER * header, DHCP_CONTROL_BLOCK* dcb);

#endif
