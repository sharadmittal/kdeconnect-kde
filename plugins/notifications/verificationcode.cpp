/**
 * SPDX-FileCopyrightText: 2026 Sharad Mittal <sharadmittals@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "verificationcode.h"

#include <KLocalizedString>
#include <KSystemClipboard>
#include <QClipboard>
#include <QList>
#include <QMimeData>
#include <QRegularExpression>
#include <QStringList>

#include <climits>

namespace VerificationCode
{

static QRegularExpression keywordRegex()
{
    QStringList keywords{QStringLiteral(
        "otp|code|passcode|password|pin|verification|verify|one[- ]time|2fa|two[- ]factor|security|authenticat\\w*|log[- ]?in|sign[- ]?in")};

    const QString translated = i18nc(
        "Words that appear next to one-time verification codes in text messages, separated by |. "
        "Used to recognise codes in notifications, in addition to the English words. "
        "Translate to the words services in your language use.",
        "code|verification");
    for (const QString &word : translated.split(QLatin1Char('|'), Qt::SkipEmptyParts)) {
        keywords.append(QRegularExpression::escape(word.trimmed()));
    }

    return QRegularExpression(QStringLiteral("\\b(") + keywords.join(QLatin1Char('|')) + QStringLiteral(")\\b"),
                              QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);
}

QString find(const QString &text)
{
    static const QRegularExpression keywords = keywordRegex();
    // Discount codes look like verification codes but aren't
    static const QRegularExpression promoRegex(QStringLiteral("\\b(promo|coupon|discount|voucher)\\b"), QRegularExpression::CaseInsensitiveOption);
    // A 4-8 digit number, or "123 456" / "123-456", that isn't part of a longer number, word,
    // amount, phone number, date, time or percentage
    static const QRegularExpression codeRegex(QStringLiteral("(?<![\\p{L}\\d$\\x{20AC}\\x{A3}\\x{20B9}])(?<!\\d[-./ :])(\\d{3}[- ]\\d{3}|\\d{4,8})(?![\\p{L}\\d%]|[-./:,]\\d)"),
                                              QRegularExpression::UseUnicodePropertiesOption);

    if (promoRegex.match(text).hasMatch()) {
        return {};
    }

    QList<QRegularExpressionMatch> keywordMatches;
    for (auto it = keywords.globalMatch(text); it.hasNext();) {
        keywordMatches.append(it.next());
    }
    if (keywordMatches.isEmpty()) {
        return {};
    }

    QString best;
    int bestDistance = INT_MAX;
    for (auto it = codeRegex.globalMatch(text); it.hasNext();) {
        const QRegularExpressionMatch code = it.next();
        for (const QRegularExpressionMatch &keyword : std::as_const(keywordMatches)) {
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

void copyToClipboard(const QString &code)
{
    auto mimeData = new QMimeData;
    mimeData->setText(code);
    mimeData->setData(QStringLiteral("x-kde-passwordManagerHint"), QByteArrayLiteral("secret"));
    KSystemClipboard::instance()->setMimeData(mimeData, QClipboard::Clipboard);
}

}
