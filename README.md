CBOR
====

**Deprecated since Qt 5.12 where built-in CBOR support**

RFC 7049 Concise Binary Object Representation

Qt Implementation

Suported types of QVariant
* Invalid (null)
* Bool
* U/Int U/LongLong
* Double
* ByteArray
* String
* List
* Map

Notes
* Unit tests
* Simple to include to project
* Integers are packed to smallest value
* Floats always packed as double
* Tagged items not supported yet

