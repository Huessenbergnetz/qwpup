/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "stats.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTextStream>

using namespace Qt::Literals::StringLiterals;

Stats::Stats(const QString &output, QObject *parent)
    : QObject{parent}
    , m_outputFilePath{output}
{
}

void Stats::addStat(QLatin1StringView key, const QJsonValue &val)
{
    m_stats.insert(key, val);
}

QString Stats::outputFilePath() const
{
    return m_outputFilePath;
}

void Stats::write()
{
    const auto json = QString::fromUtf8(QJsonDocument(m_stats).toJson(QJsonDocument::Indented));

    if (QString::compare(m_outputFilePath, "stdout"_L1, Qt::CaseInsensitive) == 0) {
        writeToStdout(json);
    } else {
        writeToFile(json);
    }
}

void Stats::writeToFile(QStringView json)
{
    QFile outFile(m_outputFilePath);
    if (!outFile.open(QIODeviceBase::WriteOnly | QIODeviceBase::Truncate | QIODeviceBase::Text)) {
        qCritical() << "Failed to open file" << outFile.fileName() << "for writing statistics:" << outFile.errorString();
        return;
    }

    QTextStream out(&outFile);
    out << json;
}

void Stats::writeToStdout(QStringView json)
{
    QTextStream out(stdout);
    out << json;
}

#include "moc_stats.cpp"
