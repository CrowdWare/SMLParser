# SMLParser

C++11 SML parser with SAX-style callbacks.

Features:
- Elements and properties
- Scalars: int, float, bool, string
- Vec2i / Vec3i from comma-separated integers
- Enum registry per property

Usage:
- Add `SMLParser` as a library target (CMake) or compile `src/sml_parser.cpp` with `include/` on your include path.
