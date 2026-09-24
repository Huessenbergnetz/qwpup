/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef QWPUP_COMPRESSOR_H
#define QWPUP_COMPRESSOR_H

#include <chrono>

#include <QObject>

class Compressor : public QObject
{
    Q_OBJECT
public:
    explicit Compressor(QObject *parent = nullptr);

    void start(const QStringList &files);

signals:
    void finished(std::chrono::nanoseconds duration);
};

#endif // QWPUP_COMPRESSOR_H
