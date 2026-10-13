/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "conductor.h"

#include "utils.h"

#include <cerrno>
#include <cstring>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTemporaryFile>
#include <QTimer>

using namespace Qt::Literals::StringLiterals;

Conductor::Conductor(QObject *parent)
    : QObject{parent}
{
    connect(this, &Conductor::siteUpdated, this, &Conductor::updateSite);
}

Error Conductor::start(const QStringList &args)
{
    QCommandLineParser parser;

    //: Application descrpition in the CLI help
    //% "Application to orchestrate multiple WordPress installation updates using qwpup."
    parser.setApplicationDescription(qtTrId("qwpup_master_cli_app_desc"));

    parser.addHelpOption();
    parser.addVersionOption();

    const QString confPathDef = QStringLiteral(QWPUP_CONFFILE);
    QCommandLineOption confPathOpt(QStringList({u"c"_s, u"config"_s}),
                                   //: Option description in the CLI help,
                                   //: %1 will be replaced by the full default file path
                                   //% "Path to configuration file. Default: %1"
                                   qtTrId("qwpup_cli_opt_config_file_path").arg(confPathDef),
                                   //: Option value name in the cli help for file and directory paths
                                   //% "path"
                                   qtTrId("qwpup_cli_opt_val_path"),
                                   confPathDef);
    parser.addOption(confPathOpt);

    QCommandLineOption logLevelOpt(
        QStringList({u"l"_s, u"log-level"_s}),
        //: Option description in the CLI help
        //% "Log level and higher for that messages are shown. Available: %1. Default: %2"
        qtTrId("qwpup_cli_opt_log_level").arg(m_locale.createSeparatedList(Utils::logLevels()), Utils::defaultLogLevel()),
        //: Option value name in the CLI help for the log level
        //% "level"
        qtTrId("qwpup_cli_opt_log_level_val"),
        Utils::defaultLogLevel());
    parser.addOption(logLevelOpt);

    QCommandLineOption dryRunOpt(u"dry-run"_s,
                                 //: Option description in the CLI help
                                 //% "Do a dry run without performing real actions."
                                 qtTrId("qwpup_cli_opt_dry_run"));
    parser.addOption(dryRunOpt);

    parser.process(args);

    if (auto ec = readConfiguration(parser.value(confPathOpt)); ec != Error::None) {
        return ec;
    }

    // Set the log level
    if (parser.isSet(logLevelOpt)) {
        const QString logLevel = parser.value(logLevelOpt).toLower();
        if (!Utils::isLogLevelValid(logLevel)) {
            //: Error message
            //% "Invalid log level."
            qCritical().noquote() << qtTrId("qwpup_err_inv_ll");
            return Error::Config;
        }
        m_logLevel = Utils::logLevel(logLevel);
        Utils::setLogLevel(m_logLevel);
    }

    // Check qwpup binary path
    if (m_qwpupPath.isEmpty()) {
        m_qwpupPath = QStringLiteral(QWPUP_BINPATH);
    }

    m_dryRun = parser.isSet(dryRunOpt);

    QTimer::singleShot(0, this, &Conductor::updateSite);
    return Error::None;
}

void Conductor::updateSite()
{
    if (m_sites.empty()) {
        sendEmail();
        return;
    }

    const Conductor::Site site = m_sites.dequeue();

    const QString logName = site.name.isEmpty() ? u"unknown-site:"_s : site.name + ':'_L1;

    //% "Starting update."
    qInfo().noquote() << logName << qtTrId("qwpup_info_start_site_update");

    if (site.name.isEmpty()) {
        //% "Empty site name. Set a site name in your configuration for better logging."
        qWarning().noquote() << logName << qtTrId("qwpup_warn_site_empty_name");
    }

    if (site.path.isEmpty()) {
        //% "Update failed: empty path to WordPress directory."
        qWarning().noquote() << logName << qtTrId("qwpup_warn_site_path_empty");
        emit siteUpdated();
        return;
    }

    const QDir wpDir{site.path};

    if (!wpDir.exists()) {
        //% "Update failed: WordPress directory path “%1” does not exist."
        qWarning().noquote() << logName << qtTrId("qwpup_warn_site_path_not_exists").arg(site.path);
        emit siteUpdated();
        return;
    }

    bool uidOk = false;
    uint uid   = 0;
    if (!site.user.isEmpty()) {
        uid = site.user.toUInt(&uidOk);
        if (!uidOk) {
            const auto passwd = getpwnam(qUtf8Printable(site.user));
            if (passwd) {
                uid   = passwd->pw_uid;
                uidOk = true;
            } else {
                //% "Unknown user set. Please check your configuration. "
                //% "Will try to get user from wp-config.php file owner."
                qWarning().noquote() << logName << qtTrId("qwpup_warn_site_invalid_username");
            }
        }
    }
    if (!uidOk) {
        auto uidFromWpConfig = Conductor::getUidFromWpConfig(wpDir.absoluteFilePath(u"wp-config.php"_s));
        if (uidFromWpConfig) {
            uid   = *uidFromWpConfig;
            uidOk = true;
        } else {
            qWarning().noquote() << logName << uidFromWpConfig.error();
        }
    }
    if (!uidOk) {
        //% "Update failed: Can not obtain user id. Please specify user in site configuration."
        qWarning().noquote() << logName << qtTrId("qwpup_warn_site_no_uid");
        emit siteUpdated();
        return;
    }

    bool gidOk = false;
    uint gid   = 0;
    if (!site.group.isEmpty()) {
        gid = site.group.toUInt(&gidOk);
        if (!gidOk) {
            const auto group = getgrnam(qUtf8Printable(site.group));
            if (group) {
                gid   = group->gr_gid;
                gidOk = true;
            } else {
                //% "Unknown group set. Please check your configuration. "
                //% "Will try to get group from wp-config.php file group."
                qWarning().noquote() << logName << qtTrId("qwpup_warn_site_invalid_groupname");
            }
        }
    }
    if (!gidOk) {
        auto gidFromWpConfig = Conductor::getGidFromWpConfig(wpDir.absoluteFilePath(u"wp-config.php"_s));
        if (gidFromWpConfig) {
            gid   = *gidFromWpConfig;
            gidOk = true;
        } else {
            qWarning().noquote() << logName << gidFromWpConfig.error();
        }
    }
    if (!gidOk) {
        //% "Update failed: Can not obtain group id. Please specify group in site configuration."
        qWarning().noquote() << logName << qtTrId("qwpup_warn_site_no_gid");
        emit siteUpdated();
        return;
    }

    // set qwpup command line arguments
    QStringList args;

    args << u"--yes"_s;

    args << u"--log-format"_s << u"json"_s;

    args << u"--log-level"_s;
    switch (m_logLevel) {
    case QtCriticalMsg:
        args << u"critical"_s;
        break;
    case QtWarningMsg:
        args << u"warning"_s;
        break;
    case QtInfoMsg:
        args << u"info"_s;
        break;
    default:
        args << u"debug"_s;
    }

    args << u"--wp-dir"_s << site.path;

    if (site.coreMajor) {
        args << u"--major"_s;
    }

    if (site.skipCompression) {
        args << u"--skip-compression"_s;
    }

    if (m_dryRun) {
        args << u"--dry-run"_s;
    }

    args << u"--plugins-version"_s << Utils::versionPartToString(site.pluginsVersion);

    args << u"--themes-version"_s << Utils::versionPartToString(site.themesVersion);

    auto statFile = new QTemporaryFile(QDir::tempPath() + "/qwpup-stats-"_L1 +
                                           site.name.simplified().replace(QChar(QChar::Space), '-'_L1) + "-XXXXXX"_L1,
                                       this);
    if (!statFile->open()) {
        //% "Update failed: Failed to create temporary file for statistics."
        qWarning().noquote() << logName << qtTrId("qwpup_warn_site_no_stat_file");
        emit siteUpdated();
        return;
    }
    args << u"--stats"_s << statFile->fileName();

    auto qwpup = new QProcess(this); // NOLINT(cppcoreguidelines-owning-memory)
    qwpup->setChildProcessModifier([uid, gid, qwpup] {
        ::setgroups(0, nullptr);
        if (::setgid(gid) == -1) {
            auto err = errno;
            qwpup->failChildProcessModifier(strerror(err), err);
            return;
        }
        if (::setuid(uid) == -1) {
            auto err = errno;
            qwpup->failChildProcessModifier(strerror(err), err);
            return;
        }
    });
    qwpup->setProcessChannelMode(QProcess::MergedChannels);
    qwpup->setCurrentReadChannel(QProcess::StandardOutput);
    qwpup->setProgram(m_qwpupPath);
    qwpup->setArguments(args);

    connect(qwpup, &QProcess::finished, this, [this, logName, qwpup](int exitCode, QProcess::ExitStatus exitStatus) {
        qwpup->deleteLater();
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            //% "Update successful."
            qInfo().noquote() << logName << qtTrId("qwpup_info_update_success");
        } else {
            //% "Update failed."
            qWarning().noquote() << logName << qtTrId("qwpup_warn_update_failed");
        }
        emit siteUpdated();
    });

    connect(qwpup, &QProcess::errorOccurred, this, [this, logName, qwpup](QProcess::ProcessError error) {
        qwpup->deleteLater();
        //: %1 will be replaced by the error string returned by QProcess
        //% "Update failed: %1"
        qWarning().noquote() << logName << qtTrId("qwpup_warn_update_failed_process").arg(qwpup->errorString());
        emit siteUpdated();
    });

    connect(qwpup, &QProcess::readyRead, this, [logName, qwpup]() {
        while (qwpup->canReadLine()) {
            const auto l = qwpup->readLine().trimmed();
            const auto o = QJsonDocument::fromJson(l).object();
            if (!o.empty()) {
                const auto type = o.value("type"_L1).toString();
                const auto msg  = o.value("message"_L1).toString();
                if (!msg.isEmpty()) {
                    if (type == "debug"_L1) {
                        qDebug().noquote() << logName << msg;
                    } else if (type == "info"_L1) {
                        qInfo().noquote() << logName << msg;
                    } else if (type == "warning"_L1) {
                        qWarning().noquote() << logName << msg;
                    } else if (type == "critical"_L1) {
                        qCritical().noquote() << logName << msg;
                    }
                }
            } else {
                qDebug().noquote() << logName << l;
            }
        }
    });

    qwpup->start();
}

void Conductor::sendEmail()
{
    if (!m_mailEnabled) {
        finish();
        return;
    }
}

void Conductor::finish()
{
    QCoreApplication::quit();
}

Error Conductor::readConfiguration(const QString &configFilePath)
{
    QFile configFile(configFilePath);
    if (!configFile.exists()) {
        //: %1 will be replaced by the file path
        //% "Can not find master configuration file at %1"
        qCritical().noquote() << qtTrId("qwpup_err_master_conf_not_found").arg(configFilePath);
        return Error::File;
    }

    if (!configFile.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text)) {
        //: %1 will be replaced by the file path, %2 by the error message
        //% "Failed to open master configuration file at %1: %2"
        qCritical().noquote() << qtTrId("qwpup_err_master_conf_fail_open").arg(configFilePath, configFile.errorString());
        return Error::File;
    }

    QByteArray configData(configFile.size(), Qt::Uninitialized);
    if (auto dataRead = configFile.read(configData.data(), configFile.size()); dataRead < 1) {
        if (dataRead < 0) {
            //: %1 will be replaced by the file path, %2 by the error message
            //% "Failed to read master configuration file at %1: %2"
            qCritical().noquote() << qtTrId("qwpup_err_read_config_error").arg(configFilePath, configFile.errorString());
            return Error::File;
        } else {
            //% "Empty configuration file at %1"
            qCritical().noquote() << qtTrId("qwpup_err_empty_config_file").arg(configFilePath);
            return Error::Config;
        }
    }

    QJsonParseError jpe;
    const auto json = QJsonDocument::fromJson(configData, &jpe);
    if (jpe.error != QJsonParseError::NoError) {
        //: Error message, %1 will be replaced by the error message from the JSON parser.
        //% "Failed to parse configuration file JSON data: %1"
        qCritical().noquote() << qtTrId("qwpup_err_config_json_parse_failed").arg(jpe.errorString());
        return Error::Config;
    }

    if (!json.isObject()) {
        //% "Unexpected JSON root type in configuration file."
        qCritical().noquote() << qtTrId("qwpup_err_config_json_unexpected_type");
        return Error::Config;
    }

    const auto config = json.object();

    if (config.empty()) {
        //% "Configuration is empty and does not contain any data."
        qCritical().noquote() << qtTrId("qwpup_err_empty_config");
        return Error::Config;
    }

    m_logLevel = Utils::logLevel(config.value("log_level"_L1).toString(Utils::defaultLogLevel()));
    Utils::setLogLevel(m_logLevel);

    const auto sites = config.value("sites"_L1).toArray();

    if (sites.empty()) {
        //% "No sites configured. Please add sites to update to your configuration."
        qCritical().noquote() << qtTrId("qwpup_err_config_empty_sites");
        return Error::Config;
    }

    m_sites.reserve(sites.size());
    for (const auto &val : sites) {
        const auto o = val.toObject();
        if (o.empty()) {
            //% "Empty entry in “sites“ configuration."
            qWarning().noquote() << qtTrId("qwpup_warn_config_emtpy_site");
            continue;
        }

        QStringList mailReceivers;
        const auto mrvl = o.value("mail_receivers"_L1).toArray().toVariantList();
        if (!mrvl.empty()) {
            mailReceivers.reserve(mrvl.size());
            for (const auto &v : mrvl) {
                mailReceivers << v.toString();
            }
        }

        Conductor::Site site{
            .name            = o.value("name"_L1).toString(),
            .path            = o.value("path"_L1).toString(),
            .user            = o.value("user"_L1).toString(),
            .group           = o.value("group"_L1).toString(),
            .mailReceivers   = mailReceivers,
            .pluginsVersion  = Utils::versionPartFromString(o.value("plugins_version"_L1).toString(), VersionPart::Major),
            .themesVersion   = Utils::versionPartFromString(o.value("themes_version"_L1).toString(), VersionPart::Major),
            .skipCompression = o.value("skip_compression"_L1).toBool(),
            .coreMajor       = o.value("core_major"_L1).toBool()};

        m_sites.enqueue(site);
    }

    m_mailUrl     = QUrl(config.value("mail_server"_L1).toString());
    m_mailEnabled = config.value("mail_enabled"_L1).toBool();
    m_mailFrom    = config.value("mail_from"_L1).toString();
    m_qwpupPath   = config.value("qwpup_path"_L1).toString();

    return Error::None;
}

std::expected<uint, QString> Conductor::getUidFromWpConfig(const QString &path)
{
    QFileInfo wpconfig(path);
    if (!wpconfig.exists()) {
        //: %1 will be replaced by the full path to the file
        //% "Can not find WordPress configurtion file at “%1”."
        return std::unexpected(qtTrId("qwpup_err_wpconfig_not_found").arg(path));
    }

    return wpconfig.ownerId();
}

std::expected<uint, QString> Conductor::getGidFromWpConfig(const QString &path)
{
    QFileInfo wpconfig(path);
    if (!wpconfig.exists()) {
        //: %1 will be replaced by the full path to the file
        //% "Can not find WordPress configurtion file at “%1”."
        return std::unexpected(qtTrId("qwpup_err_wpconfig_not_found").arg(path));
    }

    return wpconfig.groupId();
}

void Conductor::handleError(const QString &msg, Error exitCode)
{
    qCritical().noquote() << msg;

    QCoreApplication::exit(static_cast<int>(exitCode));
}

#include "moc_conductor.cpp"
