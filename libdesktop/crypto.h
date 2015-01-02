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

#define KEYDIR "keys/"  //key directory
#define BOOK "contacts.txt" //default addressbook filename
#define NONAME "guest" //default key
#define MYNAME "myself"   //default name of own key
#define MACLEN 4   //length of MAC field in bytes (32 bits)

//Packets types set
typedef enum{
        TYPE_UNKNOWN=0, //not defined
        TYPE_MELPE,     //speech data cbr
        TYPE_CODEC21,
        TYPE_LPC10,
        TYPE_MELP,
        TYPE_CODEC22,
        TYPE_CELP,
        TYPE_AMR,       //dtx
        TYPE_LPC,
        TYPE_GSMHR,
        TYPE_G723,
        TYPE_G729,
        TYPE_GSMEFR,
        TYPE_GSMFR,
        TYPE_ILBC,
        TYPE_BV16,
        TYPE_OPUS,      //vbr
        TYPE_SILK,      //vbr
        TYPE_SPEEX,     //vbr+robbust
        //encrypted  packets
        TYPE_KEY,       //key publication  500
        TYPE_KEYLAST,   //last key packet 504
        TYPE_CHAT,      //text packet for chat and invites 256
        TYPE_DAT,       //additional data (video, files) 506
                        
        TYPE_INV,       //invite autent packet 12
        TYPE_SYN,       //ctr synchro packet 8
        TYPE_REQ,       //step 1 of IKE 48
        TYPE_ANS,       //step 2 of IKE  80

        TYPE_ACK,       //step 3 of IKE  16
        TYPE_AUREQ,     //passphrase autenthication  18
        TYPE_AUANS,     //passphrase autenthication  24
        TYPE_AUACK,     //under pressing notifocation 6

        TYPE_VBR        //31: general for speach vbr codecs
        }TypeSet;

 //time tools
 unsigned int getmsec(void);
 int getsec(void);
 void psleep(int paus);
 //convert packet type and length
 short lenbytype(unsigned char type); //get length of packet by type
 unsigned char typebylen(short len);  //get type of packet by length
 void get_names(void); //print names and addresses of participants
 //process command string
 int get_opt(const char* str, char* option); //get data after specified option
 int get_keyname(const char* str, char* keyname); //find [keyname] field in the string
 int get_nickname(const char* str, char* nickname); //find #nickname field in the string
 //process key files
 int get_key(const char* keyname, unsigned char* key); //find {b54_key} in keyfile
 int get_keyinfo(const char* keyname, char* str); //find #info_string in keyfile
 int get_seckey(const char* name, unsigned char* seckey); //load ECHD secret from file
 void set_access(char* pas, unsigned char* keybody); //set access to secret key bt password
 int check_access(void);  //check current access matches to default secret key
 //process address book file
 int get_bookstr(const char* book, const char* name, char* str); //find [name]... string by name in bookfile
 //find sender's string by REQ packet's content
 int search_bookstr(const char* book, char* ourname, const unsigned char* buf, char* res);
 //autentificated key agreement lake SKEME protocol
 void reset_crp(void); //reset encryption
 int do_req(unsigned char* pkt); //send request
 void do_ans(int ok); //manual acception of incoming call
 int go_req(unsigned char* pkt); //process request and send answer
 int go_ans(unsigned char* pkt); //process answer and send ACK
 int go_ack(unsigned char* pkt); //process ACK
 //identification by common password
 int do_au(unsigned char* pkt);  //send AU1 request
 int go_aureq(unsigned char* pkt); //process AU1 and send AU12
 int go_auans(unsigned char* pkt); //process AU12  and send AU2
 int go_auack(unsigned char* pkt); //process AU2
 //encryption, decryption
 int do_data(unsigned char* pkt, unsigned char* type); //encrypt outgoing packet
 int go_data(unsigned char* pkt, short len); //decryptt incoming packet
 //key presentation
 int do_key(unsigned char* buf); //send public key packet
 int go_key(unsigned char* buf); //process incoming public key packet
 //ping
 int do_png(unsigned char* pkt); //sends ping request with full conter
 int go_png(unsigned char* pkt); //process incoming ping packet (corrects counter) and send answer for request
 //onion doubling
 int do_inv(unsigned char* pkt); //send onion diubling invite
 int go_inv(unsigned char* pkt); //process onion doubling invite
 //counter resynchronization
 int do_syn(unsigned char* pkt); //send synchro packet
 int go_syn(unsigned char* pkt); //process incoming synchro packet
 //packets wrapper
 int go_pkt(unsigned char* pkt, int len);
 //timestamp
 #ifdef _WIN32
 int gettimeofday(struct timeval *tv, struct timezone *tz);
 #endif
