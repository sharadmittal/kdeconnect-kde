/**
 * SPDX-FileCopyrightText: 2026 Sharad Mittal <sharadmittals@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "plugins/notifications/verificationcode.h"

#include <QTest>

class VerificationCodeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testFind_data();
    void testFind();
};

void VerificationCodeTest::testFind_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("expected");

    QTest::newRow("code first") << QStringLiteral("482913 is your OTP for login. Do not share it with anyone.") << QStringLiteral("482913");
    QTest::newRow("dashed") << QStringLiteral("Your verification code is 123-456") << QStringLiteral("123456");
    QTest::newRow("spaced") << QStringLiteral("Your code: 123 456") << QStringLiteral("123456");
    QTest::newRow("prefixed") << QStringLiteral("G-123456 is your verification code.") << QStringLiteral("123456");
    QTest::newRow("four digits") << QStringLiteral("Use 7788 to sign in to Example.") << QStringLiteral("7788");
    QTest::newRow("after amount and account")
        << QStringLiteral("Rs. 1500 debited from a/c XX1234 on 12-10-26. OTP for txn is 905512") << QStringLiteral("905512");
    QTest::newRow("phone number nearby") << QStringLiteral("Call 1-800-555-0199 and enter code 4417") << QStringLiteral("4417");
    QTest::newRow("reference number") << QStringLiteral("Your Example login code is 58213. Ref 20261012") << QStringLiteral("58213");
    QTest::newRow("keyword before code")
        << QStringLiteral("KeyBank: DO NOT share this code. Your security code to access online banking is 992259.") << QStringLiteral("992259");
    QTest::newRow("multiline") << QStringLiteral("Example\nYour security code is\n830142") << QStringLiteral("830142");

    QTest::newRow("no keyword") << QStringLiteral("Reminder: appt on 10/14 at 3pm. Call 650-555-0142") << QString();
    QTest::newRow("promo") << QStringLiteral("Use promo code SAVE20 for 20% off orders over 1000") << QString();
    QTest::newRow("redacted") << QStringLiteral("Sensitive notification content hidden") << QString();
    QTest::newRow("time and year") << QStringLiteral("Login at 10:42 on 2026-10-12 from Chrome") << QString();
    QTest::newRow("empty") << QString() << QString();
}

void VerificationCodeTest::testFind()
{
    QFETCH(QString, text);
    QFETCH(QString, expected);
    QCOMPARE(VerificationCode::find(text), expected);
}

QTEST_MAIN(VerificationCodeTest)
#include "verificationcodetest.moc"
