/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef QWPUP_STATS_H
#define QWPUP_STATS_H

#include <QJsonObject>
#include <QObject>

class QFile;

class Stats : public QObject
{
    Q_OBJECT
public:
    explicit Stats(const QString &output, QObject *parent = nullptr);

    void addStat(QLatin1StringView key, const QJsonValue &val);

    [[nodiscard]] QString outputFilePath() const;

    void write();

private:
    QString m_outputFilePath;
    QJsonObject m_stats;
    QFile *m_outFile{nullptr};

    void writeToFile(QStringView json);
    void writeToStdout(QStringView json);
};

#endif // QWPUP_STATS_H
