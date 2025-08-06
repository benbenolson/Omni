/* REXX:
*/
szDate     = substr( date( s ), 5, 2)||,
             substr( date( s ), 7, 2)||,
             substr( date( s ), 1, 4)

szFileName = '\OmniDeviceMixSrc'date( s )'.zip'
'echo off'
'del 'szFileName
'zip 'szFileName' * -x *.bmp *.class *.o *.exe Omni*.java Omni*.txt errors'
'zip 'szFileName' Deskjet/* -x Deskjet/*.o Deskjet/*.exe Deskjet/HP_* Deskjet/output* Deskjet/errors'
'zip 'szFileName' Epson/* -x Epson/*.o Epson/*.exe Epson/Epson_* Epson/output* Epson/errors'
'zip 'szFileName' docs/*'
'zip 'szFileName' port/*'
'zip 'szFileName' hppcl3/* -x hppcl3/*.o hppcl3/*.exe hppcl3/errors hpccl3/tags/* hppcl3/*.bmp'
'copy 'szFileName' \\gedfiles\diski\hamzy\Linux\Omni'
