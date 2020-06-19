#ifndef WONDER_H
#define WONDER_H

#include <QVector>
#include <QString>


#include "card.hpp"


typedef int WonderId;
const CardId WonderIdInvalid = 0;
typedef int MilitaryPoints;

enum WonderFace {
    WonderFaceInvalid = 0,
    WonderFaceA = 1,
    WonderFaceB = 2
};


struct Wonder {
    Wonder();
    const QVector<CardId> & getStages(WonderFace wonderFace) const;
    const QString & getImageFace(WonderFace wonderFace) const;

    WonderId id;
    QString name;
    Resource baseIncome;
    QVector<CardId> stagesA;
    QVector<CardId> stagesB;
    bool inGame;
    QString image;
    QString imageA;
    QString imageB;
};

struct AllWonders {
    static const Wonder & getWonder(WonderId wonderId);

    static QVector<Wonder> allWonders;
    static void init();

private:
    static void addWonder(const char * name, Resource baseIncome, QVector<CardId> stagesA, QVector<CardId> stagesB, const char * image, const char * imageA, const char * imageB);
};

#endif // WONDER_H
