#include <stdio.h>
#include "lpc.h"

/*#define ENCODE/**/
/*#define DECODE/**/

main(argc, argv)
  int argc;
  char **argv;
{
  int nbytes;
  unsigned char buf[160];
  lpcparams_t params;

  lpcstate_t state;

fprintf(stderr, "S = %d\n", sizeof params);
  lpc_init(&state);
#ifndef DECODE
  while ((nbytes = read(0, buf, sizeof(buf))) > 0) {
    lpc_analyze(buf, &params);
#else
  while ((nbytes = read(0, &params, sizeof(params))) > 0) {
#endif
#ifdef ENCODE
    write(1, &params, sizeof params);
#else
    lpc_synthesize(buf, &params, &state);
    write(1, buf, nbytes);
#endif
  }
}
