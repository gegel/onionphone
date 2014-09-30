/*_____________________
 |                     |
 | Basic types.        |
 |_____________________|
*/

#if defined(__BORLANDC__) || defined(__MINGW32__) || defined(__WATCOMC__) || defined(_MSC_VER) || defined(__ZTC__)
typedef short Word16;
typedef long Word32;
typedef int Flag;

#elif defined(__sun)
typedef short Word16;
typedef long Word32;
typedef int Flag;

#elif defined(__unix__) || defined(__unix)
typedef short Word16;
typedef int Word32;
typedef int Flag;

#endif
