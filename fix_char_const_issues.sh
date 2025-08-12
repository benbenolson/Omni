#!/bin/bash
# Fix char* vs const char* issues in GplDitherInstance.cpp
echo "Fixing char* vs const char* issues in GplDitherInstance.cpp..."

# Fix the struct definitions
sed -i '/typedef struct _BoolParmMapping {/,/} BOOLPARMMAPPING, \*PBOOlPARMMAPPING;/c\   typedef struct _BoolParmMapping {\n      const char *pszName;\n      bool *pfParm;\n   } BOOLPARMMAPPING, *PBOOlPARMMAPPING;' GplDitherInstance.cpp

sed -i '/typedef struct _IntParmMapping {/,/} INTPARMMAPPING, \*PINTPARMMAPPING;/c\   typedef struct _IntParmMapping {\n      const char *pszName;\n      int  *piParm;\n   } INTPARMMAPPING, *PINTPARMMAPPING;' GplDitherInstance.cpp

# Fix variable declarations
sed -i 's/char \*pszName = aBoolMappings\[i\]\.pszName;/const char *pszName = aBoolMappings[i].pszName;/g' GplDitherInstance.cpp
sed -i 's/char \*pszPos  = strstr (pszOptions, pszName);/const char *pszPos  = strstr (pszOptions, pszName);/g' GplDitherInstance.cpp

sed -i 's/char \*pszName = aIntMappings\[i\]\.pszName;/const char *pszName = aIntMappings[i].pszName;/g' GplDitherInstance.cpp
sed -i 's/char \*pszPos  = strstr (pszOptions, pszName);/const char *pszPos  = strstr (pszOptions, pszName);/g' GplDitherInstance.cpp

echo "Fixed char* vs const char* issues"