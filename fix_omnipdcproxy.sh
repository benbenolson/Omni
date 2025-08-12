#!/bin/bash
# Fix const char* to char* conversion issues and pointer to int cast issues in OmniPDCProxy.cpp
echo "Fixing OmniPDCProxy.cpp..."

# Add cstdint include if not already present
if ! grep -q "#include <cstdint>" OmniPDCProxy.cpp; then
    sed -i '/#include <cstdarg>/a #include <cstdint>' OmniPDCProxy.cpp
fi

# Fix all strchr const char* to char* conversions
sed -i 's/pszSpace = strchr (pszJPQuoted, '\'' '\'')/pszSpace = const_cast<char*>(strchr (pszJPQuoted, '\'' '\''))/g' OmniPDCProxy.cpp
sed -i 's/pszSpace = strchr (pszNetworkJobProperties, '\'' '\'')/pszSpace = const_cast<char*>(strchr (pszNetworkJobProperties, '\'' '\''))/g' OmniPDCProxy.cpp

# Fix pointer to int casts
sed -i 's/if (-1 == (int)pbBuffer1_d/if (-1 == (uintptr_t)pbBuffer1_d/g' OmniPDCProxy.cpp
sed -i 's/if (-1 == (int)pbBuffer2_d/if (-1 == (uintptr_t)pbBuffer2_d/g' OmniPDCProxy.cpp

echo "Fixed OmniPDCProxy.cpp"