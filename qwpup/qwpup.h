/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef QWPUP_H
#define QWPUP_H

#include "enums.h"

#include <expected>
#include <memory>

#include <QCoreApplication>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QLocale>
#include <QProcessEnvironment>
#include <QQueue>

class QProcess;
class QTemporaryDir;
class Stats;
class WP;

class QWpUp : public QObject
{
    Q_OBJECT
public:
    explicit QWpUp(QCoreApplication *parent = nullptr);

    Error start(const QStringList &arguments);

private slots:
    void doStart();
    void getBlogName();
    void getSiteUrl();
    void getCurrentVersion();
    void showVersionInfo();
    void listCoreVersions();
    void updateCore();
    void checkPluginUpdates();
    void updatePlugin();
    void checkThemeUpdates();
    void updateTheme();
    void checkCoreTranslations();
    void updateCoreTranslations();
    void checkPluginTranslations();
    void updatePluginTranslations();
    void checkThemeTranslations();
    void updateThemeTranslations();
    void compressAllAssets();
    void writeStats();
    void finish();

private:
    void handleError(const QString &msg, Error exitCode);
    WP *wpProcess(const QStringList &arguments);
    WP *wpGetOption(const QString &name);
    Answer askYesNoCancel(const QString &question);
    Answer askYesNo(const QString &question);
    [[nodiscard]] QStringList getAssets(const QString &basePath) const;
    [[nodiscard]] QStringList getPluginAssets(const QString &name) const;
    [[nodiscard]] QStringList getThemeAssets(const QString &name) const;
    [[nodiscard]] QStringList getAllAssets() const;
    [[nodiscard]] std::expected<QJsonArray, QString> getJsonArray(const QByteArray &ba) const;

    void addStat(QLatin1StringView key, const QJsonValue &val);
    void setConfigStats();

    QString m_wpPath;
    QString m_currentCoreVersion;
    QString m_availMajCoreVersion;
    QString m_availMinCoreVersion;
    QString m_updatedCoreVersion;
    QQueue<QJsonObject> m_pluginsToUpdate;
    QQueue<QJsonObject> m_themesToUpdate;
    Stats *m_stats{nullptr};
    QJsonArray m_skippedPlugins;
    QJsonArray m_updatedPlugins;
    QJsonArray m_failedPlugins;
    QJsonArray m_skippedThemes;
    QJsonArray m_updatedThemes;
    QJsonArray m_failedThemes;
    QJsonArray m_coreLangUps;
    QJsonArray m_plugLangUps;
    QJsonArray m_themeLangUps;
    QLocale m_locale;
    QDir m_wpDir;
    std::unique_ptr<QTemporaryDir> m_tempDir;
    QProcessEnvironment m_env;
    VersionPart m_plugsUpVersion{VersionPart::Major};
    VersionPart m_themesUpVersion{VersionPart::Major};
    QtMsgType m_logLevel{QtDebugMsg};
    bool m_skipCompression{false};
    bool m_wpUpMajor{false};
    bool m_sayYes{false};
    bool m_coreUpdated{false};
    bool m_majCoreUpAvail{false};
    bool m_minCoreUpAvail{false};
    bool m_dryRun{false};
};

#endif // QWPUP_H
