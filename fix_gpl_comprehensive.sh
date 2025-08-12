#!/bin/bash

# Comprehensive fix for GplDitherInstance.cpp for GCC 14 compatibility

echo "Fixing GplDitherInstance.cpp..."

# Add cstdint include after the first include
sed -i '1i #include <cstdint>' GplDitherInstance.cpp

# Fix pointer arithmetic issues
sed -i 's/(ULONG)pbSrc/(uintptr_t)pbSrc/g' GplDitherInstance.cpp
sed -i 's/(ULONG)pbMapEnd/(uintptr_t)pbMapEnd/g' GplDitherInstance.cpp
sed -i 's/(ULONG)pbSource/(uintptr_t)pbSource/g' GplDitherInstance.cpp
sed -i 's/(ULONG)params\.iMapSize/(uintptr_t)params.iMapSize/g' GplDitherInstance.cpp
sed -i 's/(ULONG)iMapSize/(uintptr_t)iMapSize/g' GplDitherInstance.cpp

# Fix const char* issues
sed -i 's/char \*pszName = aBoolMappings\[i\]\.pszName;/const char *pszName = aBoolMappings[i].pszName;/g' GplDitherInstance.cpp
sed -i 's/char \*pszPos  = strstr (pszOptions, pszName);/const char *pszPos  = strstr (pszOptions, pszName);/g' GplDitherInstance.cpp
sed -i 's/char \*pszName = aIntMappings\[i\]\.pszName;/const char *pszName = aIntMappings[i].pszName;/g' GplDitherInstance.cpp
sed -i 's/char \*pszPos  = strstr (pszOptions, pszName);/const char *pszPos  = strstr (pszOptions, pszName);/g' GplDitherInstance.cpp

# Fix register keyword
sed -i 's/register INT i;/INT i;/g' GplDitherInstance.cpp

echo "Fixed GplDitherInstance.cpp"