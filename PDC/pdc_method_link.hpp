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
#ifndef _PDC_METHOD_LINK_H
#define _PDC_METHOD_LINK_H 1

#include "pdc_method.hpp"

#include <glib.h>
#include <gmodule.h>

class PDCMethodLink : public PDCMethod
{
public:
                            PDCMethodLink                 (DriverMap  *pMapDriver);
  virtual                  ~PDCMethodLink                 ();

  STRINGPAIRARRAY           pdcEnumerateDevices           ();
  const char               *pdcGetDeviceHandle            (const char *pszDisplayHandle);

  const void               *pdcOpenDevice                 (const char *pszDeviceHandle);
  int                       pdcCloseDevice                (const void *pvDeviceHandle);

  int                       pdcSetTranslatableLanguage    (const void *pvDeviceHandle,
                                                           const char *pszLanguage);
  const char               *pdcGetTranslatableLanguage    (const void *pvDeviceHandle);
  STRINGARRAY               pdcQueryTranslatableLanguages (const void *pvDeviceHandle);

  const char               *pdcGetPDLInformation          (const void *pvDeviceHandle);

  const char               *pdcQueryCurrentJobProperties  (const void *pvDeviceHandle);
  int                       pdcSetJobProperties           (const void *pvDeviceHandle,
                                                           const char *pszJobProperties);
  STRINGARRAY               pdcListJobPropertyKeys        (const void *pvDeviceHandle);
  STRINGARRAY               pdcListJobPropertyKeyValues   (const void *pvDeviceHandle,
                                                           const char *pszKey);
  const char               *pdcGetJobProperty             (const void *pvDeviceHandle,
                                                           const char *pszKey);
  const char               *pdcGetJobPropertyType         (const void *pvDeviceHandle,
                                                           const char *pszKey);
  STRINGARRAY               pdcTranslateJobProperty       (const void *pvDeviceHandle,
                                                           const char *pszKey,
                                                           const char *pszValue);

  int                       pdcBeginJob                   (const void *pvDeviceHandle);
  int                       pdcStartPage                  (const void *pvDeviceHandle);
  int                       pdcStartPageWithProperties    (const void *pvDeviceHandle,
                                                           const char *pszJobProperties);
  int                       pdcEndPage                    (const void *pvDeviceHandle);
  int                       pdcEndJob                     (const void *pvDeviceHandle);
  int                       pdcAbortPage                  (const void *pvDeviceHandle);
  int                       pdcAbortJob                   (const void *pvDeviceHandle);

private:
   typedef const char      *(*PDCINITIALIZESESSION)          (const char *pszClientVersion);
   typedef void             (*PFNCLOSESESSION)               ();

   typedef STRINGPAIRARRAY  (*PFNENUMERATEDEVICES)           ();
   typedef const char      *(*PFNGETDEVICEHANDLE)            (const char *pszDisplayHandle);

   typedef const void      *(*PFNOPENDEVICE)                 (const char *pszDeviceHandle);
   typedef int              (*PFNCLOSEDEVICE)                (const void *pvDeviceHandle);

   typedef int              (*PFNSETTRANSLATABLELANGUAGE)    (const void *pvDeviceHandle,
                                                              const char *pszLanguage);
   typedef const char      *(*PFNGETTRANSLATABLELANGUAGE)    (const void *pvDeviceHandle);
   typedef STRINGARRAY      (*PFNQUERYTRANSLATABLELANGUAGES) (const void *pvDeviceHandle);

   typedef const char      *(*PFNGETPDLINFORMATION)          (const void *pvDeviceHandle);

   typedef const char      *(*PFNQUERYCURRENTJOBPROPERTIES)  (const void *pvDeviceHandle);
   typedef int              (*PFNSETJOBPROPERTIES)           (const void *pvDeviceHandle,
                                                              const char *pszJobProperties);
   typedef STRINGARRAY      (*PFNLISTJOBPROPERTYKEYS)        (const void *pvDeviceHandle);
   typedef STRINGARRAY      (*PFNLISTJOBPROPERTYKEYVALUES)   (const void *pvDeviceHandle,
                                                              const char *pszKey);
   typedef const char      *(*PFNGETJOBPROPERTY)             (const void *pvDeviceHandle,
                                                              const char *pszKey);
   typedef const char      *(*PFNGETJOBPROPERTYTYPE)         (const void *pvDeviceHandle,
                                                              const char *pszKey);
   typedef STRINGARRAY      (*PFNTRANSLATEJOBPROPERTY)       (const void *pvDeviceHandle,
                                                              const char *pszKey,
                                                              const char *pszValue);

   typedef int              (*PFNBEGINJOB)                   (const void *pvDeviceHandle);
   typedef int              (*PFNSTARTPAGE)                  (const void *pvDeviceHandle);
   typedef int              (*PFNSTARTPAGEWITHPROPERTIES)    (const void *pvDeviceHandle,
                                                              const char *pszJobProperties);
   typedef int              (*PFNENDPAGE)                    (const void *pvDeviceHandle);
   typedef int              (*PFNENDJOB)                     (const void *pvDeviceHandle);
   typedef int              (*PFNABORTPAGE)                  (const void *pvDeviceHandle);
   typedef int              (*PFNABORTJOB)                   (const void *pvDeviceHandle);

   bool       loadDriver                    (DriverMap  *pMapDriver);

   GModule                      *hmodDevice_d;

   PDCINITIALIZESESSION          pfnInitializeSession_d;
   PFNCLOSESESSION               pfnCloseSession_d;
   PFNENUMERATEDEVICES           pfnEnumerateDevices_d;
   PFNGETDEVICEHANDLE            pfnGetDeviceHandle_d;
   PFNOPENDEVICE                 pfnOpenDevice_d;
   PFNCLOSEDEVICE                pfnCloseDevice_d;
   PFNSETTRANSLATABLELANGUAGE    pfnSetTranslatableLanguage_d;
   PFNGETTRANSLATABLELANGUAGE    pfnGetTranslatableLanguage_d;
   PFNQUERYTRANSLATABLELANGUAGES pfnQueryTranslatableLanguages_d;
   PFNGETPDLINFORMATION          pfnGetPDLInformation_d;
   PFNQUERYCURRENTJOBPROPERTIES  pfnQueryCurrentJobProperties_d;
   PFNSETJOBPROPERTIES           pfnSetJobProperties_d;
   PFNLISTJOBPROPERTYKEYS        pfnListJobPropertyKeys_d;
   PFNLISTJOBPROPERTYKEYVALUES   pfnListJobPropertyKeyValues_d;
   PFNGETJOBPROPERTY             pfnGetJobProperty_d;
   PFNGETJOBPROPERTYTYPE         pfnGetJobPropertyType_d;
   PFNTRANSLATEJOBPROPERTY       pfnTranslateJobProperty_d;
   PFNBEGINJOB                   pfnBeginJob_d;
   PFNSTARTPAGE                  pfnStartPage_d;
   PFNSTARTPAGEWITHPROPERTIES    pfnStartPageWithProperties_d;
   PFNENDPAGE                    pfnEndPage_d;
   PFNENDJOB                     pfnEndJob_d;
   PFNABORTPAGE                  pfnAbortPage_d;
   PFNABORTJOB                   pfnAbortJob_d;
};

#endif /* _PDC_METHOD_LINK_H */
