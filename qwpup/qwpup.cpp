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
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLocale>
#include <QLoggingCategory>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextStream>
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

    QCommandLineOption skipCompOpt(QStringList({u"s"_s, u"skip-compression"_s}),
                                   //: Option description in the CLI help
                                   //% "Skip compressing JS and CSS assets."
                                   qtTrId("qwpup_cli_opt_skip_comp"));
    parser.addOption(skipCompOpt);

    QCommandLineOption yesOpt(QStringList({u"y"_s, u"yes"_s}),
                              //: Option description in the CLI help
                              //% "Say yes to everything."
                              qtTrId("qwpup_cli_opt_yes"));
    parser.addOption(yesOpt);

    QCommandLineOption upWpMajOpt(QStringList({u"m"_s, u"major"_s}),
                                  //: Option description in the CLI help
                                  //% "Update WordPress to a new major version. By default only minor version udpates "
                                  //% "will be performed."
                                  qtTrId("qwpup_cli_opt_up_wp_maj"));
    parser.addOption(upWpMajOpt);

    QCommandLineOption wpDirOpt(QStringList({u"w"_s, u"wp-dir"_s}),
                                //: Option description in the CLI help
                                //% "Path to the WordPress root directory. If omitted, the current directory will be used."
                                qtTrId("qwpup_cli_opt_wp_dir"),
                                qtTrId("qwpup_cli_opt_val_path"));
    parser.addOption(wpDirOpt);

    QCommandLineOption upPlugsVerOpt(u"plugins-version"_s,
                                     //: Option description in the CLI help, DO NOT TRANSLATE the terms
                                     //: major, minor and patch
                                     //% "Only perform plugin updates for major, minor or patch releases: Default: major."
                                     qtTrId("qwpup_cli_opt_plug_ver"),
                                     //: Option value name in the CLI help vor version number part like major, minor
                                     //% "part"
                                     qtTrId("qwpup_cli_opt_value_ver_part"),
                                     u"major"_s);
    parser.addOption(upPlugsVerOpt);

    QCommandLineOption upThemVerOpt(u"themes-version"_s,
                                    //: Option description in the CLI help, DO NOT TRANSLATE the terms
                                    //% "Only perform theme updates for major, minor or patch releases: Default: major."
                                    qtTrId("qwpup_cli_opt_themes_ver"),
                                    qtTrId("qwpup_cli_opt_value_ver_part"),
                                    u"major"_s);
    parser.addOption(upThemVerOpt);

    QCommandLineOption wpCliOpt(u"wp-cli"_s,
                                //: Option description in the CLI help
                                //% "Path to the WP CLI executable. By default, this will be searched in the PATH."
                                qtTrId("qwpup_cli_opt_wp_cli"),
                                //: Option value name in the cli help for file and directory paths
                                //% "path"
                                qtTrId("qwpup_cli_opt_val_path"));
    parser.addOption(wpCliOpt);

    parser.process(arguments);

    // Set the log level

    const QString logLevel = parser.value(logLevelOpt).toLower();
    if (!logLevels.contains(logLevel)) {
        //: Error message
        //% "Invalid log level."
        qCritical().noquote() << qtTrId("qwpup_err_inv_ll");
        return Error::Config;
    }
    m_logLevel = Utils::logLevel(logLevel);
    Utils::setLogLevel(m_logLevel);

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

    // Set WordPress directory

    if (parser.isSet(wpDirOpt)) {
        m_wpDir.setPath(parser.value(wpDirOpt));
        if (!m_wpDir.exists()) {
            //: Error message, %1 will be replaced by the absolute path to the directory
            //% "The directory “%1” does not exist."
            qCritical().noquote() << qtTrId("qwpup_err_wp_dir_not_exists").arg(m_wpDir.absolutePath());
            return Error::Config;
        }
    } else {
        m_wpDir = QDir::current();
    }

    const QFileInfo wpConfigFi{m_wpDir, u"wp-config.php"_s};

    if (!wpConfigFi.exists()) {
        //: Error message, %1 will be replaced with a CLI option name like --wp-dir
        //% "Can not find wp-config.php configuration file. We seem not to be inside the root directory of "
        //% "a WordPress installation. Either run this command inside a WordPress root directory or use "
        //% "“%1” to specify the path to a WordPress root directory."
        qCritical().noquote() << qtTrId("qwpup_err_wp_config_not_found").arg(u"--wp-dir"_s);
        return Error::File;
    }

    qDebug() << "WordPress directory:" << m_wpDir.absolutePath();

    // Check if current user is owner of wp-config.php

    if (wpConfigFi.owner() != user) {
        //% "Current user it not the owner of the wp-config.php file. Please run the "
        //% "command as owner of the WordPress files."
        qCritical().noquote() << qtTrId("qwpup_err_wp_config_owner_mismatch");
        return Error::Security;
    }

    // Check for wp cli executable

    if (parser.isSet(wpCliOpt)) {
        m_wpPath = QDir::cleanPath(QDir::current().absoluteFilePath(parser.value(wpCliOpt)));
        if (!QFileInfo::exists(m_wpPath)) {
            //: Error message, %1 will be replaced by the full file path
            //% "No file found at “%1“."
            qCritical().noquote() << qtTrId("qwpup_err_wp_exe_path_wrong").arg(m_wpPath);
            return Error::Config;
        }
    }

    if (m_wpPath.isEmpty()) {
        m_wpPath = QStandardPaths::findExecutable(u"wp"_s);
    }
    if (m_wpPath.isEmpty()) {
        m_wpPath = QStandardPaths::findExecutable(u"wp-cli"_s);
    }
    if (m_wpPath.isEmpty()) {
        //: Error message, %1 will be replaced with a CLI option name like --wp-cli
        //% "Can not find WP CLI executable. Check your PATH or explicitely set the path to the executable with “%1”."
        qCritical().noquote() << qtTrId("qwpup_err_wp_exe_not_found").arg(u"--wp-cli"_s);
        return Error::Config;
    }
    qDebug() << "Found WP CLI executable at" << m_wpPath;

    m_skipCompression = parser.isSet(skipCompOpt);

    qDebug() << "Skip compression:" << m_skipCompression;

    m_wpUpMajor = parser.isSet(upWpMajOpt);

    qDebug() << "Update WordPress major version:" << m_wpUpMajor;

    const QString upPlugsVerStr = parser.value(upPlugsVerOpt).toLower();
    m_plugsUpVersion            = Utils::versionPartFromString(upPlugsVerStr);

    if (m_plugsUpVersion == VersionPart::Invalid) {
        //: Error message, DO NOT TRANSLATE the terms major, minor and patch
        //% "Invalid version part identifier. Only major, minor or patch are allowed."
        qCritical().noquote() << qtTrId("qwpup_err_wp_invalid_version_part");
        return Error::Config;
    }

    qDebug() << "Update plugins version:" << upPlugsVerStr;

    const QString upThemesVerStr = parser.value(upThemVerOpt).toLower();
    m_themesUpVersion            = Utils::versionPartFromString(upThemesVerStr);

    if (m_themesUpVersion == VersionPart::Invalid) {
        qCritical().noquote() << qtTrId("qwpup_err_wp_invalid_version_part");
        return Error::Config;
    }

    qDebug() << "Update themes version:" << upThemesVerStr;

    m_tempDir = std::make_unique<QTemporaryDir>();
    if (!m_tempDir->isValid()) {
        //: Error message, %1 will be replaced by the error message
        //% "Failed to create temporary directory: %1"
        qCritical().noquote() << qtTrId("qwpup_err_invalid_tmp_dir").arg(m_tempDir->errorString());
        return Error::File;
    }

    qDebug() << "Temporary directory:" << m_tempDir->path();

    m_sayYes = parser.isSet(yesOpt);

    qDebug() << "Say yes:" << m_sayYes;

    m_env = QProcessEnvironment::systemEnvironment();
    m_env.insert(u"WP_CLI_CACHE_DIR"_s, m_tempDir->path());

    QTimer::singleShot(0, this, &QWpUp::doStart);

    return Error::None;
}

void QWpUp::doStart()
{
    getCurrentVersion();
}

void QWpUp::getCurrentVersion()
{
    auto wp = wpProcess({u"core"_s, u"version"_s});
    connect(wp, &QProcess::finished, this, [this, wp](int exitCode, QProcess::ExitStatus exitStatus) {
        wp->deleteLater();
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            m_currentCoreVersion = QString::fromLocal8Bit(wp->readAllStandardOutput()).trimmed();
            if (m_logLevel == QtDebugMsg) {
                showVersionInfo();
            } else {
                //: Info message, %1 will be replaced by the version string like 6.8.3
                //% "Current WordPress core version: %1"
                qInfo().noquote() << qtTrId("qwpup_inf_cur_wp_core_version").arg(m_currentCoreVersion);
                listCoreVersions();
            }
        } else {
            qCritical().noquote() << wp->readAllStandardError().trimmed();
            //% "Failed to get version information."
            handleError(qtTrId("qwpup_err_wp_version_info_failed"), Error::Internal);
        }
    });
    wp->start();
}

void QWpUp::showVersionInfo()
{
    auto wp = wpProcess({u"core"_s, u"version"_s, u"--extra"_s});
    connect(wp, &QProcess::finished, this, [this, wp](int exitCode, QProcess::ExitStatus exitStatus) {
        wp->deleteLater();
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            qDebug().noquote() << wp->readAllStandardOutput();
            listCoreVersions();
        } else {
            qCritical().noquote() << wp->readAllStandardError().trimmed();
            handleError(qtTrId("qwpup_err_wp_version_info_failed"), Error::Internal);
        }
    });
    wp->start();
}

void QWpUp::listCoreVersions()
{
    //% "Checking for core updates."
    qInfo().noquote() << qtTrId("qwpup_info_check_core_updates");
    auto wp = wpProcess({u"core"_s, u"check-update"_s, u"--format=json"_s});
    connect(wp, &QProcess::finished, this, [this, wp](int exitCode, QProcess::ExitStatus exitStatus) {
        wp->deleteLater();
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {

            QJsonParseError jpe;
            auto json = QJsonDocument::fromJson(wp->readAllStandardOutput(), &jpe);
            if (jpe.error != QJsonParseError::NoError) {
                //: Error message, %1 will be replaced by the error message from the JSON parser.
                //% "Failed to parse JSON data: %1"
                handleError(qtTrId("qwpup_err_json_parse_failed").arg(jpe.errorString()), Error::Internal);
                return;
            }

            if (!json.isArray()) {
                //% "Unexpected JSON type. Aborting."
                handleError(qtTrId("qwpup_err_json_unexpected_type"), Error::Internal);
                return;
            }

            auto array = json.array();

            if (array.isEmpty()) {
                qInfo() << "No Core updates available.";
                QTimer::singleShot(0, this, &QWpUp::checkPluginUpdates);
                return;
            }

            for (const auto &v : array) {
                const auto o          = v.toObject();
                const auto updateType = o.value("update_type"_L1).toString();
                if (updateType == "minor"_L1) {
                    m_minCoreUpAvail      = true;
                    m_availMinCoreVersoin = o.value("version"_L1).toString();
                    //% "New minor core version available: %1"
                    qInfo().noquote() << qtTrId("qwpup_inf_min_core_ver_avail").arg(m_availMinCoreVersoin);
                }
                if (updateType == "major"_L1) {
                    m_majCoreUpAvail      = true;
                    m_availMajCoreVersion = o.value("version"_L1).toString();
                    //% "New major core version available: %1"
                    qInfo().noquote() << qtTrId("qwpup_inf_maj_core_ver_avail").arg(m_availMajCoreVersion);
                }
            }

            if ((!m_wpUpMajor && m_minCoreUpAvail) || (m_wpUpMajor && m_majCoreUpAvail)) {
                updateCore();
            } else {
                //% "Skipping core update."
                qInfo().noquote() << qtTrId("qwpup_info_skip_core_update");
                checkPluginUpdates();
            }

        } else {
            qCritical().noquote() << wp->readAllStandardError().trimmed();
            //% "Failed to check for core updates."
            handleError(qtTrId("qwpup_err_ep_core_up_check_failed"), Error::Internal);
        }
    });
    wp->start();
}

void QWpUp::updateCore()
{
    const QString targetVersion = m_wpUpMajor ? m_availMajCoreVersion : m_availMinCoreVersoin;

    if (!m_sayYes) {
        //: %1 will be replaced by the current version number, %2 by the
        //: target version
        //% "Do you want to update WordPress core from version %1 to %2?"
        if (askYesNo(qtTrId("qwpup_ask_update_core").arg(m_currentCoreVersion, targetVersion)) != Answer::Yes) {
            QTimer::singleShot(0, this, &QWpUp::checkPluginUpdates);
            return;
        }
    }

    //: Info message, %1 will be replaced by the current WordPress core version,
    //: %2 will be replaced by the newer version
    //% "Updating WordPress core from version %1 to version %2."
    qInfo().noquote() << qtTrId("qwpup_info_update_core").arg(m_currentCoreVersion, targetVersion);

    QStringList args({u"core"_s, u"update"_s});
    if (!m_wpUpMajor) {
        args << u"--minor"_s;
    }

    auto wp = wpProcess(args);
    connect(wp, &QProcess::finished, this, [this, wp, targetVersion](int exitCode, QProcess::ExitStatus exitStatus) {
        wp->deleteLater();
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            //: Info message, %1 will be replaced by the previous WordPress core version,
            //: %2 will be replaced by the now updated version
            //% "Successfully updated WordPres core from version %1 to version %2."
            qInfo().noquote() << qtTrId("qwpup_infi_update_core_success").arg(m_currentCoreVersion, targetVersion);
            m_coreUpdated = true;
            checkPluginUpdates();
        } else {
            qCritical().noquote() << wp->readAllStandardError().trimmed();
            //% "Failed to update WordPress core."
            handleError(qtTrId("qwpup_err_wp_version_info_failed"), Error::Internal);
        }
    });
    wp->start();
}

void QWpUp::checkPluginUpdates()
{
    //% "Checking for plugin updates."
    qInfo().noquote() << qtTrId("qwpup_info_check_plugin_updates");

    QCoreApplication::exit();
}

void QWpUp::handleError(const QString &msg, Error exitCode)
{
    qCritical().noquote() << msg;

    QCoreApplication::exit(static_cast<int>(exitCode));
}

QProcess *QWpUp::wpProcess(const QStringList &arguments)
{
    auto proc = new QProcess(this); // NOLINT(cppcoreguidelines-owning-memory)
    proc->setProgram(m_wpPath);
    proc->setWorkingDirectory(m_wpDir.absolutePath());
    proc->setProcessEnvironment(m_env);
    proc->setArguments(arguments);
    return proc;
}

Answer QWpUp::askYesNo(const QString &question)
{
    QTextStream out(stdout);
    QTextStream in(stdin);

    QString line;

    out << question << " [(Y)es/(N)o/(C)ancel]: " << Qt::flush;
    while (in.readLineInto(&line)) {
        const auto trimmedLine = line.trimmed();
        if (trimmedLine.compare("Y"_L1, Qt::CaseInsensitive) == 0 ||
            trimmedLine.compare("Yes"_L1, Qt::CaseInsensitive) == 0) {
            return Answer::Yes;
        } else if (trimmedLine.compare("N"_L1, Qt::CaseInsensitive) == 0 ||
                   trimmedLine.compare("No"_L1, Qt::CaseInsensitive) == 0) {
            return Answer::No;
        } else if (trimmedLine.compare("C"_L1, Qt::CaseInsensitive) == 0 ||
                   trimmedLine.compare("Cancel"_L1, Qt::CaseInsensitive) == 0) {
            QCoreApplication::exit();
            return Answer::Cancel;
        }
        out << question << " [(Y)es/(N)o/(C)ancel]: " << Qt::flush;
    }

    QCoreApplication::exit();
    return Answer::Cancel;
}

#include "moc_qwpup.cpp"
