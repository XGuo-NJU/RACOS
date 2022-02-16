#ifndef RACOS_H
#define RACOS_H

#include "Value.h"
#include <vector>

class AttrInfo{
public:
    AttrInfo(){numAttr = 0u;}
    ~AttrInfo(){}
    inline unsigned getNumAttributes() const{
        return numAttr;
    }
    inline const std::vector<VarValue>& getAttributesRef() const {
        return attrs;
    }
    inline ValueType getTypeAt(unsigned i) const{
        assert(i<numAttr);
        return attrs[i].getType();
    }
    inline Value getLowerBoundAt(unsigned i) const{
        assert(i<numAttr);
        return attrs[i].getMinimum();
    }
    inline Value getUpperBoundAt(unsigned i) const{
        assert(i<numAttr);
        return attrs[i].getMaximum();
    }
    inline std::pair<Value,Value> getBoundAt(unsigned i) const{
        assert(i<numAttr);
        return {attrs[i].getMinimum(),attrs[i].getMaximum()};
    }
    inline void addAttribute(VarValue attr){
        numAttr++;
        attrs.push_back(attr);
    }
private:
    unsigned numAttr;
    std::vector<VarValue> attrs;
};

class Instance{
    friend std::ostream& operator<<(std::ostream&,const Instance&);

public:
    Instance(){len=0;fn=-1;}
    ~Instance(){len=0;attrs.clear();fn = -1;}

    inline unsigned getLength() const{
        return len;
    }
    inline Value getValueAt(unsigned i)const{
        assert(i<len);
        return attrs[i];
    }
    inline void setValueAt(unsigned i,Value v){
        assert(i<len);
        assert(v.getType() == attrs[i].getType());
        attrs[i] = v;
    }
    inline void setFnValue(double v){
        fn = v;
    }
    inline float getFnValue() const{
        return fn;
    }
    bool operator==(const Instance& rhs) const{
        if(len != rhs.len)
            return false;
        for(unsigned i=0;i<len;i++)
            if(attrs[i]!=rhs.attrs[i])
                return false;
        return true;
    }
    static Instance genRandomInstance(const AttrInfo&);

private:
    unsigned len;
    std::vector<Value> attrs;
    double fn;
};

class ObjectFunction
{
protected:
    AttrInfo attrInfo;
public:
    ObjectFunction(){}
    virtual ~ObjectFunction(){}
    virtual double getValue(const Instance&) = 0;
    virtual Instance getBestNeighbor(const Instance&,const Instance&) = 0;
    virtual inline unsigned getComplexity() const =0;
    virtual inline double getPrior() const =0;
    inline AttrInfo getAttrInfo() const {return attrInfo;}
    inline const AttrInfo& getAttrInfoRef() const{return attrInfo;}
};

class RACOSOptimizer{
public:
    RACOSOptimizer(ObjectFunction&);
    float optimize();
	inline Instance getSolution() const {return solution;}
private:
    void initAttrBounds();
    void initParameters();
    void initSample();
    Instance genRandomInstance();
    bool isInstanceUnique(const Instance&);
    float queryObj(const Instance&);
    void resetWkBounds();
    void updateWkBounds(const Instance&);
    void resetIsAttrBest();
    std::vector<double> calProbDistr(bool, const std::vector<unsigned>&, unsigned,const std::vector<int>&);
    unsigned selectRandomly(const std::vector<double>&);
    std::vector<unsigned> getNegativesInRegion();
    void shrinkWkBounds(const Instance&,const Instance&,int);
private:
    ObjectFunction& obj;
	const AttrInfo& attrInfo;
	const unsigned numAttr;

    std::vector<std::pair<Value,Value>> stgBnd,wkBnd;
    std::vector<bool> isAttrBest;

    std::vector<Instance> sam;
	std::vector<Instance> nextSam;
	Instance solution;
	
	
    unsigned budget = 5000u;
    unsigned sampleSize;
	unsigned maxIterations;
	unsigned numPositives = 2u;
	double randProbability = 0.95;

	double probabilityScale = 2;
    unsigned numUncertainAttr = 1u;

	unsigned upperQuerySize;
	unsigned numQuery=0u;
	double shrinkScale = 1.0;
};

#endif