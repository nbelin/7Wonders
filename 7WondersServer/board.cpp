#include "board.hpp"
#include "human.hpp"
#include "ai.hpp"
#include "boardview.hpp"
#include "playerview.hpp"

#include <algorithm>
#include <iostream>
#include <ctime>

#include <QThread>
#include <QTime>
#include <QCoreApplication>

Board::Board(QObject *parent, bool fake) : QObject(parent), tcpserver(this) {
    std::cout << "Board started" << std::endl;
    if (fake) {
        return;
    }

    state.nbPlayers = 0;
    state.nbAIs = 0;
    state.maxGenPlayerId = 0;
    state.currentRound = 0;
    state.currentAge = 0;
    state.status = StatusWaitingReady;

    random = std::default_random_engine(QTime::currentTime().msecsSinceStartOfDay());

    QObject::connect(&tcpserver, &TcpServer::addPlayerName, this, &Board::addPlayerName);
    QObject::connect(&tcpserver, &TcpServer::setNumberAIs, this, &Board::setNumberAIs);
    QObject::connect(&tcpserver, &TcpServer::setPlayerReady, this, &Board::setPlayerReady);
    QObject::connect(&tcpserver, &TcpServer::askStartGame, this, &Board::askStartGame);
    QObject::connect(&tcpserver, &TcpServer::askAction, this, &Board::askAction);

    AllCards::init();
    AllWonders::init();

    timer = new QTimer(this);
    QObject::connect(timer, &QTimer::timeout, this, &Board::gameProcess);
    timer->start(500);
}


Board::~Board() {}


void Board::mainLoop() {
    return;
    while (true) {
        bool allReady = true;
        for (Player * player : state.players) {
            if (player->status == StatusNotReady) {
                allReady = false;
            }
        }
        if (allReady) {
            tcpserver.sendDebug("start !!!!!!!!");
            askStartGame();
        }
        QThread::sleep(100);
    }
}


void Board::addPlayerName(const QTcpSocket * socket, const char * name) {
    if (state.status != StatusWaitingReady) {
        tcpserver.sendDebug("already started...");
        return;
    }
    Player * player = new Human(this, name);
    tcpserver.setPlayerId(socket, player->view.id);
    tcpserver.sendListPlayers(getListPlayers(true));
}


void Board::setNumberAIs(int number) {
    if (state.status != StatusWaitingReady) {
        tcpserver.sendDebug("already started...");
        return;
    }
    state.nbAIs = number;
    tcpserver.sendListPlayers(getListPlayers(true));
    gameProcess();
}


void Board::setPlayerReady(PlayerId playerId, int ready) {
    if (state.status != StatusWaitingReady) {
        tcpserver.sendDebug("already started...");
        return;
    }
    Player * player = getPlayer(playerId, 0);
    if (ready) {
        player->status = StatusReady;
    } else {
        player->status = StatusNotReady;
    }
    tcpserver.sendListPlayers(getListPlayers(true));
}


PlayerId Board::addPlayer(Player * player) {
    for ( int i=0; i<state.players.size(); ++i ) {
        if ( state.players[i] == player ) {
            return -1;
        }
    }
    state.players.push_back(player);
    state.nbPlayers = state.players.size();
    state.maxGenPlayerId++;
    return state.maxGenPlayerId;
}


void Board::removePlayer(Player * player) {
    for (int i=0; i<state.players.size(); ++i ) {
        if (state.players[i] == player) {
            state.players.erase(state.players.begin()+i);
        }
    }
    state.nbPlayers = state.players.size();
}


void Board::askStartGame() {
    tcpserver.sendDebug("ask game starts...");
    if (! areAllPlayers(StatusReady)) {
        tcpserver.sendDebug("not all players ready...");
        return;
    }
    if (state.nbPlayers + state.nbAIs < 3) {
        return;
    }
    tcpserver.sendDebug("game starts!");
    const QStringList & listAIs = getListAIs();
    for (const QString & ai : listAIs) {
        new AI(this, ai.toStdString().c_str());
    }

    initAllWonders();
    initAllCards();

    state.currentAge = 1;
    state.currentRound = -1;
    state.status = StatusGameStarted;
    for (Player * player : state.players) {
        player->status = StatusPlayed;
        player->view.coins = 3;
    }
    //gameLoop();
    tcpserver.startGame();

    gameProcess();
}


void Board::askAction(PlayerId playerId, const Action & action) {
    QString optMessage;
    if (! isValidAction(playerId, action, optMessage)) {
        tcpserver.confirmAction(playerId, false, optMessage);
        return;
    }
    Player * player = getPlayer(playerId);
    player->addPlayedAction(action);
    tcpserver.confirmAction(playerId, true);
}


size_t Board::getNbPlayers() const {
    return state.nbPlayers;
}


Player * Board::getPlayer(PlayerId playerId, size_t offset) const {
    int playerPos = getPlayerArrayId(playerId);
    return state.players[(playerPos + offset) % state.nbPlayers];
}


void Board::gameProcess() {
    if (state.status != StatusGameStarted) {
        return;
    }

    QStringList waitingPlayers;
    for (Player * player : state.players) {
        if (player->status != StatusPlayed) {
            waitingPlayers.append(player->view.name);
        }
    }
    //tcpserver.sendDebug(waitingPlayers.join(" ; "));


    if (isLastRound()) {
        for (Player * player : state.players) {
            if (player->canPlayBothCardsAtEndOfAge && player->status == StatusPlayed && player->actionsToPlay.size() == 1) {
                commitActionsPart1(player->view.id);
                player->showBoard();
                player->play({}, getCardsByAgeRoundPlayer(state.currentAge, state.currentRound, player->view.id));
            }
        }
    }


    if (areAllPlayersNot(StatusPlaying)) {
        tcpserver.sendDebug("all players played!");
        commitActionsParts1and2();

        for (Player * player : state.players) {
            if (player->canPlayCardFromDiscarded) {
                if (isLastRound()) {
                    discardRemainingCardsForAge(state.currentAge);
                }
                if (state.discardedCards.empty()) {
                    player->canPlayCardFromDiscarded = false;
                    return;
                }
                player->showBoard();
                tcpserver.sendMessageToPlayer(player->view.id, "Choose a discarded card to play for free");
                tcpserver.sendMessageNotToPlayer(player->view.id, "A player is choosing a discarded card");
                player->play({playDiscarded}, state.discardedCards);
                return;
            }

            if (player->canCopyNeirbyGuild && player->canCopyNeirbyGuildAlreadyUsed == false && isLastAge() && isLastRound()) {
                QVector<CardId> neirbyGuilds;
                for (Player * player2 : getApplyPlayers(player->view.id, PlayerBoth)) {
                    for (CardId cardId : player2->view.playedCards) {
                        if (AllCards::getCard(cardId).type == TypeGuild) {
                            neirbyGuilds.append(cardId);
                        }
                    }
                }
                player->canCopyNeirbyGuildAlreadyUsed = true;
                if (!neirbyGuilds.empty()) {
                    player->showBoard();
                    tcpserver.sendMessageToPlayer(player->view.id, "Choose a guild to copy from neirby guilds");
                    tcpserver.sendMessageNotToPlayer(player->view.id, "A player is choosing a neirby guild to copy");
                    player->play({copyGuild}, neirbyGuilds);
                    return;
                }
            }
        }

        if (isLastRound()) {
            resolveMilitaryConflicts();
            discardRemainingCardsForAge(state.currentAge);
            state.currentAge++;
            for (Player * player : state.players) {
                player->newAge();
            }
            state.currentRound = -1;
        }

        state.currentRound++;
        for (Player * player : state.players) {
            player->newRound();
        }
        tcpserver.showBoard(toBoardView());

        if (isGameOver()) {
            tcpserver.gameOver();
            state.status = StatusGameOver;
            return;
        }

        for (Player * player : state.players) {
            if (state.currentRound == 0) {
                player->newAge();
            }
            player->newRound();
            player->play({}, getCardsByAgeRoundPlayer(state.currentAge, state.currentRound, player->view.id));
        }
    }
}


bool Board::isValidAction(PlayerId playerId, const Action & action, QString & optMessage, QVector<Resource> * listMissing) {
    if (action.card == CardIdInvalid) {
        optMessage = "Unknown card";
        return false;
    }

    if (action.type == noAction) {
        return true;
    }

    if (action.type == copyGuild) {
        return isValidActionCopyGuild(playerId, action, optMessage);
    }

    if (action.type == playDiscarded ) {
        return isValidActionFromDiscarded(playerId, action, optMessage);
    }

    const QVector<CardId> & cardsToPlay = getCardsByAgeRoundPlayer(state.currentAge, state.currentRound, playerId);
    CardId cardId = action.card;
    if (! cardsToPlay.contains(cardId)) {
        optMessage = "Cheater!";
        for (CardId cid : cardsToPlay) {
            optMessage += " " + QString::number(cid);
        }
        return false;
    }
    if (action.type == discardCard) {
        return true;
    }

    if (action.type == playFreeCard ) {
        return isValidActionFreeCard(playerId, action, optMessage);
    }

    Player * player = getPlayer(playerId);
    int payLeft = action.boughtFromLeft.multiplyAndSum(player->priceToBuyLeft);
    int payRight = action.boughtFromRight.multiplyAndSum(player->priceToBuyRight);
    if (payLeft + payRight > player->view.coins) {
        optMessage = "Insufficient coins";
        return false;
    }

    if (! getLeftPlayer(playerId)->buyableResources.canProduce(action.boughtFromLeft)) {
        optMessage = "Left player has not enough resources ";
        optMessage += AllWonders::getWonder(getLeftPlayer(playerId)->view.wonderId).name;
        optMessage += "\ntarget=\n";
        optMessage += action.boughtFromLeft.display();
        optMessage += "\n";
        optMessage += getLeftPlayer(playerId)->buyableResources.debug();
        return false;
    }

    if (! getRightPlayer(playerId)->buyableResources.canProduce(action.boughtFromRight)) {
        optMessage = "Right player has not enough resources ";
        optMessage += AllWonders::getWonder(getRightPlayer(playerId)->view.wonderId).name;
        optMessage += "\ntarget=\n";
        optMessage += action.boughtFromRight.display();
        optMessage += "\n";
        optMessage += getRightPlayer(playerId)->buyableResources.debug();
        return false;
    }

    Resource costResources;
    if (action.type == playCard) {
        if (player->view.playedCards.contains(action.card)) {
            optMessage = "Card already played";
            return false;
        }
        const Card & refCard = AllCards::getCard(action.card);
        for ( CardId cardId : refCard.freePreviousIds ) {
            if (player->view.playedCards.contains(cardId)) {
                return true;
            }
        }
        costResources = AllCards::getCard(action.card).costResources;
    }
    if (action.type == buildWonder) {
        int wantedStage = player->view.wonderStages.size();
        const QVector<CardId> & wonderStages = AllWonders::getWonder(player->view.wonderId).getStages(player->view.wonderFace);
        if (wantedStage >= wonderStages.size()) {
            optMessage = "Wonder already fully built";
            return false;
        }
        costResources = AllCards::getCard(wonderStages[wantedStage]).costResources;
    }

    if (payLeft > 0) {
        costResources.substractResource(action.boughtFromLeft);
    }
    if (payRight > 0) {
        costResources.substractResource(action.boughtFromRight);
    }

    if (! player->production.canProduce(costResources)) {
        optMessage = "Insufficient resource production";
        optMessage += "\ntarget=\n";
        optMessage += costResources.display();
        optMessage += "\n";
        optMessage += player->production.debug();
        if (listMissing != nullptr) {
            *listMissing = player->production.listMissingRes(costResources);
        }
        return false;
    }

    return true;
}


bool Board::isNewAge() const {
    return state.currentRound == 0;
}


void Board::commitActionsParts1and2() {
    for (Player * player : state.players) {
        commitActionsPart1(player->view.id);
    }

    for (Player * player : state.players) {
        commitActionsPart2(player->view.id);
    }
}


void Board::commitActionsPart1(PlayerId playerId) {
    Player * player = getPlayer(playerId);
    for (int i=0; i<player->actionsToPlay.size(); ++i) {
        commitActionPart1(playerId, i);
    }
}


void Board::commitActionsPart2(PlayerId playerId) {
    Player * player = getPlayer(playerId);
    for (int i=0; i<player->actionsToPlay.size(); ++i) {
        commitActionPart2(playerId, i);
    }
}


void Board::commitActionPart1(PlayerId playerId, int actionId) {
    std::cout << "commitActionPart1 " << playerId << std::endl;
    Player * player = getPlayer(playerId);
    Player * leftPlayer = getLeftPlayer(playerId);
    Player * rightPlayer= getRightPlayer(playerId);
    Action & action = player->actionsToPlay[actionId];

    if (action.type == noAction || action.status != validated) {
        return;
    }
    action.status = step1commited;

    CardId cardId = action.card;

    if (action.type == playFreeCard) {
        player->canPlayCardForFreeAlreadyUsed = true;
    }

    if (action.type == playCard || action.type == buildWonder) {
        int payLeft = action.boughtFromLeft.multiplyAndSum(player->priceToBuyLeft);
        int payRight = action.boughtFromRight.multiplyAndSum(player->priceToBuyRight);
        player->view.coins -= (payLeft + payRight);
        leftPlayer->view.coins += payLeft;
        rightPlayer->view.coins += payRight;
    }

    if (action.type == playCard || action.type == playFreeCard || action.type == playDiscarded) {
        applyCardEffectsPart1(playerId, cardId);

        player->view.playedCards.append(cardId);

        if (action.type == playCard) {
            player->view.coins -= AllCards::getCard(cardId).costCoins;
        }
    }

    if (action.type == buildWonder) {
        int wantedStage = player->view.wonderStages.size();
        applyCardEffectsPart1(playerId, AllWonders::getWonder(player->view.wonderId).getStages(player->view.wonderFace)[wantedStage]);

        player->view.wonderStages.append(state.currentAge);
    }

    if (action.type == discardCard) {
        player->view.coins += 3;
        state.discardedCards.append(cardId);
    }

    if (action.type == copyGuild) {
        player->view.playedCards.append(cardId);
        return; // do not remove card from anywhere
    }

    if (action.type == playDiscarded) {
        player->canPlayCardFromDiscarded = false;
        state.discardedCards.removeOne(cardId);
        return; // do not remove card from remaining cards
    }

    removeCardForAgeRoundPlayer(cardId, state.currentAge, state.currentRound, playerId);
}


void Board::commitActionPart2(PlayerId playerId, int actionId) {
    std::cout << "commitActionPart2 " << playerId << std::endl;
    Player * player = getPlayer(playerId);
    Action & action = player->actionsToPlay[actionId];

    if (action.type == noAction || action.status != step1commited) {
        return;
    }
    action.status = step2commited;

    CardId cardId = action.card;

    if (action.type == playCard) {
        applyCardEffectsPart2(playerId, cardId);
    }

    if (action.type == buildWonder) {
        int doneStage = player->view.wonderStages.size() - 1;
        applyCardEffectsPart2(playerId, AllWonders::getWonder(player->view.wonderId).getStages(player->view.wonderFace)[doneStage]);
    }
}



void Board::applyCardEffectsPart1(PlayerId playerId, CardId cardId) {
    Player * player = getPlayer(playerId);

    const Card & card = AllCards::getCard(cardId);
    switch (card.type) {
    case TypeResourceRaw:
    case TypeResourceManufactured:
        player->production.addResource(card.incomeResources);
        player->buyableResources.addResource(card.incomeResources);
        break;
    default:
        if (card.target == PlayerNone) {
            player->production.addResource(card.incomeResources);
        }
        if (card.target & PlayerLeft) {
            player->priceToBuyLeft.replaceByNonZero(card.incomeResources);
        }
        if (card.target & PlayerRight) {
            player->priceToBuyRight.replaceByNonZero(card.incomeResources);
        }
        if (card.cardTypesCondition.empty()) {
            player->view.coins += card.incomeCoins;
        }
        player->shields += card.shields;
    }

    switch (card.special) {
    case SpecialNone:
        break;
    case BothCardsAtEndOfAge:
        player->canPlayBothCardsAtEndOfAge = true;
        break;
    case CopyNeirbyGuild:
        player->canCopyNeirbyGuild = true;
        player->canCopyNeirbyGuildAlreadyUsed = false;
        break;
    case OneFreeCardByAge:
        player->canPlayCardForFree = true;
        player->canPlayCardForFreeAlreadyUsed = false;
        break;
    case OneFreeCardFromDiscarded:
        player->canPlayCardFromDiscarded = true;
        break;
    }
}


void Board::applyCardEffectsPart2(PlayerId playerId, CardId cardId) {
    Player * player = getPlayer(playerId);

    const Card & card = AllCards::getCard(cardId);
    for ( CardType typeCondition : card.cardTypesCondition ) {
        QVector<Player *> listApplyPlayers = getApplyPlayers(playerId, card.target);
        for ( Player * applyPlayer : listApplyPlayers ) {
            if (typeCondition == TypeWonder) {
                player->view.coins += card.incomeCoins * player->view.wonderStages.size();
            } else {
                for ( CardId cardId : applyPlayer->view.playedCards ) {
                    if (AllCards::getCard(cardId).type == typeCondition) {
                        player->view.coins += card.incomeCoins;
                    }
                }
            }
        }
    }
}


void Board::resolveMilitaryConflicts() {
    for (Player * player : state.players) {
        const Player * lp = getLeftPlayer(player->view.id);
        if (player->shields < lp->shields) {
            player->view.militaryPoints.append(-1);
        } else if (player->shields > lp->shields) {
            player->view.militaryPoints.append(state.currentAge*2 - 1);
        } else {
            player->view.militaryPoints.append(0);
        }
        const Player * rp = getRightPlayer(player->view.id);
        if (player->shields < rp->shields) {
            player->view.militaryPoints.append(-1);
        } else if (player->shields > rp->shields) {
            player->view.militaryPoints.append(state.currentAge*2 - 1);
        } else {
            player->view.militaryPoints.append(0);
        }
    }
}


bool Board::isValidActionFromDiscarded(PlayerId playerId, const Action & action, QString & optMessage) {
    const Player * player = getPlayer(playerId);

    if (! player->canPlayCardFromDiscarded) {
        optMessage = "Cannot play discarded card";
        return false;
    }
    if (! state.discardedCards.contains(action.card)) {
        optMessage = "Cheater!";
        return false;
    }
    if (player->view.playedCards.contains(action.card)) {
        optMessage = "Card already played";
        return false;
    }
    return true;
}


bool Board::isValidActionFreeCard(PlayerId playerId, const Action & action, QString & optMessage) {
    const Player * player = getPlayer(playerId);

    if (! player->canPlayCardForFree) {
        optMessage = "Cannot play card for free";
        return false;
    }
    if (player->canPlayCardForFreeAlreadyUsed) {
        optMessage = "Free card already used";
        return false;
    }
    if (player->view.playedCards.contains(action.card)) {
        optMessage = "Card already played";
        return false;
    }
    return true;
}


bool Board::isValidActionCopyGuild(PlayerId playerId, const Action & action, QString & optMessage) {
    const Player * player = getPlayer(playerId);
    const Player * playerLeft = getLeftPlayer(playerId);
    const Player * playerRight = getRightPlayer(playerId);

    if (! player->canCopyNeirbyGuild) {
        optMessage = "Cannot copy guild";
        return false;
    }
    if (player->canCopyNeirbyGuildAlreadyUsed) {
        optMessage = "Guild already copied";
        return false;
    }
    if (player->view.playedCards.contains(action.card)) {
        optMessage = "Card already played";
        return false;
    }
    if (AllCards::getCard(action.card).type != TypeGuild) {
        optMessage = "Must copy a guild";
        return false;
    }
    if (! (playerLeft->view.playedCards.contains(action.card) || playerRight->view.playedCards.contains(action.card))) {
        optMessage = "Must copy a neirby guild";
        return false;
    }
    return true;
}


BoardView Board::toBoardView() {
    BoardView bv;

    for (Player * player : state.players) {
        bv.players.push_back(player->view);
    }
    for (CardId cardId : state.discardedCards) {
        bv.discardedCards.push_back(AllCards::getCard(cardId).age);
    }
    bv.currentRound = state.currentRound;
    bv.currentAge = state.currentAge;

    return bv;
}


Board::BoardState Board::saveBoardState() const {
    return state;
}


void Board::restoreFakeBoardState(const Board::BoardState & state_) {
    state = state_;
    for (Player * p : state_.players) {
        //std::cout << "old player: " << p->view.id << std::endl;
    }
    state.players.clear();
    for (const Player * player : state_.players) {
        Player * newPlayer = new FakePlayer(this, "");
        newPlayer->view = player->view;
        newPlayer->priceToBuyLeft = player->priceToBuyLeft;
        newPlayer->priceToBuyRight = player->priceToBuyRight;
        newPlayer->production = player->production;
        newPlayer->buyableResources = player->buyableResources;
        newPlayer->shields = player->shields;
        newPlayer->status = player->status;
        newPlayer->actionsToPlay = player->actionsToPlay;
        newPlayer->canPlayBothCardsAtEndOfAge = player->canPlayBothCardsAtEndOfAge;
        newPlayer->canCopyNeirbyGuild = player->canCopyNeirbyGuild;
        newPlayer->canCopyNeirbyGuildAlreadyUsed = player->canCopyNeirbyGuildAlreadyUsed;
        newPlayer->canPlayCardForFree = player->canPlayCardForFree;
        newPlayer->canPlayCardForFreeAlreadyUsed = player->canPlayCardForFreeAlreadyUsed;
        newPlayer->canPlayCardFromDiscarded = player->canPlayCardFromDiscarded;
    }
    for (Player * p : state.players) {
        //std::cout << "new player: " << p->view.id << std::endl;
    }
    std::cout << "restored board: " << this << std::endl;
}


void Board::playSingleAction(PlayerId playerId, Action & action) {
    Player * player = getPlayer(playerId);
    player->addPlayedAction(action);
    commitActionPart1(playerId, 0);
    commitActionPart2(playerId, 0);
}


void Board::initAllWonders() {
    QVector<WonderId> listWonders;
    for (const Wonder & wonder : AllWonders::allWonders) {
        std::cout << "wonder : " << wonder.id << wonder.name.toStdString() << std::endl;
        listWonders.append(wonder.id);
    }
    std::shuffle(std::begin(listWonders), std::end(listWonders), random);
    for ( int i=0; i<state.nbPlayers; ++i ) {
        state.players[i]->setWonder(listWonders[i]);
    }
}


void Board::initAllCards() {
    state.remainingCards.resize(nbAges);
    QVector<CardId> cardsByAge;

    for ( Age i=1; i<=nbAges; ++i) {
        cardsByAge = getRandomCardsByAge(i);
        state.remainingCards[i-1].resize(state.nbPlayers);

        for ( int j=0; j<state.nbPlayers; ++j) {
            QVector<CardId> newVec = cardsByAge.mid(j*nbRounds, nbRounds + 1);
            state.remainingCards[i-1][j] = newVec;
        }
    }
}


size_t Board::getPlayerArrayId(PlayerId playerId) const {
    for (int i=0; i<state.nbPlayers; ++i) {
        if (state.players[i]->view.id == playerId) {
            return i;
        }
    }
    std::cout << "ERROR player id" << std::endl;
    return 0;
}


Player * Board::getLeftPlayer(PlayerId playerId) const {
    size_t arrayId = getPlayerArrayId(playerId);
    size_t newId = (arrayId + 1) % state.nbPlayers;
    return state.players[newId];
}


Player * Board::getRightPlayer(PlayerId playerId) const {
    size_t arrayId = getPlayerArrayId(playerId);
    size_t newId = (arrayId + state.nbPlayers - 1) % state.nbPlayers;
    return state.players[newId];
}


QVector<Player *> Board::getApplyPlayers(PlayerId playerId, WhichPlayer which) const {
    QVector<Player *> res;
    if (which & PlayerSelf) {
        res.append(getPlayer(playerId));
    }
    if (which & PlayerLeft) {
        res.append(getLeftPlayer(playerId));
    }
    if (which & PlayerRight) {
        res.append(getRightPlayer(playerId));
    }
    return res;
}


QVector<CardId> Board::getRandomCardsByAge(Age age) {
    QVector<CardId> cardsByAge;
    for ( int i=0; i<AllCards::allCards.size(); ++i ) {
        const Card & refCard = AllCards::allCards[i];
        //std::cout << (int)refCard.age << " " << (int)refCard.nbMinPlayers << std::endl;
        for ( int j=0; j<refCard.nbMinPlayers.size(); ++j ) {
            if ( age == refCard.age && state.nbPlayers >= refCard.nbMinPlayers[j] && refCard.type != TypeGuild ) {
                cardsByAge.push_back(refCard.id);
                //std::cout << cardsByAge.size() << " " << (int)refCard.age << " " << (int)refCard.nbMinPlayers << std::endl;
            }
        }
    }

    if ( age == nbAges ) {
        // don't forget guild cards!
        const QVector<CardId> & guilds = getRandomGuilds();
        for ( int i=0; i<guilds.size(); ++i ) {
            cardsByAge.push_back(guilds[i]);
        }
    }

    if ( cardsByAge.size() != (nbRounds+1)*state.nbPlayers ) {
        std::cout << "WRONG CARD NUMBER: " << (int)age << " " << cardsByAge.size() << " " << state.nbPlayers << std::endl;
    }

    std::shuffle(std::begin(cardsByAge), std::end(cardsByAge), random);
    return cardsByAge;
}


QVector<CardId> Board::getRandomGuilds() {
    int nbGuilds = state.nbPlayers + 2;
    QVector<CardId> allGuilds;

    for ( int i=0; i<AllCards::allCards.size(); ++i ) {
        const Card & refCard = AllCards::allCards[i];
        if ( refCard.type == CardType::TypeGuild ) {
            allGuilds.push_back(refCard.id);
        }
    }

    std::shuffle(std::begin(allGuilds), std::end(allGuilds), random);

    allGuilds.resize(nbGuilds);
    return allGuilds;
}


QVector<CardId> & Board::getCardsByAgeRoundPlayer(Age a, int round, PlayerId p) {
    int arrayId = (getPlayerArrayId(p) + round) % state.nbPlayers;
    if (a == 2) {
        arrayId = (getPlayerArrayId(p) - round + state.nbPlayers) % state.nbPlayers;
    }
    std::cout << "getCardsByAgeRoundPlayer " << (int)a << " " << round << " " << p << "(" << arrayId << ")" << std::endl;
    return state.remainingCards[a-1][arrayId];
}


void Board::removeCardForAgeRoundPlayer(CardId cardId, Age a, int round, PlayerId p) {
    std::cout << "removeCardForAgeRoundPlayer" << std::endl;
    getCardsByAgeRoundPlayer(a, round, p).removeOne(cardId);
}


void Board::discardRemainingCardsForAge(Age a) {
    std::cout << "discardRemainingCardsForAge" << std::endl;
    for (const CardsByPlayer & cards : state.remainingCards[a-1]) {
        for (CardId cardId : cards) {
            state.discardedCards.append(cardId);
        }
    }
    for (CardsByPlayer & cards : state.remainingCards[a-1]) {
        cards.clear();
    }
}


QStringList Board::getListPlayers(bool withReady) {
    QStringList res;
    for (Player * player : state.players) {
        if (withReady) {
            res.append(QString::number(int(player->status == StatusReady)) + player->view.name);
        } else {
            res.append(player->view.name);
        }
    }
    res.append(getListAIs(withReady));
    return res;
}


QStringList Board::getListAIs(bool withReady) {
    size_t remainingPlaces = 7 - state.nbPlayers;
    size_t realNbAIs = state.nbAIs;
    if (realNbAIs > remainingPlaces) {
        realNbAIs = remainingPlaces;
    }
    QStringList res;
    for (size_t i=0; i<realNbAIs; ++i) {
        if (withReady) {
            res.append(QString("1[AI ") + QString::number(i+1) + "]");
        } else {
            res.append(QString("[AI ") + QString::number(i+1) + "]");
        }
    }
    return res;
}


void Board::setAllPlayers(StatusPlayer status) {
    for (Player * player : state.players) {
        player->status = status;
    }
}


bool Board::areAllPlayers(StatusPlayer status) {
    for (Player * player : state.players) {
        if (player->status != status) {
            return false;
        }
    }
    return true;
}


bool Board::areAllPlayersNot(StatusPlayer status) {
    for (Player * player : state.players) {
        if (player->status == status) {
            return false;
        }
    }
    return true;
}


bool Board::isLastRound() const {
    return state.currentRound == nbRounds - 1;
}


bool Board::isLastAge() const {
    return state.currentAge == nbAges;
}


bool Board::isGameOver() const {
    return state.currentAge > nbAges;
}
