#include "cbor.h"

#include <QtEndian>
#include <QTime>
#include <QDate>
#include <QDateTime>

namespace CBOR {

inline QByteArray nativeToBigEndian(quint16 v) {
    QByteArray ret;
    ret.resize(2);
    qToBigEndian(v, reinterpret_cast<uchar *>(ret.data()));
    return ret;
}

inline QByteArray nativeToBigEndian(quint32 v) {
    QByteArray ret;
    ret.resize(4);
    qToBigEndian(v, reinterpret_cast<uchar *>(ret.data()));
    return ret;
}

inline QByteArray nativeToBigEndian(quint64 v) {
    QByteArray ret;
    ret.resize(8);
    qToBigEndian(v, reinterpret_cast<uchar *>(ret.data()));
    return ret;
}

inline QByteArray nativeToBigEndian(double v) {
    QByteArray ret;
    ret.resize(8);
    qToBigEndian(v, reinterpret_cast<uchar *>(ret.data()));
    return ret;
}

inline char mkHeader(MajorType type, uchar info) {
    return char(((type & 0x07) << 5) | (info & 0x1F));
}

inline QByteArray mkHeaderEx(MajorType type, quint64 v) {
    QByteArray ret;

    if (v < TypeValue::nextByte) {
        ret += mkHeader(type, uchar(v & 0x1F));
    } else {
        if (v <= 0xFF) {
            ret += mkHeader(type, TypeValue::nextByte);
            ret += char(v);
        } else if (v <= 0xFFFF) {
            ret += mkHeader(type, TypeValue::next2Bytes);
            ret += nativeToBigEndian(quint16(v));
        } else if (v <= 0xFFFFFFFF) {
            ret += mkHeader(type, TypeValue::next4Bytes);
            ret += nativeToBigEndian(quint32(v));
        } else {
            ret += mkHeader(type, TypeValue::next8Bytes);
            ret += nativeToBigEndian(quint64(v));
        }
    }
    return ret;
}


quint64 parseHeader(QByteArray &data) {
    auto    sz  = static_cast<TypeValue>(data.at(0) & 0x1F);
    quint64 val = 0;
    if (sz < TypeValue::nextByte) {
        val  = static_cast<uchar>(sz);
        data = data.mid(1);
    } else if (sz == TypeValue::nextByte) {
        val  = static_cast<uchar>(data.at(1));
        data = data.mid(2);
    } else if (sz == TypeValue::next2Bytes) {
        val  = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(data.data() + 1));
        data = data.mid(3);
    } else if (sz == TypeValue::next4Bytes) {
        val  = qFromBigEndian<quint32>(reinterpret_cast<const uchar *>(data.data() + 1));
        data = data.mid(5);
    } else if (sz == TypeValue::next8Bytes) {
        val  = qFromBigEndian<quint64>(reinterpret_cast<const uchar *>(data.data() + 1));
        data = data.mid(9);
    } else {
        // TODO: Not implemented;
    }

    return val;
}

inline QVariant _unpack(QByteArray &data) {
    QVariant ret;

    if (data.length()) {
        auto header = static_cast<uchar>(data.at(0));
        switch (header >> 5) {
            case MajorType::positiveInt: {
                auto v = parseHeader(data);
                if (v <= 0xFFFFFFFF) {
                    ret = static_cast<quint32>(v);
                } else {
                    ret = static_cast<quint64>(v);
                }
            } break;
            case MajorType::negativeInt: {
                auto v = parseHeader(data) + 1;
                if (v <= 0xFFFFFFFF) {
                    ret = static_cast<quint32>(-v);
                } else {
                    ret = static_cast<quint64>(-v);
                }
            } break;
            case MajorType::bytes: {
                auto sz = static_cast<int>(parseHeader(data));
                ret     = data.mid(0, sz);
                data    = data.mid(sz);
            } break;
            case MajorType::str: {
                auto sz = static_cast<int>(parseHeader(data));
                ret     = QString::fromUtf8(data.mid(0, sz));
                data    = data.mid(sz);
            } break;
            case MajorType::array: {
                auto         sz = parseHeader(data);
                QVariantList array;
                for (quint64 i = 0; i < sz; ++i) {
                    array.append(_unpack(data));
                }
                ret = array;
            } break;
            case MajorType::map: {
                auto        sz = parseHeader(data);
                QVariantMap map;
                for (quint64 i = 0; i < sz; ++i) {
                    auto k            = _unpack(data);
                    auto v            = _unpack(data);
                    map[k.toString()] = v;
                }
                ret = map;
            } break;
            case MajorType::tagged:
                // TODO: Tags
                break;
            case MajorType::other: {
                auto st = header & 0x1F;
                data    = data.mid(1);
                if (st == OtherType::false_) {
                    ret = false;
                } else if (st == OtherType::true_) {
                    ret = true;
                } else if (st == OtherType::null_) {
                    ret = QVariant();
                } else if (st == OtherType::float32) {
                    float out=0.;
                    qbswap(*(const float *)data.constData(), reinterpret_cast<uchar *>(&out));
                    ret       = out;
                    data      = data.mid(4);
                } else if (st == OtherType::float64) {
                    double out=0.;
                    qbswap(*(const double *)data.constData(), reinterpret_cast<uchar *>(&out));
                    ret       = out;
                    data      = data.mid(8);
                }
            } break;
        }
    }
    return ret;
}
}

QByteArray CBOR::pack(const QVariant &v) {
    QByteArray ret;

    switch (v.type()) {
        case QVariant::Invalid: ret += mkHeader(MajorType::other, OtherType::null_); break;
        case QVariant::Bool:
            ret += mkHeader(MajorType::other, v.toBool() ? OtherType::true_ : OtherType::false_);
            break;
        case QVariant::Int:
        case QVariant::LongLong: {
            auto tp  = MajorType::positiveInt;
            auto val = v.toLongLong();
            if (val < 0) {
                tp  = MajorType::negativeInt;
                val = qAbs(val) - 1;
            }
            ret += mkHeaderEx(tp, static_cast<quint64>(val));
        } break;
        case QVariant::UInt:
        case QVariant::ULongLong: {
            ret += mkHeaderEx(MajorType::positiveInt, v.toULongLong());
        } break;
        case QVariant::Double: {
            ret += mkHeader(MajorType::other, OtherType::float64);
            ret.append(nativeToBigEndian(v.toDouble()));
        } break;
        case QVariant::ByteArray: {
            auto b = v.toByteArray();
            ret += mkHeaderEx(MajorType::bytes, static_cast<quint64>(b.length()));
            ret += b;
        } break;
        case QVariant::String: {
            auto s = v.toString().toUtf8();
            ret += mkHeaderEx(MajorType::str, static_cast<quint64>(s.length()));
            ret += s;
        } break;
        case QVariant::List: {
            auto array = v.toList();
            ret += mkHeaderEx(MajorType::array, static_cast<quint64>(array.length()));
            for (auto &item : array) {
                ret += pack(item);
            }
        } break;
        case QVariant::StringList: {
            auto array = v.toStringList();
            ret += mkHeaderEx(MajorType::array, static_cast<quint64>(array.length()));
            for (auto &item : array) {
                ret += pack(item);
            }
        } break;
        case QVariant::Map: {
            auto map = v.toMap();
            ret += mkHeaderEx(MajorType::map, static_cast<quint64>(map.size()));
            for (QString key : map.keys()) {
                ret += pack(key);
                ret += pack(map[key]);
            }
        } break;
        case QVariant::Time:
            ret += pack(v.toTime().toString("HH:mm:ss"));
            break;
        case QVariant::Date:
            ret += pack(v.toDate().toString("yyyy-MM-dd"));
            break;
        case QVariant::DateTime:
            ret += pack(v.toDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz"));
            break;
        default:; // TODO: Unsupported
    }
    return ret;
}


QVariant CBOR::unpack(const QByteArray &v) {
    auto data = v;
    return _unpack(data);
}
