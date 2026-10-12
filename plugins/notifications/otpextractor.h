/**
 * SPDX-FileCopyrightText: 2026 Sharad Mittal <sharadmittals@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QString>

namespace OtpExtractor
{
/**
 * Find a one-time passcode (verification code, OTP, 2FA code) in a notification's text.
 *
 * The text must mention a code-related keyword ("code", "OTP", "verification", ...) and
 * contain a 4-8 digit number (or two groups of 3 digits, like "123 456"). When there are
 * several numbers, the one closest to a keyword wins.
 *
 * @return the code with any separators removed, or an empty string if there is none
 */
QString extract(const QString &text);
}
