/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2000
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation; either version 2.1 of the License, or
 *   (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#ifndef _defines
#define _defines

/* Calculate the byte offset of a field in a structure of type type. */
#define FIELDOFFSET(type, field)    ((INT)&(((type *)0)->field))

#if !defined (dimof)
#define dimof(a) (sizeof (a)/sizeof (a[0]))
#endif

#ifndef OMNI_MINMAX_DEFINED
#define OMNI_MINMAX_DEFINED 1
namespace omni {
   template <class T> inline T min (T a, T b) { return (a) < (b) ? (a) : (b); }
   template <class T> inline T max (T a, T b) { return (a) > (b) ? (a) : (b); }
}
#endif

#define SAFE_PRINT_STRING(pstring)   (pstring ? *pstring : "(null)")
#define SAFE_PRINT_PSZ(pszString)    (pszString ? pszString : "(null)")

typedef char *PSZ;
typedef const char * const PSZCRO;
typedef const char *PSZRO;
typedef const unsigned char * const PUCHCRO;
typedef const unsigned char *PUCHRO;
typedef unsigned char byte, BYTE, *PBYTE;
typedef short int SHORT, *PSHORT;
typedef unsigned short int USHORT, *PUSHORT;
typedef unsigned long ULONG, *PULONG;
typedef int INT, *PINT;
typedef bool BOOL, *PBOOL;
typedef int LONG, *PLONG;

typedef struct _Sizel {
   int cx;
   int cy;
} SIZEL, *PSIZEL;

typedef struct _Rectl {
   int xLeft;
   int yBottom;
   int xRight;
   int yTop;
} RECTL, *PRECTL;

typedef struct _Pointl {
   int x;
   int y;
} POINTL, *PPOINTL;

#define CLR_WHITE                     (-2L)
#define CLR_BLACK                     (-1L)
#define CLR_BACKGROUND                  0L
#define CLR_BLUE                        1L
#define CLR_RED                         2L
#define CLR_PINK                        3L
#define CLR_GREEN                       4L
#define CLR_CYAN                        5L
#define CLR_YELLOW                      6L
#define CLR_NEUTRAL                     7L
#define CLR_MAGENTA                     CLR_PINK

#define CLR_DARKGRAY                    8L
#define CLR_DARKBLUE                    9L
#define CLR_DARKRED                    10L
#define CLR_DARKPINK                   11L
#define CLR_DARKGREEN                  12L
#define CLR_DARKCYAN                   13L
#define CLR_BROWN                      14L
#define CLR_PALEGRAY                   15L

typedef struct _RGB          /* rgb @TBD ??? */
{
   BYTE bBlue;              /* Blue component of the color definition */
   BYTE bGreen;             /* Green component of the color definition*/
   BYTE bRed;               /* Red component of the color definition  */
} RGB, *PRGB;

typedef struct _RGB2         /* rgb2 */
{
   BYTE bBlue;              /* Blue component of the color definition */
   BYTE bGreen;             /* Green component of the color definition*/
   BYTE bRed;               /* Red component of the color definition  */
   BYTE fcOptions;          /* Reserved, must be zero                 */
} RGB2, *PRGB2;

typedef struct _BitmapInfo2 {
   int  cbFix;
   int  cx;
   int  cy;
   int  cPlanes;
   int  cBitCount;
   int  ulCompresstion;
   int  cclrUsed;
   int  cclrImportant;
   RGB2 argbColor[1];
} BITMAPINFO2, *PBITMAPINFO2;

/* A special thanks goes out to Pat "Mr. C" Davis for making the following
** work with the C compiler instead of:
**
** #define HEX(h)    STRINGIZE(\x##h)
*/
#define STRINGIZE(s)                         #s
#define HEX1(h)                              STRINGIZE(h)
#define HEX(h)                               HEX1(\x##h)
#define ASCII(a)                             #a
#define OCT1(o)                              STRINGIZE(o)
#define OCT(o)                               OCT1(\0##o)

/* The following are
** ASCII definitions
*/
#define _NUL_                                HEX(00)
#define _LF_                                 HEX(0A)
#define _FF_                                 HEX(0C)
#define _CR_                                 HEX(0D)
#define _CAN_                                HEX(18)
#define _EM_                                 HEX(19)
#define _ESC_                                HEX(1B)
#define _FS_                                 HEX(1C)

#endif
