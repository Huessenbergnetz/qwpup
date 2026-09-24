/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef QWPUP_WP_H
#define QWPUP_WP_H

#include <QObject>

class QProcess;
class QProcessEnvironment;

class WP : public QObject
{
    Q_OBJECT
public:
    explicit WP(const QString &program,
                const QString &workDir,
                const QProcessEnvironment &env,
                const QStringList &args,
                QObject *parent = nullptr);

    void start();

signals:
    void succeeded(const QByteArray &data);
    void failed(const QString &error);

private:
    QProcess *m_wp;
};

#endif // QWPUP_WP_H
