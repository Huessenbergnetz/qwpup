/*
 * SPDX-FileCopyrightText: (C) 2026 Matthias Fehring <https://www.huessenbergnetz.de>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "compressor.h"

#include <brotli/encode.h>

#include <QDebug>
#include <QFile>
#include <QFutureWatcher>
#include <QtConcurrentMap>

Compressor::Compressor(QObject *parent)
    : QObject{parent}
{
    connect(this, &Compressor::finished, this, &QObject::deleteLater);
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

    QByteArray data(input.size(), Qt::Uninitialized);
    if (auto dataRead = input.read(data.data(), input.size()); dataRead < 1) {
        if (dataRead < 0) {
            //: %1 will be replaced by the full file path, %2 by the error string
            //% "Failed to read file: %1: %2"
            qWarning().noquote() << qtTrId("qwpup_warn_failed_read_asset").arg(asset, input.errorString());
        } else {
            //: %1 will be replaced by the full file path
            //% "Skipping empty file: %1"
            qDebug().noquote() << qtTrId("qwpup_dbg_skip_empty_asset").arg(asset);
        }
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

void Compressor::start(const QStringList &files)
{
    auto watcher = new QFutureWatcher<QString>(this); // NOLINT(cppcoreguidelines-owning-memory)
    const auto start{std::chrono::steady_clock::now()};

    connect(watcher, &QFutureWatcher<QString>::finished, this, [this, start]() {
        const auto end{std::chrono::steady_clock::now()};
        const std::chrono::nanoseconds duration{end - start};
        emit finished(duration);
    });

    auto future = QtConcurrent::mapped(files, compressAsset);
    if (future.isFinished()) {
        const auto end{std::chrono::steady_clock::now()};
        const std::chrono::nanoseconds duration{end - start};
        emit finished(duration);
    } else {
        watcher->setFuture(future);
    }
}

#include "moc_compressor.cpp"
