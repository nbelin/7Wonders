#ifndef HUMAN_H
#define HUMAN_H

#include <QObject>

#include "player.hpp"
#include "tcpserver.hpp"

class Human : public Player {
public:
    Human(Board * board, const char * name);

    virtual void showBoard();
    virtual void playImplem(const QVector<ActionType> & possibleActions, const QVector<CardId> & cards);
};

#endif // HUMAN_H
