#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <QVector>
#include <QDataStream>
#include <QString>

typedef int PlayerId;
const PlayerId PlayerIdInvalid = -1;

#include "card.hpp"
#include "wonder.hpp"
#include "action.hpp"

struct PlayerView {
    PlayerView();
    QString toString() const;
    void fromString(const QString & str);

    QVector<ScienceType> getSciences() const;

    PlayerId id;
    QString name;
    WonderId wonderId;
    WonderFace wonderFace;
    QVector<Age> wonderStages;
    Currency coins;
    QVector<CardId> playedCards;
    QVector<MilitaryPoints> militaryPoints;
    QVector<Action> lastPlayedActions;

private:
    static constexpr const char * sep = "~";
    static constexpr const char * subSep = "/";
};

#endif // PLAYERVIEW_H
