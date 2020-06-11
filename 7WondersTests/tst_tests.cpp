#include <QApplication>
#include <QtTest>
#include <QString>
#include <QVector>
#include <QDebug>

#include "card.hpp"
#include "playerview.hpp"
#include "boardview.hpp"
#include "resourceproduction.hpp"


class Tests : public QObject
{
    Q_OBJECT

public:
    Tests();
    QApplication * qa;

private Q_SLOTS:
    void testProduction();
    void testCountPoints();
    void testPlayerView();
    void testBoardView();
};


Tests::Tests() {
    qDebug().setVerbosity(QDebug::MaximumVerbosity);

    int argc = 0;
    char **argv = NULL;
    qa = new QApplication(argc, argv); // needed to create QWidgets

    AllCards::init();
    AllWonders::init();
}


void Tests::testProduction() {
    ResourceProduction prod;
    QVERIFY(! prod.canProduce(Resource(Resource::ONE_ORE)));

    prod.addResource(Resource(Resource::ONE_ALL));
    QVERIFY(prod.canProduce(Resource(Resource::ONE_ORE)));
    QVERIFY(! prod.canProduce(Resource(Resource::ONE_ORE, 2)));
    QVERIFY(! prod.canProduce(Resource(Resource::ONE_ORE | Resource::ONE_GLASS)));

    prod.addResource(Resource(Resource::ONE_CLAY));
    QVERIFY(prod.canProduce(Resource(Resource::ONE_CLAY, 2)));
    QVERIFY(prod.canProduce(Resource(Resource::ONE_ORE | Resource::ONE_CLAY)));
    QVERIFY(! prod.canProduce(Resource(Resource::ONE_ORE, 2)));
    QVERIFY(! prod.canProduce(Resource(Resource::ONE_ORE | Resource::ONE_GLASS)));

    prod.addResource(Resource(Resource::ONE_ALL_MANUFACTURED));
    QVERIFY(prod.canProduce(Resource(Resource::ONE_CLAY, 2)));
    QVERIFY(prod.canProduce(Resource(Resource::ONE_GLASS, 2)));
    QVERIFY(prod.canProduce(Resource(Resource::ONE_ORE | Resource::ONE_CLAY)));
    QVERIFY(prod.canProduce(Resource(Resource::ONE_ORE | Resource::ONE_CLAY | Resource::ONE_GLASS)));
    QVERIFY(! prod.canProduce(Resource(Resource::ONE_ORE, 2)));
    QVERIFY(! prod.canProduce(Resource(Resource::ONE_WOOD, 2)));
    QVERIFY(! prod.canProduce(Resource(Resource::ONE_GLASS, 3)));

    QVector<Resource> listMissing;
    listMissing = prod.listMissingRes(Resource(Resource::ONE_WOOD));
    QVERIFY(listMissing.length() == 0);

    listMissing = prod.listMissingRes(Resource(Resource::ONE_WOOD, 2));
    QVERIFY(listMissing.length() == 1);
    QVERIFY(listMissing.contains(Resource(Resource::ONE_WOOD)));

    listMissing = prod.listMissingRes(Resource(Resource::ONE_WOOD | Resource::ONE_ORE));
    QVERIFY(listMissing.length() == 2);
    QVERIFY(listMissing.contains(Resource(Resource::ONE_ORE)));
    QVERIFY(listMissing.contains(Resource(Resource::ONE_WOOD)));

    listMissing = prod.listMissingRes(Resource(Resource::ONE_WOOD | Resource::ONE_ORE, 2));
    QVERIFY(listMissing.length() == 2);
    QVERIFY(listMissing.contains(Resource(0, 1, 0, 2, 0, 0, 0)));
    QVERIFY(listMissing.contains(Resource(0, 2, 0, 1, 0, 0, 0)));

    listMissing = prod.listMissingRes(Resource(Resource::ONE_WOOD | Resource::ONE_ORE | Resource::ONE_CLAY, 2));
    QVERIFY(listMissing.length() == 3);
    QVERIFY(listMissing.contains(Resource(0, 2, 0, 2, 0, 0, 0)));
    QVERIFY(listMissing.contains(Resource(1, 1, 0, 2, 0, 0, 0)));
    QVERIFY(listMissing.contains(Resource(1, 2, 0, 1, 0, 0, 0)));

    listMissing = prod.listMissingRes(Resource(Resource::ONE_WOOD | Resource::ONE_ORE | Resource::ONE_CLAY | Resource::ONE_GLASS | Resource::ONE_LOOM));
    QVERIFY(listMissing.length() == 5);
    QVERIFY(listMissing.contains(Resource(Resource::ONE_WOOD | Resource::ONE_LOOM)));
    QVERIFY(listMissing.contains(Resource(Resource::ONE_WOOD | Resource::ONE_GLASS)));
    QVERIFY(listMissing.contains(Resource(Resource::ONE_ORE | Resource::ONE_LOOM)));
    QVERIFY(listMissing.contains(Resource(Resource::ONE_ORE | Resource::ONE_GLASS)));
    QVERIFY(listMissing.contains(Resource(Resource::ONE_ORE | Resource::ONE_WOOD)));

    listMissing = prod.listMissingRes(Resource(Resource::ONE_ALL_MANUFACTURED));
    QVERIFY(listMissing.length() == 3);
    QVERIFY(listMissing.contains(Resource(Resource::ONE_LOOM)));
    QVERIFY(listMissing.contains(Resource(Resource::ONE_GLASS)));
    QVERIFY(listMissing.contains(Resource(Resource::ONE_PAPYRUS)));
}


void Tests::testCountPoints() {
    QVector<ScienceType> listSciences;
    QVERIFY(utilCountSciencePoints(listSciences) == 0);

    listSciences.clear();
    listSciences.append(Science1);
    QVERIFY(utilCountSciencePoints(listSciences) == 1);

    listSciences.clear();
    listSciences.append(ScienceAll);
    QVERIFY(utilCountSciencePoints(listSciences) == 1);

    listSciences.clear();
    listSciences.append(ScienceAll);
    listSciences.append(Science1);
    QVERIFY(utilCountSciencePoints(listSciences) == 4);

    listSciences.clear();
    listSciences.append(ScienceAll);
    listSciences.append(ScienceAll);
    QVERIFY(utilCountSciencePoints(listSciences) == 4);

    listSciences.clear();
    listSciences.append(Science2);
    listSciences.append(Science1);
    listSciences.append(Science1);
    listSciences.append(Science3);
    listSciences.append(ScienceAll);
    listSciences.append(Science2);
    QVERIFY(utilCountSciencePoints(listSciences) == 26);

    listSciences.clear();
    listSciences.append(Science1);
    listSciences.append(Science2);
    listSciences.append(Science3);
    listSciences.append(ScienceAll);
    listSciences.append(Science3);
    QVERIFY(utilCountSciencePoints(listSciences) == 18);
}


void Tests::testPlayerView() {
    PlayerView pv;
    QVERIFY2(true, "Failure");
}


void Tests::testBoardView() {
    PlayerView pv;
    pv.id = 1;
    pv.wonderId = 1;

    pv.playedCards.append(AllCards::getCardId("Workshop")); // S2
    pv.playedCards.append(AllCards::getCardId("Dispensary")); // S1
    pv.playedCards.append(AllCards::getCardId("Lodge")); // S1
    pv.playedCards.append(AllCards::getCardId("University")); // S3
    pv.playedCards.append(AllCards::getCardId("Scientists Guild")); // SALL
    pv.playedCards.append(AllCards::getCardId("Observatory")); // S2


    BoardView bv;
    bv.players.append(pv);
    QVERIFY(bv.countSciencePoints(1) == 26);


    pv.playedCards.clear();
    pv.playedCards.append(7);
    pv.playedCards.append(14);
    pv.playedCards.append(10);
    pv.playedCards.append(2);
    pv.playedCards.append(24);
    pv.playedCards.append(25);
    pv.playedCards.append(33);
    pv.playedCards.append(30);
    pv.playedCards.append(49);
    pv.playedCards.append(29);
    pv.playedCards.append(37);
    pv.playedCards.append(53);
    pv.playedCards.append(52);
    pv.playedCards.append(62);
    pv.playedCards.append(50);
    pv.playedCards.append(75);
    pv.playedCards.append(65);

    bv.players.clear();
    bv.players.append(pv);
    QVERIFY(bv.countSciencePoints(1) == 18);

}


QTEST_APPLESS_MAIN(Tests)

#include "tst_tests.moc"
