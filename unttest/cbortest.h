#ifndef CBORTEST_H
#define CBORTEST_H

#include <QObject>
#include <QTest>
#include "cbor.h"

#include <QDebug>

class CBORTest : public QObject
{
Q_OBJECT

private slots:

    void testNull()
    {
        auto n = CBOR::pack(QVariant());
        QVERIFY(n == "\xF6");
        QVERIFY(CBOR::unpack(n).isNull());
    }

    void testBool()
    {
        auto f = CBOR::pack(false);
        auto t = CBOR::pack(true);

        QVERIFY(f == "\xF4");
        QVERIFY(t == "\xF5");
        QVERIFY(CBOR::unpack(f) == false);
        QVERIFY(CBOR::unpack(t) == true);
    }

    void testInt()
    {

        auto uSmall = CBOR::pack(+0x17);
        auto iSmall = CBOR::pack(-0x17);

        auto u8 = CBOR::pack(+0xF1);
        auto i8 = CBOR::pack(-0xF1);

        auto u16 = CBOR::pack(+0x0123);
        auto i16 = CBOR::pack(-0x0123);

        auto u32 = CBOR::pack(+0x01234567);
        auto i32 = CBOR::pack(-0x01234567);

        auto u64 = CBOR::pack(quint64(+0x0123456789012345));
        auto i64 = CBOR::pack( qint64(-0x0123456789012345));


        QVERIFY(uSmall == "\x17");
        QVERIFY(iSmall == "\x36");

        QVERIFY(u8 == "\x18\xF1");
        QVERIFY(i8 == "\x38\xF0");

        QVERIFY(u16 == "\x19\x01\x23");
        QVERIFY(i16 == "\x39\x01\x22");

        QVERIFY(u32 == "\x1A\x01\x23\x45\x67");
        QVERIFY(i32 == "\x3A\x01\x23\x45\x66");

        QVERIFY(u64 == "\x1B\x01\x23\x45\x67\x89\x01\x23\x45");
        QVERIFY(i64 == "\x3B\x01\x23\x45\x67\x89\x01\x23\x44");

        QVERIFY(CBOR::unpack(uSmall) == +0x17);
        QVERIFY(CBOR::unpack(iSmall) == -0x17);

        QVERIFY(CBOR::unpack(u8) == +0xF1);
        QVERIFY(CBOR::unpack(i8) == -0xF1);

        QVERIFY(CBOR::unpack(u16) == +0x0123);
        QVERIFY(CBOR::unpack(i16) == -0x0123);

        QVERIFY(CBOR::unpack(u32) == +0x01234567);
        QVERIFY(CBOR::unpack(i32) == -0x01234567);

        QVERIFY(CBOR::unpack(u64) == quint64(+0x0123456789012345));
        QVERIFY(CBOR::unpack(i64) ==  qint64(-0x0123456789012345));
    }

    void testFloat()
    {
        auto x = 3.14;
        QVERIFY(CBOR::unpack(CBOR::pack(x)) == x);
    }

    void testArray()
    {
        QVariantList lst;

        lst << 42;
        lst << 3.14;
        lst << "Hello world";
        QVERIFY(CBOR::unpack(CBOR::pack(lst)) == lst);
    }

    void testMap()
    {
        QVariantMap map;

        map["int"] = 42;
        map["bar"] = 3.14;
        map["baz"] = "Hello world";
        QVERIFY(CBOR::unpack(CBOR::pack(map)) == map);
    }
};


#endif // CBORTEST_H
