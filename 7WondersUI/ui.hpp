#ifndef UI_H
#define UI_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QMouseEvent>
#include <QPushButton>
#include <QSpinBox>
#include <QProcess>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QStringList>
#include <QVector>
#include <QComboBox>
#include <QLabel>

#include "boardview.hpp"
#include "playerview.hpp"
#include "tcpclient.hpp"
#include <card.hpp>
#include <action.hpp>
#include "choice.hpp"

class UI : public QMainWindow {
    Q_OBJECT

public:
    explicit UI(QWidget * parent = nullptr);
    virtual ~UI();

    void prepareMenu();
    void prepareChoice();
    void prepareChoiceFace();
    void prepareGame();
    void startGame();

private:
    struct ShowedCard {
        ShowedCard() {}
        ShowedCard(QRect area, CardId card) : area(area), card(card) {}
        QRect area;
        CardId card;
    };

    struct ShowedPlayer {
        ShowedPlayer() {}
        ShowedPlayer(QRect area, PlayerId player) : area(area), player(player) {}
        QRect area;
        PlayerId player;
    };

    struct ResourceToBuy {
        int value;
        QPushButton * button;
        QPushButton * reset;
    };

    struct ChoicePlayer {
        QLineEdit * name;
        QLabel * wonder;
        QPushButton * up;
        QPushButton * down;
    };

    PlayerId playerId;
    PlayerId playerIdPointOfView;
    BoardView board;
    QVector<ActionType> possibleActionTypes;
    QVector<CardId> cardsToPlay;
    CardId focusedCard;
    QVector<ShowedCard> showedCards;
    QVector<ShowedPlayer> showedPlayers;
    Action action;
    TcpClient tcpclient;
    bool isGameOver;
    int backGroundAlpha;

    QWidget * menuView;
    QWidget * choiceView;
    QWidget * choiceFaceView;
    QWidget * gameView;
    QProcess * serverProcess;

    // menu view
    QTextEdit * helpText;
    QLineEdit * playerName;
    QLineEdit * remoteIpAddress;
    QPushButton * createGame;
    QPushButton * joinGame;

    // choice view
    QVector<ChoicePlayer> listPlayers;
    QSpinBox * numberAIs;
    QLineEdit * numberAIsText;
    QCheckBox * choiceReady;
    QComboBox * selectWonder;
    QCheckBox * randomWonders;
    QCheckBox * randomFaces;
    QCheckBox * randomPlaces;
    QPushButton * buttonAskStartGame;

    // choice face view
    QLineEdit * selectFaceText;
    QPushButton * selectFaceRandom;
    QPushButton * selectFaceA;
    QPushButton * selectFaceB;

    // game view
    QVector<ResourceToBuy> leftResourcesBought;
    QVector<ResourceToBuy> rightResourcesBought;
    QPushButton * buttonPlay;
    QPushButton * buttonWonder;
    QPushButton * buttonDiscard;
    QPushButton * buttonFreePlay;
    QPushButton * buttonPlayDiscarded;
    QPushButton * buttonCopyGuild;
    QVector<QPixmap> imagesCardAge;
    QVector<QPixmap> imagesCards;
    QTextEdit * messageText;

    void setBackground(int alpha);
    void showCard(CardId card, const QRect & area, int rotate, bool selected=false, bool lastPlayed=false);
    void showCentral();
    void showPoints();
    void showPlayerPoints(const QRect & area, PlayerId playerId, bool header=false);
    void showPlayers();
    void showPlayer(const PlayerView & player, size_t playerPos);
    void showPlayer(const PlayerView & player, const QRect & area, int rotate);
    void showResourceToBuy(const QRect & area, const ResourceToBuy & resource, int id);
    void getRectAndRotationFromPlayerPos(size_t playerPos, QRect & area, int & rotate);
    QRect getCentralRect();
    QRect takeMarginFromRect(const QRect & area, double percentMargin);
    QColor getColorFromCard(CardId card);
    QColor getTextColorFromCard(CardId card);
    void resetShowedCards();
    void resetShowedPlayers();
    CardId getFocusedCardFromPos(QPoint point);
    PlayerId getFocusedPlayerFromPos(QPoint point);
    QRect transformPainter(QPainter & painter, const QRect & area, int rotate);
    QRect rotatedScaledRect(const QRect & parentArea, int rotate, double percentX, double percentY, double percentWidth, double percentHeight);
    QPushButton * getButtonFromAction(ActionType type);

    void paintEvent(QPaintEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void keyPressEvent(QKeyEvent * event);

signals:

public slots:
    void userMessage(const QString & message);
    void setPlayerId(PlayerId playerId);
    void showChoice(const Choice & choice);
    void showChoiceFace(WonderId wonderId);
    void showBoard(const QString & boardString);
    void showCardsToPlay(int possibleActions, const QVector<CardId> & cards);
    void confirmAction(bool valid, const QString & optMessage);
    void gameOver();

    void buttonPlayPressed();
    void buttonWonderPressed();
    void buttonDiscardPressed();
    void buttonFreePlayPressed();
    void buttonPlayDiscardedPressed();
    void buttonCopyGuildPressed();
    void buttonLeftResourceIncrementPressed(int id);
    void buttonRightResourceIncrementPressed(int id);
    void buttonLeftResourceResetPressed(int id);
    void buttonRightResourceResetPressed(int id);
    void buttonCreateGamePressed();
    void buttonJoinGamePressed();
    void buttonAskStartGamePressed();
    void numberAIsChanged(int value);
    void choiceReadyChanged(int state);
    void selectWonderChanged(int index);
    void movePlayerUp(int index);
    void movePlayerDown(int index);
    void randomWondersChanged(int state);
    void randomFacesChanged(int state);
    void randomPlacesChanged(int state);
    void selectFaceRandomPressed();
    void selectFaceAPressed();
    void selectFaceBPressed();
};

#endif // UI_H
