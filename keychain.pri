# === QKeychain Definitions ===
# later versions of these files can be downloaded from https://github.com/frankosterfeld/qtkeychain.git
#
DEFINES += QTKEYCHAIN_VERSION_6 \
    QTKEYCHAIN_NO_EXPORT

# Tell the compiler where to resolve <qtkeychain/...> style include paths
INCLUDEPATH += $$PWD/QtKeychain

SOURCES += $$PWD/securevault.cpp \
        QtKeychain/qtkeychain/keychain.cpp

HEADERS += $$PWD/securevault.h \
        QtKeychain/qtkeychain/keychain.h \
        QtKeychain/qtkeychain/keychain_p.h \
        QtKeychain/qtkeychain/gnomekeyring_p.h \
        QtKeychain/qtkeychain/libsecret_p.h \
        QtKeychain/qtkeychain/plaintextstore_p.h

# === CROSS-PLATFORM SECURITY VAULT LINKAGE ===
win32 {
    # 1. WINDOWS LAPTOP CONFIGURATION
    LIBS += -lcrypt32 \
            -ladvapi32
    SOURCES += QtKeychain/qtkeychain/keychain_win.cpp \
        QtKeychain/qtkeychain/plaintextstore.cpp
}

unix:!macx {
    # 2. UBUNTU LINUX SERVER CONFIGURATION
    SOURCES += QtKeychain/qtkeychain/keychain_unix.cpp \
    QtKeychain/qtkeychain/gnomekeyring.cpp \
    QtKeychain/qtkeychain/libsecret.cpp \
    QtKeychain/qtkeychain/plaintextstore.cpp
    QT += dbus
    DEFINES +=     KEYCHAIN_DBUS
    DBUS_INTERFACES += QtKeychain/qtkeychain/org.kde.KWallet.xml
}


macx {
    LIBS += -framework Security -framework CoreFoundation
    OBJECTIVE_SOURCES += QtKeychain/qtkeychain/keychain_apple.mm
}