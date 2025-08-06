/*
** Copyright (c) 2003 International Business Machines
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and associated documentation files (the "Software"),
** to deal in the Software without restriction, including without limitation
** the rights to use, copy, modify, merge, publish, distribute, sublicense,
** and/or sell copies of the Software, and to permit persons to whom the
** Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
** OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
*/
#ifndef _PDC_H
#define _PDC_H 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* This is the handle to a Printer Driver Communications library
*/
typedef const void *PDCHANDLE;
/* This is the handle to a printer driver
*/
typedef const void *DRIVERHANDLE;
/* This is the handle to a printer device
*/
typedef const void *DEVICEHANDLE;
/* In the form "string1\0string2\0...\0stringN\0\0"
*/
typedef const char *STRINGARRAY;
/* In the form "string1\0handle1\0string2\0handle2\0...\0...\0stringN\0handleN\0\0\0"
*/
typedef const char *STRINGPAIRARRAY;

/*
*/
PDCHANDLE       pdcOpenLibrary               ();

/* Library level calls: */

/*
*/
int             pdcCloseLibrary              (PDCHANDLE   handlePDC);
/*
*/
STRINGARRAY     pdcEnumerateDrivers          (PDCHANDLE   handlePDC);
/*
*/
DRIVERHANDLE    pdcOpenDriver                (PDCHANDLE   handlePDC,
                                              const char *pszDriver);

/* Driver level calls: */

/*
*/
int             pdcCloseDriver               (DRIVERHANDLE  handleDriver);
/*
*/
STRINGPAIRARRAY pdcEnumerateDevices          (DRIVERHANDLE  handleDriver);
/*
*/
const char     *pdcGetDeviceHandle           (DRIVERHANDLE  handleDriver,
                                              const char   *pszDisplayHandle);
/*
*/
DEVICEHANDLE    pdcOpenDevice                (DRIVERHANDLE  handleDriver,
                                              const char   *pszDeviceHandle);

/* Device level calls: */

/*
*/
int             pdcCloseDevice                (DEVICEHANDLE  handleDevice);

/*
*/
int             pdcSetTranslatableLanguage    (DEVICEHANDLE  handleDevice,
                                               const char   *pszLanguage);
/*
*/
const char     *pdcGetTranslatableLanguage    (DEVICEHANDLE  handleDevice);
/*
*/
STRINGARRAY     pdcQueryTranslatableLanguages (DEVICEHANDLE  handleDevice);

/*
*/
const char     *pdcGetPDLInformation          (DEVICEHANDLE  handleDevice);

/*
*/
const char     *pdcQueryCurrentJobProperties  (DEVICEHANDLE  handleDevice);
/*
*/
int             pdcSetJobProperties           (DEVICEHANDLE  handleDevice,
                                               const char   *pszJobProperties);
/*
*/
STRINGARRAY     pdcListJobPropertyKeys        (DEVICEHANDLE  handleDevice);
/*
*/
STRINGARRAY     pdcListJobPropertyKeyValues   (DEVICEHANDLE  handleDevice,
                                               const char   *pszKey);
/*
*/
const char     *pdcGetJobProperty             (DEVICEHANDLE  handleDevice,
                                               const char   *pszKey);
/*
*/
const char     *pdcGetJobPropertyType         (DEVICEHANDLE  handleDevice,
                                               const char   *pszKey);
/*
*/
STRINGARRAY     pdcTranslateJobProperty       (DEVICEHANDLE  handleDevice,
                                               const char   *pszKey,
                                               const char   *pszValue);

/*
*/
int             pdcBeginJob                   (DEVICEHANDLE  handleDevice);
/*
*/
int             pdcStartPage                  (DEVICEHANDLE  handleDevice);
/*
*/
int             pdcStartPageWithProperties    (DEVICEHANDLE  handleDevice,
                                               const char   *pszJobProperties);
/*
*/
int             pdcEndPage                    (DEVICEHANDLE  handleDevice);
/*
*/
int             pdcEndJob                     (DEVICEHANDLE  handleDevice);
/*
*/
int             pdcAbortPage                  (DEVICEHANDLE  handleDevice);
/*
*/
int             pdcAbortJob                   (DEVICEHANDLE  handleDevice);

/*
*/
void            pdcFree                       (void         *pvData);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _PDC_H */
