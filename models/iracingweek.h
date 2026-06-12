#pragma once

#include <QDateTime>
#include <QTimeZone>


struct IracingWeek
{
    int season;
    int year;
    int week;
    int totalWeeks;

    static IracingWeek current()
    {
        static const QDateTime s1_2026_start =
            QDateTime(QDate(2025, 12, 16), QTime(0, 0, 0), QTimeZone::utc());

        const QDateTime now = QDateTime::currentDateTimeUtc();
        const qint64 daysSince = s1_2026_start.daysTo(now);

        if (daysSince < 0) {
            return { 1, 2026, 1, 12 };
        }

        const int weekIndex = static_cast<int>(daysSince / 7);

        const int weeksPerSeason = 12;
        const int seasonIndex    = weekIndex / weeksPerSeason;

        const int totalSeasons = seasonIndex;
        const int year   = 2026 + totalSeasons / 4;
        const int season = totalSeasons % 4 + 1;
        const int week   = weekIndex % weeksPerSeason + 1;

        return { season, year, week, weeksPerSeason };
    }

    static QDateTime startOf(int year, int season, int week)
    {
        static const QDateTime anchor =
            QDateTime(QDate(2025, 12, 16), QTime(0, 0, 0), QTimeZone::utc());
        const int seasonOffset = (year - 2026) * 4 + (season - 1);
        const int weekOffset   = seasonOffset * 12 + (week - 1);

        return anchor.addDays(weekOffset * 7);
    }

    QString label() const
    {
        return QString("Week %1").arg(week);
    }
};
