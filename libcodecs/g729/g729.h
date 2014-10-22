/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */


void g729ini(int rate, int dtx);
int g729enc(short *sp16, unsigned char *br);
void g729dec(unsigned char *br, short *sp16);
