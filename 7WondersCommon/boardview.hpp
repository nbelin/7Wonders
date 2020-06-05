#ifndef BOARDVIEW_H
#define BOARDVIEW_H

#include <QObject>
#include <QVector>
#include <QDataStream>

#include <card.hpp>
#include <wonder.hpp>

#include "playerview.hpp"


class BoardView {
public:
    size_t getPlayerArrayId(PlayerId playerId) const;
    const PlayerView & getPlayer(PlayerId playerId, int offset=0) const;
    const PlayerView & getLeftPlayer(PlayerId playerId) const;
    const PlayerView & getRightPlayer(PlayerId playerId) const;
    QVector<const PlayerView *> getApplyPlayers(PlayerId playerId, WhichPlayer which) const;
    int countPoints(PlayerId playerId);
    int countMilitaryPoints(PlayerId playerId);
    int countMoneyPoints(PlayerId playerId);
    int countWonderPoints(PlayerId playerId);
    int countCivilisationPoints(PlayerId playerId);
    int countCommercePoints(PlayerId playerId);
    int countGuildPoints(PlayerId playerId);
    int countSciencePoints(PlayerId playerId);

    QString toString() const;
    void fromString(const QString & str);

public:
    QVector<PlayerView> players;
    QVector<Age> discardedCards;
    int currentRound;
    Age currentAge;

private:
    static constexpr const char * sep = "#";
    static constexpr const char * subSep = "\\";
};

#endif // BOARDVIEW_H
