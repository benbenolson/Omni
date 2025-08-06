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
#include <stdio.h>
#include <string.h>

#include "../hppcl3/hppcl3.hpp"

int
main (int argc, char *argv[])
{
   int rc;

   if (3 == argc)
   {
      printf("argc = %d\n", argc);
      printf("argv[0] = %s\n", argv[0]);
      printf("argv[1] = %s\n", argv[1]);
      printf("argv[2] = %s\n", argv[2]);

      FILE *fp = fopen (argv[2], "rb");

      if (0 == strcmp (argv[1], "os2"))
      {
         OS2BITMAPFILEHEADER bfh;

         rc = fread (&bfh, sizeof (bfh), 1, fp);

         printf ("bfh.usType        = %c%c\n", (bfh.usType & 0xFF), (bfh.usType >> 8));
         printf ("bfh.cbSize        = %d\n", bfh.cbSize       );
         printf ("bfh.xHotspot      = %d\n", bfh.xHotspot     );
         printf ("bfh.yHotspot      = %d\n", bfh.yHotspot     );
         printf ("bfh.offBits       = %d\n", bfh.offBits      );
         printf ("bfh.bmp.cbFix     = %d\n", bfh.bmp.cbFix    );
         printf ("bfh.bmp.cx        = %d\n", bfh.bmp.cx       );
         printf ("bfh.bmp.cy        = %d\n", bfh.bmp.cy       );
         printf ("bfh.bmp.cBitCount = %d\n", bfh.bmp.cBitCount);
         printf ("bfh.bmp.cPlanes   = %d\n", bfh.bmp.cPlanes  );
      }
      else if (0 == strcmp (argv[1], "win"))
      {
         WINBITMAPFILEHEADER bfh;
         WINBITMAPINFOHEADER bih;

         rc = fread (&bfh, sizeof (bfh), 1, fp);
         rc = fread (&bih, sizeof (bih), 1, fp);

         printf ("bfh.bfType          = %c%c\n", (bfh.bfType & 0xFF), (bfh.bfType >> 8));
         printf ("bfh.bfSize          = %d\n", (int)bfh.bfSize         );
         printf ("bfh.bfReserved1     = %d\n",      bfh.bfReserved1    );
         printf ("bfh.bfReserved2     = %d\n",      bfh.bfReserved2    );
         printf ("bfh.bfOffBits       = %d\n", (int)bfh.bfOffBits      );
         printf ("bih.biSize          = %d\n", (int)bih.biSize         );
         printf ("bih.biWidth         = %d\n",      bih.biWidth        );
         printf ("bih.biHeight        = %d\n",      bih.biHeight       );
         printf ("bih.biPlanes        = %d\n",      bih.biPlanes       );
         printf ("bih.biBitCount      = %d\n",      bih.biBitCount     );
         printf ("bih.biCompression   = %d\n", (int)bih.biCompression  );
         printf ("bih.biSizeImage     = %d\n", (int)bih.biSizeImage    );
         printf ("bih.biXPelsPerMeter = %d\n",      bih.biXPelsPerMeter);
         printf ("bih.biYPelsPerMeter = %d\n",      bih.biYPelsPerMeter);
         printf ("bih.biClrUsed       = %d\n", (int)bih.biClrUsed      );
         printf ("bih.biClrImportant  = %d\n", (int)bih.biClrImportant );
      }
   }

   return 0;
}
