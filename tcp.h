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

//sockets status
  typedef enum{
  SOCK_IDDL=0,  //0-no socket
  SOCK_WAIT_TOR, //1-waiting connection to Tor interface
  SOCK_WAIT_HELLO, //2-waiting SOCK5 hello
  SOCK_WAIT_HS, //3-waiting connection to HS
  SOCK_WAIT_HOST,  //4-waiting connection to host
  SOCK_READY, //5-socket ready to reading data
  SOCK_INUSE  //6-socket used for sending data
  }SockSet;

  //configuration
  int fndport(char* buf);
  //notification
  int get_ipif(unsigned int ip, unsigned short port);
  int get_sockstatus(void);
  //initialisation
  unsigned long get_local_if(void);
  int sock_init(void);
  int disconnect(void);
  void sock_close(char direction);
  //conection
  int do_connect(char* conadr);
  int connectudp(char* udpaddr);
  int connecttcp(char* tcpaddr);
  int connecttor(char* toraddr);
  //onion doubling
  void settimeout(int sec);
  void init_dbl(void);
  void reset_dbl(void);
  void reconecttor(void);
  void checkdouble(void);
  //reading
  int do_read(unsigned char* pkt);
  int readudpin(unsigned char* pkt);
  int readudpout(unsigned char* pkt);
  int readtcpout(unsigned char* pkt);
  int readtcpin(unsigned char* pkt);
  int tcpaccept(void);
  //sending
  int do_send(unsigned char* pkt, int len, char c);
  //Onion to UDP swithcing
  void do_stun(char* cmd);
  void do_nat(char* cmd);
  void stopudp(void);
  int setaddr(char* pkt);

