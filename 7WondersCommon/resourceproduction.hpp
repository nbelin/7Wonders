#ifndef RESOURCEPRODUCTION_H
#define RESOURCEPRODUCTION_H

#include <QVector>


#include "resource.hpp"


class ResourceProduction
{
public:
    ResourceProduction();

    void addResource(const Resource & resource);
    bool canProduce(const Resource & target);
    QString debug() const;
    double evaluateScore() const;

    Resource constant;
    QVector<Resource> flexibles;

private:
    bool canProduceRec(Resource & target, const QVector<Resource> & flexibles, int currentFlexible);
};

#endif // RESOURCEPRODUCTION_H
