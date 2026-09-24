/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "wp.h"

#include <QProcess>

WP::WP(const QString &program,
       const QString &workDir,
       const QProcessEnvironment &env,
       const QStringList &args,
       QObject *parent)
    : QObject{parent}
    , m_wp{new QProcess(this)}
{
    m_wp->setProgram(program);
    m_wp->setWorkingDirectory(workDir);
    m_wp->setProcessEnvironment(env);
    m_wp->setArguments(args);
}

void WP::start()
{
    connect(m_wp, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        deleteLater();
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            emit succeeded(m_wp->readAllStandardOutput().trimmed());
        } else {
            emit failed(QString::fromLocal8Bit(m_wp->readAllStandardError().trimmed()));
        }
    });
    m_wp->start();
}

#include "moc_wp.cpp"
