/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "conductor.h"

#include "utils.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

using namespace Qt::Literals::StringLiterals;

Conductor::Conductor(QObject *parent)
    : QObject{parent}
{
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

    QTimer::singleShot(0, this, &Conductor::finish);
    return Error::None;
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

        m_sites.emplace_back(o.value("name"_L1).toString(),
                             o.value("path"_L1).toString(),
                             o.value("user"_L1).toString(),
                             mailReceivers,
                             Utils::versionPartFromString(o.value("plugins_version"_L1).toString(), VersionPart::Major),
                             Utils::versionPartFromString(o.value("themes_version"_L1).toString(), VersionPart::Major),
                             o.value("skip_compression"_L1).toBool(),
                             o.value("core_major"_L1).toBool());
    }

    m_mailUrl     = QUrl(config.value("mail_server"_L1).toString());
    m_mailEnabled = config.value("mail_enabled"_L1).toBool();
    m_mailFrom    = config.value("mail_from"_L1).toString();
    m_logLevel    = Utils::logLevel(config.value("log_level"_L1).toString(Utils::defaultLogLevel()));
    Utils::setLogLevel(m_logLevel);

    return Error::None;
}

void Conductor::finish()
{
    QCoreApplication::quit();
}

void Conductor::handleError(const QString &msg, Error exitCode)
{
    qCritical().noquote() << msg;

    QCoreApplication::exit(static_cast<int>(exitCode));
}

#include "moc_conductor.cpp"
