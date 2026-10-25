#ifndef REGISTRYWATCHER_H
#define REGISTRYWATCHER_H

#include <QObject>
#include <QWinEventNotifier>
#include <windows.h>

class RegistryWatcher : public QObject {
    Q_OBJECT
public:
    RegistryWatcher(HKEY hive, const QString &subKey, QObject *parent = nullptr)
        : QObject(parent), m_hKey(nullptr), m_hEvent(nullptr), m_notifier(nullptr)
    {
        // Open the registry key
        LSTATUS status = RegOpenKeyExW(
            hive,
            reinterpret_cast<LPCWSTR>(subKey.utf16()),
            0,
            KEY_NOTIFY,
            &m_hKey
            );

        if (status != ERROR_SUCCESS) return;

        // Create an event object
        m_hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        if (!m_hEvent) return;

        // Register for registry change notifications
        status = RegNotifyChangeKeyValue(
            m_hKey,
            TRUE, // True = monitor subtree changes, False = only immediate key
            REG_NOTIFY_CHANGE_LAST_SET, // Notify on value modifications
            m_hEvent,
            TRUE // Asynchronous execution
            );

        if (status == ERROR_SUCCESS) {
            // Hook the Win32 handle into Qt's event loop
            m_notifier = new QWinEventNotifier(m_hEvent, this);
            connect(m_notifier, &QWinEventNotifier::activated, this, &RegistryWatcher::onKeyChanged);
        }
    }

    ~RegistryWatcher() {
        if (m_notifier) m_notifier->setEnabled(false);
        if (m_hKey) RegCloseKey(m_hKey);
        if (m_hEvent) CloseHandle(m_hEvent);
    }

signals:
    void registryKeyChanged();

private slots:
    void onKeyChanged() {
        m_notifier->setEnabled(false);
        ResetEvent(m_hEvent);

        emit registryKeyChanged();

        // Re-arm the notifier for the next change
        RegNotifyChangeKeyValue(
            m_hKey, TRUE, REG_NOTIFY_CHANGE_LAST_SET, m_hEvent, TRUE
            );
        m_notifier->setEnabled(true);
    }

private:
    HKEY m_hKey;
    HANDLE m_hEvent;
    QWinEventNotifier *m_notifier;
};

#endif // REGISTRYWATCHER_H
