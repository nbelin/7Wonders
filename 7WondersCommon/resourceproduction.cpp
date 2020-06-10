#include "resourceproduction.hpp"

ResourceProduction::ResourceProduction() {

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


bool ResourceProduction::canProduce(const Resource & target) {
    Resource simplifiedTarget(target);
    simplifiedTarget.substractResource(constant);

    return canProduceRec(simplifiedTarget, flexibles, 0);

    //// might need optimisations later on?

    // most simple case: no need to verify flexible resources
    if (simplifiedTarget.isZero()) {
        return true;
    }

    // quite simple case: only one resource missing, so check flexible resources for this one
    if (simplifiedTarget.hasOnlyOne()) {
        size_t oneIndex = simplifiedTarget.getOneIndex();
        unsigned char targetValue = simplifiedTarget.array[oneIndex];
        unsigned char flexValue = 0;
        for ( const Resource & flex : flexibles ) {
            flexValue += flex.array[oneIndex];
            if (flexValue >= targetValue) {
                return true;
            }
        }
    }

    // more complex: check with flexibles where only one resource is needed
    for ( const Resource & flex : flexibles ) {
    }
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


bool ResourceProduction::canProduceRec(Resource & target, const QVector<Resource> & flexibles, int currentFlexible) {
    if (target.isZero()) {
        return true;
    }
    if (currentFlexible >= flexibles.size()) {
        return false;
    }

    const QVector<size_t> & commonIndexes = target.commonIndexes(flexibles[currentFlexible]);

    if (commonIndexes.empty()) {
        return canProduceRec(target, flexibles, currentFlexible+1);
    }

    for ( size_t idx : commonIndexes ) {
        target.array[idx] -= 1; // assume flexible resources are only 1 by 1
        bool ret = canProduceRec(target, flexibles, currentFlexible+1);
        target.array[idx] += 1;
        if (ret) {
            return true;
        }
    }

    return false;
}
