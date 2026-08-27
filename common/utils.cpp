/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "utils.h"

#include <QLoggingCategory>
#include <QString>

using namespace Qt::Literals::StringLiterals;

QtMsgType Utils::logLevel(const QString &str)
{
    if (str.compare("info"_L1) == 0) {
        return QtInfoMsg;
    } else if (str.compare("warn"_L1) == 0) {
        return QtWarningMsg;
    } else if (str.compare("crit"_L1) == 0) {
        return QtCriticalMsg;
    }

    return QtDebugMsg;
}

void Utils::setLogLevel(QtMsgType type)
{
    auto cat = QLoggingCategory::defaultCategory();

    switch (type) {
    case QtCriticalMsg:
        cat->setEnabled(QtCriticalMsg, true);
        cat->setEnabled(QtWarningMsg, false);
        cat->setEnabled(QtInfoMsg, false);
        cat->setEnabled(QtDebugMsg, false);
        break;
    case QtWarningMsg:
        cat->setEnabled(QtCriticalMsg, true);
        cat->setEnabled(QtWarningMsg, true);
        cat->setEnabled(QtInfoMsg, false);
        cat->setEnabled(QtDebugMsg, false);
        break;
    case QtInfoMsg:
        cat->setEnabled(QtCriticalMsg, true);
        cat->setEnabled(QtWarningMsg, true);
        cat->setEnabled(QtInfoMsg, true);
        cat->setEnabled(QtDebugMsg, false);
        break;
    default:
        cat->setEnabled(QtCriticalMsg, true);
        cat->setEnabled(QtWarningMsg, true);
        cat->setEnabled(QtInfoMsg, true);
        cat->setEnabled(QtDebugMsg, true);
        qDebug() << "Debug log level enabled";
        break;
    }
}
