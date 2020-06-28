#include "choice.hpp"

Choice::Choice() {

}


QString Choice::toString() const {
    QString res;
    QStringList list;
    QStringList listPlayers;
    for ( const PlayerChoice & p : players ) {
        QStringList listPlayer;
        listPlayer.append(QString::number(p.id));
        listPlayer.append(p.name);
        listPlayer.append(QString::number(p.wonderId));
        listPlayer.append(QString::number(p.ready));
        listPlayers.append(listPlayer.join(subSubSep));
    }
    list.append(listPlayers.join(subSep));
    list.append(QString::number(numberAIs));
    list.append(QString::number(randomWonders));
    list.append(QString::number(randomFaces));
    list.append(QString::number(randomPlaces));
    res = list.join(sep);
    return res;
}


void Choice::fromString(const QString & str) {
    QStringList list = Tools::mySplit(str, sep);
    players.clear();
    for ( const QString & strPlayers : Tools::mySplit(list[0], subSep) ) {
        QStringList listPlayer = Tools::mySplit(strPlayers, subSubSep);
        PlayerChoice player;
        player.id = listPlayer[0].toInt();
        player.name = listPlayer[1];
        player.wonderId = listPlayer[2].toInt();
        player.ready = listPlayer[3].toInt();
        players.append(player);
    }
    numberAIs = list[1].toInt();
    randomWonders = list[2].toInt();
    randomFaces = list[3].toInt();
    randomPlaces = list[4].toInt();
}
