/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef QWPUP_CONDUCTOR_H
#define QWPUP_CONDUCTOR_H

#include "enums.h"

#include <expected>

#include <QLocale>
#include <QObject>
#include <QQueue>
#include <QUrl>

class QDir;
class QProcess;

class Conductor : public QObject
{
    Q_OBJECT
public:
    explicit Conductor(QObject *parent = nullptr);

    Error start(const QStringList &args);

signals:
    void siteUpdated();

private slots:
    void updateSite();

private:
    void sendEmail();
    void finish();

private:
    struct Site {
        QString name;
        QString path;
        QString user;
        QString group;
        QStringList mailReceivers;
        VersionPart pluginsVersion{VersionPart::Major};
        VersionPart themesVersion{VersionPart::Major};
        bool skipCompression{false};
        bool coreMajor{false};
    };

    Error readConfiguration(const QString &configFilePath);
    void handleError(const QString &msg, Error exitCode);
    static std::expected<uint, QString> getUidFromWpConfig(const QString &path);
    static std::expected<uint, QString> getGidFromWpConfig(const QString &path);

    QQueue<Site> m_sites;
    QString m_mailFrom;
    QString m_qwpupPath;
    QLocale m_locale;
    QUrl m_mailUrl;
    QtMsgType m_logLevel{QtDebugMsg};
    bool m_mailEnabled{false};
    bool m_dryRun{false};
};

#endif // QWPUP_CONDUCTOR_H
