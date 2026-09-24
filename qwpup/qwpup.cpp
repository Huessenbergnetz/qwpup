/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "qwpup.h"

#include "compressor.h"
#include "utils.h"
#include "wp.h"

#include <chrono>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonValue>
#include <QLoggingCategory>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextStream>
#include <QTimer>
#include <QVersionNumber>

using namespace Qt::Literals::StringLiterals;

QWpUp::QWpUp(QCoreApplication *parent)
    : QObject{parent}
{
}

/**
 * Parse command line arguments and set options.
 *
 * When parsing finished successfully, call doStart().
 */
Error QWpUp::start(const QStringList &arguments)
{
    QCommandLineParser parser;

    //: Application description for the CLI help overview
    //% "Qt based wrapper for WP CLI to automate WordPress updates with some extras."
    parser.setApplicationDescription(qtTrId("qwpup_cli_app_desc"));

    parser.addHelpOption();
    parser.addVersionOption();

    const QStringList logLevels({u"debug"_s, u"info"_s, u"warn"_s, u"crit"_s});
#ifdef QT_DEBUG
    const QString defLl = u"debug"_s;
#else
    const QString defLl = u"info"_s;
#endif
    QCommandLineOption logLevelOpt(QStringList({u"l"_s, u"log-level"_s}),
                                   //: Option description in the CLI help
                                   //% "Log level and higher for that messages are shown. Available: %1. Default: %2"
                                   qtTrId("qwpup_cli_opt_log_level").arg(m_locale.createSeparatedList(logLevels), defLl),
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

    QCommandLineOption dryRunOpt(u"dry-run"_s,
                                 //: Option description in the CLI help
                                 //% "Do a dry run without performing real actions."
                                 qtTrId("qwpup_cli_opt_dry_run"));
    parser.addOption(dryRunOpt);

    QCommandLineOption statsOpt(u"stats"_s,
                                //: Option description in the CLI help
                                //% "Write stats to file at path. Path may also be stdout."
                                qtTrId("qwpup_cli_opt_stats"),
                                qtTrId("qwpup_cli_opt_val_path"));
    parser.addOption(statsOpt);

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
            //% "Can not find WP CLI executable at “%1“."
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
        //% "Can not find WP CLI executable (wp or wp-cli). Check your PATH or explicitely set the path to the executable "
        //% "with “%1”."
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

    if (parser.isSet(statsOpt)) {
        m_statsFilePath = parser.value(statsOpt);
        qDebug().noquote() << "Writing statst to:" << m_statsFilePath;
    }

    m_dryRun = parser.isSet(dryRunOpt);
    if (m_dryRun) {
        //% "Doing dry run without performing real actions."
        qInfo().noquote() << qtTrId("qwpup_info_perform_dry_run");
    }

    m_env = QProcessEnvironment::systemEnvironment();
    m_env.insert(u"WP_CLI_CACHE_DIR"_s, m_tempDir->path());

    QTimer::singleShot(0, this, &QWpUp::doStart);

    return Error::None;
}

void QWpUp::doStart()
{
    if (m_statsFilePath.isEmpty()) {
        QTimer::singleShot(0, this, &QWpUp::getCurrentVersion);
        return;
    }

    QTimer::singleShot(0, this, &QWpUp::getBlogName);
}

void QWpUp::getBlogName()
{
    QTimer::singleShot(0, this, &QWpUp::getCurrentVersion);
}

void QWpUp::getCurrentVersion()
{
    auto wp = wpProcess({u"core"_s, u"version"_s});

    connect(wp, &WP::succeeded, this, [this](const QByteArray &data) {
        m_currentCoreVersion = QString::fromLocal8Bit(data);
        if (m_logLevel == QtDebugMsg) {
            QTimer::singleShot(0, this, &QWpUp::showVersionInfo);
        } else {
            //: Info message, %1 will be replaced by the version string like 6.8.3
            //% "Current WordPress core version: %1"
            qInfo().noquote() << qtTrId("qwpup_inf_cur_wp_core_version").arg(m_currentCoreVersion);
            QTimer::singleShot(0, this, &QWpUp::listCoreVersions);
        }
    });

    connect(wp, &WP::failed, this, [this](const QString &error) {
        m_stats.insert("error"_L1, error);
        qCritical().noquote() << error;
        //% "Failed to get version information."
        handleError(qtTrId("qwpup_err_wp_version_info_failed"), Error::Internal);
    });

    wp->start();
}

void QWpUp::showVersionInfo()
{
    auto wp = wpProcess({u"core"_s, u"version"_s, u"--extra"_s});

    connect(wp, &WP::succeeded, this, [this](const QByteArray &data) {
        qDebug().noquote() << data;
        QTimer::singleShot(0, this, &QWpUp::listCoreVersions);
    });

    connect(wp, &WP::failed, this, [this](const QString &error) {
        m_stats.insert("error"_L1, error);
        qCritical().noquote() << error;
        handleError(qtTrId("qwpup_err_wp_version_info_failed"), Error::Internal);
    });

    wp->start();
}

void QWpUp::listCoreVersions()
{
    //% "Checking for core updates."
    qInfo().noquote() << qtTrId("qwpup_info_check_core_updates");

    auto wp = wpProcess({u"core"_s, u"check-update"_s, u"--format=json"_s});

    connect(wp, &WP::succeeded, this, [this](const QByteArray &data) {
        const auto array = getJsonArray(data);
        if (!array) {
            m_stats.insert("error"_L1, array.error());
            handleError(array.error(), Error::Internal);
            return;
        }

        if (array->isEmpty()) {
            //% "No core updates available."
            qInfo().noquote() << qtTrId("qpwup_info_no_core_ups_avail");
            QTimer::singleShot(0, this, &QWpUp::checkPluginUpdates);
            return;
        }

        for (const auto &v : *array) {
            const auto o          = v.toObject();
            const auto updateType = o.value("update_type"_L1).toString();
            if (updateType == "minor"_L1) {
                m_minCoreUpAvail      = true;
                m_availMinCoreVersion = o.value("version"_L1).toString();
                //% "New minor core version available: %1"
                qInfo().noquote() << qtTrId("qwpup_inf_min_core_ver_avail").arg(m_availMinCoreVersion);
            }
            if (updateType == "major"_L1) {
                m_majCoreUpAvail      = true;
                m_availMajCoreVersion = o.value("version"_L1).toString();
                //% "New major core version available: %1"
                qInfo().noquote() << qtTrId("qwpup_inf_maj_core_ver_avail").arg(m_availMajCoreVersion);
            }
        }

        if ((!m_wpUpMajor && m_minCoreUpAvail) || (m_wpUpMajor && m_majCoreUpAvail)) {
            QTimer::singleShot(0, this, &QWpUp::updateCore);
        } else {
            //% "Skipping major core update."
            qInfo().noquote() << qtTrId("qwpup_info_skip_major_core_update");
            QTimer::singleShot(0, this, &QWpUp::checkPluginUpdates);
        }
    });

    connect(wp, &WP::failed, this, [this](const QString &error) {
        m_stats.insert("error"_L1, error);
        qCritical().noquote() << error;
        //% "Failed to check for core updates."
        handleError(qtTrId("qwpup_err_ep_core_up_check_failed"), Error::Internal);
    });

    wp->start();
}

void QWpUp::updateCore()
{
    const QString targetVersion = m_wpUpMajor ? m_availMajCoreVersion : m_availMinCoreVersion;

    if (!m_sayYes) {
        //: %1 will be replaced by the current version number, %2 by the
        //: target version
        //% "Do you want to update WordPress core from version %1 to %2?"
        if (askYesNoCancel(qtTrId("qwpup_ask_update_core").arg(m_currentCoreVersion, targetVersion)) != Answer::Yes) {
            QTimer::singleShot(0, this, &QWpUp::checkPluginUpdates);
            return;
        }
    }

    //: Info message, %1 will be replaced by the current WordPress core version,
    //: %2 will be replaced by the newer version
    //% "Updating WordPress core from version %1 to version %2."
    qInfo().noquote() << qtTrId("qwpup_info_update_core").arg(m_currentCoreVersion, targetVersion);

    if (m_dryRun) {
        //: Info message, %1 will be replaced by the previous WordPress core version,
        //: %2 will be replaced by the now updated version
        //% "Successfully updated WordPres core from version %1 to version %2."
        qInfo().noquote() << qtTrId("qwpup_infi_update_core_success").arg(m_currentCoreVersion, targetVersion);
        m_coreUpdated = true;
        QTimer::singleShot(0, this, &QWpUp::checkPluginUpdates);
        return;
    }

    QStringList args({u"core"_s, u"update"_s});
    if (!m_wpUpMajor) {
        args << u"--minor"_s;
    }

    auto wp = wpProcess(args);

    connect(wp, &WP::succeeded, this, [this, targetVersion]() {
        qInfo().noquote() << qtTrId("qwpup_infi_update_core_success").arg(m_currentCoreVersion, targetVersion);
        m_coreUpdated = true;
        QTimer::singleShot(0, this, &QWpUp::checkPluginUpdates);
    });

    connect(wp, &WP::failed, this, [this](const QString &error) {
        m_stats.insert("error"_L1, error);
        qCritical().noquote() << error;
        //% "Failed to update WordPress core."
        handleError(qtTrId("qwpup_err_wp_core_update_failed"), Error::Internal);
    });

    wp->start();
}

void QWpUp::checkPluginUpdates()
{
    //% "Checking for plugin updates."
    qInfo().noquote() << qtTrId("qwpup_info_check_plugin_updates");

    auto wp = wpProcess({u"plugin"_s, u"list"_s, u"--update=available"_s, u"--format=json"_s});

    connect(wp, &WP::succeeded, this, [this](const QByteArray &data) {
        const auto array = getJsonArray(data);
        if (!array) {
            qWarning().noquote() << array.error();
            QTimer::singleShot(0, this, &QWpUp::checkThemeUpdates);
            return;
        }

        if (array->empty()) {
            //% "No plugin updates available."
            qInfo().noquote() << qtTrId("qwpup_info_no_plug_ups_avail");
            QTimer::singleShot(0, this, &QWpUp::checkThemeUpdates);
            return;
        }

        QList<QJsonObject> pluginsWithUpdates;

        for (const auto &v : *array) {
            const auto o = v.toObject();
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
            const auto curVer = QVersionNumber::fromString(o.value("version"_L1).toStringView()).normalized();
            const auto updVer = QVersionNumber::fromString(o.value("update_version"_L1).toStringView()).normalized();
#else
            const auto curVer = QVersionNumber::fromString(o.value("version"_L1).toString()).normalized();
            const auto updVer = QVersionNumber::fromString(o.value("update_version"_L1).toString()).normalized();
#endif
            const auto commonPrefix = QVersionNumber::commonPrefix(curVer, updVer);

            // NOLINTNEXTLINE(bugprone-branch-clone)
            if (m_plugsUpVersion == VersionPart::Patch) {
                if (commonPrefix.segmentCount() >= 2) {
                    pluginsWithUpdates.append(o); // clazy:exclude=reserve-candidates
                    m_pluginsToUpdate.enqueue(o);
                } else {
                    m_skippedPlugins.append(v);
                }
            } else if (m_plugsUpVersion == VersionPart::Minor) {
                if (commonPrefix.segmentCount() >= 1) {
                    pluginsWithUpdates.append(o); // clazy:exclude=reserve-candidates
                    m_pluginsToUpdate.enqueue(o);
                } else {
                    m_skippedPlugins.append(v);
                }
            } else {
                pluginsWithUpdates.append(o); // clazy:exclude=reserve-candidates
                m_pluginsToUpdate.enqueue(o);
            }
        }

        if (m_logLevel == QtDebugMsg) {

            //: Used when no updates for e.g. plugins and themes are available, in
            //: a form like "Available plugin updates: none"
            //% "none"
            const QString none = qtTrId("qwpup_info_updates_none");

            if (!pluginsWithUpdates.empty()) {
                for (const auto &o : std::as_const(pluginsWithUpdates)) {
                    const auto name          = o.value("name"_L1).toString();
                    const auto version       = o.value("version"_L1).toString();
                    const auto updateVersion = o.value("update_version"_L1).toString();
                    //: %1 will be replaced by the plugin name, %2 by the current version, %3 by the udpate version
                    //% "Available plugin update: %1 %2 => %3"
                    qDebug().noquote() << qtTrId("qwpup_dbg_avail_plug_up").arg(name, version, updateVersion);
                }
            } else {
                //: %1 will be replaced by a comma separated list of plugin updates or "none".
                //% "Available plugin updates: %1."
                qDebug().noquote() << qtTrId("qwpup_info_avail_plug_ups").arg(none);
            }

            if (!m_skippedPlugins.empty()) {
                for (const auto &v : std::as_const(m_skippedPlugins)) {
                    const auto o             = v.toObject();
                    const auto name          = o.value("name"_L1).toString();
                    const auto version       = o.value("version"_L1).toString();
                    const auto updateVersion = o.value("update_version"_L1).toString();
                    //: %1 will be replaced by the plugin name, %2 by the current version, %3 by the udpate version
                    //% "Skipped plugin update: %1 %2 => %3"
                    qDebug().noquote() << qtTrId("qwpup_dbg_skipped_plug_up").arg(name, version, updateVersion);
                }
            } else {
                //: %1 will be replaced by a comma separated list of plugin updates.
                //% "Skipped plugin updates: %1."
                qDebug().noquote() << qtTrId("qwpup_info_skip_plug_ups").arg(none);
            }

        } else if (m_logLevel == QtInfoMsg) {

            const QString none = qtTrId("qwpup_info_updates_none");

            if (!pluginsWithUpdates.empty()) {
                QStringList availUpdates;
                availUpdates.reserve(pluginsWithUpdates.size());
                for (const auto &o : std::as_const(pluginsWithUpdates)) {
                    availUpdates << o.value("name"_L1).toString();
                }
                qInfo().noquote() << qtTrId("qwpup_info_avail_plug_ups").arg(m_locale.createSeparatedList(availUpdates));
            } else {
                qInfo().noquote() << qtTrId("qwpup_info_avail_plug_ups").arg(none);
            }

            if (!m_skippedPlugins.empty()) {
                QStringList skippedUpdates;
                skippedUpdates.reserve(m_skippedPlugins.size());
                for (const auto &v : std::as_const(m_skippedPlugins)) {
                    skippedUpdates << v.toObject().value("name"_L1).toString();
                }
                qInfo().noquote() << qtTrId("qwpup_info_skip_plug_ups").arg(m_locale.createSeparatedList(skippedUpdates));
            } else {
                qInfo().noquote() << qtTrId("qwpup_info_skip_plug_ups").arg(none);
            }
        }

        if (m_pluginsToUpdate.isEmpty()) {
            QTimer::singleShot(0, this, &QWpUp::checkThemeUpdates);
        } else {
            QTimer::singleShot(0, this, &QWpUp::updatePlugin);
        }
    });

    connect(wp, &WP::failed, this, [this](const QString &error) {
        qWarning().noquote() << error;
        //% "Failed to check for plugin updates."
        qWarning().noquote() << qtTrId("qwpup_err_plug_check_failed");
        QTimer::singleShot(0, this, &QWpUp::checkThemeUpdates);
    });

    wp->start();
}

void QWpUp::updatePlugin()
{
    if (m_pluginsToUpdate.empty()) {
        QTimer::singleShot(0, this, &QWpUp::checkThemeUpdates);
        return;
    }

    const QJsonObject o      = m_pluginsToUpdate.dequeue();
    const auto name          = o.value("name"_L1).toString();
    const auto version       = o.value("version"_L1).toString();
    const auto updateVersion = o.value("update_version"_L1).toString();

    if (!m_sayYes) {
        //: %1 will be replaced by the plugin’s name, %2 by the current version
        //: and %3 by the update version
        //% "Do you want to update the plugin “%1” from version %2 to %3?"
        if (askYesNo(qtTrId("qwpup_ask_update_plugin").arg(name, version, updateVersion)) != Answer::Yes) {
            m_skippedPlugins.append(o);
            QTimer::singleShot(0, this, &QWpUp::updatePlugin);
            return;
        }
    }

    //: %1 will be replaced by the plugin’s name, %2 by the current version
    //: and %3 by the update version
    //% "Updating plugin %1 from version %2 to %3."
    qInfo().noquote() << qtTrId("qwpup_info_update_plugin").arg(name, version, updateVersion);

    QStringList args({u"plugin"_s, u"update"_s, name, u"--format=json"_s});
    if (m_dryRun) {
        args << u"--dry-run"_s;
    }

    auto wp = wpProcess(args);

    connect(wp, &WP::succeeded, this, [this, o, name, version, updateVersion]() {
        m_updatedPlugins.append(o);

        //: %1 will be replaced by the plugin’s name, %2 by the current version
        //: and %3 by the update version
        //% "Successfully updated plugin %1 from version %2 to %3."
        qInfo().noquote() << qtTrId("qwpup_info_plug_up_success").arg(name, version, updateVersion);

        if (m_coreUpdated || m_dryRun || m_skipCompression) {
            QTimer::singleShot(0, this, &QWpUp::updatePlugin);
            return;
        }

        const QStringList assets = getPluginAssets(name);

        if (assets.empty()) {
            QTimer::singleShot(0, this, &QWpUp::updatePlugin);
            return;
        }

        //: %1 will be replaced by the plugin name
        //% "Start compressing assets for plugin %1."
        qInfo().noquote() << qtTrId("qwpup_info_plug_compr_assets").arg(name);

        auto c = new Compressor(this); // NOLINT(cppcoreguidelines-owning-memory)
        connect(c, &Compressor::finished, this, [this, name](std::chrono::nanoseconds duration) {
            //: %1 will be replaced by the plugin name, %2 by the duration
            //: the compression took in miliseconds
            //% "Finished compressing assets for plugin %1 in %2 ms."
            qInfo().noquote() << qtTrId("qwpup_info_plug_compr_assets_finished")
                                     .arg(name,
                                          m_locale.toString(
                                              std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()));
            QTimer::singleShot(0, this, &QWpUp::updatePlugin);
        });
        c->start(assets);
    });

    connect(wp, &WP::failed, this, [this, o, name, version, updateVersion](const QString &error) {
        QJsonObject _o = o;
        _o.insert("error"_L1, error);
        m_failedPlugins.append(_o);
        qWarning().noquote() << error;
        //: %1 will be replaced by the plugin’s name, %2 by the current version
        //: and %3 by the update version
        //% "Failed to update plugin %1 from version %2 to %3."
        qWarning().noquote() << qtTrId("qwpup_err_plug_up_failed").arg(name, version, updateVersion);
        QTimer::singleShot(0, this, &QWpUp::updatePlugin);
    });

    wp->start();
}

void QWpUp::checkThemeUpdates()
{
    //% "Checking for theme updates."
    qInfo().noquote() << qtTrId("qwpup_info_check_theme_updates");

    auto wp = wpProcess({u"theme"_s, u"list"_s, u"--update=available"_s, u"--format=json"_s});

    connect(wp, &WP::succeeded, this, [this](const QByteArray &data) {
        const auto array = getJsonArray(data);
        if (!array) {
            qWarning().noquote() << array.error();
            QTimer::singleShot(0, this, &QWpUp::checkCoreTranslations);
            return;
        }

        if (array->isEmpty()) {
            //% "No theme updates available."
            qInfo().noquote() << qtTrId("qwpup_info_no_theme_ups_avail");
            QTimer::singleShot(0, this, &QWpUp::checkCoreTranslations);
            return;
        }

        QList<QJsonObject> themesWithUpdates;

        for (const auto &v : *array) {
            const auto o = v.toObject();
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
            const auto curVer = QVersionNumber::fromString(o.value("version"_L1).toStringView()).normalized();
            const auto updVer = QVersionNumber::fromString(o.value("update_version"_L1).toStringView()).normalized();
#else
            const auto curVer = QVersionNumber::fromString(o.value("version"_L1).toString()).normalized();
            const auto updVer = QVersionNumber::fromString(o.value("update_version"_L1).toString()).normalized();
#endif
            const auto comPref = QVersionNumber::commonPrefix(curVer, updVer);

            if (m_themesUpVersion == VersionPart::Patch) {
                if (comPref.segmentCount() >= 2) {
                    themesWithUpdates.append(o); // clazy:exclude=reserve-candidates
                    m_themesToUpdate.enqueue(o);
                } else {
                    m_skippedThemes.append(v);
                }
            } else if (m_themesUpVersion == VersionPart::Minor) {
                if (comPref.segmentCount() >= 1) {
                    themesWithUpdates.append(o); // clazy:exclude=reserve-candidates
                    m_themesToUpdate.enqueue(o);
                } else {
                    m_skippedThemes.append(v);
                }
            } else {
                themesWithUpdates.append(o); // clazy:exclude=reserve-candidates
                m_themesToUpdate.enqueue(o);
            }
        }

        if (m_logLevel == QtDebugMsg) {

            const QString none = qtTrId("qwpup_info_updates_none");

            if (!themesWithUpdates.empty()) {
                for (const auto &o : std::as_const(themesWithUpdates)) {
                    const auto name   = o.value("name"_L1).toString();
                    const auto curVer = o.value("version"_L1).toString();
                    const auto updVer = o.value("update_version"_L1).toString();
                    //: %1 will be replaced by the theme name, %2 by the current version, %3 by the update version
                    //% "Available theme update: %1 %2 => %3"
                    qDebug().noquote() << qtTrId("qwpup_dbg_avail_theme_up").arg(name, curVer, updVer);
                }
            } else {
                //: %1 will be replaced by a comma separated list of theme updates or "none".
                //% "Available theme updates: %1."
                qDebug().noquote() << qtTrId("qwpup_info_avail_theme_ups").arg(none);
            }

            if (!m_skippedThemes.empty()) {
                for (const auto &v : std::as_const(m_skippedThemes)) {
                    const auto o      = v.toObject();
                    const auto name   = o.value("name"_L1).toString();
                    const auto curVer = o.value("version"_L1).toString();
                    const auto updVer = o.value("update_version"_L1).toString();
                    //: %1 will be replaced by the theme name, %2 by the current version, %3 by the update version
                    //% "Skipped theme update: %1 %2 => %3"
                    qDebug().noquote() << qtTrId("qwpup_dbg_skipped_theme_up").arg(name, curVer, updVer);
                }
            } else {
                //: %1 will be replaced by a comma separated list of theme updates.
                //% "Skipped theme updates: %1."
                qDebug().noquote() << qtTrId("qwpup_info_skip_theme_ups").arg(none);
            }

        } else if (m_logLevel == QtInfoMsg) {

            const QString none = qtTrId("qwpup_info_updates_none");

            if (!themesWithUpdates.empty()) {
                QStringList availUpdates;
                availUpdates.reserve(themesWithUpdates.size());
                for (const auto &o : std::as_const(themesWithUpdates)) {
                    availUpdates << o.value("name"_L1).toString();
                }
                qInfo().noquote() << qtTrId("qwpup_info_avail_theme_ups").arg(m_locale.createSeparatedList(availUpdates));
            } else {
                qInfo().noquote() << qtTrId("qwpup_info_avail_theme_ups").arg(none);
            }

            if (!m_skippedThemes.empty()) {
                QStringList skippedUpdates;
                skippedUpdates.reserve(m_skippedThemes.size());
                for (const auto &v : std::as_const(m_skippedThemes)) {
                    skippedUpdates << v.toObject().value("name"_L1).toString();
                }
                qInfo().noquote() << qtTrId("qwpup_info_skip_theme_ups").arg(m_locale.createSeparatedList(skippedUpdates));
            } else {
                qDebug().noquote() << qtTrId("qwpup_info_skip_theme_ups").arg(none);
            }
        }

        if (m_themesToUpdate.empty()) {
            QTimer::singleShot(0, this, &QWpUp::checkCoreTranslations);
        } else {
            QTimer::singleShot(0, this, &QWpUp::updateTheme);
        }
    });

    connect(wp, &WP::failed, this, [this](const QString &error) {
        qWarning().noquote() << error;
        //% "Failed to check for theme updates."
        qWarning().noquote() << qtTrId("qwpup_err_theme_check_failed");
        QTimer::singleShot(0, this, &QWpUp::checkCoreTranslations);
    });

    wp->start();
}

void QWpUp::updateTheme()
{
    if (m_themesToUpdate.empty()) {
        QTimer::singleShot(0, this, &QWpUp::checkCoreTranslations);
        return;
    }

    const QJsonObject o = m_themesToUpdate.dequeue();
    const auto name     = o.value("name"_L1).toString();
    const auto curVer   = o.value("version"_L1).toString();
    const auto updVer   = o.value("update_version"_L1).toString();

    if (!m_sayYes) {
        //: %1 will be replaced by the themes’s name, %2 by the current version
        //: and %3 by the update version
        //% "Do you want to update the theme “%1” from version %2 to %3?"
        if (askYesNo(qtTrId("qwpup_ask_update_theme").arg(name, curVer, updVer)) != Answer::Yes) {
            m_skippedThemes.append(o);
            QTimer::singleShot(0, this, &QWpUp::updateTheme);
            return;
        }
    }

    //: %1 will be replaced by the themes’s name, %2 by the current version
    //: and %3 by the update version
    //% "Updating theme %1 from version %2 to %3."
    qInfo().noquote() << qtTrId("qwpup_info_update_theme").arg(name, curVer, updVer);

    QStringList args({u"theme"_s, u"update"_s, name, u"--format=json"_s});
    if (m_dryRun) {
        args << u"--dry-run"_s;
    }

    auto wp = wpProcess(args);

    connect(wp, &WP::succeeded, this, [this, o, name, curVer, updVer](const QByteArray &data) {
        m_updatedThemes.append(o);

        //: %1 will be replaced by the themes’s name, %2 by the current version
        //: and %3 by the update version
        //% "Successfully updated theme %1 from version %2 to %3."
        qInfo().noquote() << qtTrId("qwpup_info_theme_up_success").arg(name, curVer, updVer);

        if (m_coreUpdated || m_dryRun || m_skipCompression) {
            QTimer::singleShot(0, this, &QWpUp::updateTheme);
            return;
        }

        const QStringList assets = getThemeAssets(name);

        if (assets.empty()) {
            QTimer::singleShot(0, this, &QWpUp::updateTheme);
            return;
        }

        //: %1 will be replaced by the theme name
        //% "Start compressing assets for theme %1."
        qInfo().noquote() << qtTrId("qwpup_info_theme_compr_assets").arg(name);

        auto c = new Compressor(this); // NOLINT(cppcoreguidelines-owning-memory)
        connect(c, &Compressor::finished, this, [this, name](std::chrono::nanoseconds duration) {
            //: %1 will be replaced by the theme name, %2 by the duration
            //: the compression took in miliseconds
            //% "Finished compressing assets for theme %1 in %2 ms."
            qInfo().noquote() << qtTrId("qwpup_info_theme_compr_assets_finished")
                                     .arg(name,
                                          m_locale.toString(
                                              std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()));
            QTimer::singleShot(0, this, &QWpUp::updateTheme);
        });
        c->start(assets);
    });

    connect(wp, &WP::failed, this, [this, o, name, curVer, updVer](const QString &error) {
        QJsonObject _o = o;
        _o.insert("error"_L1, error);
        qWarning().noquote() << error;
        //: %1 will be replaced by the plugin’s name, %2 by the current version
        //: and %3 by the update version
        //% "Failed to update theme %1 from version %2 to %3."
        qWarning().noquote() << qtTrId("qwpup_err_theme_up_failed").arg(name, curVer, updVer);
        QTimer::singleShot(0, this, &QWpUp::updateTheme);
    });

    wp->start();
}

void QWpUp::checkCoreTranslations()
{
    //% "Checking for core translation updates."
    qInfo().noquote() << qtTrId("qwpup_info_check_core_trans_updates");

    auto wp = wpProcess({u"language"_s, u"core"_s, u"list"_s, u"--update=available"_s, u"--format=json"_s});

    connect(wp, &WP::succeeded, this, [this](const QByteArray &data) {
        const auto array = getJsonArray(data);
        if (!array) {
            handleError(array.error(), Error::Internal);
            return;
        }

        if (array->empty()) {
            //% "No core language updates availabe."
            qInfo().noquote() << qtTrId("qwpup_info_no_core_lang_ups_avaqil");
            QTimer::singleShot(0, this, &QWpUp::checkPluginTranslations);
            return;
        }

        QStringList availCoreLangUps;
        availCoreLangUps.reserve(array->size());
        for (const auto &v : *array) {
            availCoreLangUps << v.toObject().value("native_name"_L1).toString();
        }
        //: %1 will be replaced by a list comma separated list of native language names
        //% "Updating core translations: %1."
        qInfo().noquote()
            << qtTrId("qwpup_info_update_core_translations").arg(m_locale.createSeparatedList(availCoreLangUps));
        QTimer::singleShot(0, this, &QWpUp::updateCoreTranslations);
    });

    connect(wp, &WP::failed, this, [this](const QString &error) {
        qWarning().noquote() << error;
        //% "Failed to check for core translation updates."
        qWarning().noquote() << qtTrId("qwpup_err_core_lang_check_failed");
        QTimer::singleShot(0, this, &QWpUp::checkPluginTranslations);
    });

    wp->start();
}

void QWpUp::updateCoreTranslations()
{
    QStringList args({u"language"_s, u"core"_s, u"update"_s});
    if (m_dryRun) {
        args << u"--dry-run"_s;
    }

    auto wp = wpProcess(args);

    connect(wp, &WP::succeeded, this, [this]() {
        //% "Successfully updated core translations."
        qInfo().noquote() << qtTrId("qwpup_info_upd_core_lang_success");
        QTimer::singleShot(0, this, &QWpUp::checkPluginTranslations);
    });

    connect(wp, &WP::failed, this, [this](const QString &error) {
        qWarning().noquote() << error;
        //% "Failed to update core translations."
        qWarning().noquote() << qtTrId("qwpup_err_upd_core_langs_failed");
        QTimer::singleShot(0, this, &QWpUp::checkPluginTranslations);
    });

    wp->start();
}

void QWpUp::checkPluginTranslations()
{
    //% "Checking for plugin translation updates."
    qInfo().noquote() << qtTrId("qwpup_info_check_plugin_trans_updates");

    auto wp = wpProcess({u"language"_s, u"plugin"_s, u"list"_s, u"--all"_s, u"--update=available"_s, u"--format=json"_s});

    connect(wp, &WP::succeeded, this, [this](const QByteArray &data) {
        const auto array = getJsonArray(data);
        if (!array) {
            qWarning().noquote() << array.error();
            QTimer::singleShot(0, this, &QWpUp::checkThemeTranslations);
            return;
        }

        if (array->empty()) {
            //% "No plugin language updates available."
            qInfo().noquote() << qtTrId("qwpup_info_no_plug_lang_ups-avail");
            QTimer::singleShot(0, this, &QWpUp::checkThemeTranslations);
            return;
        }

        QMap<QString, QStringList> availPlugLangUps;
        for (const auto &v : *array) {
            const auto object = v.toObject();
            const auto plugin = object.value("plugin"_L1).toString();
            const auto name   = object.value("native_name"_L1).toString();

            QStringList langs = availPlugLangUps.value(plugin);
            langs << name;
            availPlugLangUps.insert(plugin, langs);
        }

        for (auto i = availPlugLangUps.cbegin(), end = availPlugLangUps.cend(); i != end; ++i) {
            //: %1 will be replaced by the plugin name, %2 by a list of the languages
            //% "Updating languages for plugin „%1“: %2."
            qInfo().noquote() << qtTrId("qwpup_info_up_plug_langs").arg(i.key(), m_locale.createSeparatedList(i.value()));
        }

        QTimer::singleShot(0, this, &QWpUp::updatePluginTranslations);
    });

    connect(wp, &WP::failed, this, [this](const QString &error) {
        qWarning().noquote() << error;
        //% "Failed to check for plugin translation updates."
        qWarning().noquote() << qtTrId("qwpup_err_plugs_lang_check_failed");
        QTimer::singleShot(0, this, &QWpUp::checkThemeTranslations);
    });

    wp->start();
}

void QWpUp::updatePluginTranslations()
{
    QStringList args({u"language"_s, u"plugin"_s, u"update"_s, u"--all"_s});
    if (m_dryRun) {
        args << u"--dry-run"_s;
    }

    auto wp = wpProcess(args);

    connect(wp, &WP::succeeded, this, [this]() {
        //% "Successfully updated plugin translations."
        qInfo().noquote() << qtTrId("qwpup_info_upd_plug_lang_success");
        QTimer::singleShot(0, this, &QWpUp::checkThemeTranslations);
    });

    connect(wp, &WP::failed, this, [this](const QString &error) {
        qWarning().noquote() << error;
        //% "Failed to update plugin translations."
        qWarning().noquote() << qtTrId("qwpup_err_upd_plug_langs_failed");
        QTimer::singleShot(0, this, &QWpUp::checkThemeTranslations);
    });

    wp->start();
}

void QWpUp::checkThemeTranslations()
{
    //% "Checking for theme translation updates."
    qInfo().noquote() << qtTrId("qwpup_info_check_theme_trans_updates");

    auto wp = wpProcess({u"language"_s, u"theme"_s, u"list"_s, u"--all"_s, u"--update=available"_s, u"--format=json"_s});

    connect(wp, &WP::succeeded, this, [this](const QByteArray &data) {
        const auto array = getJsonArray(data);
        if (!array) {
            qWarning().noquote() << array.error();
            QTimer::singleShot(0, this, &QWpUp::finish);
            return;
        }

        if (array->empty()) {
            //% "No theme language updates available."
            qInfo().noquote() << qtTrId("qwpup_info_no_theme_lang_ups-avail");
            QTimer::singleShot(0, this, &QWpUp::finish);
            return;
        }

        QMap<QString, QStringList> availPlugLangUps;
        for (const auto &v : *array) {
            const auto object = v.toObject();
            const auto theme  = object.value("theme"_L1).toString();
            const auto name   = object.value("native_name"_L1).toString();

            QStringList langs = availPlugLangUps.value(theme);
            langs << name;
            availPlugLangUps.insert(theme, langs);
        }

        for (auto i = availPlugLangUps.cbegin(), end = availPlugLangUps.cend(); i != end; ++i) {
            //: %1 will be replaced by the theme name, %2 by a list of the languages
            //% "Updating languages for theme „%1“: %2."
            qInfo().noquote() << qtTrId("qwpup_info_up_theme_langs").arg(i.key(), m_locale.createSeparatedList(i.value()));
        }

        QTimer::singleShot(0, this, &QWpUp::updateThemeTranslations);
    });

    connect(wp, &WP::failed, this, [this](const QString &error) {
        qWarning().noquote() << error;
        //% "Failed to check for theme translation updates."
        qWarning().noquote() << qtTrId("qwpup_err_theme_lang_check_failed");
        QTimer::singleShot(0, this, &QWpUp::finish);
    });

    wp->start();
}

void QWpUp::updateThemeTranslations()
{
    QStringList args({u"language"_s, u"theme"_s, u"update"_s, u"--all"_s});
    if (m_dryRun) {
        args << u"--dry-run"_s;
    }

    auto wp = wpProcess(args);

    connect(wp, &WP::succeeded, this, [this]() {
        //% "Successfully updated theme translations."
        qInfo().noquote() << qtTrId("qwpup_info_upd_theme_lang_success");
        QTimer::singleShot(0, this, &QWpUp::checkThemeTranslations);
    });

    connect(wp, &WP::failed, this, [this](const QString &error) {
        QTimer::singleShot(0, this, &QWpUp::checkThemeTranslations);
        qWarning().noquote() << error;
        //% "Failed to update theme translations."
        qWarning().noquote() << qtTrId("qwpup_err_upd_theme_langs_failed");
    });

    wp->start();
}

void QWpUp::finish()
{
    if (!m_coreUpdated || m_dryRun || m_skipCompression) {
        QCoreApplication::quit();
        return;
    }

    const QStringList assets = getAllAssets();

    if (assets.empty()) {
        QCoreApplication::quit();
        return;
    }

    //% "Start compressing assets for the whole installation."
    qInfo().noquote() << qtTrId("qwpup_info_compr_all_assets");

    auto c = new Compressor(this); // NOLINT(cppcoreguidelines-owning-memory)
    connect(c, &Compressor::finished, this, [this](std::chrono::nanoseconds duration) {
        //: %1 will be replaced by the duration
        //: the compression took in miliseconds
        //% "Finished compressing all assets in %1 ms."
        qInfo().noquote() << qtTrId("qwpup_info_compr_all_assets_finished")
                                 .arg(m_locale.toString(
                                     std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()));
        QCoreApplication::quit();
    });
    c->start(assets);
}

void QWpUp::handleError(const QString &msg, Error exitCode)
{
    qCritical().noquote() << msg;

    QCoreApplication::exit(static_cast<int>(exitCode));
}

WP *QWpUp::wpProcess(const QStringList &arguments)
{
    return new WP(m_wpPath, m_wpDir.absolutePath(), m_env, arguments, this); // NOLINT(cppcoreguidelines-owning-memory)
}

WP *QWpUp::wpGetOption(const QString &name)
{
    return wpProcess({u"option"_s, u"get"_s, name});
}

/**
 * Asks a \a question on \c stdout and reads the answer from \c stdin. The anwer can be
 * yes, no or cancel.
 */
Answer QWpUp::askYesNoCancel(const QString &question)
{
    QTextStream out(stdout);
    QTextStream in(stdin);

    QString line;

    //: Answer options to a confirmation question
    //% "(Y)es/(N)o/(C)ancel"
    const QString _quest = question + " ["_L1 + qtTrId("qwpup_question_answers_yesnocancel") + "]: "_L1;
    //: Answer to a confirmation question, abbreviation for "Yes"
    //% "Y"
    const QString y = qtTrId("qwpup_quest_answer_yes_short");
    //: Answer to a confirmation question, full word
    //% "Yes"
    const QString yes = qtTrId("qwpup_quest_answer_yes");
    //: Answer to a confirmation question, abbreviation for "No"
    //% "N"
    const QString n = qtTrId("qwpup_quest_answer_no_short");
    //: Answer to a confirmation question, full word
    //% "No"
    const QString no = qtTrId("qwpup_quest_answer_no");
    //: Answer to a confirmation question, abbreviation for "Cancel"
    //% "C"
    const QString c = qtTrId("qwpup_quest_answer_cancel_short");
    //: Answer to a confirmation question, full word"
    //% "Cancel"
    const QString cancel = qtTrId("qwpup_quest_answer_cancel");

    out << _quest << Qt::flush;
    while (in.readLineInto(&line)) {
        const auto tl = line.trimmed();
        if (tl.compare(y, Qt::CaseInsensitive) == 0 || tl.compare(yes, Qt::CaseInsensitive) == 0) {
            return Answer::Yes;
        } else if (tl.compare(n, Qt::CaseInsensitive) == 0 || tl.compare(no, Qt::CaseInsensitive) == 0) {
            return Answer::No;
        } else if (tl.compare(c, Qt::CaseInsensitive) == 0 || tl.compare(cancel, Qt::CaseInsensitive) == 0) {
            QCoreApplication::exit();
            return Answer::Cancel;
        }
        out << question << _quest << Qt::flush;
    }

    QCoreApplication::exit();
    return Answer::Cancel;
}

/**
 * Asks a \a question on \c stdout and reads the answer from \c stdin. The anwer can be
 * yes or no.
 */
Answer QWpUp::askYesNo(const QString &question)
{
    QTextStream out(stdout);
    QTextStream in(stdin);

    QString line;

    //: Answer options to a confirmation question
    //% "(Y)es/(N)o"
    const QString _quest = question + " ["_L1 + qtTrId("qwpup_question_answers_yesno") + "]: "_L1;
    const QString y      = qtTrId("qwpup_quest_answer_yes_short");
    const QString yes    = qtTrId("qwpup_quest_answer_yes");
    const QString n      = qtTrId("qwpup_quest_answer_no_short");
    const QString no     = qtTrId("qwpup_quest_answer_no");

    out << _quest << Qt::flush;
    while (in.readLineInto(&line)) {
        const auto tl = line.trimmed();
        if (tl.compare(y, Qt::CaseInsensitive) == 0 || tl.compare(yes, Qt::CaseInsensitive) == 0) {
            return Answer::Yes;
        } else if (tl.compare(n, Qt::CaseInsensitive) == 0 || tl.compare(no, Qt::CaseInsensitive) == 0) {
            return Answer::No;
        }
        out << _quest << Qt::flush;
    }

    return Answer::No;
}

/**
 * Returns a list of all JS and CSS files below \a basePath. The directory at \a basePath
 * will be searched recursively for the asset files.
 */
QStringList QWpUp::getAssets(const QString &basePath) const
{
    QStringList assets;
    QDirIterator it(basePath, QStringList({u"*.js"_s, u"*.css"_s}), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        assets.emplace_back(it.next());
    }
    return assets;
}

/**
 * Returns all assets for the plugin identified by it’s \a name.
 */
QStringList QWpUp::getPluginAssets(const QString &name) const
{
    return getAssets(m_wpDir.absoluteFilePath(u"wp-content/plugins/"_s + name));
}

/**
 * Returns all assets for the theme identified by it’s \a name.
 */
QStringList QWpUp::getThemeAssets(const QString &name) const
{
    return getAssets(m_wpDir.absoluteFilePath(u"wp-content/themes/"_s + name));
}

/**
 * Returns all assets for the complete installation.
 */
QStringList QWpUp::getAllAssets() const
{
    return getAssets(m_wpDir.absolutePath());
}

/**
 * Returns a QJsonArray by reading all available data from stdout from
 * process \a p.
 *
 * If parsing the JSON data fails, the unexpected return value will contain
 * the error string.
 */
std::expected<QJsonArray, QString> QWpUp::getJsonArray(const QByteArray &ba) const
{
    QJsonParseError jpe;
    const auto json = QJsonDocument::fromJson(ba, &jpe);
    if (jpe.error != QJsonParseError::NoError) {
        //: Error message, %1 will be replaced by the error message from the JSON parser.
        //% "Failed to parse JSON data: %1"
        return std::unexpected(qtTrId("qwpup_err_json_parse_failed").arg(jpe.errorString()));
    }

    if (!json.isArray()) {
        //% "Unexpected JSON type."
        return std::unexpected(qtTrId("qwpup_err_json_unexpected_type"));
    }

    return json.array();
}

void QWpUp::setCoreStat(QLatin1StringView key, const QJsonValue &val)
{
    auto o = m_stats.value("core"_L1).toObject();
    o.insert(key, val);
    m_stats.insert("core"_L1, o);
}

void QWpUp::addPuginStat(QLatin1StringView key, const QJsonObject &plugin)
{
    auto a = m_stats.value("plugins"_L1).toArray();
    a.append(plugin);
    m_stats.insert("plugins"_L1, a);
}

void QWpUp::addThemeStat(QLatin1StringView key, const QJsonObject &theme)
{
    auto a = m_stats.value("themes"_L1).toArray();
    a.append(theme);
    m_stats.insert("themes"_L1, a);
}

#include "moc_qwpup.cpp"
