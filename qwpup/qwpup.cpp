/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "qwpup.h"

#include "utils.h"

#include <brotli/encode.h>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QJsonDocument>
#include <QJsonValue>
#include <QLocale>
#include <QLoggingCategory>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextStream>
#include <QTimer>
#include <QVersionNumber>
#include <QtConcurrentMap>

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
            qDebug().noquote() << wp->readAllStandardOutput().trimmed();
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
            const auto json = QJsonDocument::fromJson(wp->readAllStandardOutput(), &jpe);
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

            const auto array = json.array();

            if (array.isEmpty()) {
                //% "No core updates available."
                qInfo().noquote() << qtTrId("qpwup_info_no_core_ups_avail");
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
                QTimer::singleShot(0, this, &QWpUp::updateCore);
            } else {
                //% "Skipping core update."
                qInfo().noquote() << qtTrId("qwpup_info_skip_core_update");
                QTimer::singleShot(0, this, &QWpUp::checkPluginUpdates);
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
        if (askYesNoCancel(qtTrId("qwpup_ask_update_core").arg(m_currentCoreVersion, targetVersion)) != Answer::Yes) {
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
            handleError(qtTrId("qwpup_err_wp_core_update_failed"), Error::Internal);
        }
    });
    wp->start();
}

void QWpUp::checkPluginUpdates()
{
    //% "Checking for plugin updates."
    qInfo().noquote() << qtTrId("qwpup_info_check_plugin_updates");

    auto wp = wpProcess({u"plugin"_s, u"list"_s, u"--update=available"_s, u"--format=json"_s});
    connect(wp, &QProcess::finished, this, [this, wp](int exitCode, QProcess::ExitStatus exitStatus) {
        wp->deleteLater();
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QJsonParseError jpe;
            const auto json = QJsonDocument::fromJson(wp->readAllStandardOutput(), &jpe);
            if (jpe.error != QJsonParseError::NoError) {
                qWarning().noquote() << qtTrId("qwpup_err_json_parse_failed").arg(jpe.errorString());
                QTimer::singleShot(0, this, &QWpUp::checkThemeUpdates);
                return;
            }

            if (!json.isArray()) {
                qWarning().noquote() << qtTrId("qwpup_err_json_unexpected_type");
                QTimer::singleShot(0, this, &QWpUp::checkThemeUpdates);
                return;
            }

            const auto array = json.array();

            if (array.isEmpty()) {
                //% "No plugin updates available."
                qInfo().noquote() << qtTrId("qwpup_info_no_plug_ups_avail");
                QTimer::singleShot(0, this, &QWpUp::checkThemeUpdates);
                return;
            }

            QList<QJsonObject> pluginsWithUpdates;

            for (const auto &v : array) {
                const auto o             = v.toObject();
                const auto version       = QVersionNumber::fromString(o.value("version"_L1).toString()).normalized();
                const auto updateVersion = QVersionNumber::fromString(o.value("update_version"_L1).toString()).normalized();
                const auto commonPrefix  = QVersionNumber::commonPrefix(version, updateVersion);

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
                    qDebug().noquote() << qtTrId("qwpup_info_skipp_plug_ups").arg(none);
                }

            } else if (m_logLevel == QtInfoMsg) {
                QLocale locale;

                const QString none = qtTrId("qwpup_info_updates_none");

                if (!pluginsWithUpdates.empty()) {
                    QStringList availUpdates;
                    availUpdates.reserve(pluginsWithUpdates.size());
                    for (const auto &o : std::as_const(pluginsWithUpdates)) {
                        availUpdates << o.value("name"_L1).toString();
                    }
                    qInfo().noquote() << qtTrId("qwpup_info_avail_plug_ups").arg(locale.createSeparatedList(availUpdates));
                } else {
                    qInfo().noquote() << qtTrId("qwpup_info_avail_plug_ups").arg(none);
                }

                if (!m_skippedPlugins.empty()) {
                    QStringList skippedUpdates;
                    skippedUpdates.reserve(m_skippedPlugins.size());
                    for (const auto &v : std::as_const(m_skippedPlugins)) {
                        skippedUpdates << v.toObject().value("name"_L1).toString();
                    }
                    qInfo().noquote() << qtTrId("qwpup_info_skipp_plug_ups").arg(locale.createSeparatedList(skippedUpdates));
                } else {
                    qInfo().noquote() << qtTrId("qwpup_info_skipp_plug_ups").arg(none);
                }
            }

            if (m_pluginsToUpdate.isEmpty()) {
                QTimer::singleShot(0, this, &QWpUp::checkThemeUpdates);
            } else {
                QTimer::singleShot(0, this, &QWpUp::updatePlugin);
            }
        }
    });
    wp->start();
}

QString compressAsset(const QString &asset)
{
    qDebug().noquote() << "Compressing" << asset << Qt::flush;
    QFile input(asset);
    if (!input.open(QIODeviceBase::ReadOnly)) {
        //: %1 will bereplaced by the absolute file path, %2 the error message
        //% "Failed to open %1 for reading: %2"
        qWarning().noquote() << qtTrId("qwpup_warn_failed_open_asset").arg(asset, input.errorString());
        return {};
    }

    const QByteArray data = input.readAll();
    if (data.isEmpty()) {
        //: %1 will bereplaced by the absolute file path
        //% "Failed to read file or file is empty: %1"
        qWarning().noquote() << qtTrId("qwpup_warn_failed_read_asset_or_empty").arg(asset);
        return {};
    }

    input.close();

    auto outSize = BrotliEncoderMaxCompressedSize(data.size());
    if (outSize == 0) {
        //: %1 will be replaced by the size, %2 by the full path to the asset file
        //% "Required Brotli output buffer too large to compress input file of size %1: %2"
        qWarning().noquote() << qtTrId("qwpup_warn_comp_brotli_too_large").arg(QString::number(data.size()), asset);
        return {};
    }

    QByteArray outData{static_cast<qsizetype>(outSize), Qt::Uninitialized};

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto in = reinterpret_cast<const uint8_t *>(data.constData());
    auto out      = reinterpret_cast<uint8_t *>(outData.data());
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

    const BROTLI_BOOL status = BrotliEncoderCompress(
        BROTLI_DEFAULT_QUALITY, BROTLI_DEFAULT_WINDOW, BROTLI_MODE_TEXT, data.size(), in, &outSize, out);

    if (status != BROTLI_TRUE) {
        //: %1 will be replaced by the full file path
        //% "Failed to compress asset with Brotli: %1"
        qWarning().noquote() << qtTrId("qwpup_warn_comp_brotli_fail").arg(asset);
        return {};
    }

    outData.resize(static_cast<qsizetype>(outSize));

    QFile output{asset + u".br"};
    if (!output.open(QIODeviceBase::WriteOnly)) {
        //: %1 will be replaced by the file path, %2 by the error message
        //% "Failed to open %1 for writing: %2"
        qWarning().noquote() << qtTrId("qwpup_warn_comp_brotli_open_out").arg(output.fileName(), output.errorString());
        if (output.exists()) {
            output.remove();
        }
        return {};
    }

    if (output.write(outData) < 0) {
        //: %1 will be replaced by the file path, %2 by the error message
        //% "Failed to write compressed data to %1: %2"
        qWarning().noquote() << qtTrId("qwpup_warn_comp_brotli_write_out").arg(output.fileName(), output.errorString());
        if (output.exists()) {
            output.remove();
        }
        return {};
    }

    return output.fileName();
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
        //% "Do you want to update plugin %1 from version %2 to %3?"
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

    auto wp = wpProcess({u"plugin"_s, u"update"_s, name, u"--format=json"_s, u"--dry-run"_s});
    connect(wp,
            &QProcess::finished,
            this,
            [this, wp, o, name, version, updateVersion](int exitCode, QProcess::ExitStatus exitStatus) {
        wp->deleteLater();
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            m_updatedPlugins.append(o);

            //: %1 will be replaced by the plugin’s name, %2 by the current version
            //: and %3 by the update version
            //% "Successfully updated plugin %1 from version %2 to %3."
            qInfo().noquote() << qtTrId("qwpup_info_plug_up_success").arg(name, version, updateVersion);

            if (m_coreUpdated) {
                QTimer::singleShot(0, this, &QWpUp::updatePlugin);
                return;
            }

            QStringList assets = getPluginAssets(name);

            if (assets.empty()) {
                QTimer::singleShot(0, this, &QWpUp::updatePlugin);
                return;
            }

            qDebug() << assets;

            //: %1 will be replaced by the plugin name
            //% "Start compressing assets for plugin %1."
            qInfo().noquote() << qtTrId("qwpup_info_plug_compr_assets").arg(name);

            auto watcher = new QFutureWatcher<void>(this); // NOLINT(cppcoreguidelines-owning-memory)
            connect(watcher, &QFutureWatcher<void>::finished, this, [this, name, watcher]() {
                watcher->deleteLater();
                //: %1 will be replaced by the plugin name
                //% "Finished compressing assets for plugin %1."
                qInfo().noquote() << qtTrId("qwpup_info_plug_compr_assets_finished").arg(name);
                QTimer::singleShot(0, this, &QWpUp::updatePlugin);
            });
            auto future = QtConcurrent::mapped(assets, compressAsset);
            if (future.isFinished()) {
                watcher->deleteLater();
                QTimer::singleShot(0, this, &QWpUp::updatePlugin);
            } else {
                watcher->setFuture(future);
            }

        } else {
            const QString error = QString::fromLocal8Bit(wp->readAllStandardError().trimmed());
            QJsonObject _o      = o;
            _o.insert("error"_L1, error);
            m_failedPlugins.append(_o);
            qWarning().noquote() << error;
            //: %1 will be replaced by the plugin’s name, %2 by the current version
            //: and %3 by the update version
            //% "Failed to update plugin %1 from version %2 to %3."
            qWarning().noquote() << qtTrId("qwpup_err_plug_up_failed").arg(name, version, updateVersion);
            QTimer::singleShot(0, this, &QWpUp::updatePlugin);
        }
    });
    wp->start();
}

void QWpUp::checkThemeUpdates()
{
    //% "Checking for theme updates."
    qInfo().noquote() << qtTrId("qwpup_info_check_theme_updates");

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

QStringList QWpUp::getAssets(const QString &basePath) const
{
    QStringList assets;
    QDirIterator it(basePath, QStringList({u"*.js"_s, u"*.css"_s}), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        assets.emplace_back(it.next());
    }
    return assets;
}

QStringList QWpUp::getPluginAssets(const QString &pluginName) const
{
    return getAssets(m_wpDir.absoluteFilePath(u"wp-content/plugins/"_s + pluginName));
}

#include "moc_qwpup.cpp"
