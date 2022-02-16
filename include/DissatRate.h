#ifndef DISSATRATE_H
#define DISSATRATE_H

#include "Constraint.h"
#include "RACOS.h"

#include <vector>
#include <map>
#include <set>

class DissatRateCalculator : public ObjectFunction{
public:
    DissatRateCalculator(const std::vector<Constraint>&);
    double getValue(const Instance&);
    Instance getBestNeighbor(const Instance&,const Instance&);
    inline double getPrior() const{
        return 0.5;
    }
    inline unsigned getComplexity() const{
        return 2u;
    }
private:
    void initConstraints(const std::vector<Constraint>&);
    void initVaribales();
    void initVarBounds();
    void initAttrInfo();
    void initMaxRates();

    std::vector<double> compute(const Instance&);
    
private:
    std::vector<Constraint> constraints;
    std::map<std::string,VarValue> vars;
    std::map<std::string,size_t> varIDs;
    std::vector<double> maxRates;

    double warningPunishment = 1.0;
    double inequalityPunishment = 1.0;
    unsigned sampleSize = 200;

};


#endif