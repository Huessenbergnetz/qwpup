/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "qwpup.h"

#include "utils.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QLoggingCategory>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QTimer>

using namespace Qt::Literals::StringLiterals;

QWpUp::QWpUp(QCoreApplication *parent)
    : QObject{parent}
{
}

Error QWpUp::start(const QStringList &arguments)
{
    QCommandLineParser parser;

    //: Application description for the CLI help overview
    //% "Qt based wrapper for WP CLI to automate WordPress updates with some extras."
    parser.setApplicationDescription(qtTrId("qwpup_cli_app_desc"));

    parser.addHelpOption();
    parser.addVersionOption();

    const QLocale locale;
    const QStringList logLevels({u"debug"_s, u"info"_s, u"warn"_s, u"crit"_s});
#ifdef QT_DEBUG
    const QString defLl = u"debug"_s;
#else
    const QString defLl = u"info"_s;
#endif
    QCommandLineOption logLevelOpt(QStringList({u"l"_s, u"log-level"_s}),
                                   //: Option description in the CLI help
                                   //% "Log level and higher for that messages are shown. Available: %1. Default: %2"
                                   qtTrId("qwpup_cli_opt_log_level").arg(locale.createSeparatedList(logLevels), defLl),
                                   //: Option value name in the CLI help for the log level
                                   //% "level"
                                   qtTrId("qwpup_cli_opt_log_level_val"),
                                   defLl);
    parser.addOption(logLevelOpt);

    QCommandLineOption wpCliOpt(u"wp-cli"_s,
                                //: Option description in the CLI help
                                //% "Path to the WP CLI executable. By default, this will be searched in the PATH."
                                qtTrId("qwpup_cli_opt_wp_cli"),
                                //: Option value name in the cli help for file and directory paths
                                //% "path"
                                qtTrId("qwpup_cli_opt_val_path"));
    parser.addOption(wpCliOpt);

    QCommandLineOption wpDirOpt(u"wp-dir"_s,
                                //: Option description in the CLI help
                                //% "Path to the WordPress root directory. If omitted, the current directory will be used."
                                qtTrId("qwpup_cli_opt_wp_dir"),
                                qtTrId("qwpup_cli_opt_val_path"));
    parser.addOption(wpDirOpt);

    parser.process(arguments);

    // Set the log level

    const QString logLevel = parser.value(logLevelOpt).toLower();
    if (!logLevels.contains(logLevel)) {
        //: Error message
        //% "Invalid log level."
        qCritical().noquote() << qtTrId("qwpup_err_inv_ll");
        return Error::Config;
    }
    Utils::setLogLevel(logLevel);

    // Check if we are not root

    QString user = QString::fromLocal8Bit(qgetenv("USER"));
    if (user.isEmpty()) {
        user = QString::fromLocal8Bit(qgetenv("USERNAME"));
    }
    if (user.isEmpty()) {
        //% "Failed to get current user."
        qCritical().noquote() << qtTrId("qwpup_err_user_unknown");
        return Error::Internal;
    }

    if (user.compare("root"_L1, Qt::CaseInsensitive) == 0) {
        //% "Do not run this command as super user (root)."
        qCritical().noquote() << qtTrId("qwpup_err_user_root");
        return Error::Security;
    }

    qDebug() << "Running as user" << user;

    // Check for wp cli executable

    if (parser.isSet(wpCliOpt)) {
        m_wp = QDir::cleanPath(QDir::current().absoluteFilePath(parser.value(wpCliOpt)));
        if (!QFileInfo::exists(m_wp)) {
            //: Error message, %1 will be replaced by the full file path
            //% "No file found at “%1“."
            qCritical().noquote() << qtTrId("qwpup_err_wp_exe_path_wrong").arg(m_wp);
            return Error::Config;
        }
    }

    if (m_wp.isEmpty()) {
        m_wp = QStandardPaths::findExecutable(u"wp"_s);
    }
    if (m_wp.isEmpty()) {
        m_wp = QStandardPaths::findExecutable(u"wp-cli"_s);
    }
    if (m_wp.isEmpty()) {
        //: Error message, %1 will be replaced with a CLI option name like --wp-cli
        //% "Can not find WP CLI executable. Check your PATH or explicitely set the path to the executable with “%1”."
        qCritical().noquote() << qtTrId("qwpup_err_wp_exe_not_found").arg(u"--wp-cli"_s);
        return Error::Config;
    }
    qDebug() << "Found WP CLI executable at" << m_wp;

    // Set WordPress directory

    bool wpDirHasBeenSet = false;
    if (parser.isSet(wpDirOpt)) {
        m_wpDir.setPath(parser.value(wpDirOpt));
        if (!m_wpDir.exists()) {
            //: Error message, %1 will be replaced by the absolute path to the directory
            //% "The directory “%1” does not exist."
            qCritical().noquote() << qtTrId("qwpup_err_wp_dir_not_exists").arg(m_wpDir.absolutePath());
            return Error::Config;
        }
        wpDirHasBeenSet = true;
    }

    if (!wpDirHasBeenSet) {
        m_wpDir = QDir::current();
    }

    if (!m_wpDir.exists(u"wp-config.php"_s)) {
        //: Error message, %1 will be replaced with a CLI option name like --wp-dir
        //% "Can not find wp-config.php configuration file. We seem not to be inside the root directory of "
        //% "a WordPress installation. Either run this command inside a WordPress root directory or use "
        //% "“%1” to specify the path to a WordPress root directory."
        qCritical().noquote() << qtTrId("qwpup_err_wp_config_not_found").arg(u"--wp-dir"_s);
        return Error::File;
    }

    qDebug() << "WordPress directory:" << m_wpDir.absolutePath();

    QTimer::singleShot(0, this, &QWpUp::doStart);

    return Error::None;
}

void QWpUp::doStart()
{
    QCoreApplication::exit();
}

void QWpUp::handleError(const QString &msg, Error exitCode)
{
    qCritical().noquote() << msg;

    QCoreApplication::exit(static_cast<int>(exitCode));
}

#include "moc_qwpup.cpp"
