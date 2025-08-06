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
#ifndef _PDC_DRIVER_H
#define _PDC_DRIVER_H 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* In the form "string1\0string2\0...\0stringN\0\0"
*/
typedef const char *STRINGARRAY;
/* In the form "string1\0handle1\0string2\0handle2\0...\0...\0stringN\0handleN\0\0\0"
*/
typedef const char *STRINGPAIRARRAY;

const char           *pdcInitializeSession          (const char *pszClientVersion);
void                  pdcCloseSession               ();

STRINGPAIRARRAY       pdcEnumerateDevices           ();
const char           *pdcGetDeviceHandle            (const char *pszDisplayHandle);

const void           *pdcOpenDevice                 (const char *pszDeviceHandle);
int                   pdcCloseDevice                (const void *pvDeviceHandle);

int                   pdcSetTranslatableLanguage    (const void *pvDeviceHandle,
                                                     const char *pszLanguage);
const char           *pdcGetTranslatableLanguage    (const void *pvDeviceHandle);
STRINGARRAY           pdcQueryTranslatableLanguages (const void *pvDeviceHandle);

const char           *pdcGetPDLInformation          (const void *pvDeviceHandle);

const char           *pdcQueryCurrentJobProperties  (const void *pvDeviceHandle);
int                   pdcSetJobProperties           (const void *pvDeviceHandle,
                                                     const char *pszJobProperties);
STRINGARRAY           pdcListJobPropertyKeys        (const void *pvDeviceHandle);
STRINGARRAY           pdcListJobPropertyKeyValues   (const void *pvDeviceHandle,
                                                     const char *pszKey);
const char           *pdcGetJobProperty             (const void *pvDeviceHandle,
                                                     const char *pszKey);
const char           *pdcGetJobPropertyType         (const void *pvDeviceHandle,
                                                     const char *pszKey);
STRINGARRAY           pdcTranslateJobProperty       (const void *pvDeviceHandle,
                                                     const char *pszKey,
                                                     const char *pszValue);

int                   pdcBeginJob                   (const void *pvDeviceHandle);
int                   pdcStartPage                  (const void *pvDeviceHandle);
int                   pdcStartPageWithProperties    (const void *pvDeviceHandle,
                                                     const char *pszJobProperties);
int                   pdcEndPage                    (const void *pvDeviceHandle);
int                   pdcEndJob                     (const void *pvDeviceHandle);
int                   pdcAbortPage                  (const void *pvDeviceHandle);
int                   pdcAbortJob                   (const void *pvDeviceHandle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _PDC_DRIVER_H */
