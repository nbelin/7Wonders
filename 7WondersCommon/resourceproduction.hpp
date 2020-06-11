#ifndef RESOURCEPRODUCTION_H
#define RESOURCEPRODUCTION_H

#include <QVector>


#include "resource.hpp"


class ResourceProduction
{
public:
    ResourceProduction();

    void addResourceProduction(const ResourceProduction & other);
    void addResource(const Resource & resource);
    bool canProduce(const Resource & target) const;
    QVector<Resource> listMissingRes(const Resource & target) const;
    QString debug() const;
    double evaluateScore() const;

    Resource constant;
    QVector<Resource> flexibles;

private:
    bool canProduceRec(Resource & target, const QVector<Resource> & flexibles, int currentFlexible, QVector<Resource> * listMissing) const;
};

#endif // RESOURCEPRODUCTION_H
