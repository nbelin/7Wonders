#include <QApplication>
#include <QtTest>
#include <QString>
#include <QVector>


#include "card.hpp"
#include "playerview.hpp"
#include "boardview.hpp"


class Tests : public QObject
{
    Q_OBJECT

public:
    Tests();
    QApplication * qa;

private Q_SLOTS:
    void testCountPoints();
    void testPlayerView();
    void testBoardView();
};


Tests::Tests() {
    int argc = 0;
    char **argv = NULL;
    qa = new QApplication(argc, argv); // needed to create QWidgets

    AllCards::init();
    AllWonders::init();
}


void Tests::testCountPoints() {
    QVector<ScienceType> listSciences;
    listSciences.append(Science2);
    listSciences.append(Science1);
    listSciences.append(Science1);
    listSciences.append(Science3);
    listSciences.append(Science2);
    listSciences.append(ScienceAll);

    QVERIFY(utilCountSciencePoints(listSciences) == 26);
}


void Tests::testPlayerView() {
    PlayerView pv;
    QVERIFY2(true, "Failure");
}


void Tests::testBoardView() {
    PlayerView pv;
    pv.id = 1;
    pv.playedCards.append(AllCards::getCardId("Workshop")); // S2
    pv.playedCards.append(AllCards::getCardId("Dispensary")); // S1
    pv.playedCards.append(AllCards::getCardId("Lodge")); // S1
    pv.playedCards.append(AllCards::getCardId("University")); // S3
    pv.playedCards.append(AllCards::getCardId("Observatory")); // S2
    pv.playedCards.append(AllCards::getCardId("Scientists Guild")); // SALL

    pv.wonderId = 1;

    BoardView bv;
    bv.players.append(pv);

    QVERIFY(bv.countSciencePoints(1) == 26);
}


QTEST_APPLESS_MAIN(Tests)

#include "tst_tests.moc"
