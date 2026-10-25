#ifndef SECUREVAULT_H
#define SECUREVAULT_H
#pragma once
#include <QObject>
#include <QString>
#include <functional>

class SecureVault : public QObject
{
    Q_OBJECT
public:
    explicit SecureVault(QObject *parent = nullptr);

    // Securely saves or updates a password tied to a specific connection/profile name
    void savePassword(const QString &key, const QString &password);

    // Asynchronously loads a password and fires a callback function when ready
    void loadPassword(const QString &key, std::function<void(const QString&)> callback);
};
#endif // SECUREVAULT_H
