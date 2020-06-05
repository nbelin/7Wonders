#include "ai.hpp"

#include <iostream>


AI::AI(Board * board, const char * name) : Player(board, name)
{

}


void AI::showBoard() {
    // do nothing
}


void AI::playImplem(const QVector<ActionType> & possibleActions, const QVector<CardId> & cards) {
    QString dummy;

    Action action;
    for ( ActionType type : possibleActions ) {
        action.type = type;
        for ( CardId cardId : cards ) {
            action.card = cardId;
            if (board->isValidAction(view.id, action, dummy)) {
                addPlayedAction(action);
                return;
            }
        }
    }

    // no action was possible (example: play discarded cards and all of them already built)
    action.type = noAction;
    addPlayedAction(action);
}

