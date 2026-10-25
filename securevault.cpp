#include "securevault.h"
//#include <qt6keychain/keychain.h> // Change to <qt5keychain/keychain.h> if targeting Qt5
#include "keychain.h"
#include <QDebug>

// Define a permanent namespace for your app so the OS isolates its credentials
const QString APP_NAMESPACE = "CityMapperApp";

SecureVault::SecureVault(QObject *parent) : QObject(parent) {}

void SecureVault::savePassword(const QString &key, const QString &password)
{
    auto *job = new QKeychain::WritePasswordJob(APP_NAMESPACE, this);
    job->setKey(key);
    job->setTextData(password.toUtf8());

    connect(job, &QKeychain::Job::finished, this, [key, job]() {
        if (job->error()) {
            qCritical() << "Vault: Failed to write password for key:" << key << "Error:" << job->errorString();
        } else {
            qDebug() << "Vault: Securely saved password for key:" << key;
        }
        job->deleteLater();
    });
    job->start();
}

void SecureVault::loadPassword(const QString &key, std::function<void(const QString&)> callback)
{
    auto *job = new QKeychain::ReadPasswordJob(APP_NAMESPACE, this);
    job->setKey(key);

    connect(job, &QKeychain::Job::finished, this, [key, job, callback]() {
        QString password = "";
        if (job->error()) {
            qCritical() << "Vault: Failed to read password for key:" << key << "Error:" << job->errorString();
        } else {
            password = job->textData();
        }
        // Execute the lambda callback pass with the unlocked string payload
        callback(password);
        job->deleteLater();
    });
    job->start();
}
