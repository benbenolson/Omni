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
#ifndef _hppcl3_hpp
#define _hppcl3_hpp

#define assertT(x) assert (!(x));
#define assertF(x) assert (x);
#define DBPRINTF(x)
#define DBPRINTIF(x)
#define MY_PRINT_VAR(v) cerr << __FUNCTION__ << ":" << #v << " = " << v << endl

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

typedef unsigned char byte, BYTE, *PBYTE;
typedef short int SHORT, *PSHORT;
typedef unsigned short int USHORT, *PUSHORT;
typedef unsigned int ULONG, *PULONG;

/* bitmap parameterization used by GpiCreateBitmap and others */
typedef struct _OS2BITMAPINFOHEADER         /* bmp */
{
   ULONG  cbFix;
   USHORT cx;
   USHORT cy;
   USHORT cPlanes;
   USHORT cBitCount;
} __attribute__ ((aligned (1))) __attribute__ ((packed)) OS2BITMAPINFOHEADER;
typedef OS2BITMAPINFOHEADER *POS2BITMAPINFOHEADER;

typedef struct _BITMAPFILEHEADER    /* bfh */
{
   USHORT              usType;
   ULONG               cbSize;
   SHORT               xHotspot;
   SHORT               yHotspot;
   ULONG               offBits;
   OS2BITMAPINFOHEADER bmp;
} OS2BITMAPFILEHEADER;
typedef OS2BITMAPFILEHEADER *POS2BITMAPFILEHEADER;

/*************************************************************************
* These are the identifying values that go in the usType field of the
* BITMAPFILEHEADER(2) and BITMAPARRAYFILEHEADER(2).
* (BFT_ => Bit map File Type)
*************************************************************************/
#define OS2BFT_ICON           0x4349   /* 'IC' */
#define OS2BFT_BMAP           0x4d42   /* 'BM' */
#define OS2BFT_POINTER        0x5450   /* 'PT' */
#define OS2BFT_COLORICON      0x4943   /* 'CI' */
#define OS2BFT_COLORPOINTER   0x5043   /* 'CP' */
#define OS2BFT_BITMAPARRAY    0x4142   /* 'BA' */

typedef struct _OS2RGB      /* rgb */
{
   BYTE bBlue;              /* Blue component of the color definition */
   BYTE bGreen;             /* Green component of the color definition*/
   BYTE bRed;               /* Red component of the color definition  */
} OS2RGB, *POS2RGB;

typedef struct _OS2RGB2     /* rgb2 */
{
   BYTE bBlue;              /* Blue component of the color definition */
   BYTE bGreen;             /* Green component of the color definition*/
   BYTE bRed;               /* Red component of the color definition  */
   BYTE fcOptions;          /* Reserved, must be zero                 */
} OS2RGB2, *POS2RGB2;

typedef struct _OS2BitmapInfo {
   int     cbFix;
   int     cx;
   int     cy;
   int     cPlanes;
   int     cBitCount;
   int     ulCompresstion;
   int     cclrUsed;
   int     cclrImportant;
   OS2RGB2 argbColor[1];
} OS2BITMAPINFO2, *POS2BITMAPINFO2;

typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef WORD *PWORD;
typedef DWORD *PDWORD;
typedef int LONG;

typedef struct tagBITMAPINFOHEADER {
        DWORD   biSize;
        LONG    biWidth;
        LONG    biHeight;
        WORD    biPlanes;
        WORD    biBitCount;
        DWORD   biCompression;
        DWORD   biSizeImage;
        LONG    biXPelsPerMeter;
        LONG    biYPelsPerMeter;
        DWORD   biClrUsed;
        DWORD   biClrImportant;
} WINBITMAPINFOHEADER,*PWINBITMAPINFOHEADER;
typedef struct tagRGBQUAD {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} WINRGBQUAD;
typedef struct tagBITMAPINFO {
        WINBITMAPINFOHEADER bmiHeader;
        WINRGBQUAD          bmiColors[1];
} WINBITMAPINFO,*PWINBITMAPINFO;
//#pragma pack(push,2)
typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} __attribute__ ((aligned (2))) __attribute__ ((packed)) WINBITMAPFILEHEADER,*PWINBITMAPFILEHEADER;
//#pragma pack(pop)

#define WINBFT_BMAP           0x4d42   /* 'BM' */

#define STRINGIZE(s)               #s
#define HEX1(h)                    STRINGIZE(h)
#define HEX(h)                     HEX1(\x##h)
#define ASCII(a)                   #a
#define OCT1(o)                    STRINGIZE(o)
#define OCT(o)                     OCT1(\0##o)
#define _ESC_                      HEX(1B)

#define DEBUG_NONE                 0x00000000
#define DEBUG_IO                   0x80000000
#define DEBUG_COMMANDS             0x40000000
//#define DEBUG_LEVEL                (DEBUG_COMMANDS | DEBUG_IO)
#define DEBUG_LEVEL                (DEBUG_COMMANDS)

#define MEM_SIZE (4 * 1024)

#endif
