/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef QWPUP_UTILS_H
#define QWPUP_UTILS_H

#include "enums.h"

#include <QString>
#include <QStringList>
#include <QtLogging>

namespace Utils {

/**
 * Returns the deafult logging level.
 */
inline constexpr QString defaultLogLevel()
{
#ifdef QT_DEBUG
    return QStringLiteral("debug");
#else
    return QStringLiteral("info");
#endif
}

/**
 * Returns a list of supported logging levels.
 */
inline constexpr QStringList logLevels()
{
    return {QStringLiteral("debug"), QStringLiteral("info"), QStringLiteral("warning"), QStringLiteral("critical")};
}

/**
 * Returns \c true if the \a loglevel string is a valid logging level.
 */
inline bool isLogLevelValid(QStringView loglevel)
{
    return Utils::logLevels().contains(loglevel, Qt::CaseInsensitive);
}

/**
 * Returns the logging level QtMsgType from the input string \a str.
 */
QtMsgType logLevel(QStringView str);

/**
 * Sets the logging level according to the \a type.
 */
void setLogLevel(QtMsgType type);

/**
 * @overload
 */
inline void setLogLevel(QStringView type)
{
    setLogLevel(logLevel(type));
}

/**
 * Returns the VersionPart based on the input string \a str. If \a str is not
 * a valid VersionPart, the default \a defVal will be returned.
 */
VersionPart versionPartFromString(const QString &str, VersionPart defVal = VersionPart::Invalid);

} // namespace Utils

#endif // QWPUP_UTILS_H
