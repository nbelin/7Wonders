#include "human.hpp"

#include <iostream>


Human::Human(Board * board, const char * name) : Player(board, name)
{

}


void Human::showBoard() {
    board->getServer().showBoardToPlayer(view.id, board->toBoardView());
}


void Human::playImplem(const QVector<ActionType> & possibleActions, const QVector<CardId> & cards) {
    int possibleActionsInt = 0;
    for (ActionType type : possibleActions) {
        possibleActionsInt += type;
    }
    board->getServer().showCardsToPlay(view.id, possibleActionsInt, cards);
}

