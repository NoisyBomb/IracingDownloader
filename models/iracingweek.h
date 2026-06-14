#pragma once

#include <QDateTime>
#include <QTimeZone>

// iRacing season = 12 race weeks + 1 off-week (13 total)
// Seasons start on Tuesday 00:00 UTC
// 2026 S1 Week 1 = 16 December 2025
struct IracingWeek
{
    int season;
    int year;
    int week;        // 1-based, 13 = off-week
    int totalWeeks;  // 13 (12 race + 1 off)
    bool isOffWeek;  // true if week == 13

    static IracingWeek current()
    {
        static const QDateTime anchor =
            QDateTime(QDate(2025, 12, 16), QTime(0, 0, 0), QTimeZone::utc());

        const QDateTime now = QDateTime::currentDateTimeUtc();
        const qint64 daysSince = anchor.daysTo(now);

        if (daysSince < 0)
            return { 1, 2026, 1, 13, false };

        const int weekIndex      = static_cast<int>(daysSince / 7); // 0-based
        const int weeksPerSeason = 13; // 12 race + 1 off
        const int seasonIndex    = weekIndex / weeksPerSeason;
        const int week           = weekIndex % weeksPerSeason + 1;  // 1-based

        const int year   = 2026 + seasonIndex / 4;
        const int season = seasonIndex % 4 + 1;
        const bool off   = (week == 13);

        return { season, year, week, weeksPerSeason, off };
    }

    static QDateTime startOf(int year, int season, int week)
    {
        static const QDateTime anchor =
            QDateTime(QDate(2025, 12, 16), QTime(0, 0, 0), QTimeZone::utc());

        const int seasonOffset = (year - 2026) * 4 + (season - 1);
        const int weekOffset   = seasonOffset * 13 + (week - 1);
        return anchor.addDays(weekOffset * 7);
    }

    QString label() const
    {
        if (isOffWeek)
            return QString("Week 13 (Off)");
        return QString("Week %1").arg(week);
    }
};
