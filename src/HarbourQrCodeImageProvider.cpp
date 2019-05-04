/*
 * Copyright (C) 2019 Jolla Ltd.
 * Copyright (C) 2019 Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "HarbourQrCodeImageProvider.h"

#include "HarbourBase32.h"
#include "HarbourDebug.h"

#include <QVector>
#include <QColor>
#include <QRgb>

QImage HarbourQrCodeImageProvider::requestImage(const QString& aId, QSize* aSize, const QSize&)
{
    // Default background and foreground
    QColor background(Qt::transparent), color(Qt::black);

    // Parse parameters
    QString base32;
    const int sep = aId.indexOf('?');
    if (sep < 0) {
        base32 = aId;
    } else {
        base32 = aId.left(sep);
        const QStringList params(aId.mid(sep + 1).split('&', QString::SkipEmptyParts));
        const int n = params.count();
        for (int i = 0; i < n; i++) {
            const QString param(params.at(i));
            const int eq = param.indexOf('=');
            if (eq > 0) {
                static const QString BACKGROUND("background");
                static const QString COLOR("color");
                const QString name(param.left(eq).trimmed());
                const QString value(param.mid(eq + 1).trimmed());
                if (name == COLOR) {
                    const QColor colorValue(value);
                    if (colorValue.isValid()) {
                        color = colorValue;
                    } else {
                        HDEBUG("Invalid" << qPrintable(name) << value);
                    }
                } else if (name == BACKGROUND) {
                    const QColor colorValue(value);
                    if (colorValue.isValid()) {
                        background = colorValue;
                    } else {
                        HDEBUG("Invalid" << qPrintable(name) << value);
                    }
                } else {
                    HDEBUG("Invalid parameter name" << name);
                }
            } else {
                HDEBUG("Invalid parameter" << param);
            }
        }
    }

    // Decode BASE32
    const QByteArray bits(HarbourBase32::fromBase32(base32.toLocal8Bit()));
    QImage img;
    HDEBUG(base32 << "=>" << bits.size() << "bytes");
    if (bits.size() > 0) {
        // Bits are packed, rows are rounded at byte boundary
        int rows, rowSize;
        for (rows = 2; ((rowSize = (rows + 7)/8) * rows) < bits.size(); rows++);
        if ((rows * rowSize) == bits.size()) {
            HDEBUG(rows << "x" << rows);
            img = QImage(rows, rows, QImage::Format_Mono);
            QVector<QRgb> colors;
            colors.append(background.rgba());
            colors.append(color.rgba());
            img.setColorTable(colors);
            for (int y = 0; y < rows; y++) {
                memcpy(img.scanLine(y), bits.constData() + y * rowSize, rowSize);
            }
        }
    }

    if (!img.isNull() && aSize) {
        *aSize = img.size();
    }

    return img;
}
