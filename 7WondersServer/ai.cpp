#include "ai.hpp"

#include <iostream>


AI::AI(Board * board, const char * name) : Player(board, name) {
    fakeBoard = new Board(nullptr, true);
}


AI::~AI() {
    delete fakeBoard;
}


void AI::showBoard() {
    //do nothing
}


void AI::playImplem(const QVector<ActionType> & possibleActions, const QVector<CardId> & cards) {
    QString dummy;

    Action bestAction;
    double bestScore = -999.9;

    Board::BoardState state = board->saveBoardState();

    Action action;
    for ( ActionType type : possibleActions ) {
        action.type = type;
        for ( CardId cardId : cards ) {
            action.card = cardId;
            fakeBoard->restoreFakeBoardState(state);
            //std::cout << fakeBoard->toBoardView().toString().toStdString() << std::endl;
            //std::cout << "test action: " << AllCards::getCard(cardId).name << std::endl;
            action.display();
            if (fakeBoard->isValidAction(view.id, action, dummy)) {
                //std::cout << "valid!" << std::endl;
                fakeBoard->playSingleAction(view.id, action);
                double score = fakeBoard->getPlayer(view.id)->evaluateScore();
                //std::cout << "score: " << score << std::endl;
                if (score > bestScore) {
                    bestScore = score;
                    bestAction = action;
                    std::cout << "replace best score !" << std::endl;
                }
            }
            //std::cout << "not valid! " << dummy.toStdString() << std::endl;
        }
    }

    // no action is possible (example: play discarded cards and all of them already built)
    addPlayedAction(bestAction);
}

