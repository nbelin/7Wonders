#ifndef IA_H
#define IA_H

#include "player.hpp"

class AI : public Player {
public:
    AI(Board * board, const char * name);

    virtual void showBoard();
    virtual void playImplem(const QVector<ActionType> & possibleActions, const QVector<CardId> & cards);
};

#endif // IA_H
