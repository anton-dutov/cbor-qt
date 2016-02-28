#ifndef CBOR_H
#define CBOR_H

#include <QVariant>
#include <QtEndian>

namespace CBOR
{
    enum MajorType: uchar
    {
        positiveInt = 0,
        negativeInt = 1,
        bytes       = 2,
        str         = 3,
        array       = 4,
        map         = 5,
        tag         = 6,
        other       = 7,
    };

    enum IntSize: uchar
    {
        s8  = 24,
        s16 = 25,
        s32 = 26,
        s64 = 27,
    };

    enum OtherType : uchar
    {
        false_    = 20,
        true_     = 21,
        null_     = 22,
        undefined = 23,
        nextByte  = 24,
        float16   = 25,
        float32   = 26,
        float64   = 27,
        stopCode  = 31
    };

    QByteArray pack(const QVariant& v);
    QVariant unpack(const QByteArray& v);
}

#endif // CBOR_H
