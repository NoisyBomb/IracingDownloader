#pragma once

#include <QDateTime>

// Вычисляет текущую неделю iRacing сезона.
// Сезоны стартуют во вторник в 00:00 UTC.
// 2026 S1 Week 1 = 16 декабря 2025
struct IracingWeek
{
    int season;
    int year;
    int week;       // 1-based
    int totalWeeks; // обычно 12, иногда 13

    static IracingWeek current()
    {
        // Якорь: 2026 S1 Week 1 = 16 декабря 2025 00:00 UTC
        static const QDateTime s1_2026_start =
            QDateTime(QDate(2025, 12, 16), QTime(0, 0, 0), Qt::UTC);

        const QDateTime now = QDateTime::currentDateTimeUtc();
        const qint64 daysSince = s1_2026_start.daysTo(now);

        if (daysSince < 0) {
            // До старта сезона — показываем неделю 1
            return { 1, 2026, 1, 12 };
        }

        // Каждые 7 дней — новая неделя
        const int weekIndex = static_cast<int>(daysSince / 7); // 0-based

        // 12 недель на сезон (иногда 13, но используем 12 по умолчанию)
        const int weeksPerSeason = 12;
        const int seasonIndex    = weekIndex / weeksPerSeason; // 0-based

        // Год и сезон (4 сезона в году)
        const int totalSeasons = seasonIndex;
        const int year   = 2026 + totalSeasons / 4;
        const int season = totalSeasons % 4 + 1;
        const int week   = weekIndex % weeksPerSeason + 1;

        return { season, year, week, weeksPerSeason };
    }

    // Дата начала конкретной недели UTC
    static QDateTime startOf(int year, int season, int week)
    {
        static const QDateTime anchor =
            QDateTime(QDate(2025, 12, 16), QTime(0, 0, 0), Qt::UTC);

        // Смещение от якоря в неделях
        const int seasonOffset = (year - 2026) * 4 + (season - 1);
        const int weekOffset   = seasonOffset * 12 + (week - 1);

        return anchor.addDays(weekOffset * 7);
    }

    QString label() const
    {
        return QString("Week %1").arg(week);
    }
};
