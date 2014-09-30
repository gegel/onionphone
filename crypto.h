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
#define MACLEN 4

void test_crypto_sha(void);
void test_crypto_curve25519_donna(void);
void test_crypto_serpent(void);
void test_crypto_pkdf(void);
void ed25519_test(void);
void homqv_test(void);
void ooake_test(void);
void voake_test(void);
void odoake_test(void);
void fhmqv_test(void);
void TDH_test(void);


//Codecs set
typedef enum{
        TYPE_UNKNOWN=0,  //not defined
        TYPE_MELPE, //speech data cbr
        TYPE_CODEC21,
        TYPE_LPC10,
        TYPE_MELP,
        TYPE_CODEC22,
        TYPE_CELP,
        TYPE_AMR,  //dtx
        TYPE_LPC,
        TYPE_GSMHR,
        TYPE_G723,
        TYPE_G729,
        TYPE_GSMEFR,
        TYPE_GSMFR,
        TYPE_ILBC,
        TYPE_BV16,
        TYPE_OPUS,    //vbr
        TYPE_SILK,    //vbr
        TYPE_SPEEX,   //vbr+robbust
                       //encrypted  packets
        TYPE_KEY,     //key publication  500
        TYPE_KEYLAST,  //last key packet 504
        TYPE_CHAT,    //text packet for chat and invites 256
                     //not encrypted
        TYPE_INV,    //22: invite autent packet 12
        TYPE_AGR,     //agree compleet 20
        TYPE_SYN,     //ctr synchro packet 8
        TYPE_REQ,     //connection reauest (DH key agreement) 84
        TYPE_ANS,     //connection answer (DH key agreement)  80

        TYPE_ACK,     //deniable autentification  48
        TYPE_AUREQ,   //passphrase autentification  16
        TYPE_AUANS,   //passphrase autentification  22
        TYPE_AUACK,   //under pressing notofocation 6

        TYPE_VBR      //31:general for speach vbr codecs
        }TypeSet;

 unsigned long getmsec(void);
 long getsec(void);
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
 void do_ans(void);
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

 int do_inv(unsigned char* pkt);
 int go_inv(unsigned char* pkt);
 int go_pkt(unsigned char* pkt, int len);

 //counter resynchronization
 int do_syn(unsigned char* pkt); //send synchro packet
 int go_syn(unsigned char* pkt); //process incoming synchro packet


 #ifdef _WIN32
 int gettimeofday(struct timeval *tv, struct timezone *tz);
 #endif
