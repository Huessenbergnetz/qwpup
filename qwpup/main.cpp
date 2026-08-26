/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring / www.huessenbergnetz.de
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <QCoreApplication>

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName(u"Huessenbergnetz"_s);
    QCoreApplication::setOrganizationDomain(u"huessenbergnetz.de"_s);
    QCoreApplication::setApplicationName(u"qwpup"_s);
    QCoreApplication::setApplicationVersion(QStringLiteral(QWPUP_VERSION));

    return a.exec();
}
