/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef QWPUP_H
#define QWPUP_H

#include "enums.h"

#include <memory>

#include <QCoreApplication>
#include <QDir>
#include <QProcessEnvironment>

class QProcess;
class QTemporaryDir;

class QWpUp : public QObject
{
    Q_OBJECT
public:
    explicit QWpUp(QCoreApplication *parent = nullptr);

    Error start(const QStringList &arguments);

private slots:
    void doStart();
    void getCurrentVersion();
    void showVersionInfo();
    void listCoreVersions();
    void updateCore();
    void checkPluginUpdates();

private:
    void handleError(const QString &msg, Error exitCode);
    QProcess *wpProcess(const QStringList &arguments);
    Answer askYesNoCancel(const QString &question);
    Answer askYesNo(const QString &question);

    QString m_wpPath;
    QString m_currentCoreVersion;
    QString m_availMajCoreVersion;
    QString m_availMinCoreVersoin;
    QDir m_wpDir;
    std::unique_ptr<QTemporaryDir> m_tempDir;
    QProcessEnvironment m_env;
    VersionPart m_plugsUpVersion{VersionPart::Major};
    VersionPart m_themesUpVersion{VersionPart::Major};
    QtMsgType m_logLevel{QtDebugMsg};
    bool m_skipCompression{false};
    bool m_wpUpMajor{false};
    bool m_sayYes{false};
    bool m_coreUpdated{false};
    bool m_majCoreUpAvail{false};
    bool m_minCoreUpAvail{false};
};

#endif // QWPUP_H
