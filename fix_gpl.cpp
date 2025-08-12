#!/bin/bash

# Fix GplDitherInstance.cpp for GCC 14 compatibility

# Add cstdint include
sed -i '1i #include <cstdint>' GplDitherInstance.cpp

# Fix pointer arithmetic issues
sed -i 's/(ULONG)pbSrc/(uintptr_t)pbSrc/g' GplDitherInstance.cpp
sed -i 's/(ULONG)pbMapEnd/(uintptr_t)pbMapEnd/g' GplDitherInstance.cpp
sed -i 's/(ULONG)pbSource/(uintptr_t)pbSource/g' GplDitherInstance.cpp
sed -i 's/(ULONG)params\.iMapSize/(uintptr_t)params.iMapSize/g' GplDitherInstance.cpp
sed -i 's/(ULONG)iMapSize/(uintptr_t)iMapSize/g' GplDitherInstance.cpp

# Fix const char* issues
sed -i 's/char \*pszName;/const char *pszName;/g' GplDitherInstance.cpp
sed -i 's/char \*pszPos;/const char *pszPos;/g' GplDitherInstance.cpp

echo "Fixed GplDitherInstance.cpp"