;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 2.9.4 #5582 (Dec  9 2009) (UNIX)
; This file was generated Wed Dec 16 00:29:59 2009
;--------------------------------------------------------
; PIC16 port for the Microchip 16-bit core micros
;--------------------------------------------------------
	list	p=18f97j60

	radix dec

;--------------------------------------------------------
; public variables in this module
;--------------------------------------------------------
	global _BerkeleyTCPClientDemo

;--------------------------------------------------------
; extern variables in this module
;--------------------------------------------------------
	extern _EBSTCONbits
	extern _MISTATbits
	extern _EFLOCONbits
	extern _MACON1bits
	extern _MACON2bits
	extern _MACON3bits
	extern _MACON4bits
	extern _MACLCON1bits
	extern _MACLCON2bits
	extern _MICONbits
	extern _MICMDbits
	extern _EWOLIEbits
	extern _EWOLIRbits
	extern _ERXFCONbits
	extern _EIEbits
	extern _ESTATbits
	extern _ECON2bits
	extern _EIRbits
	extern _EDATAbits
	extern _SSP2CON2bits
	extern _SSP2CON1bits
	extern _SSP2STATbits
	extern _ECCP2DELbits
	extern _ECCP2ASbits
	extern _ECCP3DELbits
	extern _ECCP3ASbits
	extern _RCSTA2bits
	extern _TXSTA2bits
	extern _CCP5CONbits
	extern _CCP4CONbits
	extern _T4CONbits
	extern _ECCP1DELbits
	extern _BAUDCON2bits
	extern _BAUDCTL2bits
	extern _BAUDCONbits
	extern _BAUDCON1bits
	extern _BAUDCTLbits
	extern _BAUDCTL1bits
	extern _PORTAbits
	extern _PORTBbits
	extern _PORTCbits
	extern _PORTDbits
	extern _PORTEbits
	extern _PORTFbits
	extern _PORTGbits
	extern _PORTHbits
	extern _PORTJbits
	extern _LATAbits
	extern _LATBbits
	extern _LATCbits
	extern _LATDbits
	extern _LATEbits
	extern _LATFbits
	extern _LATGbits
	extern _LATHbits
	extern _LATJbits
	extern _DDRAbits
	extern _TRISAbits
	extern _DDRBbits
	extern _TRISBbits
	extern _DDRCbits
	extern _TRISCbits
	extern _DDRDbits
	extern _TRISDbits
	extern _DDREbits
	extern _TRISEbits
	extern _DDRFbits
	extern _TRISFbits
	extern _DDRGbits
	extern _TRISGbits
	extern _DDRHbits
	extern _TRISHbits
	extern _DDRJbits
	extern _TRISJbits
	extern _OSCTUNEbits
	extern _MEMCONbits
	extern _PIE1bits
	extern _PIR1bits
	extern _IPR1bits
	extern _PIE2bits
	extern _PIR2bits
	extern _IPR2bits
	extern _PIE3bits
	extern _PIR3bits
	extern _IPR3bits
	extern _EECON1bits
	extern _RCSTAbits
	extern _RCSTA1bits
	extern _TXSTAbits
	extern _TXSTA1bits
	extern _PSPCONbits
	extern _T3CONbits
	extern _CMCONbits
	extern _CVRCONbits
	extern _ECCP1ASbits
	extern _CCP3CONbits
	extern _ECCP3CONbits
	extern _CCP2CONbits
	extern _ECCP2CONbits
	extern _CCP1CONbits
	extern _ECCP1CONbits
	extern _ADCON2bits
	extern _ADCON1bits
	extern _ADCON0bits
	extern _SSP1CON2bits
	extern _SSPCON2bits
	extern _SSP1CON1bits
	extern _SSPCON1bits
	extern _SSP1STATbits
	extern _SSPSTATbits
	extern _T2CONbits
	extern _T1CONbits
	extern _RCONbits
	extern _WDTCONbits
	extern _ECON1bits
	extern _OSCCONbits
	extern _T0CONbits
	extern _STATUSbits
	extern _INTCON3bits
	extern _INTCON2bits
	extern _INTCONbits
	extern _STKPTRbits
	extern _stdin
	extern _stdout
	extern _AppConfig
	extern _activeUDPSocket
	extern _UDPSocketInfo
	extern _UDPTxCount
	extern _UDPRxCount
	extern _LCDText
	extern _MAADR5
	extern _MAADR6
	extern _MAADR3
	extern _MAADR4
	extern _MAADR1
	extern _MAADR2
	extern _EBSTSD
	extern _EBSTCON
	extern _EBSTCS
	extern _EBSTCSL
	extern _EBSTCSH
	extern _MISTAT
	extern _EFLOCON
	extern _EPAUS
	extern _EPAUSL
	extern _EPAUSH
	extern _MACON1
	extern _MACON2
	extern _MACON3
	extern _MACON4
	extern _MABBIPG
	extern _MAIPG
	extern _MAIPGL
	extern _MAIPGH
	extern _MACLCON1
	extern _MACLCON2
	extern _MAMXFL
	extern _MAMXFLL
	extern _MAMXFLH
	extern _MICON
	extern _MICMD
	extern _MIREGADR
	extern _MIWR
	extern _MIWRL
	extern _MIWRH
	extern _MIRD
	extern _MIRDL
	extern _MIRDH
	extern _EHT0
	extern _EHT1
	extern _EHT2
	extern _EHT3
	extern _EHT4
	extern _EHT5
	extern _EHT6
	extern _EHT7
	extern _EPMM0
	extern _EPMM1
	extern _EPMM2
	extern _EPMM3
	extern _EPMM4
	extern _EPMM5
	extern _EPMM6
	extern _EPMM7
	extern _EPMCS
	extern _EPMCSL
	extern _EPMCSH
	extern _EPMO
	extern _EPMOL
	extern _EPMOH
	extern _EWOLIE
	extern _EWOLIR
	extern _ERXFCON
	extern _EPKTCNT
	extern _EWRPT
	extern _EWRPTL
	extern _EWRPTH
	extern _ETXST
	extern _ETXSTL
	extern _ETXSTH
	extern _ETXND
	extern _ETXNDL
	extern _ETXNDH
	extern _ERXST
	extern _ERXSTL
	extern _ERXSTH
	extern _ERXND
	extern _ERXNDL
	extern _ERXNDH
	extern _ERXRDPT
	extern _ERXRDPTL
	extern _ERXRDPTH
	extern _ERXWRPT
	extern _ERXWRPTL
	extern _ERXWRPTH
	extern _EDMAST
	extern _EDMASTL
	extern _EDMASTH
	extern _EDMAND
	extern _EDMANDL
	extern _EDMANDH
	extern _EDMADST
	extern _EDMADSTL
	extern _EDMADSTH
	extern _EDMACS
	extern _EDMACSL
	extern _EDMACSH
	extern _EIE
	extern _ESTAT
	extern _ECON2
	extern _EIR
	extern _EDATA
	extern _SSP2CON2
	extern _SSP2CON1
	extern _SSP2STAT
	extern _SSP2ADD
	extern _SSP2BUF
	extern _ECCP2DEL
	extern _ECCP2AS
	extern _ECCP3DEL
	extern _ECCP3AS
	extern _RCSTA2
	extern _TXSTA2
	extern _TXREG2
	extern _RCREG2
	extern _SPBRG2
	extern _CCP5CON
	extern _CCPR5
	extern _CCPR5L
	extern _CCPR5H
	extern _CCP4CON
	extern _CCPR4
	extern _CCPR4L
	extern _CCPR4H
	extern _T4CON
	extern _PR4
	extern _TMR4
	extern _ECCP1DEL
	extern _ERDPT
	extern _ERDPTL
	extern _ERDPTH
	extern _BAUDCON2
	extern _BAUDCTL2
	extern _SPBRGH2
	extern _BAUDCON
	extern _BAUDCON1
	extern _BAUDCTL
	extern _BAUDCTL1
	extern _SPBRGH
	extern _SPBRGH1
	extern _PORTA
	extern _PORTB
	extern _PORTC
	extern _PORTD
	extern _PORTE
	extern _PORTF
	extern _PORTG
	extern _PORTH
	extern _PORTJ
	extern _LATA
	extern _LATB
	extern _LATC
	extern _LATD
	extern _LATE
	extern _LATF
	extern _LATG
	extern _LATH
	extern _LATJ
	extern _DDRA
	extern _TRISA
	extern _DDRB
	extern _TRISB
	extern _DDRC
	extern _TRISC
	extern _DDRD
	extern _TRISD
	extern _DDRE
	extern _TRISE
	extern _DDRF
	extern _TRISF
	extern _DDRG
	extern _TRISG
	extern _DDRH
	extern _TRISH
	extern _DDRJ
	extern _TRISJ
	extern _OSCTUNE
	extern _MEMCON
	extern _PIE1
	extern _PIR1
	extern _IPR1
	extern _PIE2
	extern _PIR2
	extern _IPR2
	extern _PIE3
	extern _PIR3
	extern _IPR3
	extern _EECON1
	extern _EECON2
	extern _RCSTA
	extern _RCSTA1
	extern _TXSTA
	extern _TXSTA1
	extern _TXREG
	extern _TXREG1
	extern _RCREG
	extern _RCREG1
	extern _SPBRG
	extern _SPBRG1
	extern _PSPCON
	extern _T3CON
	extern _TMR3L
	extern _TMR3H
	extern _CMCON
	extern _CVRCON
	extern _ECCP1AS
	extern _CCP3CON
	extern _ECCP3CON
	extern _CCPR3
	extern _CCPR3L
	extern _CCPR3H
	extern _CCP2CON
	extern _ECCP2CON
	extern _CCPR2
	extern _CCPR2L
	extern _CCPR2H
	extern _CCP1CON
	extern _ECCP1CON
	extern _CCPR1
	extern _CCPR1L
	extern _CCPR1H
	extern _ADCON2
	extern _ADCON1
	extern _ADCON0
	extern _ADRES
	extern _ADRESL
	extern _ADRESH
	extern _SSP1CON2
	extern _SSPCON2
	extern _SSP1CON1
	extern _SSPCON1
	extern _SSP1STAT
	extern _SSPSTAT
	extern _SSP1ADD
	extern _SSPADD
	extern _SSP1BUF
	extern _SSPBUF
	extern _T2CON
	extern _PR2
	extern _TMR2
	extern _T1CON
	extern _TMR1L
	extern _TMR1H
	extern _RCON
	extern _WDTCON
	extern _ECON1
	extern _OSCCON
	extern _T0CON
	extern _TMR0L
	extern _TMR0H
	extern _STATUS
	extern _FSR2L
	extern _FSR2H
	extern _PLUSW2
	extern _PREINC2
	extern _POSTDEC2
	extern _POSTINC2
	extern _INDF2
	extern _BSR
	extern _FSR1L
	extern _FSR1H
	extern _PLUSW1
	extern _PREINC1
	extern _POSTDEC1
	extern _POSTINC1
	extern _INDF1
	extern _WREG
	extern _FSR0L
	extern _FSR0H
	extern _PLUSW0
	extern _PREINC0
	extern _POSTDEC0
	extern _POSTINC0
	extern _INDF0
	extern _INTCON3
	extern _INTCON2
	extern _INTCON
	extern _PROD
	extern _PRODL
	extern _PRODH
	extern _TABLAT
	extern _TBLPTR
	extern _TBLPTRL
	extern _TBLPTRH
	extern _TBLPTRU
	extern _PC
	extern _PCL
	extern _PCLATH
	extern _PCLATU
	extern _STKPTR
	extern _TOS
	extern _TOSL
	extern _TOSH
	extern _TOSU
	extern _strlen
	extern _socket
	extern _connect
	extern _send
	extern _recv
	extern _closesocket
	extern _DNSBeginUsage
	extern _DNSResolve
	extern _DNSIsResolved
	extern _DNSEndUsage
;--------------------------------------------------------
;	Equates to used internal registers
;--------------------------------------------------------
STATUS	equ	0xfd8
PCL	equ	0xff9
PCLATH	equ	0xffa
PCLATU	equ	0xffb
WREG	equ	0xfe8
FSR1L	equ	0xfe1
FSR2L	equ	0xfd9
POSTDEC1	equ	0xfe5
PREINC1	equ	0xfe4
PRODL	equ	0xff3


	idata
_ServerName	db	0x77, 0x77, 0x77, 0x2e, 0x67, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x2e, 0x63
	db	0x6f, 0x6d, 0x00
_sendRequest	db	0x47, 0x45, 0x54, 0x20, 0x2f, 0x73, 0x65, 0x61, 0x72, 0x63, 0x68, 0x3f
	db	0x61, 0x73, 0x5f, 0x71, 0x3d, 0x4d, 0x69, 0x63, 0x72, 0x6f, 0x63, 0x68
	db	0x69, 0x70, 0x26, 0x61, 0x73, 0x5f, 0x73, 0x69, 0x74, 0x65, 0x73, 0x65
	db	0x61, 0x72, 0x63, 0x68, 0x3d, 0x6d, 0x69, 0x63, 0x72, 0x6f, 0x63, 0x68
	db	0x69, 0x70, 0x2e, 0x63, 0x6f, 0x6d, 0x20, 0x48, 0x54, 0x54, 0x50, 0x2f
	db	0x31, 0x2e, 0x30, 0x0d, 0x0a, 0x48, 0x6f, 0x73, 0x74, 0x3a, 0x20, 0x77
	db	0x77, 0x77, 0x2e, 0x67, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x2e, 0x63, 0x6f
	db	0x6d, 0x0d, 0x0a, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x69, 0x6f
	db	0x6e, 0x3a, 0x20, 0x63, 0x6c, 0x6f, 0x73, 0x65, 0x0d, 0x0a, 0x0d, 0x0a
	db	0x00
_BerkeleyTCPClientDemo_BSDClientState_1_1	db	0x07


; Internal registers
.registers	udata_ovr	0x0000
r0x00	res	1
r0x01	res	1
r0x02	res	1
r0x03	res	1
r0x04	res	1
r0x05	res	1
r0x06	res	1
r0x07	res	1

udata_BerkeleyTCPClientDemo_0	udata
_BerkeleyTCPClientDemo_bsdClientSocket_1_1	res	1

udata_BerkeleyTCPClientDemo_1	udata
_BerkeleyTCPClientDemo_addr_1_1	res	16

udata_BerkeleyTCPClientDemo_2	udata
_BerkeleyTCPClientDemo_recvBuffer_1_1	res	9

;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
; I code from now on!
; ; Starting pCode block
S_BerkeleyTCPClientDemo__BerkeleyTCPClientDemo	code
_BerkeleyTCPClientDemo:
;	.line	85; BerkeleyTCPClientDemo.c	void BerkeleyTCPClientDemo(void)
	MOVFF	FSR2L, POSTDEC1
	MOVFF	FSR1L, FSR2L
	MOVFF	r0x00, POSTDEC1
	MOVFF	r0x01, POSTDEC1
	MOVFF	r0x02, POSTDEC1
	MOVFF	r0x03, POSTDEC1
	MOVFF	r0x04, POSTDEC1
	MOVFF	r0x05, POSTDEC1
;	.line	106; BerkeleyTCPClientDemo.c	switch(BSDClientState)
	MOVLW	0x08
	BANKSEL	_BerkeleyTCPClientDemo_BSDClientState_1_1
	SUBWF	_BerkeleyTCPClientDemo_BSDClientState_1_1, W, B
	BTFSC	STATUS, 0
	BRA	_00136_DS_
	MOVFF	r0x06, POSTDEC1
	MOVFF	r0x07, POSTDEC1
	CLRF	r0x07
	BANKSEL	_BerkeleyTCPClientDemo_BSDClientState_1_1
	RLCF	_BerkeleyTCPClientDemo_BSDClientState_1_1, W, B
	RLCF	r0x07, F
	RLCF	WREG, W
	RLCF	r0x07, F
	ANDLW	0xfc
	MOVWF	r0x06
	MOVLW	UPPER(_00150_DS_)
	MOVWF	PCLATU
	MOVLW	HIGH(_00150_DS_)
	MOVWF	PCLATH
	MOVLW	LOW(_00150_DS_)
	ADDWF	r0x06, F
	MOVF	r0x07, W
	ADDWFC	PCLATH, F
	BTFSC	STATUS, 0
	INCF	PCLATU, F
	MOVF	r0x06, W
	MOVFF	PREINC1, r0x07
	MOVFF	PREINC1, r0x06
	MOVWF	PCL
_00150_DS_:
	GOTO	_00105_DS_
	GOTO	_00108_DS_
	GOTO	_00113_DS_
	GOTO	_00116_DS_
	GOTO	_00119_DS_
	GOTO	_00128_DS_
	GOTO	_00130_DS_
	GOTO	_00131_DS_
_00105_DS_:
;	.line	109; BerkeleyTCPClientDemo.c	if(DNSBeginUsage())
	CALL	_DNSBeginUsage
	MOVWF	r0x00
	MOVF	r0x00, W
	BTFSC	STATUS, 2
	BRA	_00136_DS_
;	.line	112; BerkeleyTCPClientDemo.c	DNSResolve(ServerName, DNS_TYPE_A);
	MOVLW	HIGH(_ServerName)
	MOVWF	r0x01
	MOVLW	LOW(_ServerName)
	MOVWF	r0x00
	MOVLW	0x80
	MOVWF	r0x02
	MOVLW	0x01
	MOVWF	POSTDEC1
	MOVF	r0x02, W
	MOVWF	POSTDEC1
	MOVF	r0x01, W
	MOVWF	POSTDEC1
	MOVF	r0x00, W
	MOVWF	POSTDEC1
	CALL	_DNSResolve
	MOVLW	0x04
	ADDWF	FSR1L, F
;	.line	113; BerkeleyTCPClientDemo.c	BSDClientState = DNS_GET_RESULT;
	MOVLW	0x01
	BANKSEL	_BerkeleyTCPClientDemo_BSDClientState_1_1
	MOVWF	_BerkeleyTCPClientDemo_BSDClientState_1_1, B
;	.line	115; BerkeleyTCPClientDemo.c	break;
	BRA	_00136_DS_
_00108_DS_:
;	.line	118; BerkeleyTCPClientDemo.c	if(!DNSIsResolved((IP_ADDR*)&addr.sin_addr.S_un.S_addr))
	MOVLW	HIGH(_BerkeleyTCPClientDemo_addr_1_1 + 4)
	MOVWF	r0x01
	MOVLW	LOW(_BerkeleyTCPClientDemo_addr_1_1 + 4)
	MOVWF	r0x00
	MOVLW	0x80
	MOVWF	r0x02
	MOVF	r0x02, W
	MOVWF	POSTDEC1
	MOVF	r0x01, W
	MOVWF	POSTDEC1
	MOVF	r0x00, W
	MOVWF	POSTDEC1
	CALL	_DNSIsResolved
	MOVWF	r0x00
	MOVLW	0x03
	ADDWF	FSR1L, F
	MOVF	r0x00, W
	BTFSC	STATUS, 2
	BRA	_00136_DS_
;	.line	121; BerkeleyTCPClientDemo.c	if(!DNSEndUsage())
	CALL	_DNSEndUsage
	MOVWF	r0x00
	MOVF	r0x00, W
	BNZ	_00112_DS_
;	.line	126; BerkeleyTCPClientDemo.c	BSDClientState = BSD_DONE;
	MOVLW	0x07
	BANKSEL	_BerkeleyTCPClientDemo_BSDClientState_1_1
	MOVWF	_BerkeleyTCPClientDemo_BSDClientState_1_1, B
;	.line	127; BerkeleyTCPClientDemo.c	break;
	BRA	_00136_DS_
_00112_DS_:
;	.line	130; BerkeleyTCPClientDemo.c	BSDClientState = BSD_START;			
	MOVLW	0x02
	BANKSEL	_BerkeleyTCPClientDemo_BSDClientState_1_1
	MOVWF	_BerkeleyTCPClientDemo_BSDClientState_1_1, B
_00113_DS_:
;	.line	135; BerkeleyTCPClientDemo.c	if((bsdClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET )
	MOVLW	0x00
	MOVWF	POSTDEC1
	MOVLW	0x06
	MOVWF	POSTDEC1
	MOVLW	0x00
	MOVWF	POSTDEC1
	MOVLW	0x64
	MOVWF	POSTDEC1
	MOVLW	0x00
	MOVWF	POSTDEC1
	MOVLW	0x02
	MOVWF	POSTDEC1
	CALL	_socket
	MOVWF	r0x00
	MOVLW	0x06
	ADDWF	FSR1L, F
	MOVFF	r0x00, _BerkeleyTCPClientDemo_bsdClientSocket_1_1
	MOVF	r0x00, W
	XORLW	0xfe
	BNZ	_00115_DS_
;	.line	136; BerkeleyTCPClientDemo.c	return;
	BRA	_00136_DS_
_00115_DS_:
;	.line	145; BerkeleyTCPClientDemo.c	BSDClientState = BSD_CONNECT;
	MOVLW	0x03
	BANKSEL	_BerkeleyTCPClientDemo_BSDClientState_1_1
	MOVWF	_BerkeleyTCPClientDemo_BSDClientState_1_1, B
;	.line	146; BerkeleyTCPClientDemo.c	break;
	BRA	_00136_DS_
_00116_DS_:
;	.line	150; BerkeleyTCPClientDemo.c	addr.sin_port = PORTNUM;
	MOVLW	0x50
	BANKSEL	(_BerkeleyTCPClientDemo_addr_1_1 + 2)
	MOVWF	(_BerkeleyTCPClientDemo_addr_1_1 + 2), B
	BANKSEL	(_BerkeleyTCPClientDemo_addr_1_1 + 3)
	CLRF	(_BerkeleyTCPClientDemo_addr_1_1 + 3), B
;	.line	152; BerkeleyTCPClientDemo.c	if(connect( bsdClientSocket, (struct sockaddr*)&addr, addrlen) < 0)
	MOVLW	HIGH(_BerkeleyTCPClientDemo_addr_1_1)
	MOVWF	r0x01
	MOVLW	LOW(_BerkeleyTCPClientDemo_addr_1_1)
	MOVWF	r0x00
	MOVLW	0x80
	MOVWF	r0x02
	MOVLW	0x00
	MOVWF	POSTDEC1
	MOVLW	0x10
	MOVWF	POSTDEC1
	MOVF	r0x02, W
	MOVWF	POSTDEC1
	MOVF	r0x01, W
	MOVWF	POSTDEC1
	MOVF	r0x00, W
	MOVWF	POSTDEC1
	BANKSEL	_BerkeleyTCPClientDemo_bsdClientSocket_1_1
	MOVF	_BerkeleyTCPClientDemo_bsdClientSocket_1_1, W, B
	MOVWF	POSTDEC1
	CALL	_connect
	MOVWF	r0x00
	MOVFF	PRODL, r0x01
	MOVLW	0x06
	ADDWF	FSR1L, F
	BSF	STATUS, 0
	BTFSS	r0x01, 7
	BCF	STATUS, 0
	BNC	_00118_DS_
;	.line	153; BerkeleyTCPClientDemo.c	return;
	BRA	_00136_DS_
_00118_DS_:
;	.line	155; BerkeleyTCPClientDemo.c	BSDClientState = BSD_SEND;
	MOVLW	0x04
	BANKSEL	_BerkeleyTCPClientDemo_BSDClientState_1_1
	MOVWF	_BerkeleyTCPClientDemo_BSDClientState_1_1, B
_00119_DS_:
;	.line	160; BerkeleyTCPClientDemo.c	send(bsdClientSocket, (const char*)sendRequest, strlen((char*)sendRequest), 0);  
	MOVLW	HIGH(_sendRequest)
	MOVWF	r0x01
	MOVLW	LOW(_sendRequest)
	MOVWF	r0x00
	MOVLW	0x80
	MOVWF	r0x02
	MOVLW	HIGH(_sendRequest)
	MOVWF	r0x04
	MOVLW	LOW(_sendRequest)
	MOVWF	r0x03
	MOVLW	0x80
	MOVWF	r0x05
	MOVF	r0x05, W
	MOVWF	POSTDEC1
	MOVF	r0x04, W
	MOVWF	POSTDEC1
	MOVF	r0x03, W
	MOVWF	POSTDEC1
	CALL	_strlen
	MOVWF	r0x03
	MOVFF	PRODL, r0x04
	MOVLW	0x03
	ADDWF	FSR1L, F
	MOVLW	0x00
	MOVWF	POSTDEC1
	MOVLW	0x00
	MOVWF	POSTDEC1
	MOVF	r0x04, W
	MOVWF	POSTDEC1
	MOVF	r0x03, W
	MOVWF	POSTDEC1
	MOVF	r0x02, W
	MOVWF	POSTDEC1
	MOVF	r0x01, W
	MOVWF	POSTDEC1
	MOVF	r0x00, W
	MOVWF	POSTDEC1
	BANKSEL	_BerkeleyTCPClientDemo_bsdClientSocket_1_1
	MOVF	_BerkeleyTCPClientDemo_bsdClientSocket_1_1, W, B
	MOVWF	POSTDEC1
	CALL	_send
	MOVLW	0x08
	ADDWF	FSR1L, F
;	.line	161; BerkeleyTCPClientDemo.c	BSDClientState = BSD_OPERATION;
	MOVLW	0x05
	BANKSEL	_BerkeleyTCPClientDemo_BSDClientState_1_1
	MOVWF	_BerkeleyTCPClientDemo_BSDClientState_1_1, B
;	.line	162; BerkeleyTCPClientDemo.c	break;
	BRA	_00136_DS_
_00128_DS_:
;	.line	168; BerkeleyTCPClientDemo.c	i = recv(bsdClientSocket, recvBuffer, sizeof(recvBuffer)-1, 0); //get the data from the recv queue
	MOVLW	HIGH(_BerkeleyTCPClientDemo_recvBuffer_1_1)
	MOVWF	r0x01
	MOVLW	LOW(_BerkeleyTCPClientDemo_recvBuffer_1_1)
	MOVWF	r0x00
	MOVLW	0x80
	MOVWF	r0x02
	MOVLW	0x00
	MOVWF	POSTDEC1
	MOVLW	0x00
	MOVWF	POSTDEC1
	MOVLW	0x00
	MOVWF	POSTDEC1
	MOVLW	0x08
	MOVWF	POSTDEC1
	MOVF	r0x02, W
	MOVWF	POSTDEC1
	MOVF	r0x01, W
	MOVWF	POSTDEC1
	MOVF	r0x00, W
	MOVWF	POSTDEC1
	BANKSEL	_BerkeleyTCPClientDemo_bsdClientSocket_1_1
	MOVF	_BerkeleyTCPClientDemo_bsdClientSocket_1_1, W, B
	MOVWF	POSTDEC1
	CALL	_recv
	MOVWF	r0x00
	MOVFF	PRODL, r0x01
	MOVLW	0x08
	ADDWF	FSR1L, F
;	.line	170; BerkeleyTCPClientDemo.c	if(i == 0)
	MOVF	r0x00, W
	IORWF	r0x01, W
	BZ	_00136_DS_
;	.line	173; BerkeleyTCPClientDemo.c	if(i< 0) //error condition
	BSF	STATUS, 0
	BTFSS	r0x01, 7
	BCF	STATUS, 0
	BNC	_00124_DS_
;	.line	175; BerkeleyTCPClientDemo.c	BSDClientState = BSD_CLOSE;
	MOVLW	0x06
	BANKSEL	_BerkeleyTCPClientDemo_BSDClientState_1_1
	MOVWF	_BerkeleyTCPClientDemo_BSDClientState_1_1, B
;	.line	176; BerkeleyTCPClientDemo.c	break;
	BRA	_00136_DS_
_00124_DS_:
	BANKSEL	_BerkeleyTCPClientDemo_BSDClientState_1_1
;	.line	184; BerkeleyTCPClientDemo.c	if(BSDClientState == BSD_OPERATION)
	MOVF	_BerkeleyTCPClientDemo_BSDClientState_1_1, W, B
	XORLW	0x05
	BNZ	_00128_DS_
;	.line	185; BerkeleyTCPClientDemo.c	break;
	BRA	_00136_DS_
_00130_DS_:
	BANKSEL	_BerkeleyTCPClientDemo_bsdClientSocket_1_1
;	.line	190; BerkeleyTCPClientDemo.c	closesocket(bsdClientSocket);
	MOVF	_BerkeleyTCPClientDemo_bsdClientSocket_1_1, W, B
	MOVWF	POSTDEC1
	CALL	_closesocket
	INCF	FSR1L, F
;	.line	191; BerkeleyTCPClientDemo.c	BSDClientState = BSD_DONE;
	MOVLW	0x07
	BANKSEL	_BerkeleyTCPClientDemo_BSDClientState_1_1
	MOVWF	_BerkeleyTCPClientDemo_BSDClientState_1_1, B
_00131_DS_:
;	.line	195; BerkeleyTCPClientDemo.c	if(BUTTON2_IO == 0u)
	BTFSC	_PORTBbits, 1
	BRA	_00136_DS_
	BANKSEL	_BerkeleyTCPClientDemo_BSDClientState_1_1
;	.line	196; BerkeleyTCPClientDemo.c	BSDClientState = DNS_START_RESOLUTION;
	CLRF	_BerkeleyTCPClientDemo_BSDClientState_1_1, B
_00136_DS_:
;	.line	201; BerkeleyTCPClientDemo.c	}
	MOVFF	PREINC1, r0x05
	MOVFF	PREINC1, r0x04
	MOVFF	PREINC1, r0x03
	MOVFF	PREINC1, r0x02
	MOVFF	PREINC1, r0x01
	MOVFF	PREINC1, r0x00
	MOVFF	PREINC1, FSR2L
	RETURN	



; Statistics:
; code size:	  630 (0x0276) bytes ( 0.48%)
;           	  315 (0x013b) words
; udata size:	   26 (0x001a) bytes ( 0.68%)
; access size:	    8 (0x0008) bytes


	end
