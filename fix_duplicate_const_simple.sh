#!/bin/bash
# Fix duplicate const in GplDitherInstance.cpp
echo "Fixing duplicate const in GplDitherInstance.cpp..."

# Replace all instances of "const const char *pszPos" with "const char *pszPos"
sed -i 's/const const char \*pszPos/const char *pszPos/g' GplDitherInstance.cpp

echo "Fixed duplicate const"