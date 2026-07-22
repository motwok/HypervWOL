# Copilot Instructions

## Projektrichtlinien
- MAC-Adressen sollen im Projekt als std::array<unsigned char, 6> (wie in VmMacCatalog::MacKey) gehandhabt werden, statt als rohe Byte-Arrays.
- WOL-Datenpakete sollen als std::array statt als roher Pointer übergeben werden.
