#include "resourceproduction.hpp"


ResourceProduction::ResourceProduction() {

}


void ResourceProduction::addResourceProduction(const ResourceProduction & other) {
    constant.addResource(other.constant);
    for (const Resource & res : other.flexibles) {
        flexibles.append(res);
    }
}


void ResourceProduction::addResource(const Resource & resource) {
    if (resource.isZero()) {
        return;
    }
    if (resource.hasOnlyOne()) {
        constant.addResource(resource);
        return;
    }
    flexibles.append(resource);
}


bool ResourceProduction::canProduce(const Resource & target) const {
    Resource simplifiedTarget(target);
    simplifiedTarget.substractResource(constant);

    return canProduceRec(simplifiedTarget, flexibles, 0, nullptr);
}


QVector<Resource> ResourceProduction::listMissingRes(const Resource & target) const {
    Resource simplifiedTarget(target);
    simplifiedTarget.substractResource(constant);
    QVector<Resource> listMissing;

    (void) canProduceRec(simplifiedTarget, flexibles, 0, &listMissing);
    return listMissing;
}


QString ResourceProduction::debug() const {
    QString res;
    res += "constant=\n";
    res += constant.display();
    res += "\nflexibles=";
    for ( const Resource & resource : flexibles ) {
        res += "\n";
        res += resource.display();
    }
    return res;
}


double ResourceProduction::evaluateScore() const {
    double score = 0;
    for (int i=0; i<Resource::IdMAX; ++i) {
        int minProd = constant.array[i];
        for ( const Resource & resource : flexibles ) {
            minProd += resource.array[i];
        }

        if (i >= Resource::IdGlass) {
            // simple case, only 1 ResourceManufactured is enough!
            if (minProd > 0) {
                score += 7;
            }
        } else {
            if (minProd > 0) {
                score += 8;
            }
            if (minProd > 1) {
                score += 4;
            }
            if (minProd > 2) {
                score += 2;
            }
        }
    }
    return score;
}


bool ResourceProduction::canProduceRec(Resource & target, const QVector<Resource> & flexibles, int currentFlexible, QVector<Resource> * listMissing) const {
    if (target.isZero()) {
        return true;
    }
    if (currentFlexible >= flexibles.size()) {
        if (listMissing != nullptr && ! listMissing->contains(target)) {
            listMissing->append(target);
        }
        return false;
    }

    const QVector<size_t> & commonIndexes = target.commonIndexes(flexibles[currentFlexible]);

    if (commonIndexes.empty()) {
        return canProduceRec(target, flexibles, currentFlexible+1, listMissing);
    }

    for ( size_t idx : commonIndexes ) {
        target.array[idx] -= 1; // assume flexible resources are only 1 by 1
        bool ret = canProduceRec(target, flexibles, currentFlexible+1, listMissing);
        target.array[idx] += 1;
        if (listMissing == nullptr && ret) {
            return true;
        }
    }

    return false;
}
