#include "ai.hpp"

#include <iostream>


AI::AI(Board * board, const char * name) : Player(board, name) {
    fakeBoard = new Board(nullptr, true);
    status = StatusReady;
}


AI::~AI() {
    delete fakeBoard;
}


void AI::chooseFace() {
    board->selectFace(view.id, WonderFaceInvalid); // will be random
    status = StatusPlayed;
}


void AI::showBoard() {
    //do nothing
}


void AI::playImplem(const QVector<ActionType> & possibleActions, const QVector<CardId> & cards) {
    QString dummy;

    Action bestAction;
    double bestScore = -999.9;
    QVector<Resource> listMissing;

    const ResourceProduction & buyableLeft = board->getLeftPlayer(view.id)->buyableResources;
    const ResourceProduction & buyableRight = board->getRightPlayer(view.id)->buyableResources;
    ResourceProduction buyable;
    buyable.addResourceProduction(buyableLeft);
    buyable.addResourceProduction(buyableRight);

    Board::BoardState state = board->saveBoardState();
    std::cout << "current score: " << board->getPlayer(view.id)->evaluateScore() << std::endl;

    Action action;
    for ( ActionType type : possibleActions ) {
        for ( CardId cardId : cards ) {
            action.reset();
            action.type = type;
            action.card = cardId;
            action.display();
            listMissing.clear();
            fakeBoard->restoreFakeBoardState(state);
            if (fakeBoard->isValidAction(view.id, action, dummy, &listMissing)) {
                fakeBoard->playSingleAction(view.id, action);
                double score = fakeBoard->getPlayer(view.id)->evaluateScore();
                std::cout << "score: " << score << std::endl;
                if (score > bestScore) {
                    bestScore = score;
                    bestAction = action;
                }
            } else if (listMissing.length() > 0 && listMissing.length() <= view.coins) {
                // cannot play because of missing resources, try to see if we buy them
                int minToPay = view.coins + 1;

                for (const Resource & missingRes : listMissing) {
                    if (buyable.canProduce(missingRes)) {
                        Resource buyLeft;
                        Resource buyRight;

                        for (int i=0; i<Resource::IdMAX; ++i) {
                            for (int j=0; j<missingRes.array[i]; ++j) {
                                Resource maybeToBuyLeft = buyLeft;
                                maybeToBuyLeft.array[i]++;
                                Resource maybeToBuyRight = buyRight;
                                maybeToBuyRight.array[i]++;
                                if (priceToBuyLeft.array[i] < priceToBuyRight.array[i] && buyableLeft.canProduce(maybeToBuyLeft)) {
                                    buyLeft.array[i]++;
                                } else if (priceToBuyLeft.array[i] > priceToBuyRight.array[i] && buyableRight.canProduce(maybeToBuyRight)) {
                                    buyRight.array[i]++;
                                } else {
                                    if (buyableLeft.canProduce(maybeToBuyLeft)) {
                                        buyLeft.array[i]++;
                                    } else {
                                        buyRight.array[i]++;
                                    }
                                }
                            }
                        }

                        int payLeft = buyLeft.multiplyAndSum(priceToBuyLeft);
                        int payRight = buyRight.multiplyAndSum(priceToBuyRight);
                        int pay = payLeft + payRight;
                        if (pay < minToPay) {
                            minToPay = pay;
                            action.boughtFromLeft = buyLeft;
                            action.boughtFromRight = buyRight;
                        }

                    }
                }

                action.display();
                fakeBoard->restoreFakeBoardState(state);
                if (fakeBoard->isValidAction(view.id, action, dummy)) {
                    fakeBoard->playSingleAction(view.id, action);
                    double score = fakeBoard->getPlayer(view.id)->evaluateScore();
                    std::cout << "score: " << score << std::endl;
                    if (score > bestScore) {
                        bestScore = score;
                        bestAction = action;
                    }
                }
            }
        }
    }

    // no action is possible (example: play discarded cards and all of them already built)
    std::cout << "best action for " << view.name.toStdString() << std::endl;
    bestAction.display();
    std::cout << "last error: " << dummy.toStdString() << std::endl;
    addPlayedAction(bestAction);
}

