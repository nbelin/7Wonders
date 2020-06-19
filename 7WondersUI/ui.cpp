#include "ui.hpp"
#include "tools.hpp"

#include <iostream>

#include <QLabel>
#include <QString>
#include <QPainter>
#include <QPainterPath>
#include <QImageReader>
#include <QImage>
#include <QDir>
#include <QMessageBox>
#include <QThread>
#include <QDesktopServices>
#include <QTemporaryFile>


UI::UI(QWidget * parent) : QMainWindow(parent),
    playerId(0),
    focusedCard(CardIdInvalid),
    tcpclient(this),
    isGameOver(false),
    backGroundAlpha(0),
    serverProcess(nullptr) {

    if (tcpclient.connectServer("127.0.0.1")) {
        QMessageBox::warning(this, "Warning", QString("Local server already running"));
        tcpclient.disconnectServer();
    }

    std::cout << "UI created" << std::endl;

    AllCards::init();
    AllWonders::init();

    QObject::connect(&tcpclient, &TcpClient::userMessage, this, &UI::userMessage);
    QObject::connect(&tcpclient, &TcpClient::setPlayerId, this, &UI::setPlayerId);
    QObject::connect(&tcpclient, &TcpClient::showChoice, this, &UI::showChoice);
    QObject::connect(&tcpclient, &TcpClient::showChoiceFace, this, &UI::showChoiceFace);
    QObject::connect(&tcpclient, &TcpClient::startGame, this, &UI::startGame);
    QObject::connect(&tcpclient, &TcpClient::showBoard, this, &UI::showBoard);
    QObject::connect(&tcpclient, &TcpClient::showCardsToPlay, this, &UI::showCardsToPlay);
    QObject::connect(&tcpclient, &TcpClient::confirmAction, this, &UI::confirmAction);
    QObject::connect(&tcpclient, &TcpClient::gameOver, this, &UI::gameOver);

    this->resize(1300, 700);
    this->setMinimumSize(600, 400);

    menuView = new QWidget(this);
    choiceView = new QWidget(this);
    choiceFaceView = new QWidget(this);
    gameView = new QWidget(this);

    // menu view

    helpText = new QTextEdit(menuView);
    helpText->setFontPointSize(13);
    helpText->setPlainText("F1: quick rules (EN)\nF2: full rules (EN)\nF3: règles rapides (FR)\nF4: règles complètes (FR)");
    helpText->setGeometry(100, 50, 0, 0);
    QPalette palette;
    palette.setColor(QPalette::Base, QColor(0, 0, 0, 180));
    palette.setColor(QPalette::Text, Qt::white);
    helpText->setPalette(palette);
    helpText->setReadOnly(true);
    helpText->document()->adjustSize();
    helpText->setFixedSize(helpText->document()->size().toSize() + QSize(15, 5));
    helpText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    helpText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    playerName = new QLineEdit(menuView);
    playerName->setPlaceholderText("player name");
    playerName->setValidator(new QRegExpValidator (QRegExp("\\w{0,15}"), this));
    playerName->setText(qgetenv("USER"));
    if (playerName->text().length() == 0) {
        playerName->setText(qgetenv("USERNAME"));
    }
    playerName->setGeometry(QRect(500, 450, 150, 20));

    remoteIpAddress = new QLineEdit(menuView);
    remoteIpAddress->setPlaceholderText("remote IP address");
    remoteIpAddress->setGeometry(QRect(600, 500, 150, 20));

    createGame = new QPushButton("Create game", menuView);
    createGame->setGeometry(QRect(400, 530, 150, 30));
    QObject::connect(createGame, &QPushButton::released, this, &UI::buttonCreateGamePressed);

    joinGame = new QPushButton("Join game", menuView);
    joinGame->setGeometry(QRect(600, 530, 150, 30));
    QObject::connect(joinGame, &QPushButton::released, this, &UI::buttonJoinGamePressed);


    // choice view

    for ( size_t i=0; i<7; ++i ) {
        ChoicePlayer cp;

        cp.name = new QLineEdit(choiceView);
        cp.name->setReadOnly(true);
        cp.name->setGeometry(500, 500 + i*50, 150, 20);

        cp.wonder = new QLabel(choiceView);
        cp.wonder->setGeometry(660, 500 + i*50, 60, 40);

        cp.up = new QPushButton(choiceView);
        cp.up->setFlat(true);
        cp.up->setIcon(QIcon(Tools::imageTokenPath("up.png")));
        cp.up->setGeometry(740, 500 + i*50, 15, 15);
        QObject::connect(cp.up, &QPushButton::released, this, [this, i]{ movePlayerUp(i); });

        cp.down = new QPushButton(choiceView);
        cp.down->setFlat(true);
        cp.down->setIcon(QIcon(Tools::imageTokenPath("down.png")));
        cp.down->setGeometry(740, 520 + i*50, 15, 15);
        QObject::connect(cp.down, &QPushButton::released, this, [this, i]{ movePlayerDown(i); });

        listPlayers.push_back(cp);
    }

    numberAIs = new QSpinBox(choiceView);
    numberAIs->setRange(0, 7);
    numberAIs->setGeometry(400, 460, 35, 25);
    QObject::connect(numberAIs, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &UI::numberAIsChanged);

    numberAIsText = new QLineEdit(choiceView);
    numberAIsText->setText("number of AIs");
    numberAIsText->setGeometry(440, 460, 100, 20);
    numberAIsText->setReadOnly(true);
    numberAIsText->setFrame(false);
    palette.setColor(QPalette::Base, QColor(0, 0, 0, 0));
    palette.setColor(QPalette::Text, Qt::black);
    numberAIsText->setPalette(palette);

    choiceReady = new QCheckBox("ready", choiceView);
    choiceReady->setGeometry(500, 430, 60, 25);
    QObject::connect(choiceReady, &QCheckBox::stateChanged, this, &UI::choiceReadyChanged);

    selectWonder = new QComboBox(choiceView);
    selectWonder->setGeometry(300, 330, 250, 55);
    selectWonder->setIconSize(QSize(100, 50));
    selectWonder->setMaxVisibleItems(5);
    for (Wonder w : AllWonders::allWonders) {
        if (w.id == WonderIdInvalid) {
            selectWonder->addItem(QIcon(Tools::imageTokenPath("dice.png")), "random");
        } else {
            selectWonder->addItem(QIcon(w.image), w.name);
        }
    }
    QObject::connect(selectWonder, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &UI::selectWonderChanged);

    randomWonders = new QCheckBox("wonders: all random", choiceView);
    randomWonders->setGeometry(400, 330, 100, 25);
    QObject::connect(randomWonders, &QCheckBox::stateChanged, this, &UI::randomWondersChanged);

    randomFaces = new QCheckBox("faces: all random", choiceView);
    randomFaces->setGeometry(500, 330, 100, 25);
    QObject::connect(randomFaces, &QCheckBox::stateChanged, this, &UI::randomFacesChanged);

    randomPlaces = new QCheckBox("places: all random", choiceView);
    randomPlaces->setGeometry(600, 330, 100, 25);
    QObject::connect(randomPlaces, &QCheckBox::stateChanged, this, &UI::randomPlacesChanged);

    buttonAskStartGame = new QPushButton("Start game", choiceView);
    buttonAskStartGame->setGeometry(QRect(580, 450, 90, 30));
    QObject::connect(buttonAskStartGame, &QPushButton::released, this, &UI::buttonAskStartGamePressed);


    // choice face view

    selectFaceText = new QLineEdit(choiceFaceView);
    selectFaceText->setText("Please select a face");
    selectFaceText->setGeometry(440, 160, 150, 20);
    selectFaceText->setReadOnly(true);
    selectFaceText->setFrame(false);
    palette.setColor(QPalette::Base, QColor(0, 0, 0, 0));
    palette.setColor(QPalette::Text, Qt::black);
    selectFaceText->setPalette(palette);

    selectFaceRandom = new QPushButton(choiceFaceView);
    selectFaceRandom->setFlat(true);
    selectFaceRandom->setIcon(QIcon(Tools::imageTokenPath("dice.png")));
    selectFaceRandom->setGeometry(400, 400, 100, 100);
    selectFaceRandom->setIconSize(selectFaceRandom->size());
    QObject::connect(selectFaceRandom, &QPushButton::released, this, &UI::selectFaceRandomPressed);

    selectFaceA = new QPushButton(choiceFaceView);
    selectFaceA->setFlat(true);
    selectFaceA->setIcon(QIcon(Tools::imageTokenPath("dice.png")));
    selectFaceA->setGeometry(50, 400, 300, 140);
    selectFaceA->setIconSize(selectFaceA->size());
    QObject::connect(selectFaceA, &QPushButton::released, this, &UI::selectFaceAPressed);

    selectFaceB = new QPushButton(choiceFaceView);
    selectFaceB->setFlat(true);
    selectFaceB->setIcon(QIcon(Tools::imageTokenPath("dice.png")));
    selectFaceB->setGeometry(550, 400, 300, 140);
    selectFaceB->setIconSize(selectFaceB->size());
    QObject::connect(selectFaceB, &QPushButton::released, this, &UI::selectFaceBPressed);


    // game view

    buttonPlay = new QPushButton(gameView);
    buttonPlay->setFlat(true);
    buttonPlay->setIcon(QIcon(Tools::imageTokenPath("card.png")));
    QObject::connect(buttonPlay, &QPushButton::released, this, &UI::buttonPlayPressed);

    buttonWonder = new QPushButton(gameView);
    buttonWonder->setFlat(true);
    buttonWonder->setIcon(QIcon(Tools::imageTokenPath("pyramid-stage1.png")));
    QObject::connect(buttonWonder, &QPushButton::released, this, &UI::buttonWonderPressed);

    buttonDiscard = new QPushButton(gameView);
    buttonDiscard->setFlat(true);
    buttonDiscard->setIcon(QIcon(Tools::imageTokenPath("trash.png")));
    QObject::connect(buttonDiscard, &QPushButton::released, this, &UI::buttonDiscardPressed);

    buttonFreePlay = new QPushButton(gameView);
    buttonFreePlay->setFlat(true);
    buttonFreePlay->setIcon(QIcon(Tools::imageTokenPath("free.png")));
    QObject::connect(buttonFreePlay, &QPushButton::released, this, &UI::buttonFreePlayPressed);

    buttonPlayDiscarded = new QPushButton(gameView);
    buttonPlayDiscarded->setFlat(true);
    buttonPlayDiscarded->setIcon(QIcon(Tools::imageTokenPath("free.png")));
    QObject::connect(buttonPlayDiscarded, &QPushButton::released, this, &UI::buttonPlayDiscardedPressed);

    buttonCopyGuild = new QPushButton(gameView);
    buttonCopyGuild->setFlat(true);
    buttonCopyGuild->setIcon(QIcon(Tools::imageTokenPath("card.png")));
    QObject::connect(buttonCopyGuild, &QPushButton::released, this, &UI::buttonCopyGuildPressed);


    for ( size_t i=0; i<Resource::IdMAX; ++i ) {
        ResourceToBuy leftRes;
        leftRes.value = 0;

        leftRes.button = new QPushButton(gameView);
        leftRes.button->setFlat(true);
        leftRes.button->setIcon(QIcon(Tools::imageTokenPath("res" + QString::number(i) + ".png")));
        QObject::connect(leftRes.button, &QPushButton::released, this, [this, i]{ buttonLeftResourceIncrementPressed(i); });

        leftRes.reset = new QPushButton(gameView);
        leftRes.reset->setFlat(true);
        leftRes.reset->setIcon(QIcon(Tools::imageTokenPath("reset.png")));
        QObject::connect(leftRes.reset, &QPushButton::released, this, [this, i]{ buttonLeftResourceResetPressed(i); });

        leftResourcesBought.push_back(leftRes);


        ResourceToBuy rightRes;
        rightRes.value = 0;

        rightRes.button = new QPushButton(gameView);
        rightRes.button->setFlat(true);
        rightRes.button->setIcon(QIcon(Tools::imageTokenPath("res" + QString::number(i) + ".png")));
        QObject::connect(rightRes.button, &QPushButton::released, this, [this, i]{ buttonRightResourceIncrementPressed(i); });

        rightRes.reset = new QPushButton(gameView);
        rightRes.reset->setFlat(true);
        rightRes.reset->setIcon(QIcon(Tools::imageTokenPath("reset.png")));
        QObject::connect(rightRes.reset, &QPushButton::released, this, [this, i]{ buttonRightResourceResetPressed(i); });

        rightResourcesBought.push_back(rightRes);
    }


    imagesCardAge.push_back(new QPixmap(Tools::imageTokenPath("age1.png")));
    imagesCardAge.push_back(new QPixmap(Tools::imageTokenPath("age2.png")));
    imagesCardAge.push_back(new QPixmap(Tools::imageTokenPath("age3.png")));
}


UI::~UI() {
    std::cout << "UI destroyed" << std::endl;
    if (serverProcess) {
        delete serverProcess;
    }
}


void UI::prepareMenu() {
    menuView->show();
    choiceView->hide();
    choiceFaceView->hide();
    gameView->hide();
    setMouseTracking(false);
    backGroundAlpha = 30;
}


void UI::prepareChoice() {
    menuView->hide();
    choiceView->show();
    choiceFaceView->hide();
    gameView->hide();
    setMouseTracking(false);
    backGroundAlpha = 60;
}


void UI::prepareChoiceFace() {
    menuView->hide();
    choiceView->hide();
    choiceFaceView->show();
    gameView->hide();
    setMouseTracking(false);
    backGroundAlpha = 60;
}


void UI::prepareGame() {
    menuView->hide();
    choiceView->hide();
    choiceFaceView->hide();
    gameView->show();
    gameView->setMouseTracking(true);
    setMouseTracking(true);
    backGroundAlpha = 120;
}


void UI::startGame() {
    prepareGame();
}


void UI::setBackground(int alpha) {
    QPixmap bkgnd(Tools::imagePath("welcome.png"));
    bkgnd = bkgnd.scaled(this->size(), Qt::KeepAspectRatioByExpanding);
    QPixmap blend(bkgnd.size());
    blend.fill(QColor(255, 255, 255, alpha));
    QPainter painter(&bkgnd);
    painter.drawImage(bkgnd.rect(), blend.toImage());
    QPalette palette;
    palette.setBrush(QPalette::Window, bkgnd);
    this->setPalette(palette);
}


void UI::userMessage(const QString & message) {
    QMessageBox::information(this, "Info", message);
}


void UI::setPlayerId(PlayerId playerId) {
    this->playerId = playerId;
    playerIdPointOfView = playerId;
}


void UI::showChoice(const Choice & choice) {
    for (int i=0; i<listPlayers.size(); ++i) {
        if (i < choice.players.size()) {
            const Choice::PlayerChoice & pc = choice.players[i];

            listPlayers[i].name->setText(pc.name);
            QPalette palette;
            if (pc.ready) {
                palette.setColor(QPalette::Base, Qt::gray);
            } else {
                palette.setColor(QPalette::Base, Qt::white);
            }
            listPlayers[i].name->setPalette(palette);

            QPixmap pixmap;
            if (pc.wonderId == WonderIdInvalid) {
                pixmap.load(Tools::imageTokenPath("dice.png"));
            } else {
                const Wonder & wonder = AllWonders::getWonder(pc.wonderId);
                pixmap.load(wonder.image);
            }
            listPlayers[i].wonder->setPixmap(pixmap.scaled(listPlayers[i].wonder->size(), Qt::KeepAspectRatio));
        } else {
            listPlayers[i].name->setText("");
            QPalette palette;
            palette.setColor(QPalette::Base, Qt::white);
            listPlayers[i].name->setPalette(palette);

            listPlayers[i].wonder->clear();
        }
    }

    numberAIs->blockSignals(true);
    numberAIs->setValue(choice.numberAIs);
    numberAIs->blockSignals(false);

    randomWonders->blockSignals(true);
    randomWonders->setChecked(choice.randomWonders);
    randomWonders->blockSignals(false);

    randomFaces->blockSignals(true);
    randomFaces->setChecked(choice.randomFaces);
    randomFaces->blockSignals(false);

    randomPlaces->blockSignals(true);
    randomPlaces->setChecked(choice.randomPlaces);
    randomPlaces->blockSignals(false);
}


void UI::showChoiceFace(WonderId wonderId) {
    prepareChoiceFace();
    const Wonder & wonder = AllWonders::getWonder(wonderId);

    selectFaceA->setIcon(QIcon(wonder.imageA));
    selectFaceB->setIcon(QIcon(wonder.imageB));
}


void UI::showBoard(const QString & boardString) {
    std::cout << "showBoard called" << std::endl;
    board.fromString(boardString);
    for ( const PlayerView & player : board.players ) {
        std::cout << player.name.toStdString() << ": " << player.lastPlayedActions.size() << " actions" << std::endl;
        for ( const Action & action : player.lastPlayedActions ) {
            std::cout << action.toString().toStdString() << std::endl;
        }
    }
    update();
}


void UI::showCardsToPlay(int possibleActions, const QVector<CardId> & cards) {
    std::cout << "showCardsToPlay called " << cards.size() << std::endl;
    possibleActionTypes.clear();
    if (possibleActions & playCard) {
        possibleActionTypes.append(playCard);
    }
    if (possibleActions & buildWonder) {
        possibleActionTypes.append(buildWonder);
    }
    if (possibleActions & discardCard) {
        possibleActionTypes.append(discardCard);
    }
    if (possibleActions & playFreeCard) {
        possibleActionTypes.append(playFreeCard);
    }
    if (possibleActions & playDiscarded) {
        possibleActionTypes.append(playDiscarded);
    }
    if (possibleActions & copyGuild) {
        possibleActionTypes.append(copyGuild);
    }
    cardsToPlay = cards;
    update();
}


void UI::confirmAction(bool valid, const QString & optMessage) {
    if (valid) {
        cardsToPlay.clear();
        action.reset();
        for ( size_t i=0; i<Resource::IdMAX; ++i ) {
            leftResourcesBought[i].value = 0;
            rightResourcesBought[i].value = 0;
        }
    } else {
        userMessage(QString("Confirm action: ") + QString::number(valid) + " " + optMessage);
    }
    update();
}


void UI::gameOver() {
    isGameOver = true;
    update();
}


void UI::showCard(CardId card, const QRect & area, int rotate, bool selected, bool lastPlayed) {
    (void) rotate;
    if (card == CardIdInvalid) {
        return;
    }
    const Card & cardRef = AllCards::getCard(card);
    if (cardRef.type == TypeNone) {
        return;
    }

    QPainter painter(this);
    painter.drawRect(area);
    showedCards.push_back(ShowedCard(area, card));

    painter.fillRect(area, getColorFromCard(card));

    if ( selected == true || lastPlayed ) {
        QPainterPath path;
        path.addRect(area);
        QColor color = Qt::black;
        QPen pen(color, 3);
        painter.setPen(pen);
        painter.drawPath(path);
    }

    if ( lastPlayed == true ) {
        QPainterPath path;
        path.addRect(area);
        QColor color = Qt::white;
        QPen pen(color, 1);
        painter.setPen(pen);
        painter.drawPath(path);
    }

    if ( area.width() < 40 || area.height() < 40 ) {
        painter.drawText(area, Qt::AlignCenter, cardRef.getShortText());
        return;
    }

    if ( cardRef.name != nullptr ) {
        painter.setPen(getTextColorFromCard(card));
        painter.setFont(QFont("Arial", 10));
        painter.drawText(area, Qt::AlignCenter, cardRef.name);

        if ( area.height() < 30 ) {
            // if small area, only show the name
            return;
        }

        QString costText = QString::number(cardRef.costCoins) + " " + cardRef.costResources.display();
        for ( CardId cardId : cardRef.freePreviousIds ) {
            costText += "\n";
            costText += AllCards::getCard(cardId).name;
        }
        painter.drawText(area, Qt::AlignLeading, costText);
        painter.drawText(area, Qt::AlignBottom, cardRef.getMainText());
    }
}


void UI::showCentral() {
    double buyWidthCoef = 0.12;
    double menuHeightCoef = 0.15;

    QRect area = getCentralRect();
    QRect focusedCardArea(area.left(), area.top(), area.height() * 0.6, area.height());

    QRect buyLeftArea(focusedCardArea.right(), area.top(), area.width() * buyWidthCoef, area.height());
    QRect buyRightArea(area.right() - area.width() * buyWidthCoef, area.top(), area.width() * buyWidthCoef, area.height());
    QRect cardsToPlayArea(buyLeftArea.right(), area.top(), area.height() * 0.7, area.height());

    int buttonsWidth = buyRightArea.left() - buyLeftArea.right();
    int buttonsHeight = area.height() * menuHeightCoef;
    int nonButtonsHeight = area.height() - buttonsHeight;
    QRect buttonsArea(cardsToPlayArea.right(), area.bottom() - area.height() * menuHeightCoef, buttonsWidth, buttonsHeight);
    QRect discardedArea(cardsToPlayArea.right(), area.top(), buyLeftArea.left() - cardsToPlayArea.right(), nonButtonsHeight);

    buyLeftArea = takeMarginFromRect(buyLeftArea, 2.0);
    buyRightArea = takeMarginFromRect(buyRightArea, 2.0);
    buttonsArea = takeMarginFromRect(buttonsArea, 2.0);
    discardedArea = takeMarginFromRect(discardedArea, 2.0);
    focusedCardArea = takeMarginFromRect(focusedCardArea, 2.0);
    cardsToPlayArea = takeMarginFromRect(cardsToPlayArea, 2.0);


    for (int i=0; i<Resource::IdMAX; ++i) {
        showResourceToBuy(buyLeftArea, leftResourcesBought[i], i);
        showResourceToBuy(buyRightArea, rightResourcesBought[i], i);
    }


    QPainter painter(this);


    buttonPlay->hide();
    buttonWonder->hide();
    buttonDiscard->hide();
    buttonFreePlay->hide();
    buttonPlayDiscarded->hide();
    buttonCopyGuild->hide();
    int stepButtonsX = buttonsHeight * 1.5;
    int minButtonsX = buttonsArea.center().x() - stepButtonsX * 2 - buttonsHeight / 2;

    for (int i=0; i<possibleActionTypes.size(); ++i) {
        ActionType type = possibleActionTypes[i];
        QPushButton * button = nullptr;
        switch (type) {
        case noAction:
            break;
        case playCard:
            button = buttonPlay;
            break;
        case buildWonder:
            button = buttonWonder;
            break;
        case discardCard:
            button = buttonDiscard;
            break;
        case playFreeCard:
            button = buttonFreePlay;
            break;
        case playDiscarded:
            button = buttonPlayDiscarded;
            break;
        case copyGuild:
            button = buttonCopyGuild;
            break;
        }

        button->setGeometry(minButtonsX + i * stepButtonsX, buttonsArea.top(), buttonsHeight, buttonsHeight);
        button->setIconSize(button->size());
        button->show();
    }


    int discardedHeight = discardedArea.height() * 0.6;
    int discardedWidth = discardedHeight / 1.5;
    double discardedStep = 10;
    if (board.discardedCards.size() > 1) {
        discardedStep = qMin(discardedStep, double(discardedArea.height() - discardedHeight) / (board.discardedCards.size()-1));
    }
    for (int i=0; i<board.discardedCards.size(); ++i) {
        const QPixmap * imagePtr = imagesCardAge[board.discardedCards[i] -  1];
        int x = discardedArea.left() + discardedStep * i / 3;
        int y = discardedArea.top() + discardedStep * i;
        painter.drawPixmap(x, y, discardedWidth, discardedHeight, *imagePtr);
    }


    showCard(focusedCard, focusedCardArea, 0);


    int nbMaxCardsToPlay = 7;
    int cardsToPlayHeight = cardsToPlayArea.height() / (nbMaxCardsToPlay + 1);
    int cardsToPlayWidth = cardsToPlayHeight * 4;
    int cardsToPlayStep = (cardsToPlayArea.height() - cardsToPlayHeight * nbMaxCardsToPlay) / (nbMaxCardsToPlay - 1) + cardsToPlayHeight;
    for (int i=0; i<cardsToPlay.size(); ++i) {
        int iFake = i;
        if (iFake > 12) {
            iFake++;
        }
        if (iFake > 19) {
            iFake++;
        }
        if (iFake > 26) {
            iFake++;
        }
        if (iFake > 33) {
            iFake++;
        }
        int x = cardsToPlayArea.left() + cardsToPlayWidth * (iFake / nbMaxCardsToPlay) * 1.05;
        int y = cardsToPlayArea.top() + cardsToPlayStep * (iFake % nbMaxCardsToPlay);
        QRect showCardRect(x, y, cardsToPlayWidth, cardsToPlayHeight);
        showCard(cardsToPlay[i], showCardRect, 0, cardsToPlay[i] == action.card);
    }
}


void UI::showPoints() {
    for (int i=0; i<Resource::IdMAX; ++i) {
        leftResourcesBought[i].button->hide();
        leftResourcesBought[i].reset->hide();
        rightResourcesBought[i].button->hide();
        rightResourcesBought[i].reset->hide();
    }
    buttonPlay->hide();
    buttonWonder->hide();
    buttonDiscard->hide();
    buttonFreePlay->hide();

    QRect area = getCentralRect();
    QRect focusedCardArea(area.left(), area.top(), area.height() * 0.7, area.height());
    focusedCardArea = takeMarginFromRect(focusedCardArea, 2.0);
    showCard(focusedCard, focusedCardArea, 0);

    int x = focusedCardArea.right();
    double playerWidth = (area.width() - focusedCardArea.width())/(board.players.size() + 1);

    QRect headerArea(x, area.top(), playerWidth, area.height());
    showPlayerPoints(headerArea, PlayerIdInvalid, true);

    for (int i=0; i<board.players.size(); ++i) {
        int x = focusedCardArea.right() + playerWidth * (i+1);
        QRect playerArea(x, area.top(), playerWidth, area.height());
        showPlayerPoints(playerArea, board.players[i].id);
    }
}


void UI::showPlayerPoints(const QRect & area, PlayerId playerId, bool header) {
    QRect marginArea = takeMarginFromRect(area, 1.0);
    double partHeight = marginArea.height() / 10.0;
    int fontSize = partHeight * 0.8;
    int xLarge = marginArea.left();
    int x = xLarge + marginArea.width() / 4;

    QString name1;
    QString name2;
    QString milp;
    QString monp;
    QString wonp;
    QString civp;
    QString comp;
    QString guip;
    QString scip;
    QString totp;

    if (header) {
        x = xLarge;
        fontSize *= 0.8;
        name1 = "";
        name2 = "";
        milp = "Military";
        monp = "Coins";
        wonp = "Wonder";
        civp = "Civilisation";
        comp = "Commerce";
        guip = "Guild";
        scip = "Science";
        totp = "Total";
    } else {
        name1 = AllWonders::getWonder(board.getPlayer(playerId).wonderId).name;
        name2 = board.getPlayer(playerId).name;
        milp = QString::number(board.countMilitaryPoints(playerId));
        monp = QString::number(board.countMoneyPoints(playerId));
        wonp = QString::number(board.countWonderPoints(playerId));
        civp = QString::number(board.countCivilisationPoints(playerId));
        comp = QString::number(board.countCommercePoints(playerId));
        guip = QString::number(board.countGuildPoints(playerId));
        scip = QString::number(board.countSciencePoints(playerId));
        totp = QString::number(board.countPoints(playerId));
    }

    QPainter painter(this);

    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", fontSize * 0.5));
    painter.drawText(xLarge, marginArea.top() + 0 * partHeight, name1);
    painter.drawText(xLarge, marginArea.top() + 0.8 * partHeight, name2);

    painter.setFont(QFont("Arial", fontSize));
    painter.setPen(Qt::red);
    painter.drawText(x, marginArea.top() + 2 * partHeight, milp);

    painter.setPen(Qt::darkYellow);
    painter.drawText(x, marginArea.top() + 3 * partHeight, monp);

    painter.setPen(Qt::darkRed);
    painter.drawText(x, marginArea.top() + 4 * partHeight, wonp);

    painter.setPen(Qt::blue);
    painter.drawText(x, marginArea.top() + 5 * partHeight, civp);

    painter.setPen(Qt::yellow);
    painter.drawText(x, marginArea.top() + 6 * partHeight, comp);

    painter.setPen(Qt::darkMagenta);
    painter.drawText(x, marginArea.top() + 7 * partHeight, guip);

    painter.setPen(Qt::darkGreen);
    painter.drawText(x, marginArea.top() + 8 * partHeight, scip);

    painter.setPen(Qt::black);
    painter.drawText(x, marginArea.top() + 9 * partHeight, totp);
}


void UI::showPlayers() {
    for (int i=0; i<board.players.size(); ++i) {
        const PlayerView & pv = board.getPlayer(playerIdPointOfView, i);
        showPlayer(pv, i);
    }
}


void UI::showPlayer(const PlayerView & player, size_t playerPos) {
    QRect area;
    int rotate;
    getRectAndRotationFromPlayerPos(playerPos, area, rotate);

    showPlayer(player, area, rotate);
}


void UI::showPlayer(const PlayerView & player, const QRect & area, int rotate) {
    QPainter painter(this);
    painter.save();
    double margin = 0.0;
    switch (rotate) {
    case 0:
        margin = 0.2;
        break;
    case 90:
    case -90:
        margin = 0.4;
        break;
    case 180:
        margin = 0.8;
        break;
    default:
        break;
    }
    QRect marginArea = takeMarginFromRect(area, margin);
    QRect fakeArea = transformPainter(painter, marginArea, rotate);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    showedPlayers.push_back(ShowedPlayer(area, player.id));

    const Wonder & wonder = AllWonders::getWonder(player.wonderId);
    QPixmap image(wonder.getImageFace(player.wonderFace));
    if ( image.isNull() == true ) {
        std::cout << "cannot load " << wonder.image.toStdString() << std::endl;
    }
    painter.drawPixmap(fakeArea, image);

    if (player.id == playerId) {
        painter.restore(); // use unmodified painter to make sure all players get visible border
        QPainterPath path;
        path.addRect(marginArea);
        QPen pen(Qt::yellow, 5);
        painter.setPen(pen);
        painter.drawPath(path);
        painter.save();
        transformPainter(painter, marginArea, rotate);
    }

    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 20));
    painter.drawText(800, 42, 160, 40, Qt::AlignRight, player.name);

    int nbCoins3 = (player.coins - 1) / 3; // ensure at least 1 coin1
    int nbCoins1 = player.coins - 3*nbCoins3;
    QPixmap imageCoin3(Tools::imageTokenPath("coin3.png"));
    QPixmap imageCoin1(Tools::imageTokenPath("coin1.png"));
    for ( int i=0; i<nbCoins3; ++i) {
        int width = 50;
        int x = 5 + 40*(i%5);
        int y = 90 + 40*(i/5);
        painter.drawPixmap(x, y, width, width, imageCoin3);
    }
    for ( int i=0; i<nbCoins1; ++i) {
        int width = 45;
        int x = 5 + 40*((i+nbCoins3)%5);
        int y = 95 + 40*((i+nbCoins3)/5);
        painter.drawPixmap(x, y, width, width, imageCoin1);
    }

    QPixmap imageWin_1(Tools::imageTokenPath("win-1.png"));
    QPixmap imageWin1(Tools::imageTokenPath("win1.png"));
    QPixmap imageWin3(Tools::imageTokenPath("win3.png"));
    QPixmap imageWin5(Tools::imageTokenPath("win5.png"));
    for ( int i=0; i<player.militaryPoints.size(); ++i ) {
        const QPixmap * imagePtr = nullptr;
        int width;
        int height;
        switch (player.militaryPoints[i]) {
        case -1:
            imagePtr = &imageWin_1;
            width = 50;
            height = width;
            break;
        case 1:
            imagePtr = &imageWin1;
            width = 45;
            height = width * 1.15;
            break;
        case 3:
            imagePtr = &imageWin3;
            width = 50;
            height = width * 1.15;
            break;
        case 5:
            imagePtr = &imageWin5;
            width = 55;
            height = width * 1.15;
            break;
        default:
            break;
        }
        if (imagePtr) {
            int x = 10 + 60*(i/2);
            int y = 200 + 60*(i%2);
            painter.drawPixmap(x, y, width, height, *imagePtr);
        }
    }

    int nbLastActionsWondersBuilt = 0;
    for ( const Action & action : player.lastPlayedActions ) {
        if (action.type == buildWonder) {
            nbLastActionsWondersBuilt++;
        }
    }
    const QVector<CardId> & stages = wonder.getStages(player.wonderFace);
    for ( int i=0; i<player.wonderStages.size(); ++i ) {
        int x = 0;
        switch (stages.size()) {
        case 2:
            x = 75 + 300 * (i+1);
            break;
        case 3:
            x = 75 + 300 * i;
            break;
        case 4:
            x = -20 + 265 * i;
            break;
        }
        int y = 350;
        int width = 250;
        int height = 100;

        if (x < 5) {
            width += (x-5);
            x = 5;
        }
        if (x + width > 995) {
            width = 995 - x;
        }

        QPainterPath path;
        QRect rectArea = QRect(x, y, width, height);
        path.addRoundedRect(rectArea, 15, 15);
        Qt::GlobalColor color = Qt::black;
        switch (player.wonderStages[i]) {
        case 1:
            color = Qt::darkRed;
            break;
        case 2:
            color = Qt::blue;
            break;
        case 3:
            color = Qt::magenta;
            break;
        }
        if (i >= player.wonderStages.size() - nbLastActionsWondersBuilt) {
            color = Qt::white;
        }
        QPen pen(color, 8);
        painter.setPen(pen);
        painter.drawPath(path);
    }

    QVector<QVector<CardId>> cardsToShow;
    cardsToShow.resize(7);
    for ( CardId cardId : player.playedCards ) {
        const Card & cardRef = AllCards::getCard(cardId);
        switch (cardRef.type) {
        case TypeNone:
            break;
        case TypeResourceRaw:
            cardsToShow[0].append(cardId);
            break;
        case TypeResourceManufactured:
            cardsToShow[1].append(cardId);
            break;
        case TypeCommerce:
            cardsToShow[2].append(cardId);
            break;
        case TypeCivilisation:
            cardsToShow[3].append(cardId);
            break;
        case TypeMilitary:
            cardsToShow[4].append(cardId);
            break;
        case TypeScience:
            cardsToShow[5].append(cardId);
            break;
        case TypeGuild:
            cardsToShow[6].append(cardId);
            break;
        case TypeWonder:
        case TypeMilitaryDefeat:
            break;
        }
    }

    for ( int i=0; i<cardsToShow.size(); ++i ) {
        int len = cardsToShow[i].size();
        if (len == 0) {
            cardsToShow.removeAt(i);
            i--;
        }
        if (len > 6) {
            QVector<QVector<CardId>> newCardsToShow;
            for ( int j=0; j<i; ++j ) {
                newCardsToShow.append(cardsToShow[j]);
            }
            newCardsToShow.append(cardsToShow[i].mid(0, 6));
            newCardsToShow.append(cardsToShow[i].mid(6));
            for ( int j=i+1; j<cardsToShow.size(); ++j ) {
                newCardsToShow.append(cardsToShow[j]);
            }
            cardsToShow = newCardsToShow;
        }
    }

    for ( int i=0; i<cardsToShow.size(); ++i ) {
        for ( int j=0; j<cardsToShow[i].size(); ++j ) {
            CardId curCardId = cardsToShow[i][j];
            double percentX = 23.0 + 12.5 * i;
            double percentY = 3.0 + 13.0 * j;
            double percentWidth = 12.0;
            double percentHeight = 12.0;
            QRect cardArea = rotatedScaledRect(marginArea, rotate, percentX, percentY, percentWidth, percentHeight);
            bool lastPlayed = false;
            for ( const Action & action : player.lastPlayedActions ) {
                if (action.card == curCardId) {
                    lastPlayed = true;
                }
            }
            showCard(curCardId, cardArea, rotate, false, lastPlayed);
        }
    }

    painter.restore();
}


void UI::showResourceToBuy(const QRect & area, const ResourceToBuy & resource, int id) {
    int buyHeight = area.height() / (Resource::IdMAX + 1);
    int buyWidth = qMin(int(area.width() * 0.35), buyHeight);
    buyHeight = buyWidth;
    int buyStep = buyHeight + (area.height() - buyHeight * Resource::IdMAX) / (Resource::IdMAX - 1);
    int minY = area.top() + id * buyStep;

    int dispWidth = buyWidth * 0.5;
    int resetWidth = buyWidth * 0.9;

    resource.button->setGeometry(area.left(), minY, buyWidth, buyHeight);
    resource.button->setIconSize(resource.button->size());
    resource.reset->setGeometry(area.left() + area.width() * 0.6, minY, resetWidth, resetWidth);
    resource.reset->setIconSize(resource.reset->size());

    if (resource.value > 0) {
        QPainter painter(this);
        painter.setPen(Qt::black);
        painter.setFont(QFont("Arial", dispWidth));
        painter.drawText(area.left() + area.width() * 0.4, minY + buyHeight * 0.8, QString::number(resource.value));
    }
}


void UI::getRectAndRotationFromPlayerPos(size_t playerPos, QRect & area, int & rotate) {
    double areaRatio = 2.2;
    size_t nbPlayers = board.players.size();

    if ( playerPos == 0 ) {
        int height = this->height() * 0.45;
        int width = height * areaRatio;
        if (width > this->width() * 0.65) {
            width = this->width() * 0.65;
            height = width / areaRatio;
        }
        int x = this->width() / 2 - width / 2;
        int y = this->height() - height;
        area = QRect(x, y, width, height);
        rotate = 0;

    } else if ( playerPos == 1) {
        QRect tmpArea;
        int tmpRotate;
        getRectAndRotationFromPlayerPos(0, tmpArea, tmpRotate);
        int maxPossibleWidth = tmpArea.left();
        int maxPossibleHeight = this->height() * 0.8;
        if (nbPlayers <= 4) {
            maxPossibleHeight = this->height();
        }
        int width = qMin(maxPossibleWidth, int(maxPossibleHeight / areaRatio));
        int height = width * areaRatio;
        int x = 0;
        int y = this->height() - height;
        area = QRect(x, y, width, height);
        rotate = 90;

    } else if ( playerPos == nbPlayers-1 ) {
        QRect tmpArea;
        int tmpRotate;
        getRectAndRotationFromPlayerPos(0, tmpArea, tmpRotate);
        int maxPossibleWidth = this->width() - tmpArea.right();
        int maxPossibleHeight = this->height() * 0.8;
        if (nbPlayers <= 4) {
            maxPossibleHeight = this->height();
        }
        int width = qMin(maxPossibleWidth, int(maxPossibleHeight / areaRatio));
        int height = width * areaRatio;
        int x = this->width() - width;
        int y = this->height() - height;
        area = QRect(x, y, width, height);
        rotate = -90;

    } else {
        int maxPossibleY = this->height() * 0.25;
        if (nbPlayers > 4) {
            QRect tmpArea;
            int tmpRotate;
            getRectAndRotationFromPlayerPos(1, tmpArea, tmpRotate);
            maxPossibleY = tmpArea.top();
        }
        int nbTopPlayers = nbPlayers - 3;
        int posTopRightPlayer = nbPlayers - 2;
        int height = qMin(maxPossibleY, int((this->width() / nbTopPlayers) / areaRatio));
        int width = height * areaRatio;
        int freeSpace = this->width() - (((this->width() * (posTopRightPlayer - 2)) / nbTopPlayers) + width);
        int x = (this->width() * (playerPos - 2)) / nbTopPlayers + freeSpace / 2;
        int y = 0;
        area = QRect(x, y, width, height);
        rotate = 180;
    }
}


QRect UI::getCentralRect() {
    size_t centralMinX;
    size_t centralMinY;
    size_t centralMaxX;
    size_t centralMaxY;
    QRect area;
    int rotate;
    getRectAndRotationFromPlayerPos(0, area, rotate);
    centralMaxY = area.top();
    getRectAndRotationFromPlayerPos(1, area, rotate);
    centralMinX = area.right();
    if (board.players.size() == 3) {
        centralMinY = 0;
    } else {
        getRectAndRotationFromPlayerPos(2, area, rotate);
        centralMinY = area.bottom();
    }
    getRectAndRotationFromPlayerPos(board.players.size()-1, area, rotate);
    centralMaxX = area.left();

    int width = centralMaxX - centralMinX;
    int height = centralMaxY - centralMinY;
    if (height > width / 2.8) {
        height = width / 2.8;
        centralMinY = centralMaxY - height;
    }

    return takeMarginFromRect(QRect(centralMinX, centralMinY, width, height), 1);
}


QRect UI::takeMarginFromRect(const QRect & area, double percentMargin) {
    int centerX = area.center().x();
    int centerY = area.center().y();
    int width = area.width() * (100.0 - 2*percentMargin) / 100;
    int height = area.height() * (100.0 - 2*percentMargin) / 100;
    return QRect(centerX - width/2, centerY - height/2, width, height);
}


QColor UI::getColorFromCard(CardId card) {
    switch (AllCards::getCard(card).type) {
    case TypeNone:
    case TypeMilitaryDefeat:
        return Qt::black;
    case TypeResourceRaw:
        return Qt::darkRed;
    case TypeResourceManufactured:
        return Qt::gray;
    case TypeCommerce:
        return Qt::yellow;
    case TypeCivilisation:
        return Qt::blue;
    case TypeMilitary:
        return Qt::red;
    case TypeScience:
        return Qt::green;
    case TypeGuild:
        return Qt::magenta;
    case TypeWonder:
        return Qt::white;
    }
    return Qt::black;
}


QColor UI::getTextColorFromCard(CardId card) {
    switch (AllCards::getCard(card).type) {
    case TypeNone:
    case TypeMilitaryDefeat:
    case TypeResourceRaw:
    case TypeResourceManufactured:
    case TypeCommerce:
    case TypeWonder:
        return Qt::black;
    case TypeCivilisation:
    case TypeMilitary:
    case TypeScience:
    case TypeGuild:
        return Qt::white;
    }
    return Qt::black;
}


void UI::resetShowedCards() {
    showedCards.clear();
}


void UI::resetShowedPlayers() {
    showedPlayers.clear();
}


CardId UI::getFocusedCardFromPos(QPoint point) {
    for ( const ShowedCard & showedCard : showedCards ) {
        if ( showedCard.area.contains(point) ) {
            return showedCard.card;
        }
    }
    return CardIdInvalid;
}


PlayerId UI::getFocusedPlayerFromPos(QPoint point) {
    for ( const ShowedPlayer & showedPlayer : showedPlayers ) {
        if ( showedPlayer.area.contains(point) ) {
            return showedPlayer.player;
        }
    }
    return PlayerIdInvalid;
}


QRect UI::transformPainter(QPainter & painter, const QRect & area, int rotate) {
    switch (rotate) {
    case 0:
        painter.translate(area.topLeft());
        break;
    case 90:
        painter.translate(area.topRight());
        break;
    case -90:
        painter.translate(area.bottomLeft());
        break;
    case 180:
        painter.translate(area.bottomRight());
        break;
    }

    painter.rotate(rotate);
    double scale = qMax(area.width(), area.height()) / 1000.0;
    painter.scale(scale, scale);

    double ratioArea = double(qMax(area.width(), area.height())) / qMin(area.width(), area.height());
    return QRect (0, 0, 1000, 1000 / ratioArea);
}


QRect UI::rotatedScaledRect(const QRect & parentArea, int rotate, double percentX, double percentY, double percentWidth, double percentHeight) {
    int leftX = 0;
    int rightX = 0;
    int topY = 0;
    int bottomY = 0;
    switch (rotate) {
    case 0:
        leftX = parentArea.topLeft().x() + percentX * parentArea.width() / 100;
        rightX = leftX + percentWidth * parentArea.width() / 100;
        topY = parentArea.topLeft().y() + percentY * parentArea.height() / 100;
        bottomY = topY + percentHeight * parentArea.height() / 100;
        break;
    case 90:
        leftX = parentArea.topRight().x() - (percentY + percentHeight) * parentArea.width() / 100;
        rightX = leftX + percentHeight * parentArea.width() / 100;
        topY = parentArea.topRight().y() + percentX * parentArea.height() / 100;
        bottomY = topY + percentWidth * parentArea.height() / 100;
        break;
    case -90:
        leftX = parentArea.bottomLeft().x() + percentY * parentArea.width() / 100;
        rightX = leftX + percentHeight * parentArea.width() / 100;
        topY = parentArea.bottomLeft().y() - (percentX + percentWidth) * parentArea.height() / 100;
        bottomY = topY + percentWidth * parentArea.height() / 100;
        break;
    case 180:
        rightX = parentArea.bottomRight().x() - percentX * parentArea.width() / 100;
        leftX = rightX - percentWidth * parentArea.width() / 100;
        bottomY = parentArea.bottomRight().y() - percentY * parentArea.height() / 100;
        topY = bottomY - percentHeight * parentArea.height() / 100;
        break;
    default:
        break;
    }
    //std::cout << leftX << " " << topY << " " << rightX - leftX << " " << bottomY - topY << std::endl;
    return QRect(leftX, topY, rightX - leftX, bottomY - topY);
}


void UI::paintEvent(QPaintEvent * event) {
    (void) event;
    menuView->resize(this->size());
    choiceView->resize(this->size());
    choiceFaceView->resize(this->size());
    gameView->resize(this->size());
    setBackground(backGroundAlpha);

    resetShowedCards();
    resetShowedPlayers();

    showPlayers();

    if (isGameOver) {
        showPoints();
    } else {
        showCentral();
    }
}


void UI::mousePressEvent(QMouseEvent * event) {
    PlayerId player = getFocusedPlayerFromPos(event->pos());
    if ( player != PlayerIdInvalid && player != playerIdPointOfView ) {
        playerIdPointOfView = player;
        update();
        return;
    }

    CardId card = getFocusedCardFromPos(event->pos());
    if ( card == CardIdInvalid ) {
        return;
    }

    if ( card != action.card) {
        action.reset();
        action.card = card;
    }

    std::cout << "click: " << ((AllCards::getCard(card).name==nullptr)?"null":AllCards::getCard(card).name)
              << " " << AllCards::getCard(card).getMainText().toStdString() << std::endl;
    update();
}


void UI::mouseMoveEvent(QMouseEvent * event) {
    CardId card = getFocusedCardFromPos(event->pos());
    if ( card == CardIdInvalid ) {
        return;
    }
    if ( card == focusedCard ) {
        return;
    }
    focusedCard = card;
    update();
}


void UI::keyPressEvent(QKeyEvent * event) {
    QString pdf;
    switch (event->key()) {
    case Qt::Key_F1:
        pdf = "7WONDERS_QUICKRULES_US_COLOR.pdf";
        break;
    case Qt::Key_F2:
        pdf = "7WONDERS_RULES_US_COLOR.pdf";
        break;
    case Qt::Key_F3:
        pdf = "7WONDERS_QUICKRULES_FR_COLOR.pdf";
        break;
    case Qt::Key_F4:
        pdf = "7WONDERS_RULES_FR_COLOR.pdf";
        break;
    default:
        return;
    }

    QString path = Tools::imagePath(pdf);
    QString tmpPath = QDir::tempPath() + "/tmp.4vr9firoxa-" + pdf; //add random string in filename to reduce the risk of removing a user file in case of bug
    if (QFile::exists(tmpPath)) {
        QFile(tmpPath).setPermissions(QFile::ReadOther | QFile::WriteOther);
        QFile(tmpPath).remove();
    }
    QFile(path).copy(tmpPath);
    if (! QDesktopServices::openUrl(tmpPath)) {
        userMessage(QString("Error opening file: ") + tmpPath);
    }
}


void UI::buttonPlayPressed() {
    if (action.card == CardIdInvalid) {
        return;
    }
    for ( size_t i=0; i<Resource::IdMAX; ++i ) {
        action.boughtFromLeft.array[i] = leftResourcesBought[i].value;
        action.boughtFromRight.array[i] = rightResourcesBought[i].value;
    }
    std::cout << "play: ";
    action.type = playCard;
    action.display();
    tcpclient.askAction(action);
    update();
}


void UI::buttonWonderPressed() {
    if (action.card == CardIdInvalid) {
        return;
    }
    for ( size_t i=0; i<Resource::IdMAX; ++i ) {
        action.boughtFromLeft.array[i] = leftResourcesBought[i].value;
        action.boughtFromRight.array[i] = rightResourcesBought[i].value;
    }
    std::cout << "wonder: ";
    action.type = buildWonder;
    action.display();
    tcpclient.askAction(action);
    update();
}


void UI::buttonDiscardPressed() {
    if (action.card == CardIdInvalid) {
        return;
    }
    std::cout << "discard: ";
    action.type = discardCard;
    action.display();
    tcpclient.askAction(action);
    update();
}


void UI::buttonFreePlayPressed() {
    if (action.card == CardIdInvalid) {
        return;
    }
    std::cout << "free play: ";
    action.type = playFreeCard;
    action.display();
    tcpclient.askAction(action);
    update();
}


void UI::buttonPlayDiscardedPressed() {
    if (action.card == CardIdInvalid) {
        return;
    }
    std::cout << "play discarded: ";
    action.type = playDiscarded;
    action.display();
    tcpclient.askAction(action);
    update();
}


void UI::buttonCopyGuildPressed() {
    if (action.card == CardIdInvalid) {
        return;
    }
    std::cout << "copy guild: ";
    action.type = copyGuild;
    action.display();
    tcpclient.askAction(action);
    update();
}


void UI::buttonLeftResourceIncrementPressed(int id) {
    std::cout << "leftid: " << id << std::endl;
    std::cout << "leftvalue: " << leftResourcesBought[id].value << std::endl;
    if (leftResourcesBought[id].value < 9) {
        leftResourcesBought[id].value++;
    }
    update();
}


void UI::buttonRightResourceIncrementPressed(int id) {
    std::cout << "rightid: " << id << std::endl;
    std::cout << "rightvalue: " << rightResourcesBought[id].value << std::endl;
    if (rightResourcesBought[id].value < 9) {
        rightResourcesBought[id].value++;
    }
    update();
}


void UI::buttonLeftResourceResetPressed(int id) {
    leftResourcesBought[id].value = 0;
    update();
}


void UI::buttonRightResourceResetPressed(int id) {
    rightResourcesBought[id].value = 0;
    update();
}


void UI::buttonCreateGamePressed() {
    if (playerName->text().length() == 0) {
        QMessageBox::warning(this, "Warning", QString("Please enter a valid name"));
        return;
    }
    if (serverProcess) {
        delete serverProcess;
    }
#ifdef __linux__
    QString serverExePath = Tools::createPath({"7WondersServer", "7WondersServer"});
#else
    QString serverExePath = Tools::createPath({"..", "7WondersServer", "debug", "7WondersServer"});
#endif
    std::cout << "server path: " << serverExePath.toStdString() << std::endl;
    serverProcess = new QProcess(this);
    serverProcess->start(serverExePath);
    QThread::msleep(200);
    if (! tcpclient.connectServer("127.0.0.1")) {
        QMessageBox::critical(this, "Error", QString("Error connecting to local server\n" + tcpclient.lastError()));
        return;
    }
    tcpclient.setPlayerName(playerName->text().toStdString().c_str());
    prepareChoice();
}


void UI::buttonJoinGamePressed() {
    if (playerName->text().length() == 0) {
        QMessageBox::warning(this, "Warning", QString("Please enter a valid name"));
        return;
    }
    if (! tcpclient.connectServer(remoteIpAddress->text())) {
        QMessageBox::critical(this, "Error", QString("Error connecting to: " + remoteIpAddress->text() + "\n" + tcpclient.lastError()));
        return;
    }
    tcpclient.setPlayerName(playerName->text().toStdString().c_str());
    prepareChoice();
}


void UI::buttonAskStartGamePressed() {
    tcpclient.askStartGame();
}


void UI::numberAIsChanged(int value) {
    tcpclient.setNumberAIs(value);
}


void UI::choiceReadyChanged(int state) {
    tcpclient.setPlayerReady(state > 0);
}


void UI::selectWonderChanged(int index) {
    tcpclient.askWonder(index);
}


void UI::movePlayerUp(int index) {
    tcpclient.movePlayerUp(index);
}


void UI::movePlayerDown(int index) {
    tcpclient.movePlayerDown(index);
}


void UI::randomWondersChanged(int state) {
    tcpclient.setRandomWonders(state > 0);
}


void UI::randomFacesChanged(int state) {
    tcpclient.setRandomFaces(state > 0);
}


void UI::randomPlacesChanged(int state) {
    tcpclient.setRandomPlaces(state > 0);
}


void UI::selectFaceRandomPressed() {
    tcpclient.selectFace(WonderFaceInvalid);
}


void UI::selectFaceAPressed() {
    tcpclient.selectFace(WonderFaceA);
}


void UI::selectFaceBPressed() {
    tcpclient.selectFace(WonderFaceB);
}

