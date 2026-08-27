/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "errorcodes.h"
#include "qwpup.h"

#include <QDebug>
#include <QLocale>
#include <QTranslator>

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName(u"Huessenbergnetz"_s);
    QCoreApplication::setOrganizationDomain(u"huessenbergnetz.de"_s);
    QCoreApplication::setApplicationName(u"qwpup"_s);
    QCoreApplication::setApplicationVersion(QStringLiteral(QWPUP_VERSION));

    {
        const QLocale locale;
        auto trans = new QTranslator(&app); // NOLINT(cppcoreguidelines-owning-memory)
        if (Q_LIKELY(trans->load(locale, QCoreApplication::applicationName(), u"_"_s, QStringLiteral(QWPUP_I18NDIR)))) {
            if (Q_UNLIKELY(!QCoreApplication::installTranslator(trans))) {
                qWarning() << "Failed to install translator for" << locale;
            }
        } else {
            qWarning() << "Failed to load translations for" << locale << "from" << QWPUP_I18NDIR;
        }
    }

    auto qwpup = new QWpUp(&app); // NOLINT(cppcoreguidelines-owning-memory)
    if (auto ec = qwpup->start(QCoreApplication::arguments()); ec != Error::None) {
        return static_cast<int>(ec);
    }

    return app.exec();
}
