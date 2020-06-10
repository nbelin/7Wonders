#include "player.hpp"


#include <iostream>


Player::Player(Board * board, const char * name):
    board(board),
    shields(0),
    status(StatusNotReady),
    canPlayBothCardsAtEndOfAge(false),
    canCopyNeirbyGuild(false),
    canPlayCardForFree(false),
    canPlayCardForFreeAlreadyUsed(false),
    canPlayCardFromDiscarded(false)
{
    view.id = board->addPlayer(this);
    view.name = name;
    priceToBuyLeft = Resource(Resource::ONE_ALL, 2);
    priceToBuyRight = Resource(Resource::ONE_ALL, 2);
}


Player::~Player() {
    board->removePlayer(this);
}


void Player::setWonder(WonderId wonderId) {
    if (view.wonderId != WonderIdInvalid) {
        std::cout << "WARNING wonder already set" << std::endl;
    }
    view.wonderId = wonderId;
    view.wonderFace = WonderFaceB;
    const Wonder & wonder = AllWonders::getWonder(wonderId);
    production.addResource(wonder.baseIncome);
    buyableResources.addResource(wonder.baseIncome);
    std::cout << "set wonder: " << view.name.toStdString() << ": " << wonderId << " " << wonder.name.toStdString() << std::endl;
}


void Player::newRound() {
    view.lastPlayedActions = actionsToPlay;
    for ( Action & action : view.lastPlayedActions ) {
        action.simplifyForView();
    }
    actionsToPlay.clear();
}


void Player::newAge() {
    canPlayCardForFreeAlreadyUsed = false;
}


void Player::play(const QVector<ActionType> & possibleActions, const QVector<CardId> & cards) {
    std::cout << view.name.toStdString() << " plays: " << cards.size() << std::endl;
    for (CardId cardId : cards) {
        std::cout << "  " << cardId << " (" << AllCards::getCard(cardId).name << ")" << std::endl;
    }

    status = StatusPlaying;

    if (possibleActions.empty()) {
        QVector<ActionType> tmpActions;
        tmpActions.append(playCard);
        tmpActions.append(buildWonder);
        tmpActions.append(discardCard);
        if (canPlayCardForFree && !canPlayCardForFreeAlreadyUsed) {
            tmpActions.append(playFreeCard);
        }
        playImplem(tmpActions, cards);
    } else {
        playImplem(possibleActions, cards);
    }
}


void Player::addPlayedAction(const Action & action) {
    status = StatusPlayed;
    actionsToPlay.append(action);
    actionsToPlay[actionsToPlay.size() - 1].status = validated;

    // need to check right away if player can play another action in the same round!
    if (action.type == buildWonder) {
        QVector<CardId> stages = AllWonders::getWonder(view.wonderId).getStages(view.wonderFace);
        stages.resize(view.wonderStages.size() + 1);
        for (CardId stage : stages) {
            if (AllCards::getCard(stage).special == BothCardsAtEndOfAge) {
                canPlayBothCardsAtEndOfAge = true;
            }
        }
    }
}


double Player::evaluateScore() {
    BoardView bv = board->toBoardView();
    double points = bv.countPoints(view.id);


    // military
    Shield leftShields = board->getLeftPlayer(view.id)->shields;
    Shield rightShields = board->getRightPlayer(view.id)->shields;

    if (shields < leftShields) {
        points -= 1;
    }
    if (shields < rightShields) {
        points -= 1;
    }
    if (shields > leftShields) {
        points += 5;
    }
    if (shields > rightShields) {
        points += 5;
    }


    // science
    double futureScienceCoef = 0.0;
    switch (board->state.currentAge) {
    case 1:
        futureScienceCoef = 0.5;
        break;
    case 2:
        futureScienceCoef = 0.3;
        break;
    case 3:
        if (board->nbRounds - board->state.currentRound > 3) {
            futureScienceCoef = 0.1;
        } else {
            futureScienceCoef = 0.0;
        }
        break;
    default:
        std::cout << "ERROR invalid age" << std::endl;
        break;
    }

    QVector<ScienceType> listSciences = view.getSciences();
    points += listSciences.count(ScienceAll) * 0.1; // AI wants to play ScienceAll even if score is the same
    listSciences.append(ScienceAll);
    points += utilCountSciencePoints(listSciences) * futureScienceCoef;


    // economy
    points += production.evaluateScore();


    // special
    if (canPlayBothCardsAtEndOfAge) {
        points += 4.5;
    }

    if (canCopyNeirbyGuild) {
        points += 6.5;
    }

    if (canPlayCardForFree) {
        points += 2.5;
    }

    if (canPlayCardForFreeAlreadyUsed) {
        points -= 0.5;
    }

    if (canPlayCardFromDiscarded) {
        points += board->isLastRound();
        points += board->state.currentAge;
    }

    return points;
}


