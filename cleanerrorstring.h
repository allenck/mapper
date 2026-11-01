#ifndef CLEANERRORSTRING_H
#define CLEANERRORSTRING_H
#include <QString>
QString cleanErrorString(const QString &rawError) {
#ifdef Q_OS_MAC
    // 1. Convert the garbage QString back to its raw physical bytes
    QByteArray rawBytes = rawError.toUtf8();

    // 2. Check if it looks like standard ASCII text falsely padded with nulls (UTF-16 trait)
    // If the 2nd byte is 0, it's highly likely to be a UTF-16 string misinterpreted as UTF-8/ASCII
    if (rawBytes.size() > 1 && rawBytes.at(1) == '\0') {
        // Reinterpret the raw data as actual UTF-16 LE characters
        return QString::fromUtf16(reinterpret_cast<const char16_t*>(rawBytes.constData()), rawBytes.size() / 2);
    }
#endif

    return rawError; // Return untouched for Windows/Ubuntu or if it's already clean
}
#endif // CLEANERRORSTRING_H
