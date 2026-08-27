/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef QWPUP_H
#define QWPUP_H

#include "errorcodes.h"

#include <QCoreApplication>
#include <QDir>

class QWpUp : public QObject
{
    Q_OBJECT
public:
    explicit QWpUp(QCoreApplication *parent = nullptr);

    Error start(const QStringList &arguments);

private slots:
    void doStart();

private:
    void handleError(const QString &msg, Error exitCode);

    QString m_wp;
    QDir m_wpDir;
};

#endif // QWPUP_H
