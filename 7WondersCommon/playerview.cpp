#include "playerview.hpp"

#include <QStringList>
#include <iostream>

#include "tools.hpp"

PlayerView::PlayerView()
{
    id = PlayerIdInvalid;
    wonderId = WonderIdInvalid;
    wonderFace = WonderFaceInvalid;
    coins = 0;
}


QString PlayerView::toString() const {
    QString res;
    QStringList list;
    list.append(QString::number(id));
    list.append(name);
    list.append(QString::number(wonderId));
    list.append(QString::number(wonderFace));
    QStringList listStages;
    for ( Age age : wonderStages ) {
        listStages.append(QString::number(age));
    }
    list.append(listStages.join(subSep));
    list.append(QString::number(coins));
    QStringList listPlayedCards;
    for ( CardId cardId : playedCards ) {
        listPlayedCards.append(QString::number(cardId));
    }
    list.append(listPlayedCards.join(subSep));
    QStringList listMilitaryPoints;
    for ( MilitaryPoints mp : militaryPoints ) {
        listMilitaryPoints.append(QString::number(mp));
    }
    list.append(listMilitaryPoints.join(subSep));
    QStringList listActions;
    for ( const Action & action : lastPlayedActions ) {
        listActions.append(action.toString());
    }
    list.append(listActions.join(subSep));
    res = list.join(sep);
    return res;
}


void PlayerView::fromString(const QString & str) {
    QStringList list = str.split(sep);
    id = list[0].toInt();
    name = list[1];
    wonderId = list[2].toInt();
    wonderFace = (WonderFace)list[3].toInt();
    wonderStages.clear();
    for ( const QString & strStage : Tools::mySplit(list[4], subSep) ) {
        wonderStages.append(strStage.toInt());
    }
    coins = list[5].toInt();
    playedCards.clear();
    for ( const QString & strCardId : Tools::mySplit(list[6], subSep) ) {
        playedCards.append(strCardId.toInt());
    }
    militaryPoints.clear();
    for ( const QString & strMp : Tools::mySplit(list[7], subSep) ) {
        militaryPoints.append(strMp.toInt());
    }
    lastPlayedActions.clear();
    for ( const QString & strAction : Tools::mySplit(list[8], subSep) ) {
        Action tmpAction;
        tmpAction.fromString(strAction);
        lastPlayedActions.append(tmpAction);
    }
}


QVector<ScienceType> PlayerView::getSciences() const {
    QVector<ScienceType> listSciences;
    for (CardId cardId : playedCards) {
        const Card & card = AllCards::getCard(cardId);
        if (card.science != ScienceNo) {
            listSciences.append(card.science);
        }
    }
    QVector<CardId> buildStages = AllWonders::getWonder(wonderId).getStages(wonderFace);
    buildStages.resize(wonderStages.size());
    for (CardId cardId : buildStages) {
        const Card & card = AllCards::getCard(cardId);
        if (card.science != ScienceNo) {
            listSciences.append(card.science);
        }
    }
    return listSciences;
}
