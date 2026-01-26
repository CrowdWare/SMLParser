# SMLParser

C++11 Parser fuer SML (SimpleMarkupLanguage) mit SAX-Style Callbacks. Wird in SMLUI und RaidBuilder genutzt.

## Features
- Elemente und Properties
- Datentypen: int, float, bool, string
- Vec2i / Vec3i (aus "x,y" bzw. "x,y,z")
- Enum-Registrierung pro Property
- Fehler mit Zeilen/Spalten-Info (SmlParseException)

## Build
```sh
cmake -S . -B build
cmake --build build
```
oder `src/sml_parser.cpp` direkt in dein Projekt einbinden.

## Kurzbeispiel
```cpp
sml::SmlSaxParser parser(text);
MyHandler handler;
parser.registerEnumValue("icon", "play");
parser.parse(handler);
```

## API Einstieg
- Header: `include/sml_parser.h`
- Zentral: `sml::SmlSaxParser` + `sml::SmlHandler`
