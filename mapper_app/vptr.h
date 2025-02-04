#ifndef VPTR_H
#define VPTR_H
#include <QVariant>

template <class T> class VPtr
{
public:
    static T* asPtr(QVariant v)
    {
    return  (T *) v.value<void *>();
    }

    static QVariant asQVariant(T* ptr)
    {
    return QVariant::fromValue((void *) ptr);
    }
};


#endif // VPTR_H
