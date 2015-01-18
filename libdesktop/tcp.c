   // Contact: <torfone@ukr.net>
// Author: Van Gegel
//
// THIS IS A FREE SOFTWARE
//
// This software is released under GNU LGPL:
//
// * LGPL 3.0 <http://www.gnu.org/licenses/lgpl.html>
//
// You're free to copy, distribute and make commercial use
// of this software under the following conditions:
//
// * You have to cite the author (and copyright owner): Van Gegel
// * You have to provide a link to the author's Homepage: <http://torfone.org/>
//
///////////////////////////////////////////////
#define MAXTCPSIZE 512   //size of internet packet
#define DEFPORT 17447       //listening port
#define DEFWEBPORT 8000        //control port
#define SOCKS5_INTERFACE "127.0.0.1:9051"  //Tor intrface

#define CONTIMEOUT 10  //timeout in iddle state, sec
#define UDPTIMEOUT 30   //timeout in UDP established state
#define TCPTIMEOUT 10  //timeout in UDP established state
#define TORTIMEOUT 30  //timeout in UDP established state
#define DBLTIMEOUT 2 //timeout for send REQ duble

#define DEFSTUNPORT 3478 //port of STUN server for NAT traversal
#define NATTRIES 30;      //number of NAT traversal packets
#define NATINTERVAL 17  //interval between NAT penentrate packets 2^n microseconds
#define MAXPKTCNT 300  //packets for measure traffics bitrate


#include <stdio.h>

#ifdef _WIN32

#include <stddef.h>
#include <stdlib.h>
#include <basetsd.h>
#include <stdint.h>
#include <winsock2.h>
#include <windows.h>
#include <htons.h>
#define ioctl ioctlsocket
#define close closesocket
#define EWOULDBLOCK WSAEWOULDBLOCK  //no data for assync polling
#define ENOTCONN WSAENOTCONN        //wait for connection for assinc polling
#define ECONNRESET WSAECONNRESET    //no remote udp interface in local network

char sock_buf[32768];   //WSA sockets buffer


#else //linux

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/tcp.h>
#include <netdb.h>

#ifdef LINUX_FPU_FIX
#include <fpu_control.h>
#endif

#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "string.h"
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ifaddrs.h>

#endif


#ifndef INVALID_SOCKET
 #define INVALID_SOCKET -1
#endif

#ifndef SOCKET_ERROR
 #define SOCKET_ERROR -1
#endif



#include "libcrp.h"
#include "crypto.h"
#include "tcp.h"
#include "cntrls.h"
#include "codecs.h"
#include "sha1.h"
//#include "audio.h"

int web_listener=INVALID_SOCKET; //web listening socket
int web_sock=INVALID_SOCKET;   //web control socket
char web_sock_flag=0; //status of websocket connection
int tcp_listener=INVALID_SOCKET; //tcp listening socket
int tcp_insock=INVALID_SOCKET;  //tcp accepting socket (incoming)
int tcp_outsock=INVALID_SOCKET; //tcp connected socket (outgoing)
int udp_insock=INVALID_SOCKET;  //udp listening/incoming socket
int udp_outsock=INVALID_SOCKET;  //udp created socket (outgoing)
char tcp_insock_flag=0;  //status of tcp incoming connection
char tcp_outsock_flag=0; //status of tcp outgoing connection
char udp_insock_flag=0;  //status of tcp incomin connection/listener
char udp_outsock_flag=0; //status of udp outgoing/switch connection
char onion_flag=0; //flag of onion connection over Tor
int con_time=0; //timestamp of next connection reset
int rc_level=0; //trashhold for change slowest onion connection
int rc_state=0; //status of onion rate after receiving a packet
int rc_cnt=0;   //counter for change slowest onion connection
int rc_in=0;  //counter of incoming packets over tcp_in
int rc_out=0; //counter of incoming packets over tcp_out
int u_cnt=0; //counter of tries of NAT traversal
char d_flg=0; //flag of REQ dubles for sending
int bytes_sended=0; //traffic during current session (includes tcp/udp headers)
int bytes_received=0;
int pkt_counter=0; //packets for bitrate calculation
int last_sended=0; //bytes senden at last mesure time
int last_received=0;//received
int last_timestamp=0; //last measure timestamp
float up_bitrate=0; //upstream bitrate, Kbit/s
float down_bitrate=0; //downstream bitrate, Kbit/s

struct sockaddr_in saddrUDPTo;  //udp destination address
struct sockaddr_in saddrUDPFrom; //udp source  address
struct sockaddr_in saddrTCP;    //adress structure

unsigned char br_out[MAXTCPSIZE+2];	//TCP out_sock reading buffer
int tr_out=0, pr_out=0;		//Current packet: bytes too read, bytes readed
unsigned char br_in[MAXTCPSIZE+2];	//TCP in_sock reading buffer			//UDP buffer
int tr_in=0, pr_in=0;		//Current packet: bytes too read, bytes readed

unsigned char torbuf[48]; //socks5 request for Tor
short torbuflen=0; //length of stored request
char msgbuf[264]; //work buffer
char webmsgbuf[512]; //web message buffer

unsigned long naddrUDPlistener=INADDR_NONE; //UDP listener if
unsigned short portUDPlistener=0;
unsigned long naddrTCPlistener=INADDR_NONE; //TCP listener if
unsigned short portTCPlistener=0;

unsigned long naddrTor=INADDR_NONE; //Tor interface from configuration file
unsigned short portTor=0;
unsigned long naddrSTUN=INADDR_NONE; //STUN server interface for NAT traversal
unsigned short portSTUN=0;

unsigned long Their_naddrUDPint=INADDR_NONE; //Their local UDP interface for connection in local network
unsigned short Their_portUDPint=0;
unsigned long Their_naddrUDPext=INADDR_NONE; //Their external UDP interface for NAT traversal
unsigned short Their_portUDPext=0;
unsigned long Our_naddrUDPint=INADDR_NONE; //Our local UDP interface for connection in local network
unsigned short Our_portUDPint=0;
unsigned long Our_naddrUDPext=INADDR_NONE; //Our external UDP interface for NAT traversal
unsigned short Our_portUDPext=0;

unsigned long Our_naddrUDPloc=INADDR_NONE; //Our local system IP
unsigned short Our_portUDPloc=0; //Our UDP listener port

extern char crp_state;      //status of connection crypto-handshake (crypto.c)
extern unsigned int in_ctr; //counter of incoming packets (crypto.c)
extern char their_onion[32]; //remote onion adress (from connection command or from remote) (crypto.c)
extern char our_onion[32];   //our onion adress (from connection command or conf file) (crypto.c)
extern int bad_mac; //counter of bad autentificating packets (crypto.c)
extern char sound_loop; //sound test mode flag

//*****************************************************************************
//returns error reading socket
int getsockerr(void)
{
 #ifdef _WIN32
 return (WSAGetLastError());
 #else
 return (errno);
 #endif
}

//*****************************************************************************
//process address string: separate address part and return port number
int fndport(char* buf)
{
 //scan address string for port separator ':'
 char* pp=NULL;
 int port=0;
 
 pp=strchr(buf, ':'); //search separator
 if(pp) //if port specified
 {
  pp[0]=0; //replace it by strings terminator
  pp++;  //pointer to port string
  port=atoi(pp); //convert port to integer
 }
 return port; //returns port number
}

//*****************************************************************************
//inites sockets
//creates TCP and UDP listeners by configuration
int sock_init(void)
{
   unsigned long opt = 1; //for ioctl
   int flag=1; //for setsockopt
   int ret = 0; //error code
   unsigned long naddrTCP=0; //temporary IP adress
   unsigned short port=0;      //temporary port
   
   //Inites WSA for Windows
   #ifdef _WIN32
   //WSA inialize
    if (WSAStartup(0x202, (WSADATA *)&sock_buf[0]))
    {
     printf("WSAStartup error: %d\n", WSAGetLastError());
     return -1;
    }
   #endif

   //Scan ini file for Tor interface
  strcpy(msgbuf, "Tor_interface"); //parameter name
  parseconf(msgbuf); //get parameter from ini file
  portTor=fndport(msgbuf); //separate address from port
  naddrTor=inet_addr(msgbuf); //get Tor address
  strcpy(msgbuf, SOCKS5_INTERFACE); //set defaults
  if(!portTor) portTor=fndport(msgbuf); //set default Tor port if not specified in inifile
  if(naddrTor== INADDR_NONE) //if Tor address also not specified in ini
  {
   fndport(msgbuf);  //separate address from port
   naddrTor=inet_addr(msgbuf); //set default Tor address
  }
  //Scan ini file for Dobling Permission
  strcpy(msgbuf, "Tor_doubling"); //parameter name
  if(parseconf(msgbuf)>0) //get parameter from ini file
  {
   rc_level=atoi(msgbuf); //set dobling permission/level as integer
  }
  else rc_level=0;  //defaults: onion verification enabling bat dubling not

  //reset sockets and flags
  if(tcp_listener!=INVALID_SOCKET) close(tcp_listener);
  if(tcp_insock!=INVALID_SOCKET) close(tcp_insock);
  if(tcp_outsock!=INVALID_SOCKET) close(tcp_outsock);
  if(udp_insock!=INVALID_SOCKET) close(udp_insock);
  if(udp_outsock!=INVALID_SOCKET) close(tcp_insock);

  tcp_listener=INVALID_SOCKET;
  tcp_insock=INVALID_SOCKET;
  tcp_outsock=INVALID_SOCKET;
  udp_insock=INVALID_SOCKET;
  udp_outsock=INVALID_SOCKET;

  //states of sockets
  tcp_insock_flag=0;
  tcp_outsock_flag=0;
  udp_insock_flag=0;
  udp_outsock_flag=0;
  onion_flag=0; //0-not onion connection, 1-from unverified address, 2-verified

  //look for local interface
   Our_naddrUDPloc=get_local_if();
  //look for tcp listener enabled
  strcpy(msgbuf, "TCP_listen");
  if(parseconf(msgbuf)<=0) goto tryudp; //search in confile
  if(msgbuf[0]!='1') goto tryudp; //TCP listener not enabled

  //look interface for tcp-listener
  strcpy(msgbuf, "TCP_interface");
  if( parseconf(msgbuf)>0 ) //search in conf-file
  {
       port=fndport(msgbuf);  //check for port specified
       if(!port) port=DEFPORT; //if not use default
       naddrTCP=inet_addr(msgbuf); //check for IP-address
       if(naddrTCP!=INADDR_NONE) //if address is valid IP
       {  //create tcp socket
        if ((tcp_listener = socket(AF_INET, SOCK_STREAM, 0)) <0)
        {
         perror("Error TcpListener");
         tcp_listener=INVALID_SOCKET;
        }
        else
        {
         //unblock socket
         opt=1;
         ioctl(tcp_listener, FIONBIO, &opt);
         //bind socket to interface
         memset(&saddrTCP, 0, sizeof(saddrTCP));
	 saddrTCP.sin_family = AF_INET;
	 saddrTCP.sin_port = htons(port);
         saddrTCP.sin_addr.s_addr=naddrTCP;

         portTCPlistener = port;
         naddrTCPlistener = naddrTCP;
         setsockopt(tcp_listener, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag));
         if (bind(tcp_listener, (struct sockaddr*)&saddrTCP, sizeof(saddrTCP)) < 0)
         {
          perror("Error bind TCP");
          close(tcp_listener);
          tcp_listener=INVALID_SOCKET;
         }
         else
         { //set socket to listening mode
          listen(tcp_listener, 2);
          printf("TCP listen on %s:%d\r\n", inet_ntoa(saddrTCP.sin_addr), port);
          fflush(stdout);
          ret=1;
         }
       }
      }
  }
tryudp:

 //look for udp listener enabled
  strcpy(msgbuf, "UDP_listen");
  if(parseconf(msgbuf)<=0) goto trynext; //searc in conf-file
  if(msgbuf[0]!='1') goto trynext;  //UDP listener not enabled

 //look config for tcp-listener
  strcpy(msgbuf, "UDP_interface");
  if( parseconf(msgbuf)>0 )  //search interface address in conf-file
  {
       port=fndport(msgbuf);  //check for port speecified
       if(!port) port=DEFPORT;  //if not use default
       naddrTCP=inet_addr(msgbuf); //check for IP-address
       if(naddrTCP!=INADDR_NONE) //if IP valid
       {  //create udp socket
        if ((udp_insock = socket(AF_INET, SOCK_DGRAM, 0)) <0)
        {
         perror("Error UdpListener");
         udp_insock=INVALID_SOCKET;
        }
        else
        {
         //unblock socket
         opt=1;
         ioctl(udp_insock, FIONBIO, &opt);
         //bind to interface
         memset(&saddrTCP, 0, sizeof(saddrTCP));
	 saddrTCP.sin_family = AF_INET;
	 saddrTCP.sin_port = htons(port);
         saddrTCP.sin_addr.s_addr=naddrTCP;

         portUDPlistener = port;
         naddrUDPlistener = naddrTCP;
         setsockopt(udp_insock, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag));
         if (bind(udp_insock, (struct sockaddr*)&saddrTCP, sizeof(saddrTCP)) < 0)
         {
          perror("Error bind UDP");
          close(udp_insock);
          udp_insock=INVALID_SOCKET;
         }
         else
         {
          printf("UDP listen on %s:%d\r\n", inet_ntoa(saddrTCP.sin_addr), port);
          fflush(stdout);
          Our_portUDPloc=port;
          udp_insock_flag=SOCK_READY;
          ret+=2;
         }
       }
      }
  }

trynext:
  disconnect(); //reset all connections state

     //look interface for ctrl-listener
  strcpy(msgbuf, "WEB_interface");
  if( parseconf(msgbuf)>0 ) //search in conf-file
  {
       port=fndport(msgbuf);  //check for port specified
       if(!port) port=DEFWEBPORT; //if not use default
       naddrTCP=inet_addr(msgbuf); //check for IP-address
       if(naddrTCP!=INADDR_NONE) //if address is valid IP
       {  //create tcp socket
        if ((web_listener = socket(AF_INET, SOCK_STREAM, 0)) <0)
        {
         perror("Error WebListener");
         web_listener=INVALID_SOCKET;
        }
        else
        {
         //unblock socket
         opt=1;
         ioctl(web_listener, FIONBIO, &opt);
         //bind socket to interface
         memset(&saddrTCP, 0, sizeof(saddrTCP));
	 saddrTCP.sin_family = AF_INET;
	 saddrTCP.sin_port = htons(port);
         saddrTCP.sin_addr.s_addr=naddrTCP;

         setsockopt(web_listener, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag));
         if (bind(web_listener, (struct sockaddr*)&saddrTCP, sizeof(saddrTCP)) < 0)
         {
          perror("Error bind WEB");
          close(web_listener);
          web_listener=INVALID_SOCKET;
         }
         else
         { //set socket to listening mode
          listen(web_listener, 2);
          printf("WEB listen on %s:%d\r\n", inet_ntoa(saddrTCP.sin_addr), port);
          fflush(stdout);
         }
       }
      }
  }

  return ret; //listeners
}


//*****************************************************************************
int get_ipif(unsigned int ip, unsigned short port)
{
 web_printf("%d.%d.%d.%d", ip&0xFF, (ip>>8)&0xFF,
        (ip>>16)&0xFF, ip>>24);
 if(port) web_printf(":%d", (int)port);
 printf("\r\n");
 fflush(stdout);
}


//*****************************************************************************
int get_sockstatus(void)
{
 int sst=0; //status flags
 //local IP interface
 web_printf("Local IP if is ");
 get_ipif(Our_naddrUDPloc, 0);
 //UDP listener
 if(udp_insock!=INVALID_SOCKET)
 {
  sst+=1;
  web_printf("UDP listening IF is ");
  get_ipif(naddrUDPlistener, portUDPlistener);
 }
 //UDP outgoing socket
 if(udp_outsock!=INVALID_SOCKET)
 {
  sst+=2;
  web_printf("UDP socket now bound on port %d\r\n", Our_portUDPint);
  //External  UDP outgoing IP:port reported by STUN
  if((Our_naddrUDPext!=INADDR_NONE)&&Our_portUDPext)
  {
   web_printf("UDP external IF is ");
   get_ipif(Our_naddrUDPext, Our_portUDPext);
  }
 }
 //TCP listenet
 if(tcp_listener!=INVALID_SOCKET)
 {
  web_printf("TCP listening IF is ");
  get_ipif(naddrTCPlistener, portTCPlistener);
 }

 if(tcp_insock!=INVALID_SOCKET) sst+=4;
 if(tcp_outsock!=INVALID_SOCKET) sst+=8;
 return sst;
}


//*****************************************************************************
//disconnect all connections
int disconnect(void)
{
 //terminate udp incoming connection
 if(udp_insock!=INVALID_SOCKET) //if socket exist
 {
  if(udp_insock_flag==SOCK_INUSE) //if socket in use
  {
   if(crp_state>2)  //if connection was completely established
   {
    msgbuf[0]=0; //make finalise syn
    do_syn((unsigned char*)msgbuf);   //terminate encrypted connection
    sendto(udp_insock, msgbuf, 9, 0, &saddrUDPTo, sizeof(saddrUDPTo));
   }
   else //if connection in agreement stage
   {
    msgbuf[0]=0;
    do_inv((unsigned char*)msgbuf); //terminate unecrypted
    sendto(udp_insock, msgbuf, 13, 0, &saddrUDPTo, sizeof(saddrUDPTo));
   }
   web_printf("! Incoming UDP connection terminated\r\n");
   fflush(stdout);
  }
  udp_insock_flag=SOCK_READY; //set flag for listening socket
 }
 else udp_insock_flag=SOCK_IDDL; //no socket: set iddle flag

 //terminate udp outgoing connection
 if(udp_outsock!=INVALID_SOCKET) //if socket exist
 {
  if(udp_outsock_flag==SOCK_INUSE) //if socket in use
  {
   if(crp_state>2)  //if connection was completely established
   {
    msgbuf[0]=0; //make finalise syn
    do_syn((unsigned char*)msgbuf);   //terminate encrypted connection
    sendto(udp_outsock, msgbuf, 9, 0, &saddrUDPTo, sizeof(saddrUDPTo));
   }
   else //if connection in agreement stage
   {
    msgbuf[0]=0;
    do_inv((unsigned char*)msgbuf); //terminate unecrypted
    sendto(udp_outsock, msgbuf, 13, 0, &saddrUDPTo, sizeof(saddrUDPTo));
   }
   web_printf("! Outgoing UDP connection terminated\r\n");
   fflush(stdout);
  }
  close(udp_outsock);
  udp_outsock=INVALID_SOCKET;
  udp_outsock_flag=SOCK_IDDL; //set flag for iddle
 }

 //terminate tcp incoming connection
 if(tcp_insock!=INVALID_SOCKET) //if socket exist
 {
  if(tcp_insock_flag==SOCK_INUSE) //if socket in use
  {
   if(crp_state>2)  //if connection was completely established
   {
    msgbuf[0]=0; //make finalise syn
    do_syn((unsigned char*)msgbuf);   //terminate encrypted connection
    send(tcp_insock, msgbuf, 9, 0);
   }
   else //if connection in agreement stage
   {
    msgbuf[0]=0;
    do_inv((unsigned char*)msgbuf); //terminate unecrypted
    send(tcp_insock, msgbuf, 13, 0);
   }
   web_printf("! Incoming connection terminated\r\n");
   fflush(stdout);
  }
  close(tcp_insock);
  tcp_insock=INVALID_SOCKET;
  tcp_insock_flag=SOCK_IDDL; //set flag for iddle
 }

  //terminate tcp outgoing connection
 if(tcp_outsock!=INVALID_SOCKET) //if socket exist
 {
  if(tcp_outsock_flag==SOCK_WAIT_TOR)
  web_printf("! Failed attempt to connect to Tor\r\n");
  else if(tcp_outsock_flag==SOCK_WAIT_HELLO)
  web_printf("! No SOCKS5 on specified Tor interface\r\n");
  else if(tcp_outsock_flag==SOCK_WAIT_HS)
  web_printf("! Hidden Service unavaliable\r\n");
  else if(tcp_outsock_flag==SOCK_WAIT_HOST)
  web_printf("! Host service unavaliable\r\n");
  else if(tcp_outsock_flag==SOCK_INUSE) //if socket in use
  {
   if(crp_state>2)  //if connection was completely established
   {
    msgbuf[0]=0; //make finalise syn
    do_syn((unsigned char*)msgbuf);   //terminate encrypted connection
    send(tcp_outsock, msgbuf, 9, 0);
   }
   else //if connection in agreement stage
   {
    msgbuf[0]=0;
    do_inv((unsigned char*)msgbuf); //terminate unecrypted
    send(tcp_outsock, msgbuf, 13, 0);
   }
   web_printf("! Outgoing connection terminated\r\n");
   fflush(stdout);
  }
  close(tcp_outsock);
  tcp_outsock=INVALID_SOCKET;
  tcp_outsock_flag=SOCK_IDDL; //set flag for iddle
 }

 //if(!sound_loop) soundrec(0); //stop audio input
 reset_crp();  //reset encryption engine
 onion_flag=0; //reset onion flag

 Their_naddrUDPint=INADDR_NONE; //Their local UDP interface for connection in local network
 Their_portUDPint=0;
 Their_naddrUDPext=INADDR_NONE; //Their external UDP interface for NAT traversal
 Their_portUDPext=0;

 Our_naddrUDPext=INADDR_NONE; //Our external UDP interface for NAT traversal
 Our_portUDPext=0;
 Our_naddrUDPint=INADDR_NONE; //Our local UDP interface for connection in local network
 Our_portUDPint=0;

 //Notify traffic
 if(bytes_sended | bytes_received)
 {
  web_printf("Last session traffic: %0.3f MB sent, %0.3f MB received\r\n",
         (float)bytes_sended/1000000, (float)bytes_received/1000000);
  bytes_sended=0;
  bytes_received=0;
  pkt_counter=0;
  last_sended=0;
  last_received=0;
  last_timestamp=0;
  up_bitrate=0;
  down_bitrate=0;
  rc_cnt=0;
  rc_in=0;
  rc_out=0;
  u_cnt=0;
  d_flg=0;
 }

 //reload our onion adress specified in config file
 memset(our_onion, 0, sizeof(our_onion));
 strcpy((char*)msgbuf, "Our_onion");
 if( parseconf(msgbuf)>0 )  //search interface address in conf-file
 {
  msgbuf[255]=0;
  if( strlen((char*)msgbuf) <31 ) strcpy(our_onion, msgbuf);
 }
 return 0;
}


//*****************************************************************************
//close invalid socket for UDP (-1), TCP out(0) or TCP in(1)
void sock_close(char direction)
{
 //TCP out
 if(!direction)
 {
  if(tcp_insock!=INVALID_SOCKET)
  {
   close(tcp_outsock);
   tcp_outsock=INVALID_SOCKET;
   tcp_outsock_flag=0;
  }
  else disconnect();
 }
 //TCP in
 else if(direction==1)
 {
  if(tcp_outsock!=INVALID_SOCKET)
  {
   close(tcp_insock);
   tcp_insock=INVALID_SOCKET;
   tcp_insock_flag=0;
  }
  else disconnect();
 }
 //UDP out
 else
 {
  if((tcp_outsock!=INVALID_SOCKET)||(tcp_insock!=INVALID_SOCKET))
  {
   close(udp_outsock);
   udp_outsock=INVALID_SOCKET;
   udp_outsock_flag=0;
  }
  else disconnect();
 }

}



//*****************************************************************************
//initial connections wrapper by command_str
//returns 0 if OK else returns error code <0
int do_connect(char* conadr)
{
 char str[256];
 int i;

 //check for no connection established
 if(crp_state) return -5;
 //disconnect and reset all
 off_tx();
 disconnect();
 //search for transport/adress option in command string
 str[0]='U';  //for UDP transport
 i=get_opt(conadr, str);
 if(i>=0) return (connectudp(conadr)); //proceed UDP connection
 str[0]='T'; //for TCP
 i=get_opt(conadr, str);
 if(i>=0) return (connecttcp(conadr)); //proceed TCP connection
 str[0]='O'; //onion
 i=get_opt(conadr, str);
 if(i>0) return (connecttor(conadr));  //proceed Tor connection
 else return -4; //no transport or address specified

  //Todo: apply -Ccodec and -Jjitter from command line
 //after connection will be established

}

//*****************************************************************************
//set remote address to structure saddrUDPTo
//creates udp socket, bind to random port
//send [0] cmd to remote for connection request
//set udp_outsock_flag to SOCK_READY
int connectudp(char* udpaddr)
{
  unsigned long opt = 1;  //for ioctl
  int i;
  char c;
  unsigned long naddrTCP=0;  //temporary address and port
  unsigned short port=0;
  struct hostent *hh;  //for domain resolving

  //find adress in command
  msgbuf[0]='U';
  i=get_opt(udpaddr, msgbuf);
  if(i>0) //conection address is specified
  {
   //set remote addr to saddrUDPTo
   port=fndport(msgbuf);  //check for port speecified
   if(!port) port=DEFPORT;  //if not use default
   naddrTCP=inet_addr(msgbuf); //check for IP-address
   if(naddrTCP==INADDR_NONE) //if IP invalid, try resolve domain name
   {
    web_printf("Resolving, please wait...");
    fflush(stdout);
    hh = gethostbyname(msgbuf); //resolve domain name
    if (hh == 0) //no DNS reported
    {
     web_printf("! %s: unknown host\r\n", msgbuf);
     fflush(stderr);
     return -3;
    }
    //store resolved IP and notify it
    memcpy((char *) &naddrTCP, (char *) hh->h_addr, sizeof naddrTCP);
    //bcopy((char *) hh->h_addr, (char *) &naddrTCP, sizeof naddrTCP);
    saddrTCP.sin_addr.s_addr=naddrTCP;
    web_printf(" %s\r\n", inet_ntoa(saddrTCP.sin_addr));
    fflush(stdout);
   }
  }
  else if(i==0)  //connection address not specified: set remote addr:port to zero
  {  //this is for NAT traversal tries
   naddrTCP=INADDR_NONE;
   port=0;
  }
  else return -4; //specified not UDP address

  //set remote address/port to structure 'To'
  memset(&saddrUDPTo, 0, sizeof(saddrUDPTo));
  saddrUDPTo.sin_family = AF_INET;
  saddrUDPTo.sin_port = htons(port);
  saddrUDPTo.sin_addr.s_addr=naddrTCP;
  //clears structure 'From'
  memset(&saddrUDPFrom, 0, sizeof(saddrUDPFrom));
  saddrUDPFrom.sin_family = AF_INET;
  saddrUDPFrom.sin_port = 0;
  saddrUDPFrom.sin_addr.s_addr=INADDR_NONE;
  naddrTCP=INADDR_NONE;
  //check for udp socket alredy exist, creates new if not 
  if(udp_outsock==INVALID_SOCKET)
  {
   //generate random port
   randFetch((uchar*)msgbuf, 2);
   port=2000+(*((short*)msgbuf));
   if(port>65530) port/=2;
   //set port of UDP local interface
   Our_portUDPint=port;  //out UDP port in intranet
   Our_portUDPext=port; //also temporary set it as an external port for most NAT types
   //search interface address in conf-file or use any_addr
   strcpy(msgbuf, "UDP_interface"); //look config for local interface
   if( parseconf(msgbuf)<=0 ) naddrTCP=INADDR_ANY;
   else
   {
    //look for specified IP-interface for UDP
    fndport(msgbuf);  //check for port speecified
    naddrTCP=inet_addr(msgbuf); //check for IP-address
    if(naddrTCP==INADDR_NONE) //invalid IP specified in config
    {
     web_printf("! Invalid UDP_interface config!\r\n");
     fflush(stdout);
     return -2; //if IP valid
    }
   }

  //set UDP local interface
  if(naddrTCP==INADDR_ANY) Our_naddrUDPint=Our_naddrUDPloc; //set to local if
  else Our_naddrUDPint=naddrTCP; //from config

   //create udp out_socket
   if ((udp_outsock = socket(AF_INET, SOCK_DGRAM, 0)) <0)
   {
    perror("UdpOutgoing");
    udp_outsock=INVALID_SOCKET;
    return -1;
   }
   //unblock socket
   opt=1;
   ioctl(udp_outsock, FIONBIO, &opt);
   //prepare local IP interface
   memset(&saddrTCP, 0, sizeof(saddrTCP));
   saddrTCP.sin_family = AF_INET;
   saddrTCP.sin_port = htons(port);
   saddrTCP.sin_addr.s_addr=naddrTCP;

   //bind socket to locat UDP interface and random port
   if (bind(udp_outsock, (struct sockaddr*)&saddrTCP, sizeof(saddrTCP)) < 0)
   {
    perror("bind UDP");
    close(udp_outsock);
    udp_outsock=INVALID_SOCKET;
    return -1;
   }
  }

  //check for remote port specified (native outgoing udp connection inites)
  if(!saddrUDPTo.sin_port)
  {
   //port not specified: 
   //for swith from tcp to udp set ready state only (read but no send)
   udp_outsock_flag=SOCK_READY;
  }
  else
  {
   //for initial UDP connection set socket state as work (read and send)
   udp_outsock_flag=SOCK_INUSE; //set socket state for now
   web_printf("Connect over UDP\r\n");
   //send initial connection request to remote
   strncpy(msgbuf, udpaddr, 31); //use initial connection command with -Nname specified
   msgbuf[31]=0;
   i=do_req((unsigned char*)msgbuf); //send request
   if(i>0) i=do_data((unsigned char*)msgbuf, (unsigned char*)&c);
   //if onion/tcp connection exist: try switch to udp
   if(i>0) do_send((unsigned char*)msgbuf, i, c); //send packet over udp i bytes
  }
  settimeout(UDPTIMEOUT); //set time for disconnect
  return 0;
}


//*****************************************************************************
//initiate outgoing connection over TCP:
//check specified remote adress or resolve domain name
//create tcp socket, async it and connect to remote
//set socket status for waiting connection to host
int connecttcp(char* tcpaddr)
{
 struct hostent *hh; //for resolving
 unsigned long naddrTCP=0; //temporary IP, port
 unsigned short port=0;
 int flag=1;
 unsigned long opt=1; //for ioctl
 int i;

 //Check: remote address must be specified
 if(!tcpaddr[0]) return -4;
 //Check for outgoing tcp exist
 if(tcp_outsock!=INVALID_SOCKET)
 {
  close(tcp_outsock);
  tcp_outsock=INVALID_SOCKET;
  tcp_outsock_flag=0;
 }

  //search adress in command string
  msgbuf[0]='T';
  i=get_opt(tcpaddr, msgbuf);
  if(i>0) //conection address is specified
  {
   //get remote address from string
   port=fndport(msgbuf);  //check for port speecified
   if(!port) port=DEFPORT;  //if not use default
   naddrTCP=inet_addr(msgbuf); //check for IP-address
   if(naddrTCP==INADDR_NONE) //if IP invalid, try resolve domain name
   {
    web_printf("Resolving, please wait...");
    fflush(stdout);
    hh = gethostbyname(msgbuf); //resolve domain name
    if (hh == 0) //no DNS reported
    {
     web_printf("! %s: unknown host\r\n", msgbuf);
     return -3;
    }
    //notify
    memcpy((char *) &naddrTCP, (char *) hh->h_addr, sizeof naddrTCP);
    saddrTCP.sin_addr.s_addr=naddrTCP;
    web_printf(" %s\r\n", inet_ntoa(saddrTCP.sin_addr));
    fflush(stdout);
   }
  }
  else return -4; //specified not TCP address

  //Prepare remote addr structure
  memset(&saddrTCP, 0, sizeof(saddrTCP));
  saddrTCP.sin_family = AF_INET;
  saddrTCP.sin_port = htons(port);
  saddrTCP.sin_addr.s_addr=naddrTCP;

 //Create TCP socket
  tcp_outsock=socket(AF_INET, SOCK_STREAM, 0);
  if (tcp_outsock < 0)
  {
   perror("opening TCP outgoing socket");
   tcp_outsock=INVALID_SOCKET;
   return -4;
  }

 //disable Nagle algo
 if (setsockopt(tcp_outsock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)) < 0)
 {
  perror( "disable Nagle for accepted socket" );
  close(tcp_outsock);
  tcp_outsock=INVALID_SOCKET;
  return -5;
 }
 //unblock socket
 opt=1;
 ioctl(tcp_outsock, FIONBIO, &opt);
 //Connect to remote host asynchronosly
 connect(tcp_outsock, &saddrTCP, sizeof(saddrTCP));
 //init connection state
 tcp_outsock_flag=SOCK_WAIT_HOST; //set soccket status
 settimeout(TCPTIMEOUT);  //set timeout for waiting connection
 strncpy(msgbuf, tcpaddr, 31); //store command for do request after connection
 msgbuf[31]=0;
 web_printf("Connecting over TCP, please wait...\r\n");
 fflush(stdout);
 return 0;
}


//*****************************************************************************
//initiate outgoing connection over Tor:
//create tcp socket, async it and connect to Tor interface from ini file
//set socket status for waiting connection to Tor intrface
int connecttor(char* toraddr)
{
 unsigned short port=0;
 int flag=1;
 int i;
 unsigned long opt=1;

 //Check for Tor interface exist (loded fron conf in sock_init
 if((naddrTor==INADDR_NONE)||(!portTor))
 {
  web_printf("! Tor interface was not specified!\r\n");
  return 0;
 }

 //if this is a initial connection adress must be specified
 if(toraddr) //if address string specified
 {
  msgbuf[0]='O';
  i=get_opt(toraddr, msgbuf); //find -Oonion option
  if(i<=0) //if no addreses for onion in command string
  {
   web_printf("! Onion address must be specified!\r\n");
   return 0;
  }
  else
  {
   msgbuf[255]=0;
   if(strlen(msgbuf)<31) strcpy(their_onion, msgbuf);
   else
   {
    web_printf("! Onion address too long (max 31 chars)!\r\n");
    return 0;
   }
  }
 }

 //use their_onion for make sock5 request
 if(their_onion[0]) strncpy(msgbuf, their_onion, 31);
 else
 {
  web_printf("! Onion address is not specified!\r\n");
  return 0;
 }
 msgbuf[31]=0;
 //scan address string for port
 port=fndport(msgbuf);  //check for port speecified
 if(!port) port=DEFPORT;  //if not use default
 //add .onion suffix to address if suffix not specified
 if(!strchr(msgbuf,'.')) strcpy(msgbuf+strlen(msgbuf), ".onion");
 if(strlen(msgbuf)>31)
 {
  web_printf("! Address too long (must be less then 31 chars)\r\n");
  return 0;
 }
 //Make socks5 request in torbuf
 strcpy((char*)torbuf+5, msgbuf); //hostname or IP string
 i=4; //IPv4 Len
 torbuf[3]=0x01; //for IPv4 socks request
 //check for adress string is IP, replace string by integer
 if(inet_addr((const char*)torbuf+5)!= INADDR_NONE) (*(unsigned int *)(torbuf+4)) = (unsigned int) inet_addr((const char*)torbuf+5);
 else
 {
  i=strlen((const char*)torbuf+5); //length of hostname string
  torbuf[3]=0x03; //for hostname socks request
 }
 torbuf[4]=i;  //length of hostname or integer IP
 torbuf[0]=0x05; //socks5 ver
 torbuf[1]=0x01; //socks request type: connect
 torbuf[2]=0x00; //reserved
 torbuf[i+5]=(port>>8);
 torbuf[i+6]=(port&0xFF); //remote port
 torbuflen=i+7; //total length of request packet

 //close old socket if exist
 if(tcp_outsock!=INVALID_SOCKET)
 {
  //Tor not pass socket closing immediately, we must notify other side first
  i=0;
  send(tcp_outsock, &i, 1, 0); //send 0 for close socket on remote side during change doubling
  close(tcp_outsock);  //now closing socket
  tcp_outsock=INVALID_SOCKET;
  tcp_outsock_flag=0;
 }
 //Create new TCP socket
 tcp_outsock=socket(AF_INET, SOCK_STREAM, 0);
 if (tcp_outsock < 0)
 {
  perror("opening TCP outgoing socket");
  tcp_outsock=INVALID_SOCKET;
  return -4;
 }
 //disable nagle
 if (setsockopt(tcp_outsock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)) < 0)
 {
  perror( "disable Nagle for created socket" );
  close(tcp_outsock);
  tcp_outsock=INVALID_SOCKET;
  return -5;
 }
 //unblock socket
 opt=1;
 ioctl(tcp_outsock, FIONBIO, &opt);
 //Prepare remote addr structure for Tor interface
 memset(&saddrTCP, 0, sizeof(saddrTCP));
 saddrTCP.sin_family = AF_INET;
 saddrTCP.sin_port = htons(portTor);
 saddrTCP.sin_addr.s_addr=naddrTor;
 //Connect to Tor interface asynchronosly
 connect(tcp_outsock, &saddrTCP, sizeof(saddrTCP));
 //set socket status wor waiting connection to tor interface
 tcp_outsock_flag=SOCK_WAIT_TOR;
 if(!onion_flag) onion_flag=1;
 settimeout(DBLTIMEOUT);
 if(toraddr)
 {  //for initial connection, not for doubling reconections
  strncpy(msgbuf, toraddr, 31); //store command for do request after connection
  msgbuf[31]=0;
  web_printf("Connecting over Tor, please wait...\r\n");
  fflush (stdout);
 }
 return 0;
}
//*****************************************************************************

//reset onion doubling process:
//close existing incoming connection
//and send invite to remote
//remote must establish new connection to our onion
void reset_dbl(void)
{
 unsigned char bb[264];
 int l;
 char c;

 if((crp_state<3)||(!onion_flag))
 web_printf("! No onion connection, command impossible\r\n");
 else if(rc_level<=0)
 web_printf("! Doubling not permitted, set interval first\r\n");
 else if(!our_onion[0])
 web_printf("! Our onion adress is not specified in config file!\r\n");
 else if((tcp_outsock_flag!=SOCK_INUSE)||(tcp_outsock==INVALID_SOCKET))
 {
  web_printf("! Impossible: outgoing connection inactive now\r\n");
  web_printf("Ask the remote party to perform this command\r\n");
 }
 else
 {
  if(tcp_insock!=INVALID_SOCKET)
  {
   close(tcp_insock);
   tcp_insock=INVALID_SOCKET;
   tcp_insock_flag=0;
   web_printf("Incoming socket already exists, destroyed\r\n");
  }
  web_printf("Doubling request sended to remote party\r\n");
  rc_in=0; //reset packets counters for new measurement
  rc_out=0;
  rc_cnt=0;
  //make chat message for remote: ask him to reconect his outgoing channel
  memset(bb, 0, sizeof(bb));
  bb[0]=TYPE_CHAT|0x80; //chat header
  sprintf((char*)(bb+1), "-W%s", our_onion); //represent our onion
  l=do_data(bb, (unsigned char*)&c); //encrypt
  if(l>0) do_send(bb, l, c);  //send packet
  //now send SYN req over outgoing socket if it is in work
  bb[0]=1; //syn request must be generates
  if(0<do_syn(bb)) send(tcp_outsock, bb, 9, 0);
 }
}
//*****************************************************************************


//manually reinit onion doubling process:
//check for incoming connection exist and
//re-establish outgoing connection to remote onion
void init_dbl(void)
{
 if((crp_state<3)||(!onion_flag))
 web_printf("! No onion connection, command impossible\r\n");
 else if(rc_level<=0)
 web_printf("! Doubling not permitted, set interval first\r\n");
 else if(!their_onion[0])
 web_printf("! Our onion adress not received yet!\r\n");
 else if((tcp_insock_flag!=SOCK_INUSE)||(tcp_insock==INVALID_SOCKET))
 {
  web_printf("! Impossible: incoming connection inactive now\r\n");
  web_printf("Ask the remote party to perform this command\r\n");
 }
 else
 {
  web_printf("Tries to reconnect outgoing connection\r\n");
  connecttor(0); //close old connection and established new
  rc_cnt=0;
  rc_in=0;
  rc_out=0;
  msgbuf[0]=1; //syn request must be generates
  if(0<do_syn((unsigned char*)msgbuf)) send(tcp_insock, msgbuf, 9, 0);
 }
}


//*****************************************************************************
//initiate doubling reconection procedure
void reconecttor(void)
{
 //check for onion connection type,
 //incoming connection is exist
 // and doubling permissed
 if( onion_flag && (tcp_insock_flag==SOCK_INUSE) && (rc_level>=0) && (rc_cnt<=5) && (!u_cnt))
 {
  fflush(stdout);
  connecttor(0); //close old connection and established new
  rc_cnt=0;
  rc_in=0;
  rc_out=0;
  msgbuf[0]=1; //syn request must be generates
  if(0<do_syn((unsigned char*)msgbuf)) send(tcp_insock, msgbuf, 9, 0);
 }
}


//***************************************************************************
//Poll listener and accepts incoming TCP connections
int tcpaccept(void)
{
 int flag=1;
 int ll;
 int sTemp=INVALID_SOCKET; //accepted socket
 int oflag=0; //incoming from tor
 unsigned long opt=1;
 //struct linger ling;

 if(tcp_listener==INVALID_SOCKET) return 0;
 ll = sizeof(saddrTCP);
 //try accept incoming asynchronosly
 sTemp  = accept(tcp_listener, (struct sockaddr *) &saddrTCP, (socklen_t*)&ll);
 if(sTemp<=0) return -1; //no incoming connections accepted
 if(saddrTCP.sin_addr.s_addr==0x0100007F) //if incoming from localhost (Tor)
 {
  if(!crp_state) //only for initial incoming, not for doubling reconection
  web_printf("\r\nAccepted incoming from Tor\r\n");
  oflag=1; //set temporary onion flag
 }
 else //external incoming
 {
  web_printf("\r\nAccepted TCP from %s\r\n", inet_ntoa(saddrTCP.sin_addr));
  fflush(stdout);
 }

 //disable nagle on sTemp ->Err
 if (setsockopt(sTemp, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)) < 0)
 {
  perror( "disable Nagle for accepted socket" );
  close(sTemp);
  return -2;
 }
 //unblock socket
 opt=1;
 ioctl(sTemp, FIONBIO, &opt);

//check for connections already exist
ll=0; //busy flag
if(tcp_insock!=INVALID_SOCKET) ll=1; //another incoming exist: we are busy
if((tcp_outsock!=INVALID_SOCKET) && (!onion_flag)) ll=1; //outgoing exist: we are busy except onion doubling
if(onion_flag && (!oflag)) ll=1; //busy for external connect during onion seans
if(ll) //send busy notification and close
 {
  char buf[13];
  buf[0]=0x80|TYPE_INV;
  memset(buf+1,0, 12); //zeroed invite for busy notification
  if(13!=send(sTemp, (char*)(&ll), 13, 0)) perror("Sending Busy");//send busy nitification

  //close socket
  //ling.l_onoff  = TRUE;
  //ling.l_linger = 0;
  //setsockopt(sTemp, SOL_SOCKET, SO_LINGER, (char*) &ling, sizeof(ling));
  close(sTemp);
  web_printf("! Reject incoming: busy\r\n");
  return 1;
 }

 //set socket mode (ready for doubling, other inuse)
 if(tcp_outsock!=INVALID_SOCKET) tcp_insock_flag=SOCK_READY;
 else
 {
  tcp_insock_flag=SOCK_INUSE;
  if(!onion_flag) onion_flag=oflag; //set global onion flag by temp flag
 }

 //initial new incoming connection
 tcp_insock=sTemp;
 tr_in=0;  //bytes to read
 pr_in=0;  //bytes readed
 psleep(50);
 return 1;
}


//******************************************************************************
//packets sending wrapper
//c is char for replacing packets first byte
//(types header) in UDP mode. It containg type bit
//(fixed/variable length) and 7 synchrobits
//(low bits of conter) for decryption process
int do_send(unsigned char* pkt, int len, char c)
{
 //if UDP out_sock exist, sends only over it
 //if TCP in_sock or TCP out_sock esistes, send over one or both (doubling)
 //else if UDP in_sock active (INUSE mode) send over it

 int i=0;
 //check for udp out_sock inuse (outgoing UDP or switch from onion)
 if((udp_outsock!=INVALID_SOCKET)&&(udp_outsock_flag==SOCK_INUSE)&&(saddrUDPTo.sin_port))
 {
  pkt[0]=c; //set udp header and send udp
  sendto(udp_outsock, pkt, len, 0, &saddrUDPTo, sizeof(saddrUDPTo));
  bytes_sended+=(len+28);
  pkt_counter++;
  return 0; //this is incoming UDP direct, no other connection can be active at time
 }
 //check for tcp sockets in use (both can be)
 if((tcp_outsock!=INVALID_SOCKET)&&(tcp_outsock_flag==SOCK_INUSE))
 {
  send(tcp_outsock, pkt, len, 0);
  bytes_sended+=(len+40);
  pkt_counter++;
  i=1;
 }
 if((tcp_insock!=INVALID_SOCKET)&&(tcp_insock_flag==SOCK_INUSE))
 {
  send(tcp_insock, pkt, len, 0);
  bytes_sended+=(len+40);
  pkt_counter++;
  i=1;
 }
 if(i) return 0;

 //else ckeck for udp in socket (only one can be while incoming UDP direct)
 if((udp_insock!=INVALID_SOCKET)&&(udp_insock_flag==SOCK_INUSE)&&(saddrUDPTo.sin_port))
 {
  pkt[0]=c; //set udp header and send udp
  sendto(udp_insock, pkt, len, 0, &saddrUDPTo, sizeof(saddrUDPTo));
  bytes_sended+=(len+28);
  pkt_counter++;
 }
 return 0;
}

//***********************************************************************

//poll unblocked udp_in socket, returns result
int readudpin(unsigned char* pkt)
{
 int i; //data length or error code
 char b=0; //busy flag
 int l=sizeof(saddrTCP); //addr structure size

 //try read socket asynchronosly
 i=recvfrom(udp_insock, pkt, MAXTCPSIZE, 0, &saddrTCP, (socklen_t*)&l);
 if(i==SOCKET_ERROR) //error/no data
 {
  i=getsockerr();  //get error code
  if(i!=EWOULDBLOCK) //any socket erron except blocking
  {
   if(i==ECONNRESET) web_printf("! No remote services\r\n");
   else
   {
    //close socket
    close(udp_insock);
    udp_insock=INVALID_SOCKET;
    udp_insock_flag=0;
    web_printf("! udp_in socket error %d\r\nPlease restart to continue UDP listening!\r\n", i);
   }
  }
  return -2; //error
 }

 if(!i) return -1; //no data

 //---------------------------------------------
  //check for received packet is STUN answer
   if((pkt[0]==1)&&(pkt[1]==1)&&(i>31)&&(saddrTCP.sin_port==htons(portSTUN)))
   {
    unsigned short Our_portUDP;
    unsigned long Our_naddrUDP;
    Our_portUDP=htons(*(unsigned short*)(pkt+26)); //our external PORT reported by STUN
    Our_naddrUDP=(*(unsigned int*)(pkt+28)); //our external IP reported by STUN
    saddrTCP.sin_addr.s_addr=Our_naddrUDP; //for convertion IP to strin
    web_printf("STUN  report: UDP listener on %s:%d\r\n",
    inet_ntoa(saddrTCP.sin_addr), Our_portUDP);
    saddrTCP.sin_addr.s_addr=Our_naddrUDPloc; //our local IP discovered
    web_printf("Local report: UDP listener on %s:%d\r\n",
    inet_ntoa(saddrTCP.sin_addr), Our_portUDPloc);
    fflush(stdout);
    return -1;
   }
//-----------------------------------

 //some data received
 if( (udp_outsock!=INVALID_SOCKET)||
     (tcp_insock!=INVALID_SOCKET)||
     (tcp_outsock!=INVALID_SOCKET) )
 {
  b=1; //if any other socket exist set busy flag
 }

 //check for state: init new remote or check remote is same
 if(!crp_state)
 { //set current adress_from as contact address
  memcpy(&saddrUDPTo, &saddrTCP, sizeof(saddrUDPTo));
  udp_insock_flag=SOCK_INUSE; //set state in_use
 }
 //connection in progress: compare contact adress with current address_from
 else
 {
  if(saddrTCP.sin_port!=saddrUDPTo.sin_port) b=1;
  if(saddrTCP.sin_addr.s_addr!=saddrUDPTo.sin_addr.s_addr) b=1; //set busy flag for aliens
 }
 //send busy notification to remote
 if(b)
 {
  pkt[0]=TYPE_INV|0x80; //zeroed invite
  memset(pkt+1, 0, 12);
  sendto(udp_insock, pkt, 13, 0, &saddrTCP, sizeof(saddrTCP));
  return -1; //no data
 }

 //------------------------------------------------------------
 //check for received packet is invite
 if(i==13) //check length first
 {
  l=go_inv(pkt); //process invite
  //check for zeroed invite (remote is busy)
  if(l==-1)
  {
   if(crp_state<3) disconnect(); //remote terminates his call request
   web_printf("! Remote is busy\r\n"); //notify only
   return -1;
  }
 }
 //-------------------------------------------------------------
 //check for SYN there (crp_state>2)
 if((i==9)&&(crp_state>2))
 {

  if(0<go_syn(pkt)) sendto(udp_insock, pkt, 9, 0, &saddrTCP, sizeof(saddrTCP));
  else web_printf(" ping on UDP incoming\r\n");
  return -1;
 }
 bytes_received+=(i+28);
 pkt_counter++;
 return i; //return packet length
}



//************************************************************************

//poll unblocked udp_out socket, returns result
int readudpout(unsigned char* pkt)
{
 int i; //data length or error code
 char c;
 int l=sizeof(saddrTCP); //addr structure size

//===========================================
 //try read socket
 i=recvfrom(udp_outsock, pkt, MAXTCPSIZE, 0, &saddrTCP, (socklen_t*)&l);
 if(i==SOCKET_ERROR) //error/no data
 {
  i=getsockerr();  //get error code
  if(i!=EWOULDBLOCK) //any socket erron except blocking
  { //close socket
   if(i==ECONNRESET) web_printf("! No remote services\r\n");
   else
   {  //fatal errors
    web_printf("! udp_out socket error %d\r\n", i);
    sock_close(-1);
   }
  }
  return -2; //something wrong
 }
 if(!i) return -1; //no data
//================================================

   //some data received

//==============================================
 //check for exist tcp sockets:
 if( (tcp_insock!=INVALID_SOCKET)||
     (tcp_outsock!=INVALID_SOCKET) )
 {  //this is swith from tcp to udp
//------------------------------------------
  //check for connection already established
  if(crp_state<3) //connection must established for swith, else send busy
  {
   pkt[0]=TYPE_INV|0x80; //send zeroed invite
   memset(pkt+1, 0, 12);
   sendto(udp_outsock, pkt, 13, 0, &saddrTCP, sizeof(saddrTCP));
   return 0;
  }

//---------------------------------------------
  //check for received packet is STUN answer
   if(u_cnt&&(pkt[0]==1)&&(pkt[1]==1)&&(i>31)&&(saddrTCP.sin_port==htons(portSTUN)))
   {
    Our_portUDPext=htons(*(unsigned short*)(pkt+26)); //our external PORT reported by STUN
    Our_naddrUDPext=(*(unsigned int*)(pkt+28)); //our external IP reported by STUN
    //make chat packet with own external IP:port reported by STUN
    pkt[0]=TYPE_CHAT|0x80; //chat packet header
    sprintf((char*)(pkt+1), "-S%d.%d.%d.%d:%d",
        pkt[2+26], pkt[3+26], pkt[4+26], pkt[5+26],
        256*pkt[0+26]+pkt[1+26]);
    i=1+strlen((char*)(pkt+1)); //-S111.111.111.111:11111
    memset(pkt+i, 0, 256-i); //zeroed rest string
    i=lenbytype(TYPE_CHAT);
    if(i>0) i=do_data(pkt, (unsigned char*)&c); //encrypt
    if(i>0) do_send(pkt, i, c); //send packet
    return 0;
   }

//------------------------------------
  //check for received packet is SYN
  if(i==9)
  {
   if(u_cnt) //check for NAT traversal in progress and length matches
   {
    if(0<=go_syn(pkt)) //check for syn valid
    {
     //first UDP packet received from remote (NAT penentrated)
     //fix senders address as remote address for answering
     memcpy(&saddrUDPTo, &saddrTCP, sizeof(saddrUDPTo));
     //send answer over UDP for notify himself address:port for remote
     sendto(udp_outsock, pkt, 9, 0, &saddrTCP, sizeof(saddrTCP));
     pkt[0]=1; //make syn request for fixing timestamp
     do_syn(pkt);
     u_cnt=0; //terminate NAT travelsal procedure
     Their_portUDPint=0;  //clear their ports
     Their_portUDPext=0;
     udp_outsock_flag=SOCK_INUSE; //set socket status for regullar sending over it
     web_printf("! UDP socket in use now\r\n");
    } //if(0<=go_syn(pkt))
   } //if(u_cnt)
   else
   {  //!u_cnt: send SYN to answer
    if(0<go_syn(pkt)) sendto(udp_outsock, pkt, 9, 0, &saddrTCP, sizeof(saddrTCP));
    else web_printf(" ping on UDP outgoing\r\n");
   } //!u_cnt
   return 0;
  } //if(i==9)
//-------------------------------------
 } //if tcp sockets exist


       //this is native UDP connection
 //=====================================

 //check for sender valid
 if(memcmp(&saddrTCP, &saddrUDPTo, sizeof(saddrUDPTo)))
 {  //this is a packet from another sender then was specified on connect
  pkt[0]=TYPE_INV|0x80; //send zeroed invite back: busy
  memset(pkt+1, 0, 12);
  sendto(udp_outsock, pkt, 13, 0, &saddrTCP, sizeof(saddrTCP));
  return -1;
 }
 bytes_received+=(i+28);
 pkt_counter++;
 return i; //returns packets length
}


//****************************************************************************
//poll unblocked tcp_out socket, read stream, returns compleet packet length or 0
int readtcpout(unsigned char* pkt)
{
 int i; //data length or error code
 //check socket status, read all data if socket not ready
 if((!tcp_outsock_flag)||(tcp_outsock==INVALID_SOCKET)) return 0; //invalid socket
 else if(tcp_outsock_flag<SOCK_READY) //socket not ready: read all avaliable data
 {
  i=recv(tcp_outsock, br_out, MAXTCPSIZE, 0); //try read socket

  if(!i)  //no data but recv event
  {  //socket remotely closed (like eof)
   web_printf("! TCP outgoing connection closed remotely\r\n");
   sock_close(0);
   return 0;
  }

  if(i==SOCKET_ERROR) //error/no data
  {  //process error code
   i=getsockerr();  //get error code
   if(i==ENOTCONN) return 0; //no connection yet

   //ckeck for other fatal errors
   if(i!=EWOULDBLOCK)
   {
    web_printf("! TCP outgoing connection terminated remotely\r\n");
    sock_close(0);
   }

   //Event: connection established

   //TCP connection established: change socket's status and send crypto request
   if(tcp_outsock_flag==SOCK_WAIT_HOST) //if TCP mode
   {
    tcp_outsock_flag=SOCK_INUSE; //socket ready for tx/rx
    tr_out=0; //init bytes counter
    pr_out=0; //init bytes pointer
    //send initial connection crypto request to remote
    i=do_req((unsigned char*)msgbuf); //make and send request
    if(i>0)
    {
     i=send(tcp_outsock, msgbuf, i+5, 0);
     if(i<=0)
     {
      d_flg=SOCK_WAIT_HOST;
      settimeout(DBLTIMEOUT);
     }
     else d_flg=0;
    }
    else disconnect(); //if sender not in adress book
   }
   //Established connection with Tor: change status and send Hello
   else if(tcp_outsock_flag==SOCK_WAIT_TOR) //if Tor mode and connected to Tor
   {
    tcp_outsock_flag=SOCK_WAIT_HELLO; //wait hello from Tor
    //send socks5 hello
    i=send(tcp_outsock, torbuf, 3, 0);
    if(i<=0) d_flg=SOCK_WAIT_TOR;
    else d_flg=0;
   }
   return 0;  //no data for now, read again
  }

  //some data received: check for actual socket status for interprete data

  if(tcp_outsock_flag==SOCK_WAIT_HELLO) //if we are waiting Hello from Tor
  {
   //check for socks5 Hello pattern
   if( (i<2) || (i>9) || (br_out[0]!=5) || br_out[1] ) return 0;
   tcp_outsock_flag=SOCK_WAIT_HS; //wait connection to specified Hidden Service
   i=send(tcp_outsock, torbuf, torbuflen, 0); //send sock5 HS-request to Tor
   if(i>0) settimeout(TORTIMEOUT);
   return 0;
  }

  if(tcp_outsock_flag==SOCK_WAIT_HS) //if we are waiting connection to HS
  { //check for socks5 connection compleet pattern
   if((i<10)||(br_out[0]!=5)||(br_out[1])) return 0;
   if(tcp_insock==INVALID_SOCKET) //this is initil connection, send req
   {
    //send initial connection crypto request to remote
    tr_out=0; //init bytes counter
    pr_out=0; //init bytes pointer
    tcp_outsock_flag=SOCK_INUSE;
    i=do_req((unsigned char*)msgbuf); //make and send request
    if(i>0) send(tcp_outsock, msgbuf, i+5, 0);
    else disconnect(); //contact not in addressbook
   }
   else //this is doubling connection: incoming exist
   {
    //send doubling invite
    tr_out=0; //init bytes counter
    pr_out=0; //init bytes pointer
    tcp_outsock_flag=SOCK_READY; //wait for answer invite
    msgbuf[0]=1; //non-zero invite must be generates
    i=do_inv((unsigned char*)msgbuf); //generate answer invite
    if(i>0) send(tcp_outsock, msgbuf, 13, 0); //send it to remote over Tor
    else //key not agree
    {
     //Tor not pass socket closing immediately, we must notify other side first
     i=0;
     send(tcp_outsock, &i, 1, 0); //send 0 for close socket on remote side during change doubling
     web_printf("! No key agreement!\r\n");
     sock_close(0);
    }
   }
   return 0;
  }
  return 0;
 }

 //socket ready now: read one byte first (header) specifies length
 if(!tr_out) //no data in buffer: new packet estimates
 {
  i=recv(tcp_outsock, br_out, 1, 0); //read first byte
  if(!i) //socket remotely closed (like eof)
  {
   web_printf("! TCP outgoing connection closed remotely\r\n");
   sock_close(0);
   return 0;
  }
  //process error code
  if(i==SOCKET_ERROR) //error/no data
  {
   i=getsockerr();  //get error code
   //ckeck for other fatal errors
   if(i!=EWOULDBLOCK)
   {
    web_printf("! TCP outgoing connection terminated remotely\r\n");
    sock_close(0);
   }
   return 0;
  }
  if(i!=1) return 0; //one bytes must be readed

  //Check for !br[0] and close socket for onion doubling
  if(!br_out[0])
  {
    web_printf("! TCP outgoing closed remotely\r\n");
    sock_close(0);
    return 0;
  }

  //determines expected packet length by readed header and set tr_out
  if(br_out[0]>=0xC0) tr_out=4+lenbytype(1);  //for melpe packets
  else if(br_out[0]>=0x80) tr_out=4+lenbytype(br_out[0]); //cbr voice and ctrl packets
  else tr_out=4+(0x7F&br_out[0]); //vbr voice packets
  pr_out=1; //clear buffer pointer, first byte alredy readed
 } //if(!tr_out)

  //try to read rest of packets bytes
  i=recv(tcp_outsock, br_out+pr_out, tr_out, 0); //read up to all expected bytes of packet
  if(!i) //socket remotely closed (like eof)
  {
   web_printf("! TCP outgoing connection closed remotely\r\n");
   sock_close(0);
   return 0;
  }
  //process error code
  if(i==SOCKET_ERROR) //error/no data
  {
   i=getsockerr();  //get error code
   //ckeck for other fatal errors
   if(i!=EWOULDBLOCK)
   {
    web_printf("! TCP outgoing connection terminated remotely\r\n");
    sock_close(0);
   }
   return 0;
  }
  //corrects number of readed bytes
  tr_out-=i; //bytes to read
  pr_out+=i; //move buffer's pointer
  if(tr_out) return 0; //packet not compleet: read more bytes

  //compleet packet readed
  
  i=pr_out; //packet length
  tr_out=0; //clear counter for new packet
  pr_out=0; //clear buffer's pointer

 //check for SYN there (crp_state>2)
 if((i==9)&&(crp_state>2))
 {
  if(0<go_syn(br_out)) send(tcp_outsock, br_out, 9, 0);
  else web_printf(" ping on TCP outgoing\r\n");
  return 0;
 }

 //check for this is invite
 if((i==13)&&(tcp_outsock_flag==SOCK_READY)) //fixed length=13 for invite
 {
  i=go_inv(br_out); //check invite validity
  if(i==-1) //this is busy notify
  {
   web_printf("! Remote is busy\r\n");
   sock_close(0);
  }
  else if(i) //valid invite
  {
   //check this is the first invite
   //if yes, noyify their_onion is realy true
   if((onion_flag==1)&&(crp_state==4))
   {
    onion_flag=2;
    web_printf("Remote onion address'%s' verified\r\n", their_onion);
    fflush(stdout);
   }

   //there check for doubling permissed: time value T>0
   //if yes, set status INUSE
   //if no, fclose
   if(rc_level>0)
   {
    tcp_outsock_flag=SOCK_INUSE;
    web_printf(">>\r\n");  //note renew onion outgoing
   }
   else
   {
    //Tor not pass socket closing immediately, we must notify other side first
    i=0;
    send(tcp_outsock, &i, 1, 0); //send 0 for close socket on remote side during change doubling
    web_printf("! Connection closed because doubling is not permitted in config\r\n");
    sock_close(0);
   }
  }
  return 0;
 }
 memcpy(pkt, br_out, i); //copy received packet to output
 if(onion_flag)
 {
  rc_state=rc_cnt+1;//set state for receives data from tcp_out
  rc_out++; //increment counter of incoming packets received ovet tcp_out socket
 }
 bytes_received+=(i+40);
 pkt_counter++;
 return i+512;  //returns incoming packet length +512 (tcp flag)
}



//*****************************************************************************
//poll unblocked tcp_out socket, returns compleet packet length or 0
int readtcpin(unsigned char* pkt)
{
 int i; //data length or error code

 //check socket status, read all data if socket not ready
 if((tcp_insock_flag<SOCK_READY)||(tcp_insock==INVALID_SOCKET)) return 0; //invalid socket

 //socket ready: read one byte first (header) specifies length
 if(!tr_in) //no data in buffer: new packet estimates
 {
  i=recv(tcp_insock, br_in, 1, 0); //read first byte
  if(!i) //socket remotely closed (like eof)
  {
   web_printf("! TCP incoming connection closed remotely\r\n");
   sock_close(1);
   return 0;
  }
  //process error code
  if(i==SOCKET_ERROR) //error/no data
  {
   i=getsockerr();  //get error code
   //ckeck for other fatal errors
   if(i!=EWOULDBLOCK)
   {
    sock_close(1);
    web_printf("! TCP incoming connection terminated remotely\r\n");
   }
   return 0;
  }
  if(i!=1) return 0; //one bytes must be readed
  //Check for !br[0] and close socket for onion doubling
  if(!br_in[0])
  {
    sock_close(1);
    return 0;
  }

  //determines packets length set to tr_out
  if(br_in[0]>=0xC0) tr_in=4+lenbytype(1);  //for melpe packets
  else if(br_in[0]>=0x80) tr_in=4+lenbytype(br_in[0]); //cbr voice and ctrl
  else tr_in=4+(0x7F&br_in[0]); //for vbr voice packets
  pr_in=1; //clear buffer pointer, first byte alredy readed
 }

  //try to read rest of packets bytes
  
  i=recv(tcp_insock, br_in+pr_in, tr_in, 0); //read up to all expected bytes of packet
  if(!i) //socket remotely closed (like eof)
  {
   sock_close(1);
   web_printf("! TCP incoming connection closed remotely\r\n");
   return 0;
  }
  //process error code
  if(i==SOCKET_ERROR) //error/no data
  {
   i=getsockerr();  //get error code
   //ckeck for other fatal errors
   if(i!=EWOULDBLOCK)
   {
    sock_close(1);
    web_printf("! TCP incoming connection terminated remotely\r\n");
   }
   return 0;
  }
  tr_in-=i; //bytes to read
  pr_in+=i; //move buffer's pointer
  if(tr_in) return 0; //packet not compleet: read more bytes

  //Packet completely readed

  i=pr_in; //packet length
  tr_in=0; //clear counter for new packet
  pr_in=0; //clear buffer's pointer

 //check for SYN there (crp_state>2)
 if((i==9)&&(crp_state>2))
 {
  if(0<go_syn(br_in)) send(tcp_insock, br_in, 9, 0);
  else web_printf(" ping on TCP incoming\r\n");
  return 0;
 }

 //check for this is invite
 if((i==13)&&(tcp_insock_flag==SOCK_READY)) //fixed length=13 for invite
 {
  i=go_inv(br_in); //check invite validity
  if(i==-1) //this is busy notify
  {
   sock_close(1);
   web_printf("! Remote is busy\r\n");
  }
  else if(i && onion_flag) //valid invite
  {
   //send invite to answer
   msgbuf[0]=1; //non-zero invite must be generates
   i=do_inv((unsigned char*)msgbuf); //generate answer invite
   if(i>0) send(tcp_insock, msgbuf, 13, 0); //send it to remote over Tor
   else //key not agree
   {
    //Tor not pass socket closing immediately, we must notify other side first
    i=0;
    send(tcp_insock, &i, 1, 0); //send 0 for close socket on remote side during change doubling
    sock_close(1);
    return 0;
   }
   //check for doubling permissed  time value t>0
   //if yes, set status INUSE
   //if no, fclose
   if(rc_level>0)
   {
    web_printf("<<\r\n");  //note renew onion incoming
    tcp_insock_flag=SOCK_INUSE;
   }
   else
   {
    //Tor not pass socket closing immediately, we must notify other side first
    i=0;
    send(tcp_insock, &i, 1, 0); //send 0 for close socket on remote side during change doubling
    sock_close(1);
    web_printf("! Connecting closed because doubling is not permitted in config\r\n");
   }
  }
  return 0;
 }
 memcpy(pkt, br_in, i); //copy received packet to output
 if(onion_flag)
 {
  rc_state=rc_cnt-1; //set state for receives data from tcp_in
  rc_in++; //increment counter of incoming packets received ovet tcp_in socket
 }
 bytes_received+=(i+40);
 pkt_counter++;
 return i+512;  //returns incoming packet length +512 (TCP flag)
}


//*****************************************************************************
//socket polling and reading wrapper
int do_read(unsigned char* pkt)
{
 int i;
 struct timeval time1;

 //------------------------------------------------------
 //check for timeout of connection
 if(crp_state<3) //not connected or connection in progress
 {
  gettimeofday(&time1, NULL); //get time now
  if(time1.tv_sec>con_time) //compare with moment of connection started
  {
      if(d_flg==SOCK_WAIT_HOST) //duble REQ for TCP direct connection
      {
       d_flg=0;
       i=send(tcp_outsock, msgbuf, i+5, 0);
       if(i>5) settimeout(TCPTIMEOUT);
       return 0;
      }
      else if(d_flg==SOCK_WAIT_TOR) //Duble HELLO for connection to Tor SOCKS5
      {
       d_flg=0;
       i=send(tcp_outsock, torbuf, 3, 0);
       if(i==3) settimeout(DBLTIMEOUT);
       return 0;
      }
      else //Timeout: terminate all connections
      {
       disconnect();
       con_time=CONTIMEOUT+time1.tv_sec; //set global next disconnection timestamp
      }
  }
 }
 //--------------------------------------------------
 //NAT traversal daemon
 else if(u_cnt && onion_flag) //if NAT traversal in progress and connection is onion
 {  //at least one remote interface must be preseted by invite
  if(Their_portUDPext || Their_portUDPint)
  {
   gettimeofday(&time1, NULL); //get time now
   i=time1.tv_usec>>NATINTERVAL; //divise to interpacket interval
   i=i&1; //compare time bit and packets counter's LSB
   if((u_cnt&1)!=i) //if not equal: send next packet and decrement counter
   {
    //prepare adress structure of remote
    memset(&saddrTCP, 0, sizeof(saddrTCP));
    saddrTCP.sin_family = AF_INET;

    /*
    //VARIANT 1: INTERLEAVE
    //packet will be sended to their internal or external interface
    if(i)
    {
     saddrTCP.sin_port = htons(Their_portUDPint);
     saddrTCP.sin_addr.s_addr=Their_naddrUDPint;
    }
    else  //interleave packets to external and internal remote interface
    {
     saddrTCP.sin_port = htons(Their_portUDPext);
     saddrTCP.sin_addr.s_addr=Their_naddrUDPext;
    }
    */

    //VARIANT 2: one int then ext
    if(Their_portUDPint)
    {
     saddrTCP.sin_port = htons(Their_portUDPint);
     saddrTCP.sin_addr.s_addr=Their_naddrUDPint;
     Their_portUDPint=0;
    }
    else
    {
     saddrTCP.sin_port = htons(Their_portUDPext);
     saddrTCP.sin_addr.s_addr=Their_naddrUDPext;
    }

    //make and send SYN packet for NAT traversal
    if((saddrTCP.sin_port)&&(saddrTCP.sin_addr.s_addr!=INADDR_NONE)&&(udp_outsock!=INVALID_SOCKET))
    {
     msgbuf[0]=1; //syn request must be generates
     i=do_syn((unsigned char*)msgbuf); //send UDP over our NAT to remote NAT
     if(i>0) i=sendto(udp_outsock, msgbuf, 9, 0, &saddrTCP, sizeof(saddrTCP));
    }
    u_cnt--; //decrement packets to send
    //exhausted: unsuccessful 
    if(!u_cnt) //terminate NAT traversal if all packets sended
    {
     Their_portUDPint=0;  //clear ports
     Their_portUDPext=0;
    } //if(!u_cnt)
   } //if((u_cnt&1)!=i)
  }  //if(Their_portUDPext || Their_portUDPint)
 }   //else if(u_cnt && onion_flag)
 else if(pkt_counter>MAXPKTCNT)
 { //on work: computes bitrate
  pkt_counter=0; //clear packets counter for measure next interval
  i=getsec()-last_timestamp; //measurement interval in seconds
  last_timestamp+=i; //fix current timestamp for next measurement
  up_bitrate=8*(bytes_sended-last_sended); //bits sended during this interval
  last_sended=bytes_sended; //fix current outgoing traffic value
  up_bitrate/=i; //computes bitrate for 1 sec
  down_bitrate=8*(bytes_received-last_received); //for downstream
  last_received=bytes_received;
  down_bitrate/=i;
 }
 else if(bad_mac>64)
 {
  web_printf("! Lost crypto synchronization, trying to restore...\r\n");
  bad_mac=60; //a little decrease bad counter for next
  in_ctr+=128; //move input counter out of synchro window
  pkt[0]=0x98; //emulate received syn request packet type:
  memset(pkt+1, 0, 8); //syn answer will be senden
  return 9; //SYN packet length
 }
 //---------------------------------------------------
 //ring signal
 if(crp_state==2)
 {
  //with 1/4 silency, 1/2 signal and 1/4 silency
  playring();
 }

 //------------------------------------------------------
 //changing onion dowbling slowes connection measurement
 //check for difference beetwen good packet received first by incoming and outgoing
 if( (rc_cnt>rc_level)&&(rc_level>0) )
 { //incoming too slow
  unsigned char bb[264];
  int l;
  char c;

  //anti-freezing of doubling: if there is no traffic over tcp_in
  //and there is some traffic over tcp_out and both sockets are in work state
  //then tcp_in freezed and must be destroyed because
  //in this state tcp_in can't be closed remotely
  //and we can't receives new incoming connections
  if((!rc_in) && rc_out && (tcp_outsock_flag==SOCK_INUSE)
        && (tcp_insock_flag==SOCK_INUSE))
  {
   close(tcp_insock);
   tcp_insock=INVALID_SOCKET;
   tcp_insock_flag=0;
   web_printf("! Incoming socket is frozen, destroyed\r\n");
  }

  rc_in=0; //reset packets counters for new measurement
  rc_out=0;
  //make chat message for remote: ask him to reconect his outgoing channel
  memset(bb, 0, sizeof(bb));
  bb[0]=TYPE_CHAT|0x80; //chat header
  sprintf((char*)(bb+1), "-W%s", our_onion); //represent our onion
  l=do_data(bb, (unsigned char*)&c); //encrypt
  if(l>0) do_send(bb, l, c);  //send packet
  //decrease counter for next measurement
  rc_cnt=rc_level/2;
  //now send SYN req over outgoing socket if it is in work
  if(tcp_outsock_flag==SOCK_INUSE)
  {
   bb[0]=1; //syn request must be generates
   if(0<do_syn(bb)) send(tcp_outsock, bb, 9, 0);
  }
 }  //restrict minimal counter value if outgoing too slow
     //only other part must check this and send reconection invite for us
 else if(rc_cnt<(-rc_level)) rc_cnt=(-(rc_level/2));

 //------------------------------------------------------
 //check for udp sockets exist and poll
 if(udp_outsock!=INVALID_SOCKET)
 {
  i=readudpout(pkt);
  if(i>0) return i;
 }
 //-------------------------------------------------------
 if(udp_insock!=INVALID_SOCKET) //this is also udp listener
 {
  i=readudpin(pkt);
  if(i>0) return i;
 }
 //------------------------------------------------------
 //poll tcp listener and accept tcp_insock
 if(tcp_listener) tcpaccept();
//--------------------------------------------------------
 //check for exist and poll tcp sockets
 if(tcp_outsock!=INVALID_SOCKET)
 {
  i=readtcpout(pkt);
  if(i>0) return i;
 }
//----------------------------------------------------------
 if(tcp_insock!=INVALID_SOCKET)
 {
  i=readtcpin(pkt);
  if(i>0) return i;
 }

 //int do_read(unsigned char* pkt)
 if(web_listener!=INVALID_SOCKET) webaccept();
 if(web_sock!=INVALID_SOCKET) readweb();

 return 0;
}


//*****************************************************************************
//reset conection timeout (called from connection init procedure)
void settimeout(int sec)
{
 struct timeval time1;
 gettimeofday(&time1, NULL); //get time now
 con_time=sec+time1.tv_sec;
}


//*****************************************************************************
//check for incoming onion connection significantly slowest then outgoing
//and send onion invite to remote using chat for initiate remote reconect
void checkdouble(void)
{
 //check for onion connection established, doubling permitted and our onion specified
 if( onion_flag && (rc_level>0) && our_onion[0])
 {
  rc_cnt=rc_state; //set counter by current state after data were readed
 }
}


//*****************************************************************************
//send STUN request for info
void do_stun(char* cmd)
{

 if(udp_insock==INVALID_SOCKET)
 {
  web_printf("! Unavaliable: UDP listener disabled by config!\r\n");
  return;
 }
 strncpy(msgbuf, cmd, 255);
 msgbuf[255]=0;
 naddrSTUN=INADDR_NONE; //reset STUN interface IP and port
 portSTUN=0;
 if(!msgbuf[0]) //STUN interface  is not specified in command
 { //search STUN interface in conf-file
  strcpy(msgbuf, "STUN_server"); //look config for STUN interface
  if( parseconf(msgbuf)<=0 ) msgbuf[0]=0; //not found
 }
 //check for STUN server was specified in command or ini-file
 if(!msgbuf[0])
 {
  web_printf("! STUN server is not specified!\r\n");
  return;
 }
  //determines STUN IP and port from string
  portSTUN=fndport(msgbuf);  //check for port speecified
  if(!portSTUN) portSTUN=DEFSTUNPORT;  //if not use default
  naddrSTUN=inet_addr(msgbuf); //check for IP-address
  if(naddrSTUN==INADDR_NONE) //if IP invalid, try resolve domain name
  {
   struct hostent *hh;
   web_printf("Resolving STUN, please wait...");
   fflush(stdout);
   hh = gethostbyname(msgbuf); //resolve domain name
   if (hh == 0) //no DNS reported
   {
    web_printf("! %s: unknown STUN\r\n", msgbuf);
   }
   else  //IP finded
   {
    memcpy((char *) &naddrSTUN, (char *) hh->h_addr, sizeof naddrSTUN);
    saddrTCP.sin_addr.s_addr=naddrSTUN; //STUN IP
    web_printf(" %s\r\n", inet_ntoa(saddrTCP.sin_addr));
    fflush(stdout);
   }
  } //if(naddrTCP==INADDR_NONE)
   //send stun request over it
  if(portSTUN && (naddrSTUN!=INADDR_NONE))
  {
   //prepare STUN request
   memset(msgbuf, 0, 28);
   msgbuf[1]=1; //type: request
   msgbuf[3]=8; //len of atributes
   msgbuf[21]=3; //Change
   msgbuf[23]=4; //len of value
   randFetch((unsigned char*)(msgbuf+4), 16); //random data
   //prepare adress structure of STUN server address
   memset(&saddrTCP, 0, sizeof(saddrTCP));
   saddrTCP.sin_family = AF_INET;
   saddrTCP.sin_port = htons(portSTUN); //port of used STUN server
   saddrTCP.sin_addr.s_addr=naddrSTUN; //IP adress of used STUN server
   //send STUN request
   sendto(udp_insock, msgbuf, 28, 0, &saddrTCP, sizeof(saddrTCP));
  } //if(portSTUN && (naddrSTUN!=INADDR_NONE))
}



//*****************************************************************************
//if command specified resolve STUN interface from it or from ini-file
//send request of our external IP to STUN server
//send NAT-traversal invite with local IP to remote
void do_nat(char* cmd)
{
 int i;
 char c;

 //check for onion connection established
 if((!onion_flag)||(crp_state<3)) return;
 //ckeck for TCP channel exist
 if((tcp_outsock_flag!=SOCK_INUSE)&&(tcp_insock_flag!=SOCK_INUSE)) return;

 //look for STUN address in command string then in config file
  msgbuf[0]='S'; //searh STUN adress in commands string
  i=get_opt(cmd, msgbuf);
  if(i<0) return; //no -S command specified
  naddrSTUN=INADDR_NONE; //reset STUN interface IP and port
  portSTUN=0;
  if(!i) //STUN interface  is not specified in command
  { //search STUN interface in conf-file
   strcpy(msgbuf, "STUN_server"); //look config for STUN interface
   if( parseconf(msgbuf)<=0 ) msgbuf[0]=0; //not found
  }
  //check for STUN server was specified in command or ini-file
  if(!msgbuf[0])
  {
   web_printf("! STUN server is not specified!\r\n");
   return;
  }
  else if(msgbuf!='0') //check for STUN disabled
  {
   //determines STUN IP and port from string
   portSTUN=fndport(msgbuf);  //check for port speecified
   if(!portSTUN) portSTUN=DEFSTUNPORT;  //if not use default
   naddrSTUN=inet_addr(msgbuf); //check for IP-address
   if(naddrSTUN==INADDR_NONE) //if IP invalid, try resolve domain name
   {
    struct hostent *hh;
    web_printf("Resolving STUN, please wait...");
    fflush(stdout);
    hh = gethostbyname(msgbuf); //resolve domain name
    if (hh == 0) //no DNS reported
    {
     web_printf("! %s: unknown STUN\r\n", msgbuf);
     fflush(stderr);
    }
    else  //IP finded
    {
     memcpy((char *) &naddrSTUN, (char *) hh->h_addr, sizeof naddrSTUN);
     saddrTCP.sin_addr.s_addr=naddrSTUN; //STUN IP
     web_printf(" %s\r\n", inet_ntoa(saddrTCP.sin_addr));
     fflush(stdout);
    }
   } //if(naddrTCP==INADDR_NONE)
  } //if(msgbuf!='0')
  //creates outgoing UDP socket if it is not created before
  if(udp_outsock==INVALID_SOCKET) connectudp("-U");
  //check for udp socket is ready
  if((udp_outsock==INVALID_SOCKET)||(udp_outsock_flag!=SOCK_READY)) return;

  //send stun request over it
  if(portSTUN && (naddrSTUN!=INADDR_NONE))
  {
   //prepare STUN request
   memset(msgbuf, 0, 28);
   msgbuf[1]=1; //type: request
   msgbuf[3]=8; //len of atributes
   msgbuf[21]=3; //Change
   msgbuf[23]=4; //len of value
   randFetch((unsigned char*)(msgbuf+4), 16); //random data
   //prepare adress structure of STUN server address
   memset(&saddrTCP, 0, sizeof(saddrTCP));
   saddrTCP.sin_family = AF_INET;
   saddrTCP.sin_port = htons(portSTUN); //port of used STUN server
   saddrTCP.sin_addr.s_addr=naddrSTUN; //IP adress of used STUN server
   //send STUN request
   sendto(udp_outsock, msgbuf, 28, 0, &saddrTCP, sizeof(saddrTCP));
  }

  //make chat packet contains string -Uudpaddr:udpport
  xmemset(msgbuf, 0, 256); //clear for safe
  (*(unsigned char*)msgbuf)=TYPE_CHAT|0x80; //chat header
  saddrTCP.sin_addr.s_addr=Our_naddrUDPint; //our local UDP interfaces IP address
  sprintf((char*)(msgbuf+1), "-U%s:%d", inet_ntoa(saddrTCP.sin_addr), Our_portUDPint);
  i=do_data((unsigned char*)msgbuf, (unsigned char*)&c); //encrypt answer, returns pkt len
  if(i>0) do_send((unsigned char*)msgbuf, i, c); //send answer

  //set number of NAT penentrates packets to send
  u_cnt=NATTRIES;
}


//*****************************************************************************
//set their internal or internal UDP interface by incoming -S or -U invite 
int setaddr(char* pkt)
{
 int i;
 int ext=0; //flag of external addresses invite received
 unsigned short port=0;
 unsigned long naddr=INADDR_NONE;

  //parse remote addr:port from incoming chat UDP invite
  msgbuf[0]='U'; //search remote's local adress in commands string
  i=get_opt(pkt, msgbuf);
  if(i<0)
  {
   msgbuf[0]='S'; //search remote's external adress in commands string
   i=get_opt(pkt, msgbuf);
   ext=1;
  }
  if(i<1) return -2; //no -U or -S options or parameter

  //convert string to integers
  port=fndport(msgbuf);  //check for port speecified
  if(!port) port=DEFPORT;  //if not use default
  naddr=inet_addr(msgbuf); //check for IP-address valid
  if(naddr==INADDR_NONE) return -1;

  //set their internal or external interface
  if(ext)
  {
   Their_naddrUDPext=naddr;
   Their_portUDPext=port;
  }
  else
  {
   Their_naddrUDPint=naddr;
   Their_portUDPint=port;
  }
  return 0;
}


//*****************************************************************************
//terminates UDP direct connection and swith back to onion
void stopudp(void)
{
 if((crp_state>2)&&(onion_flag)) //only in onion mode
 {
  int i;
  char c;
  if(udp_outsock) close(udp_outsock); //close UDP socket
  udp_outsock=INVALID_SOCKET;
  udp_outsock_flag=0;
  Their_portUDPint=0;  //clear ports
  Their_portUDPext=0;
  u_cnt=0;  //terminates NAT traversal procedure
  //udp packets contains synchro bits in header and
  //synchronised decryption himself if lost ocures
  //TCP is lossless transport but while UDP to TCP switches
  //we must synchronize counter by first TCP SYN packet after
  //self-synchronized udp packets and not use UDP after this
  msgbuf[0]=1; //syn request must be generates
  i=do_syn((unsigned char*)msgbuf);   //synchronize in_counter first
  if(i>0) i=do_data((unsigned char*)msgbuf, (unsigned char*)&c);
  //send first TCP after UDP: this is a SYN
  if(i>0) do_send((unsigned char*)msgbuf, i, c); 
  web_printf("    Stop UDP");
 }
}


//*****************************************************************************
//returns local ip specified in config or discovered in system
unsigned long get_local_if(void)
{
 char str[256];
 unsigned long our_ip=INADDR_NONE;
 int i;
 
 strcpy(str, "Local_interface"); //parameter name
 i=parseconf(str); //get parameter from ini file
 if((i>0)&&(str[0]!='0')) our_ip=inet_addr(str);
 
 if(our_ip==INADDR_NONE)
 {
   struct sockaddr_in *paddr;
#ifdef _WIN32
 //look for local interface
   struct hostent *hh;
   //get hostname first
   if(gethostname(str, sizeof(str)) != SOCKET_ERROR)
   {
    hh = gethostbyname(str); //get local IP and copy to naddrUDP
    if(hh) 
    {
     i=0;
     while(i<1) //check for mingw bag!!! bcb compiler OK!
     {
      paddr->sin_addr.s_addr=*(unsigned long*)hh->h_addr_list[i++];
      our_ip=paddr->sin_addr.s_addr; //for inet_ntoa only
      saddrTCP.sin_addr.s_addr=our_ip;
      web_printf("Local interface: %s\r\n", inet_ntoa(saddrTCP.sin_addr) ); //notify
     }     
    }
   }
#else //Linux
   struct ifaddrs *addrs, *tmp;
    
   if(0==getifaddrs(&addrs))
   {
    tmp=addrs;
    while(tmp)
    {
     if(tmp->ifa_addr && tmp->ifa_addr->sa_family==AF_INET)
     {
      paddr=(struct sockaddr_in *)tmp->ifa_addr;
      our_ip=paddr->sin_addr.s_addr;
      web_printf("%s: %s\r\n", tmp->ifa_name, inet_ntoa(paddr->sin_addr));
     }
     tmp=tmp->ifa_next;   
    }
    freeifaddrs(addrs);
   }
#endif
 }
 return our_ip;
}

  //***************************************************************************
//Poll web listener and accepts incoming WEBSOCKET connections
int webaccept(void)
{
 int flag=1;
 int ll;
 int sTemp=INVALID_SOCKET; //accepted socket
 unsigned long opt=1;
 //struct linger ling;

 if(web_listener==INVALID_SOCKET) return 0;
 ll = sizeof(saddrTCP);
 //try accept incoming asynchronosly
 sTemp  = accept(web_listener, (struct sockaddr *) &saddrTCP, (socklen_t*)&ll);
 if(sTemp<=0) return -1; //no incoming connections accepted
 if(saddrTCP.sin_addr.s_addr!=0x0100007F) //if incoming not from localhost (Tor)
 {
  printf("\r\nWarning: external control\r\n");
  //close(sTemp);
  //return -1;
 }
 else printf("\r\nAccepted control\r\n");

 //disable nagle on sTemp ->Err
 if (setsockopt(sTemp, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)) < 0)
 {
  perror( "Error disabling Nagle for accepted control" );
  close(sTemp);
  return -2;
 }

 //unblock socket
 opt=1;
 ioctl(sTemp, FIONBIO, &opt);

 //close old control connection
 if(web_sock!=INVALID_SOCKET)
 {
  close(web_sock);
  printf("Existed control closed\r\n");
 }

 //set new control connection
 web_sock=sTemp;
 web_sock_flag=0; //clear ready flag: no protocol defined
 psleep(1);
 return 1;
}



//*****************************************************************************
//poll unblocked tcp_out socket, returns compleet packet length or 0
int readweb(void)
{
 int l; //data length or error code
 char* p=0;
 //check socket status, read all data if socket not ready
 if(web_sock==INVALID_SOCKET) return 0; //invalid socket

  l=recv(web_sock, webmsgbuf, sizeof(webmsgbuf)-1, 0); //read all bytes
  if(!l) //socket remotely closed (like eof)
  {
   printf("Web control closed remotely\r\n");
   close(web_sock);
   web_sock=INVALID_SOCKET;
   web_sock_flag=0;
   return 0;
  }
  //process error code
  if(l==SOCKET_ERROR) //error/no data
  {
   l=getsockerr();  //get error code
   //ckeck for other fatal errors
   if(l!=EWOULDBLOCK)
   {
    close(web_sock);
    web_sock=INVALID_SOCKET;
    web_sock_flag=0;
    printf("web control terminated remotely\r\n");
   }
   return 0;
  }
  //terminate string
  webmsgbuf[l]=0;
  //check for handshake
  if(!web_sock_flag) //if protocol not specified yet
  {
   if((webmsgbuf[0]=='-')||(webmsgbuf[0]=='#')) web_sock_flag=SOCK_INUSE; //set telnet mode
   else p=strstr(webmsgbuf, "Sec-WebSocket-Key: "); //search for websock handshake
   if(p) //prepare answer for websock handshake
   {
    unsigned char h[20];
    strcpy(p+43, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"); //UID
    sha1((const unsigned char*)(p+19), 60, h ); //sha1(key | UID)
    //doing answer: add http header
    strcpy(webmsgbuf, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept:");
    //add accept data (sha in base64 format)
    p=webmsgbuf+strlen(webmsgbuf);
    b64estr(h, 20, p);
    p[0]=32;  //skip bracket {
    //skip barcket }, add extra: header's tail
    strcpy(webmsgbuf+strlen(webmsgbuf)-1, "\r\nSec-WebSocket-Protocol: chat\r\n\r\n");
    send(web_sock, webmsgbuf, strlen(webmsgbuf), 0); //send answer
    web_sock_flag=SOCK_READY; //set websocket mode
   }
  }
  else if(web_sock_flag==SOCK_INUSE) //parse telnet command
  {
   setcmd(webmsgbuf); //apply remote command
  }
  else if(l>1)//parse websock command
  {
   int i, j;
   p=webmsgbuf+2; //data pointer for short packet
   i=0x7F&webmsgbuf[1];  //data length for short packet
   if(i>125) //this is a long packet
   {
    i=webmsgbuf[3]<<8+webmsgbuf[4]; //data length for long packet
    p+=2; //skip 2 byte length
   }
   if(0x80&webmsgbuf[1]) //check for mask
   {
    p+=4; //skip 4 mask bytes
    for(j=0;j<i;j++) p[j]^=*(p-4+(j%4));  //demask
   }
   if(l!=(p-webmsgbuf+i)) return 0; //check for actual length is correct
   if((unsigned char)(webmsgbuf[0])==0x88) //check for close packet
   {
    webmsgbuf[1]=0;  //for our close packet to answer
    send(web_sock, webmsgbuf, 2, 0); //send close
    close(web_sock);   //close control socket
    web_sock=INVALID_SOCKET;
    web_sock_flag=0;
    printf("web control gracefully closed by remote\r\n");
   }
   else if(((unsigned char)(webmsgbuf[0])==0x89)&&(i<126)) //check for ping packet
   {  //only short ping precessed
    webmsgbuf[0]=0x8A; //set pong type
    webmsgbuf[1]=0x80+i; //set data length
    for(j=0;j<i;j++) webmsgbuf[2+j]=p[j]; //copy data from ping
    send(web_sock, webmsgbuf, i+2, 0); //send pong to answer
   }
   else if((unsigned char)(webmsgbuf[0])==0x81) //check for text packet
   {
    p[i]=0; //terminate string
    setcmd(p); //apply remote command
   }
  }
 return 0;
}

//*****************************************************************************
//send data over control socket
int sendweb(char* str)
{
 int l;
 //check for socket exist
 if(web_sock==INVALID_SOCKET) return -1;
 //check max data length
 strncpy(webmsgbuf, str, 511);
 webmsgbuf[511]=0;
 l=strlen(webmsgbuf);
 if(l>(sizeof(webmsgbuf)-5)) return -2;
 //websock protocol
 if(web_sock_flag==SOCK_READY)
 {
  webmsgbuf[0]=0x81; //header for final (one frame) text data packet
  if(l<126) //short packet
  {
   webmsgbuf[1]=l; //7 bits length
   strcpy(webmsgbuf+2, str); //payload
   l+=2;
  }
  else  //long packet
  {
   webmsgbuf[1]=126;  //flag of long packet
   webmsgbuf[2]=l>>8;
   webmsgbuf[3]=l&0xFF; //16 bits length
   strcpy(webmsgbuf+4, str); //payload
   l+=4;
  }
  send(web_sock, webmsgbuf, l, 0); //send packet to websocket
  return 0;
 }
 else if(web_sock_flag==SOCK_INUSE) //telnet mode
 {
  send(web_sock, str, l, 0);  //send raw data string
  return 0;
 }
 return -3; //there was no handshake
}


