/**
 * SPDX-FileCopyrightText: 2026 Sharad Mittal <sharadmittals@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "otpextractor.h"

#include <QList>
#include <QRegularExpression>

#include <climits>

namespace OtpExtractor
{

QString extract(const QString &text)
{
    static const QRegularExpression keywordRegex(
        QStringLiteral("\\b(otp|code|passcode|password|pin|verification|verify|one[- ]time|2fa|two[- ]factor|security|authenticat\\w*|log[- ]?in|sign[- ]?in)\\b"),
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);
    // Discount codes look like verification codes but aren't
    static const QRegularExpression promoRegex(QStringLiteral("\\b(promo|coupon|discount|voucher)\\b"), QRegularExpression::CaseInsensitiveOption);
    // A 4-8 digit number, or "123 456" / "123-456", that isn't part of a longer number, word,
    // amount, phone number, date, time or percentage
    static const QRegularExpression codeRegex(QStringLiteral("(?<![\\p{L}\\d$€£₹])(?<!\\d[-./ :])(\\d{3}[- ]\\d{3}|\\d{4,8})(?![\\p{L}\\d%]|[-./:,]\\d)"),
                                              QRegularExpression::UseUnicodePropertiesOption);

    if (promoRegex.match(text).hasMatch()) {
        return {};
    }

    QList<QRegularExpressionMatch> keywords;
    for (auto it = keywordRegex.globalMatch(text); it.hasNext();) {
        keywords.append(it.next());
    }
    if (keywords.isEmpty()) {
        return {};
    }

    QString best;
    int bestDistance = INT_MAX;
    for (auto it = codeRegex.globalMatch(text); it.hasNext();) {
        const QRegularExpressionMatch code = it.next();
        for (const QRegularExpressionMatch &keyword : std::as_const(keywords)) {
            int distance = 0;
            if (code.capturedStart() >= keyword.capturedEnd()) {
                distance = code.capturedStart() - keyword.capturedEnd();
            } else if (keyword.capturedStart() >= code.capturedEnd()) {
                distance = keyword.capturedStart() - code.capturedEnd();
            }
            if (distance < bestDistance) {
                bestDistance = distance;
                best = code.captured(1);
            }
        }
    }

    best.remove(QLatin1Char(' '));
    best.remove(QLatin1Char('-'));
    return best;
}

}
