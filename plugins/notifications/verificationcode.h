/**
 * SPDX-FileCopyrightText: 2026 Sharad Mittal <sharadmittals@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QString>

namespace VerificationCode
{
/**
 * Find a one-time verification code (OTP, sign-in code, 2FA code) in a notification's text.
 *
 * The text must contain a code-related keyword ("code", "OTP", "verification", ..., in English
 * or the user's language) and a 4-8 digit number (or two groups of 3 digits, like "123 456").
 * When there are several numbers, the one closest to a keyword wins.
 *
 * @return the code with any separators removed, or an empty string if there is none
 */
QString find(const QString &text);

/**
 * Put a code on the clipboard, marked as secret so that it stays out of the clipboard
 * history and the clipboard plugin doesn't send it back to the device
 */
void copyToClipboard(const QString &code);
}
