/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef QWPUP_UTILS_H
#define QWPUP_UTILS_H

#include <QtLogging>

namespace Utils {

QtMsgType logLevel(const QString &str);

void setLogLevel(QtMsgType type);

inline void setLogLevel(const QString &level)
{
    setLogLevel(logLevel(level));
}
} // namespace Utils

#endif // QWPUP_UTILS_H
