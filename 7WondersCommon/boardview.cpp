#include "boardview.hpp"

#include "tools.hpp"

#include <iostream>

size_t BoardView::getPlayerArrayId(PlayerId playerId) const {
    for (int i=0; i<players.size(); ++i) {
        if (players[i].id == playerId) {
            return i;
        }
    }
    return 0;
}


const PlayerView & BoardView::getPlayer(PlayerId playerId, int offset) const {
    size_t arrayId = getPlayerArrayId(playerId);
    size_t newId = (arrayId + offset) % players.size();
    return players[newId];
}


const PlayerView & BoardView::getLeftPlayer(PlayerId playerId) const {
    size_t arrayId = getPlayerArrayId(playerId);
    size_t newId = (arrayId + 1) % players.size();
    return players[newId];
}


const PlayerView & BoardView::getRightPlayer(PlayerId playerId) const {
    size_t arrayId = getPlayerArrayId(playerId);
    size_t newId = (arrayId + players.size() - 1) % players.size();
    return players[newId];
}


QVector<const PlayerView *> BoardView::getApplyPlayers(PlayerId playerId, WhichPlayer which) const {
    QVector<const PlayerView *> res;
    if (which & PlayerSelf) {
        res.append(&getPlayer(playerId));
    }
    if (which & PlayerLeft) {
        res.append(&getLeftPlayer(playerId));
    }
    if (which & PlayerRight) {
        res.append(&getRightPlayer(playerId));
    }
    return res;
}


int BoardView::countPoints(PlayerId playerId) {
    int count = 0;
    count += countMilitaryPoints(playerId);
    count += countMoneyPoints(playerId);
    count += countWonderPoints(playerId);
    count += countCivilisationPoints(playerId);
    count += countCommercePoints(playerId);
    count += countGuildPoints(playerId);
    count += countSciencePoints(playerId);
    return count;
}


int BoardView::countMilitaryPoints(PlayerId playerId) {
    const PlayerView & player = getPlayer(playerId);
    int count = 0;
    for (MilitaryPoints mp : player.militaryPoints) {
        count += mp;
    }
    return count;
}


int BoardView::countMoneyPoints(PlayerId playerId) {
    const PlayerView & player = getPlayer(playerId);
    return player.coins / 3;
}


int BoardView::countWonderPoints(PlayerId playerId) {
    const PlayerView & player = getPlayer(playerId);
    int count = 0;
    QVector<CardId> actualStages = AllWonders::getWonder(player.wonderId).getStages(player.wonderFace);
    actualStages.resize(player.wonderStages.size());
    for (CardId cardId : actualStages) {
        count += AllCards::getCard(cardId).points;
    }
    return count;
}


int BoardView::countCivilisationPoints(PlayerId playerId) {
    const PlayerView & player = getPlayer(playerId);
    int count = 0;
    for (CardId cardId : player.playedCards) {
        const Card & card = AllCards::getCard(cardId);
        if (card.type == TypeCivilisation) {
            count += card.points;
        }
    }
    return count;
}


int BoardView::countCommercePoints(PlayerId playerId) {
    const PlayerView & player = getPlayer(playerId);
    int count = 0;
    for (CardId cardId : player.playedCards) {
        const Card & card = AllCards::getCard(cardId);
        if (card.type == TypeCommerce) {
            if (card.cardTypesCondition.empty()) {
                count += card.points;
            } else {
                for ( CardType typeCondition : card.cardTypesCondition ) {
                    if (typeCondition == TypeWonder) {
                        count += card.points * player.wonderStages.size();
                    } else {
                        for ( CardId cardId : player.playedCards ) {
                            if (AllCards::getCard(cardId).type == typeCondition) {
                                count += card.points;
                            }
                        }
                    }
                }
            }
        }
    }
    return count;
}


int BoardView::countGuildPoints(PlayerId playerId) {
    const PlayerView & player = getPlayer(playerId);
    int count = 0;
    for (CardId cardId : player.playedCards) {
        const Card & card = AllCards::getCard(cardId);
        if (card.type == TypeGuild) {
            std::cout << "count points from card: " << card.name << std::endl;
            if (card.cardTypesCondition.empty()) {
                std::cout << "no condition: " << card.points << std::endl;
                count += card.points;
            } else {
                for ( CardType typeCondition : card.cardTypesCondition ) {
                    const QVector<const PlayerView *> & listApplyPlayers = getApplyPlayers(playerId, card.target);
                    for ( const PlayerView * applyPlayer : listApplyPlayers ) {
                        if (typeCondition == TypeWonder) {
                            count += card.points * applyPlayer->wonderStages.size();
                        } else if (typeCondition == TypeMilitaryDefeat ) {
                            for ( MilitaryPoints mp : applyPlayer->militaryPoints ) {
                                if (mp < 0) {
                                    count += card.points;
                                }
                            }
                        } else {
                            for ( CardId cardId : applyPlayer->playedCards ) {
                                if (AllCards::getCard(cardId).type == typeCondition) {
                                    count += card.points;
                                }
                            }
                        }
                    }
                }
            }
            std::cout << "total count: " << count << std::endl;
        }
    }
    return count;

}


int BoardView::countSciencePoints(PlayerId playerId) {
    const PlayerView & player = getPlayer(playerId);
    QVector<ScienceType> listSciences;
    for (CardId cardId : player.playedCards) {
        const Card & card = AllCards::getCard(cardId);
        if (card.science != ScienceNo) {
            listSciences.append(card.science);
        }
    }
    QVector<CardId> buildStages = AllWonders::getWonder(player.wonderId).getStages(player.wonderFace);
    buildStages.resize(player.wonderStages.size());
    for (CardId cardId : buildStages) {
        const Card & card = AllCards::getCard(cardId);
        if (card.science != ScienceNo) {
            listSciences.append(card.science);
        }
    }
    return utilCountSciencePoints(listSciences);
}


QString BoardView::toString() const {
    QString res;
    QStringList list;
    QStringList listPlayers;
    for ( const PlayerView & pv : players ) {
        listPlayers.append(pv.toString());
    }
    list.append(listPlayers.join(subSep));
    QStringList listDiscardedCards;
    for ( Age age : discardedCards ) {
        listDiscardedCards.append(QString::number(age));
    }
    list.append(listDiscardedCards.join(subSep));
    list.append(QString::number(currentRound));
    list.append(QString::number(currentAge));
    res = list.join(sep);
    return res;
}


void BoardView::fromString(const QString & str) {
    QStringList list = str.split(sep);
    players.clear();
    for ( const QString & strPlayer : Tools::mySplit(list[0], subSep) ) {
        PlayerView pv;
        pv.fromString(strPlayer);
        players.append(pv);
    }
    discardedCards.clear();
    for ( const QString & strAge : Tools::mySplit(list[1], subSep) ) {
        discardedCards.append(strAge.toInt());
    }
    currentRound = list[2].toInt();
    currentAge = list[3].toInt();
}

