#ifndef CBOR_H
#define CBOR_H

#include <QVariant>

namespace CBOR {

enum MajorType : uchar {
    positiveInt = 0,
    negativeInt = 1,
    bytes       = 2,
    str         = 3,
    array       = 4,
    map         = 5,
    tagged      = 6,
    other       = 7,
};

enum TypeValue : uchar {
    nextByte   = 24,
    next2Bytes = 25,
    next4Bytes = 26,
    next8Bytes = 27,
};

enum OtherType : uchar {
    false_    = 20,
    true_     = 21,
    null_     = 22,
    undefined = 23,
    float16   = 25,
    float32   = 26,
    float64   = 27,
    stopCode  = 31
};

QByteArray pack(const QVariant &v);
QVariant unpack(const QByteArray &v);

} // CBOR namespace

#endif // CBOR_H
