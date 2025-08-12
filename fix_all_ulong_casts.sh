#!/bin/bash
# Fix all remaining ULONG casts in GplDitherInstance.cpp
echo "Fixing all remaining ULONG casts in GplDitherInstance.cpp..."

# Add cstdint include if not already present
if ! grep -q "#include <cstdint>" GplDitherInstance.cpp; then
    sed -i '1i #include <cstdint>' GplDitherInstance.cpp
fi

# Replace all (ULONG)pbSrc with (uintptr_t)pbSrc
sed -i 's/(ULONG)pbSrc/(uintptr_t)pbSrc/g' GplDitherInstance.cpp

# Replace all (ULONG)pbMapEnd with (uintptr_t)pbMapEnd
sed -i 's/(ULONG)pbMapEnd/(uintptr_t)pbMapEnd/g' GplDitherInstance.cpp

# Replace all (ULONG)pbSource with (uintptr_t)pbSource
sed -i 's/(ULONG)pbSource/(uintptr_t)pbSource/g' GplDitherInstance.cpp

# Replace all (ULONG)params.iMapSize with (uintptr_t)params.iMapSize
sed -i 's/(ULONG)params\.iMapSize/(uintptr_t)params.iMapSize/g' GplDitherInstance.cpp

# Replace all (ULONG)iMapSize with (uintptr_t)iMapSize
sed -i 's/(ULONG)iMapSize/(uintptr_t)iMapSize/g' GplDitherInstance.cpp

# Remove register keyword
sed -i 's/register INT i;/INT i;/g' GplDitherInstance.cpp

echo "Fixed all ULONG casts and register keyword"