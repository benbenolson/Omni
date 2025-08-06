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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sstream>

#include "hppcl3.hpp"
#include "databuf.hpp"
#include "scanline.hpp"

/* Function prototypes...
*/
void          PrintUsage              (char       *pszName);
void          ProcessFile             (DataBuffer *pDataBuffer);
int           ReadNumber              (PBYTE       pbStart);
void          PrintCharactersAndBytes (PBYTE       pbData,
                                       int         iLength,
                                       int         iStartOffset);

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
int
main (int argc, char *argv[])
{
   DataBuffer *pDataBuffer;

   argv[0] = "hppcl3.exe";

#ifdef DEBUG
{
   register int i;

   DBPRINTF (("argc = %d\n", argc));
   for (i = 0; i < argc; i++)
      DBPRINTF (("argv[%d] = '%s'\n", i, argv[i]));
}
#endif

   if (2 != argc)
   {
      PrintUsage (argv[0]);
      return 1;
   }

   pDataBuffer = new DataBuffer (argv[1], MEM_SIZE);

   ProcessFile (pDataBuffer);

   delete pDataBuffer;

   return 0;
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
void
PrintUsage (char *pszName)
{
   fprintf (stderr, "Usage: %s filename.in\n", pszName);
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
void
ProcessFile (DataBuffer *pDataBuffer)
{
   bool       fCommandKnown        = false;
   bool       fCommandMulti        = false;
   BYTE       bHeadCommand         = 0;
   BYTE       bSecondCommand       = 0;
   BYTE       bTailCommand         = 0;
#ifdef DEBUG
   int        iPosHeadCommand;
   int        iPosSecondCommand;
   int        iPosStartTail;
#endif
   int        iPosTailCommand;
   int        iBytesToSave;
   int        iBytesInCommand      = 0;
   int        iCommandVarSize;
   char      *pszCommandDesc       = (char *)NULL;
   char       cData;
   int        iData;
   PBYTE      pbVarData;
   PBYTE      pbNumber;
   ScanLines *pScanLines           = NULL;
   int        iCurrentForm         = 2;

   do
   {
      if (!fCommandMulti)
      {
         // Skip non-command bytes
         do
         {
            iData = pDataBuffer->GetByte (0);
            if (0 > iData)
               // Error
               break;

            cData = (char)iData;

            if (0x1B != cData)
            {
///////////////if (DEBUG_LEVEL & DEBUG_IO)
                  cerr << hex << "Skipping non-command bytes @0x" << pDataBuffer->CurrentPosition () - 1 << endl;
            }

         } while (0x1B != cData);

         if (  0 > iData                      // Error
            || !pDataBuffer->MoreDataLeft ()  // Out of data
            )
         {
            if (0 > iData)
               cerr << dec << "@" << __LINE__ << ": " << "pDataBuffer->GetByte (0) returns < 0" << endl;
            if (!pDataBuffer->MoreDataLeft ())
               cerr << dec << "@" << __LINE__ << ": " << "pDataBuffer->MoreDataLeft () return false" << endl;
            break;
         }

         if (0x1B == cData)
         {
            DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "Found an escape!\n"));

            iData = pDataBuffer->GetByte (1);
            if (0 > iData)
            {
               // Error
               cerr << dec << "@" << __LINE__ << ": " << "pDataBuffer->GetByte (0) returns < 0" << endl;
               break;
            }

            cData           = (char)iData;
            bHeadCommand    = cData;
#ifdef DEBUG
            iPosHeadCommand = pDataBuffer->CurrentPosition ();
#endif
         }
      }

      if (  bHeadCommand         // If we have a head
         && !bTailCommand        // and we dont have a tail
         )
      {
         if ('%' == bHeadCommand)
         {
            // Its a special tail
            iBytesToSave = 2;          // _ESC_ and '%'

            while (  0x0A != cData
                  && pDataBuffer->MoreDataLeft ()
                  )
            {
               iData = pDataBuffer->GetByte (iBytesToSave);
               if (0 > iData)
               {
                  // Error
                  cerr << dec << "@" << __LINE__ << ": " << "pDataBuffer->GetByte (0) returns < 0" << endl;
                  break;
               }

               cData = (char)iData;

               iBytesToSave++;
            }

            if (0 == strncmp ((char *)pDataBuffer->PreviousData (iBytesToSave),
                              _ESC_"%-12345X",
                              9))
            {
               fCommandKnown   = true;
               iCommandVarSize = 0;
               pszCommandDesc  = "PJL Command";
            }

            iPosTailCommand = pDataBuffer->CurrentPosition ();
            bTailCommand    = cData;

            iBytesInCommand = iBytesToSave;
DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "@ line %d iBytesInCommand now %d\n", __LINE__, iBytesInCommand));
         }
         else
         {
            // One byte commands
            switch (bHeadCommand)
            {
            case 'z':                     pszCommandDesc = "Self Test"; break;
            case 'Y':               pszCommandDesc = "Turn on Display"; break;
            case 'Z':              pszCommandDesc = "Turn off Display"; break;
            case 'E':                 pszCommandDesc = "Printer Reset"; break;
            case '9':      pszCommandDesc = "Clear Horizontal Margins"; break;
            }

            if (pszCommandDesc)
            {
               fCommandKnown   = true;
               iBytesInCommand++;
               iCommandVarSize = 0;
DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "@ line %d iBytesInCommand now %d\n", __LINE__, iBytesInCommand));
            }
         }
      }

      if (!fCommandKnown)
      {
         // Multi byte commands
         if (!bSecondCommand)
         {
            iData = pDataBuffer->GetByte (0);
            if (0 > iData)
            {
               // Error
               cerr << dec << "@" << __LINE__ << ": " << "pDataBuffer->GetByte (0) returns < 0" << endl;
               break;
            }

            bSecondCommand       = (char)iData;
#ifdef DEBUG
            iPosSecondCommand    = pDataBuffer->CurrentPosition ();
#endif
            iBytesInCommand++;
DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "@ line %d iBytesInCommand now %d\n", __LINE__, iBytesInCommand));
         }

         iBytesToSave  = 0;
#ifdef DEBUG
         iPosStartTail = pDataBuffer->CurrentPosition ();
#endif

         DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "Starting to look @%d\n", iPosStartTail));

         while (pDataBuffer->MoreDataLeft ())
         {
            iData = pDataBuffer->GetByte (iBytesToSave);
            if (0 > iData)
               // Error
               break;

            iBytesToSave++;

            cData = (char)iData;

            DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "'%c'\n", cData));

            if ('A' <= cData && cData <= 'Z')
            {
               fCommandMulti = false;
               break;
            }
            else if ('a' <= cData && cData <= 'z')
            {
               DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "'%c' %d\n", cData, fCommandMulti));

               fCommandMulti = true;
               break;
            }
         }

         if (!pDataBuffer->MoreDataLeft ())
         {
            cerr << dec << "@" << __LINE__ << ": " << "pDataBuffer->MoreDataLeft () return false" << endl;
            break;
         }

         bTailCommand     = toupper (cData);
         iPosTailCommand  = pDataBuffer->CurrentPosition () - 1;
         iBytesInCommand += iBytesToSave;
DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "iPosTailCommand is @ %d\n", iPosTailCommand));
DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "iPosStartTail is @ %d\n", iPosStartTail));
DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "@ line %d iBytesInCommand now %d\n", __LINE__, iBytesInCommand));

         pbNumber = pDataBuffer->PreviousData (iBytesToSave);

         switch (bHeadCommand)
         {
         case '&':
         {
            switch (bSecondCommand)
            {
            case 'b':
            {
               switch (bTailCommand)
               {
               case 'W':             pszCommandDesc = "Configuration Apple Talk";
                  iCommandVarSize = ReadNumber (pbNumber);
                  break;

               case 'E': pszCommandDesc = "????????????????????????????????????";
                  break;

               // @HACK - seen as <ESC> &b3t
               case 'T': pszCommandDesc = "????????????????????????????????????";
                  fCommandMulti = false;
                  break;
               }

               if (pszCommandDesc)
               {
                  fCommandKnown = true;
               }
               break;
            }

            case 'l':
            {
               switch (bTailCommand)
               {
               case 'X':                     pszCommandDesc = "Number of copies"; break;
               case 'S':                 pszCommandDesc = "Simplex/Duplex Print"; break;
               case 'U': pszCommandDesc = "Left (Long Edge) Offset Registration"; break;
               case 'Z': pszCommandDesc = "Top (Short Edge) Offset Registration"; break;
               case 'T':                       pszCommandDesc = "Job Separation"; break;
               case 'G':                           pszCommandDesc = "Output Bin"; break;
               case 'A':                            pszCommandDesc = "Page Size"; break;
               case 'P':                          pszCommandDesc = "Page Length"; break;
               case 'H':                         pszCommandDesc = "Paper Source"; break;
               case 'O':                     pszCommandDesc = "Page Orientation"; break;
               case 'F':                          pszCommandDesc = "Text Length"; break;
               case 'L':                     pszCommandDesc = "Preforation Skip"; break;
               case 'M':                       pszCommandDesc = "Set Media Type"; break;
               case 'E':              pszCommandDesc = "Set Top Margin in Lines"; break;
               }

               if (pszCommandDesc)
               {
                  fCommandKnown = true;
               }
               break;
            }

            case 'a':
            {
               switch (bTailCommand)
               {
               case 'G':           pszCommandDesc = "Duplex Page Side Selection"; break;
               case 'P':                      pszCommandDesc = "Print Direction"; break;
               case 'L':                          pszCommandDesc = "Left Margin"; break;
               case 'M':                         pszCommandDesc = "Right Margin"; break;
               case 'E':                           pszCommandDesc = "Top Margin"; break;
               }

               if (pszCommandDesc)
               {
                  fCommandKnown = true;
               }
               break;
            }

            case 'u':
            {
               switch (bTailCommand)
               {
               case 'D':                     pszCommandDesc = "Units of Measure"; break;
               }

               if (pszCommandDesc)
               {
                  fCommandKnown = true;
               }
               break;
            }

            case 'k':
            {
               switch (bTailCommand)
               {
               case 'G':                 pszCommandDesc = "Set Line Termination"; break;
               }

               if (pszCommandDesc)
               {
                  fCommandKnown = true;
               }
               break;
            }

            case 'p':
            {
               switch (bTailCommand)
               {
               case 'X':                pszCommandDesc = "Transparent Data Mode"; break;
               }

               if (pszCommandDesc)
               {
                  fCommandKnown = true;
               }
               break;
            }
            }
            break;
         }

         case '*':
         {
            switch (bSecondCommand)
            {
            case 'o':
            {
               switch (bTailCommand)
               {
               case 'W': pszCommandDesc = "????????????????????????????????????";
                  iCommandVarSize = ReadNumber (pbNumber);
                  break;

               case 'M':                        pszCommandDesc = "Print Quality"; break;
               case 'D': pszCommandDesc = "????????????????????????????????????"; break;
               }

               if (pszCommandDesc)
               {
                  fCommandKnown = true;
               }
               break;
            }

            case 't':
            {
               switch (bTailCommand)
               {
               case 'R':                    pszCommandDesc = "Select Resolution"; break;
               }

               if (pszCommandDesc)
               {
                  fCommandKnown = true;
               }
               break;
            }

            case 'c':
            {
               switch (bTailCommand)
               {
               case 'F':             pszCommandDesc = "Download Font Management"; break;
               }

               if (pszCommandDesc)
               {
                  fCommandKnown = true;
               }
               break;
            }

            case 'p':
            {
               switch (bTailCommand)
               {
               case 'X':       pszCommandDesc = "Horizontail Cursor Positioning"; break;
               case 'Y':          pszCommandDesc = "Vertical Cursor Positioning"; break;
               }

               if (pszCommandDesc)
               {
                  fCommandKnown = true;
               }
               break;
            }

            case 'g':
            {
               switch (bTailCommand)
               {
               case 'W':
                                        pszCommandDesc = "Configure Raster Data";
                  iCommandVarSize = ReadNumber (pbNumber);
                  break;
               }

               if (pszCommandDesc)
               {
                  fCommandKnown = true;
               }
               break;
            }

            case 'r':
            {
               switch (bTailCommand)
               {
               case 'A':                    pszCommandDesc = "Start Raster Mode"; break;
               case 'B':                      pszCommandDesc = "End Raster Mode"; break;
               case 'C':                      pszCommandDesc = "End Raster Mode"; break;
               case 'S':                  pszCommandDesc = "Source Raster Width"; break;
               case 'U':                         pszCommandDesc = "Simple Color"; break;
               case 'Q':       pszCommandDesc = "Select Raster Graphics Quality"; break;
               }

               if (pszCommandDesc)
               {
                  fCommandKnown = true;
               }
               break;
            }

            case 'v':
            {
               switch (bTailCommand)
               {
               case 'S':                     pszCommandDesc = "Foreground Color"; break;
               }

               if (pszCommandDesc)
               {
                  fCommandKnown = true;
               }
               break;
            }

            case 'b':
            {
               switch (bTailCommand)
               {
               case 'W':         pszCommandDesc = "Transfer Raster by Row/Block";
                  iCommandVarSize = ReadNumber (pbNumber);
                  break;

               case 'V':             pszCommandDesc = "Transfer Raster by Plane";
                  iCommandVarSize = ReadNumber (pbNumber);
                  break;

               case 'M':               pszCommandDesc = "Set Compression Method"; break;
               case 'S':                      pszCommandDesc = "Seed Row Source"; break;
               case 'Y':                             pszCommandDesc = "Y Offset"; break;
               }

               if (pszCommandDesc)
               {
                  fCommandKnown = true;
               }
               break;
            }
            }
            break;
         }
         }
      }

      if (bHeadCommand)
      {
         if (isprint (bHeadCommand))
            DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "Head   is '%c' 0x%02X @%d\n",
                       bHeadCommand,
                       bHeadCommand,
                       iPosHeadCommand));
         else
            DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "Head   is     0x%02X @%d\n",
                       bHeadCommand,
                       iPosHeadCommand));

         if (bSecondCommand)
         {
            if (isprint (bSecondCommand))
               DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "Second is '%c' 0x%02X @%d\n",
                          bSecondCommand,
                          bSecondCommand,
                          iPosSecondCommand));
            else
               DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "Second is     0x%02X @%d\n",
                          bSecondCommand,
                          iPosSecondCommand));
         }
      }
      else
      {
         DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "No bHeadCommand!\n"));
         break;
      }

      if (bTailCommand)
      {
         if (isprint (bTailCommand))
            DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "Tail   is '%c' 0x%02X @%d\n",
                       bTailCommand,
                       bTailCommand,
                       iPosTailCommand));
         else
            DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "Tail   is     0x%02X @%d\n",
                       bTailCommand,
                       iPosTailCommand));
      }
      else
      {
         DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "No bTailCommand!\n"));
      }

      if (fCommandKnown)
      {
         if (DEBUG_LEVEL & DEBUG_IO)
            cerr << dec << "Command size = " << iBytesInCommand << endl;

         iBytesInCommand += iCommandVarSize;
         if (0 < iCommandVarSize)
            if (DEBUG_LEVEL & DEBUG_IO)
               cerr << dec << "Command variable size = " << iCommandVarSize << endl;
         if (DEBUG_LEVEL & DEBUG_IO)
            cerr << dec << "Command final size = " << iBytesInCommand << endl;

         pbVarData = pDataBuffer->BufferBytes (iCommandVarSize,
                                               iBytesToSave);

         if (pszCommandDesc)
         {
            if (DEBUG_LEVEL & DEBUG_IO)
               cerr << pszCommandDesc << endl;
         }
         else
         {
            if (DEBUG_LEVEL & DEBUG_IO)
               cerr << "No command found!" << endl;
         }

         if (  '&' == bHeadCommand
            && 'l' == bSecondCommand
            && 'A' == bTailCommand
            )
         {
#ifdef DEBUG
            char  achForm[10];
            char *pszForm;

            if (1 == iCurrentForm)
               pszForm = "Executive";
            else if (2 == iCurrentForm)
               pszForm = "US Letter";
            else
            {
               sprintf (achForm, "%d", iCurrentForm);
               pszForm = achForm;
            }

            DBPRINTF (("Form is %s\n", pszForm));
#endif

            iCurrentForm = ReadNumber (pbNumber);

            if (pScanLines)
               pScanLines->SetPageSize (iCurrentForm);
         }
         else if (  '*' == bHeadCommand
                 && 'g' == bSecondCommand
                 && 'W' == bTailCommand
                 )
         {
            if (pScanLines)
               delete pScanLines;

            PrintCharactersAndBytes (pbVarData, iCommandVarSize, iPosTailCommand + 1);

            pScanLines = new ScanLines (pbVarData, iCommandVarSize);

            if (pScanLines)
               pScanLines->SetPageSize (iCurrentForm);
         }
         else if (  '*' == bHeadCommand
                 && 'b' == bSecondCommand
                 && (  'W' == bTailCommand
                    || 'V' == bTailCommand
                    )
                 )
         {
            if (pScanLines)
               pScanLines->AddScanLine (iCommandVarSize,
                                        pbVarData);

         }
         else if (  '*' == bHeadCommand
                 && 'b' == bSecondCommand
                 && 'M' == bTailCommand
                 )
         {
            if (pScanLines)
               pScanLines->SetCompressionLevel (ReadNumber (pbNumber));
         }
         else if (  '*' == bHeadCommand
                 && 'b' == bSecondCommand
                 && 'Y' == bTailCommand
                 )
         {
            if (pScanLines)
               pScanLines->MoveToY (ReadNumber (pbNumber));
         }

         if (!pDataBuffer->MoreDataLeft ())
         {
            if (DEBUG_LEVEL & DEBUG_IO)
               cerr << dec << "@" << __LINE__ << ": " << "pDataBuffer->MoreDataLeft () return false" << endl;
            break;
         }

         fCommandKnown   = false;
         iCommandVarSize = 0;
         iBytesInCommand = 0;
         pszCommandDesc  = (char *)NULL;

         if (!fCommandMulti)
         {
            // Done with this command
            bHeadCommand   = 0;
            bSecondCommand = 0;
            bTailCommand   = 0;
            pbNumber       = NULL;
         }
      }
      else
      {
         cerr << hex << "Error: @0x" << iPosHeadCommand << " ";

         if (bHeadCommand && bSecondCommand && bTailCommand)
            cerr << hex << "Unknown command! (0x" << (int)bHeadCommand << ")(0x" << (int)bSecondCommand << ")(0x" << (int)bTailCommand << ")" << endl;
         else if (bHeadCommand && bTailCommand)
            cerr << hex << "Unknown command! (0x" << (int)bHeadCommand << ")(0x" << (int)bSecondCommand << ")" << endl;
         else if (bHeadCommand)
            cerr << hex << "Unknown command! (0x" << (int)bHeadCommand << ")" << endl;
         else
            cerr << hex << "Unknown command with nothing!" << endl;
         break;
      }
   } while (pDataBuffer->MoreDataLeft ());

   // Clean up
   if (pScanLines)
      delete pScanLines;
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
int
ReadNumber (PBYTE pbStart)
{
   int  iRet   = 0;

   while (  '0' <= *pbStart
         && *pbStart <= '9'
         )
   {
      iRet = iRet * 10 + (*pbStart - '0');

      pbStart++;
   }

   DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO, "%s returns %d\n", __FUNCTION__, iRet));

   return iRet;
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
void
PrintCharactersAndBytes (PBYTE pbData,
                         int   iLength,
                         int   iStartOffset)
{
   char         achLine[256];
   BYTE         abLine[16];
   int          iBlock;
   int          iOffset = iStartOffset;
   register int i;

   while (0 < iLength)
   {
      iBlock = omni::min ((int)sizeof (abLine), iLength);

      memcpy (abLine, pbData, iBlock);

      sprintf (achLine, "%06X:    ", iOffset);

      // print the hex view
      for (i = 0; i < iBlock; i++)
      {
         sprintf (achLine + strlen (achLine), "%02X ", abLine[i]);
      }
      // pad if necessary
      while (i < (int)sizeof (abLine))
      {
         sprintf (achLine + strlen (achLine), "   ");
         i++;
      }
      // Now print the character view
      sprintf (achLine + strlen (achLine), "  ");
      for (i = 0; i < iBlock; i++)
      {
         sprintf (achLine + strlen (achLine), "%c", isprint (abLine[i]) ? abLine[i] : '.');
      }

      sprintf (achLine + strlen (achLine), "\n");

      DBPRINTF ((achLine));

      iOffset += 16;
      iLength -= iBlock;
   }
}
