/**
 * SPDX-FileCopyrightText: 2019 Simon Redman <simon@ergotech.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "smsapp/smshelper.h"
#include "networkpacket.h"

#include <QTest>

/**
 * This class tests the working of device class
 */
class SmsHelperTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testSimplePhoneMatch();
    void testMessyPhoneMatch();
    void testMissingCountryCode();
    void testLeadingZeros();
    void testLongDistancePhoneNumber();
    void testShortCodePhoneNumberNonMatch();
    void testShortCodeMatch();
    void testShortCodeNonMatch();
    void testAustralianShortCodeNumberNonMatch();
    void testAustralianShortCodeMatch();
    void testAustralianShortCodeNonMatch();
    void testCzechRepublicShortCodeNumberNonMatch();
    void testCzechRepublicShortCodeMatch();
    void testCzechRepublicShortCodeNonMatch();
    void testDifferentPhoneNumbers1();
    void testDifferentPhoneNumbers2();
    void testAllZeros();
    void testEmptyInput();
    void testFormatConversationDate_data();
    void testFormatConversationDate();
    void testFormatDaySeparator();
    void testSameMessageGroup();
    void benchmark_serialize()
    {
        NetworkPacket pack(QStringLiteral("bla"), QVariantMap{{QStringLiteral("test"), 54321}});
        pack.setPayloadTransferInfo({{QStringLiteral("testme"), 123}});
        QBENCHMARK {
            pack.serialize();
        }
    }

    void benchmark_deserialize()
    {
        NetworkPacket pack(QStringLiteral("bla"), QVariantMap{{QStringLiteral("test"), 54321}});
        pack.setPayloadTransferInfo({{QStringLiteral("testme"), 123}});
        auto serialized = pack.serialize();
        QBENCHMARK {
            NetworkPacket packet;
            NetworkPacket::unserialize(serialized, &packet);
        }
    }
};

/**
 * Two phone numbers which are exactly the same should match
 */
void SmsHelperTest::testSimplePhoneMatch()
{
    const QString &phoneNumber = QLatin1String("+1 (222) 333-4444");

    QVERIFY2(SmsHelper::isPhoneNumberMatch(phoneNumber, phoneNumber), "Failed to match a phone number with itself");
}

/**
 * Two phone numbers which have been entered with different formatting should match
 */
void SmsHelperTest::testMessyPhoneMatch()
{
    const QString &phoneNumber = QLatin1String("12223334444");
    const QString &messyPhoneNumber = QLatin1String("+1 (222) 333-4444");

    QVERIFY2(SmsHelper::isPhoneNumberMatch(phoneNumber, messyPhoneNumber), "Failed to match same number with different formatting characters");
}

/**
 * I don't think most people in the US even know what a country code *is*, and I know lots of people
 * who don't enter it into their contacts book here. Make sure that kind of match works.
 */
void SmsHelperTest::testMissingCountryCode()
{
    const QString &shortForm = QLatin1String("(222) 333-4444");
    const QString &longForm = QLatin1String("+1 (222) 333-4444");

    QVERIFY2(SmsHelper::isPhoneNumberMatch(shortForm, longForm), "Failed to match two same numbers with missing country code");
}

/**
 * I don't quite understand which cases this applies, but sometimes you see a message where the
 * country code has been prefixed with a few zeros. Make sure that works too.
 */
void SmsHelperTest::testLeadingZeros()
{
    const QString &shortForm = QLatin1String("+1 (222) 333-4444");
    const QString &zeroForm = QLatin1String("001 (222) 333-4444");

    QVERIFY2(SmsHelper::isPhoneNumberMatch(shortForm, zeroForm), "Failed to match two same numbers with padded zeros");
}

/**
 * At least on my phone, it is possible to leave the area code and country code off but still have
 * the phone call or text message go through. Some people's contacts books might be this way, so make
 * sure we support matching that too
 */
void SmsHelperTest::testLongDistancePhoneNumber()
{
    const QString &shortForm = QLatin1String("333-4444");
    const QString &longForm = QLatin1String("+1 (222) 333-4444");

    QVERIFY2(SmsHelper::isPhoneNumberMatch(shortForm, longForm), "Failed to suffix match two phone numbers");
}

/**
 * Phone operators define short codes for a variety of reasons. Even if they have the same suffix,
 * they should not match a regular phone number
 */
void SmsHelperTest::testShortCodePhoneNumberNonMatch()
{
    const QString &shortCode = QLatin1String("44455");
    const QString &normalNumber = QLatin1String("222-334-4455");

    QVERIFY2(!SmsHelper::isPhoneNumberMatch(shortCode, normalNumber), "Short code matched with normal number");
}

/**
 * Two of the same short code should be able to match
 */
void SmsHelperTest::testShortCodeMatch()
{
    const QString &shortCode = QLatin1String("44455");
    QVERIFY2(SmsHelper::isPhoneNumberMatch(shortCode, shortCode), "Did not match two of the same short code");
}

/**
 * Two different short codes should not match
 */
void SmsHelperTest::testShortCodeNonMatch()
{
    const QString &shortCode1 = QLatin1String("44455");
    const QString &shortCode2 = QLatin1String("66677");
    QVERIFY2(!SmsHelper::isPhoneNumberMatch(shortCode1, shortCode2), "Incorrectly matched two different short codes");
}

/**
 * Even in the land down under, a short code phone number should not match a regular phone number
 */
void SmsHelperTest::testAustralianShortCodeNumberNonMatch()
{
    const QString &australianShortCode = QLatin1String("19333444");
    // I'm not sure if this is even a valid Australian phone number, but whatever...
    const QString &australianPhoneNumber = QLatin1String("+41 02 1233 3444");

    QVERIFY2(!SmsHelper::isPhoneNumberMatch(australianShortCode, australianPhoneNumber), "Matched Australian short code with regular phone number");
}

/**
 * Two of the same Australian short code numbers should be able to match
 */
void SmsHelperTest::testAustralianShortCodeMatch()
{
    const QString &australianShortCode = QLatin1String("12333444");

    QVERIFY2(SmsHelper::isPhoneNumberMatch(australianShortCode, australianShortCode), "Failed to match Australian short code number");
}

/**
 * Two different Australian short code numbers should be able to match
 */
void SmsHelperTest::testAustralianShortCodeNonMatch()
{
    const QString &australianShortCode1 = QLatin1String("12333444");
    const QString &australianShortCode2 = QLatin1String("12555666");

    QVERIFY2(!SmsHelper::isPhoneNumberMatch(australianShortCode1, australianShortCode2), "Matched two different Australian short code numbers");
}

/**
 * A Czech Republic short code phone number should not match a regular phone number
 */
void SmsHelperTest::testCzechRepublicShortCodeNumberNonMatch()
{
    const QString &czechRepShortCode = QLatin1String("9090930");
    // I'm not sure if this is even a valid Czech Republic phone number, but whatever...
    const QString &czechRepPhoneNumber = QLatin1String("+420 809 090 930");

    QVERIFY2(!SmsHelper::isPhoneNumberMatch(czechRepShortCode, czechRepPhoneNumber), "Matched Czech Republic short code with regular phone number");
}

/**
 * Two of the same Czech Republic short code numbers should be able to match
 */
void SmsHelperTest::testCzechRepublicShortCodeMatch()
{
    const QString &czechRepShortCode = QLatin1String("9090930");

    QVERIFY2(SmsHelper::isPhoneNumberMatch(czechRepShortCode, czechRepShortCode), "Failed to match Czech Republic short code number");
}

/**
 * Two different Czech Republic short code numbers should be able to match
 */
void SmsHelperTest::testCzechRepublicShortCodeNonMatch()
{
    const QString &czechRepShortCode1 = QLatin1String("9090930");
    const QString &czechRepShortCode2 = QLatin1String("9080990");

    QVERIFY2(!SmsHelper::isPhoneNumberMatch(czechRepShortCode1, czechRepShortCode2), "Matched two different Czech Republic short code numbers");
}

/**
 * Two phone numbers which are different should not be reported as the same
 */
void SmsHelperTest::testDifferentPhoneNumbers1()
{
    const QString &phone1 = QLatin1String("+1 (222) 333-4444");
    const QString &phone2 = QLatin1String("+1 (333) 222-4444");

    QVERIFY2(!SmsHelper::isPhoneNumberMatch(phone1, phone2), "Incorrectly marked two different numbers as same");
}

/**
 * Two phone numbers which are different should not be reported as the same
 */
void SmsHelperTest::testDifferentPhoneNumbers2()
{
    const QString &phone1 = QLatin1String("+1 (222) 333-4444");
    const QString &phone2 = QLatin1String("122-2333");

    QVERIFY2(!SmsHelper::isPhoneNumberMatch(phone1, phone2), "Incorrectly *prefix* matched two phone numbers");
}

/**
 * Some places allow a message with all zeros to be a short code. We should allow that too.
 */
void SmsHelperTest::testAllZeros()
{
    const QString &zeros = QLatin1String("00000");
    const QString &canonicalized = SmsHelper::canonicalizePhoneNumber(zeros);

    QCOMPARE(canonicalized.length(), zeros.length());
}

/**
 * An empty string is not a valid phone number and should not match anything
 */
void SmsHelperTest::testEmptyInput()
{
    const QString &empty = QLatin1String("");
    const QString &shortCode = QLatin1String("44455");
    const QString &realNumber = QLatin1String("12223334444");

    QVERIFY2(!SmsHelper::isPhoneNumberMatch(empty, shortCode), "The empty string matched a shortcode phone number");
    QVERIFY2(!SmsHelper::isPhoneNumberMatch(empty, realNumber), "The empty string matched a real phone number");
}

void SmsHelperTest::testFormatConversationDate_data()
{
    QTest::addColumn<QDateTime>("date");
    QTest::addColumn<QString>("expected");

    QTest::newRow("today") << QDateTime(QDate(2026, 10, 12), QTime(9, 5)) << QStringLiteral("09:05");
    QTest::newRow("later today") << QDateTime(QDate(2026, 10, 12), QTime(15, 0)) << QStringLiteral("15:00");
    QTest::newRow("yesterday") << QDateTime(QDate(2026, 10, 11), QTime(23, 59)) << QStringLiteral("Yesterday");
    QTest::newRow("this week") << QDateTime(QDate(2026, 10, 6), QTime(8, 0)) << QStringLiteral("Tue");
    QTest::newRow("this year") << QDateTime(QDate(2026, 10, 5), QTime(8, 0)) << QStringLiteral("5 Oct");
    QTest::newRow("last year") << QDateTime(QDate(2025, 12, 31), QTime(8, 0)) << QStringLiteral("31 Dec 2025");
}

void SmsHelperTest::testFormatConversationDate()
{
    QFETCH(QDateTime, date);
    QFETCH(QString, expected);

    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedKingdom));
    const QDateTime now(QDate(2026, 10, 12), QTime(14, 30));
    QCOMPARE(SmsHelper::formatConversationDate(date, now), expected);
}

void SmsHelperTest::testFormatDaySeparator()
{
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedKingdom));
    const QDateTime now(QDate(2026, 10, 12), QTime(14, 30));

    QCOMPARE(SmsHelper::formatDaySeparator(QDateTime(QDate(2026, 10, 12), QTime(0, 1)), now), QStringLiteral("Today"));
    QCOMPARE(SmsHelper::formatDaySeparator(QDateTime(QDate(2026, 10, 11), QTime(23, 59)), now), QStringLiteral("Yesterday"));
    QCOMPARE(SmsHelper::formatDaySeparator(QDateTime(QDate(2026, 10, 6), QTime(8, 0)), now), QStringLiteral("Tue 6 Oct"));
    QCOMPARE(SmsHelper::formatDaySeparator(QDateTime(QDate(2025, 12, 31), QTime(8, 0)), now), QStringLiteral("Wed 31 Dec 2025"));
}

void SmsHelperTest::testSameMessageGroup()
{
    const QDateTime first(QDate(2026, 10, 12), QTime(14, 30));

    QVERIFY(SmsHelper::isSameMessageGroup(first, first));
    QVERIFY(SmsHelper::isSameMessageGroup(first, first.addSecs(4 * 60 + 59)));
    QVERIFY(!SmsHelper::isSameMessageGroup(first, first.addSecs(5 * 60)));
    // A new day always starts a new group
    const QDateTime lateNight(QDate(2026, 10, 12), QTime(23, 59));
    QVERIFY(!SmsHelper::isSameMessageGroup(lateNight, lateNight.addSecs(2 * 60)));
}

QTEST_GUILESS_MAIN(SmsHelperTest);
#include "smshelpertest.moc"
