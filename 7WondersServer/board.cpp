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


Board::Board(QObject *parent) : QObject(parent),
    nbPlayers(0),
    nbAIs(0),
    maxGenPlayerId(0),
    currentRound(0),
    currentAge(0),
    status(StatusWaitingReady),
    tcpserver(this) {

    std::cout << "Board started" << std::endl;

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
        for (Player * player : players) {
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
    if (status != StatusWaitingReady) {
        tcpserver.sendDebug("already started...");
        return;
    }
    Player * player = new Human(this, name);
    tcpserver.setPlayerId(socket, player->view.id);
    tcpserver.sendListPlayers(getListPlayers(true));
}


void Board::setNumberAIs(int number) {
    if (status != StatusWaitingReady) {
        tcpserver.sendDebug("already started...");
        return;
    }
    nbAIs = number;
    tcpserver.sendListPlayers(getListPlayers(true));
    gameProcess();
}


void Board::setPlayerReady(PlayerId playerId, int ready) {
    if (status != StatusWaitingReady) {
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
    for ( int i=0; i<players.size(); ++i ) {
        if ( players[i] == player ) {
            return -1;
        }
    }
    players.push_back(player);
    nbPlayers = players.size();
    maxGenPlayerId++;
    return maxGenPlayerId;
}


void Board::removePlayer(Player * player) {
    for (int i=0; i<players.size(); ++i ) {
        if (players[i] == player) {
            players.erase(players.begin()+i);
        }
    }
    nbPlayers = players.size();
}


void Board::askStartGame() {
    tcpserver.sendDebug("ask game starts...");
    if (! areAllPlayers(StatusReady)) {
        tcpserver.sendDebug("not all players ready...");
        return;
    }
    if (nbPlayers + nbAIs < 3) {
        return;
    }
    tcpserver.sendDebug("game starts!");
    const QStringList & listAIs = getListAIs();
    for (const QString & ai : listAIs) {
        new AI(this, ai.toStdString().c_str());
    }

    initAllWonders();
    initAllCards();

    currentAge = 1;
    currentRound = -1;
    status = StatusGameStarted;
    for (Player * player : players) {
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
    return nbPlayers;
}


Player * Board::getPlayer(PlayerId playerId, size_t offset) const {
    int playerPos = getPlayerArrayId(playerId);
    return players[(playerPos + offset) % nbPlayers];
}


void Board::gameProcess() {
    if (status != StatusGameStarted) {
        return;
    }

    QStringList waitingPlayers;
    for (Player * player : players) {
        if (player->status != StatusPlayed) {
            waitingPlayers.append(player->view.name);
        }
    }
    //tcpserver.sendDebug(waitingPlayers.join(" ; "));


    if (isLastRound()) {
        for (Player * player : players) {
            if (player->canPlayBothCardsAtEndOfAge && player->status == StatusPlayed && player->actionsToPlay.size() == 1) {
                commitActionsPart1(player->view.id);
                player->showBoard();
                player->play({}, getCardsByAgeRoundPlayer(currentAge, currentRound, player->view.id));
            }
        }
    }


    if (areAllPlayersNot(StatusPlaying)) {
        tcpserver.sendDebug("all players played!");
        commitActionsParts1and2();

        for (Player * player : players) {
            if (player->canPlayCardFromDiscarded) {
                if (isLastRound()) {
                    discardRemainingCardsForAge(currentAge);
                }
                if (discardedCards.empty()) {
                    player->canPlayCardFromDiscarded = false;
                    return;
                }
                player->showBoard();
                tcpserver.sendMessageToPlayer(player->view.id, "Choose a discarded card to play for free");
                tcpserver.sendMessageNotToPlayer(player->view.id, "A player is choosing a discarded card");
                player->play({playDiscarded}, discardedCards);
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
            discardRemainingCardsForAge(currentAge);
            currentAge++;
            for (Player * player : players) {
                player->newAge();
            }
            currentRound = -1;
        }

        currentRound++;
        for (Player * player : players) {
            player->newRound();
        }
        tcpserver.showBoard(toBoardView());

        if (isGameOver()) {
            tcpserver.gameOver();
            status = StatusGameOver;
            return;
        }

        for (Player * player : players) {
            if (currentRound == 0) {
                player->newAge();
            }
            player->newRound();
            player->play({}, getCardsByAgeRoundPlayer(currentAge, currentRound, player->view.id));
        }
    }
}


bool Board::isValidAction(PlayerId playerId, const Action & action, QString & optMessage) {
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

    const QVector<CardId> & cardsToPlay = getCardsByAgeRoundPlayer(currentAge, currentRound, playerId);
    CardId cardId = action.card;
    if (! cardsToPlay.contains(cardId)) {
        optMessage = "Cheater!";
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
        return false;
    }

    return true;
}


bool Board::isNewAge() const {
    return currentRound == 0;
}


void Board::commitActionsParts1and2() {
    for (Player * player : players) {
        commitActionsPart1(player->view.id);
    }

    for (Player * player : players) {
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

    int payLeft = action.boughtFromLeft.multiplyAndSum(player->priceToBuyLeft);
    int payRight = action.boughtFromRight.multiplyAndSum(player->priceToBuyRight);
    if (action.type == playFreeCard) {
        player->canPlayCardForFreeAlreadyUsed = true;
    } else if (player->canPlayCardFromDiscarded) {
        player->canPlayCardFromDiscarded = false;
    } else {
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

        player->view.wonderStages.append(currentAge);
    }

    if (action.type == discardCard) {
        player->view.coins += 3;
        discardedCards.append(cardId);
    }

    if (action.type == copyGuild) {
        player->view.playedCards.append(cardId);
        return; // do not remove card from anywhere
    }

    if (action.type == playDiscarded) {
        discardedCards.removeOne(cardId);
        return; // do not remove card from remaining cards
    }

    removeCardForAgeRoundPlayer(cardId, currentAge, currentRound, playerId);
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
    for (Player * player : players) {
        const Player * lp = getLeftPlayer(player->view.id);
        if (player->shields < lp->shields) {
            player->view.militaryPoints.append(-1);
        } else if (player->shields > lp->shields) {
            player->view.militaryPoints.append(currentAge*2 - 1);
        } else {
            player->view.militaryPoints.append(0);
        }
        const Player * rp = getRightPlayer(player->view.id);
        if (player->shields < rp->shields) {
            player->view.militaryPoints.append(-1);
        } else if (player->shields > rp->shields) {
            player->view.militaryPoints.append(currentAge*2 - 1);
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
    if (! discardedCards.contains(action.card)) {
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

    for (Player * player : players) {
        bv.players.push_back(player->view);
    }
    for (CardId cardId : discardedCards) {
        bv.discardedCards.push_back(AllCards::getCard(cardId).age);
    }
    bv.currentRound = currentRound;
    bv.currentAge = currentAge;

    return bv;
}


void Board::initAllWonders() {
    QVector<WonderId> listWonders;
    for (const Wonder & wonder : AllWonders::allWonders) {
        listWonders.append(wonder.id);
    }
    std::shuffle(std::begin(listWonders), std::end(listWonders), random);
    for ( size_t i=0; i<nbPlayers; ++i ) {
        players[i]->setWonder(listWonders[i]);
    }
}


void Board::initAllCards() {
    remainingCards.resize(nbAges);
    QVector<CardId> cardsByAge;

    for ( Age i=1; i<=nbAges; ++i) {
        cardsByAge = getRandomCardsByAge(i);
        remainingCards[i-1].resize(nbPlayers);

        for ( size_t j=0; j<nbPlayers; ++j) {
            QVector<CardId> newVec = cardsByAge.mid(j*nbRounds, nbRounds + 1);
            remainingCards[i-1][j] = newVec;
        }
    }
}


size_t Board::getPlayerArrayId(PlayerId playerId) const {
    for (size_t i=0; i<nbPlayers; ++i) {
        if (players[i]->view.id == playerId) {
            return i;
        }
    }
    std::cout << "ERROR player id" << std::endl;
    return 0;
}


Player * Board::getLeftPlayer(PlayerId playerId) const {
    size_t arrayId = getPlayerArrayId(playerId);
    size_t newId = (arrayId + 1) % nbPlayers;
    return players[newId];
}


Player * Board::getRightPlayer(PlayerId playerId) const {
    size_t arrayId = getPlayerArrayId(playerId);
    size_t newId = (arrayId + nbPlayers - 1) % nbPlayers;
    return players[newId];
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
            if ( age == refCard.age && nbPlayers >= refCard.nbMinPlayers[j] && refCard.type != TypeGuild ) {
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

    if ( cardsByAge.size() != (nbRounds+1)*nbPlayers ) {
        std::cout << "WRONG CARD NUMBER: " << (int)age << " " << cardsByAge.size() << " " << nbPlayers << std::endl;
    }

    std::shuffle(std::begin(cardsByAge), std::end(cardsByAge), random);
    return cardsByAge;
}


QVector<CardId> Board::getRandomGuilds() {
    size_t nbGuilds = nbPlayers + 2;
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
    std::cout << "getCardsByAgeRoundPlayer " << round << " " << p << std::endl;
    int arrayId = (getPlayerArrayId(p) + round) % nbPlayers;
    if (a == 2) {
        arrayId = (getPlayerArrayId(p) - round + nbPlayers) % nbPlayers;
    }
    return remainingCards[a-1][arrayId];
}


void Board::removeCardForAgeRoundPlayer(CardId cardId, Age a, int round, PlayerId p) {
    std::cout << "removeCardForAgeRoundPlayer" << std::endl;
    getCardsByAgeRoundPlayer(a, round, p).removeOne(cardId);
}


void Board::discardRemainingCardsForAge(Age a) {
    std::cout << "discardRemainingCardsForAge" << std::endl;
    for (const CardsByPlayer & cards : remainingCards[a-1]) {
        for (CardId cardId : cards) {
            discardedCards.append(cardId);
        }
    }
    for (CardsByPlayer & cards : remainingCards[a-1]) {
        cards.clear();
    }
}


QStringList Board::getListPlayers(bool withReady) {
    QStringList res;
    for (Player * player : players) {
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
    size_t remainingPlaces = 7 - nbPlayers;
    size_t realNbAIs = nbAIs;
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
    for (Player * player : players) {
        player->status = status;
    }
}


bool Board::areAllPlayers(StatusPlayer status) {
    for (Player * player : players) {
        if (player->status != status) {
            return false;
        }
    }
    return true;
}


bool Board::areAllPlayersNot(StatusPlayer status) {
    for (Player * player : players) {
        if (player->status == status) {
            return false;
        }
    }
    return true;
}


bool Board::isLastRound() const {
    return currentRound == nbRounds - 1;
}


bool Board::isLastAge() const {
    return currentAge == nbAges;
}


bool Board::isGameOver() const {
    return currentAge > nbAges;
}
