#include "RACOS.h"
#include "Utils.h"

using namespace std;

ostream& operator<<(ostream& out,const Instance& ins){
    if(ins.len==0u)
        out<<"Instance has no attributes."<<endl;
    else{
        auto it = ins.attrs.begin();
        auto end = ins.attrs.end();
        out<<"0:"<<*it;
        it++;
        unsigned cnt = 1;
        while(it!=end){
            out<<", "<<cnt<<":"<<*it;
            it++;
            cnt++;
        }
    }
    return out;
}

Instance Instance::genRandomInstance(const AttrInfo& info){
    Instance ins;
    ins.fn = -1.0;
    ins.len = info.getNumAttributes();
    for(auto attr : info.getAttributesRef())
        ins.attrs.push_back(attr.getRandomValue());
    return ins;
}

RACOSOptimizer::RACOSOptimizer(ObjectFunction& _obj)
:obj(_obj),attrInfo(_obj.getAttrInfoRef()),numAttr(_obj.getAttrInfoRef().getNumAttributes())
{
    
    for(unsigned i=0;i<numAttr;i++)
        isAttrBest.push_back(false);
    initAttrBounds();
    initParameters();
}

static bool insCmp(const Instance& a, const Instance& b){
    return a.getFnValue() < b.getFnValue();
}

float RACOSOptimizer::optimize(){
    
    sam.clear();
    nextSam.clear();
    
    vector<int> weights;
    for(unsigned i=0;i<numAttr;i++)
        weights.push_back(1);


    initSample();
    sort(sam.begin(),sam.end(),insCmp);
    solution = sam[0];
    for(unsigned i=0;i<maxIterations;i++){
        if(solution.getFnValue()==0.0)
            return solution.getFnValue();

        nextSam.clear();
        for(unsigned j = 0;j<sampleSize;j++){
            while (true) {
				unsigned posChosen = genRandomInt(0,numPositives-1);
				updateWkBounds(sam[posChosen]);
				resetIsAttrBest();
                vector<int> balanceWeights(weights.begin(),weights.end());
					
                vector<unsigned> attrMask;
				for(unsigned k=0; k<numAttr; k++)
					attrMask.push_back(k);

				vector<unsigned> negInRegion;
				for(unsigned ri=numPositives; ri<sampleSize; ri++)
					negInRegion.push_back(ri);
				
                while(true){
					auto distr = calProbDistr(true, attrMask, numAttr, balanceWeights);
					unsigned attrChosen = attrMask[selectRandomly(distr)];
                    auto negChosen = negInRegion[genRandomInt(0,negInRegion.size()-1)];

					shrinkWkBounds(sam[posChosen],sam[negChosen],attrChosen);
					negInRegion = getNegativesInRegion();
					if(negInRegion.empty())
						break;
					else{
						for(unsigned bw=0; bw<numAttr; bw++){
                            if(bw==attrChosen && balanceWeights[bw]>=2){
                                balanceWeights[bw]--;
                                break;
                            }				
                        }			
					}
				}
                unsigned numAttrBest = 0;
				unsigned sizeAttrMask = 0;
				
                for(unsigned lu=0; lu<numAttr; lu++){
					if(isAttrBest[lu])
						numAttrBest++;
					else
						attrMask[sizeAttrMask++] = lu;
				}
					
				auto distr = calProbDistr(true, attrMask, sizeAttrMask,weights);
				while(numAttr - numAttrBest > numUncertainAttr){						
					auto attrChosen = selectRandomly(distr);
		            isAttrBest[attrMask[attrChosen]] = true;
					attrMask[attrChosen] = attrMask[sizeAttrMask-1];
					sizeAttrMask--;
					numAttrBest++;
				}
					
                double lambda;
				if(sizeAttrMask==0)
					lambda = 1;
				else
					lambda = genRandomDouble(0,1);

				if (lambda >= randProbability) {
					resetIsAttrBest();
					resetWkBounds();
                }
				auto ins = genRandomInstance();
				
                ins.setFnValue(queryObj(ins));

				if(numQuery>upperQuerySize){
                    if(ins.getFnValue()<solution.getFnValue())
                        solution = ins;
                    return solution.getFnValue();
                }
                
				ins = obj.getBestNeighbor(ins,solution);
                numQuery++;

				if(numQuery>upperQuerySize){
                    if(ins.getFnValue()<solution.getFnValue())
                        solution = ins;
                    return solution.getFnValue();
                }

				if(ins.getFnValue()>solution.getFnValue()){
					for(unsigned k=0; k<numAttr; k++)
						if(!isAttrBest[k])
							weights[k]++;
				}
					
				if (isInstanceUnique(ins)){
                    nextSam.push_back(ins);
                    break;
                }						
			}				
		}
        for (unsigned i = 0; i < sampleSize; i++)
		    sam[i] = nextSam[i];
        
        sort(sam.begin(),sam.end(),insCmp);
        solution = sam[0];
        cout<<solution.getFnValue()<<endl;

    }
    return solution.getFnValue();
}

void RACOSOptimizer::initAttrBounds(){
    stgBnd.clear();
    wkBnd.clear();
    for(unsigned i=0;i<numAttr;i++){
        stgBnd.push_back(attrInfo.getBoundAt(i));
        wkBnd.push_back(attrInfo.getBoundAt(i));
    }
}

void RACOSOptimizer::initParameters(){
    sampleSize = (int)floor(sqrt(budget));
    maxIterations = (int)(ceil(budget/sampleSize)/2);
    upperQuerySize = 50*sampleSize*maxIterations;
}

void RACOSOptimizer::initSample(){
    sam.clear();
    for(unsigned i=0;i<sampleSize;){
        Instance ins = genRandomInstance();
        if(isInstanceUnique(ins)){
            ins.setFnValue(queryObj(ins));
            sam.push_back(ins);
            i++;
        }
    }
}

Instance RACOSOptimizer::genRandomInstance(){
	Instance ins = Instance::genRandomInstance(attrInfo);
	for(unsigned i=0; i<numAttr; i++){
        auto ty = attrInfo.getTypeAt(i);
		if(!isAttrBest[i]){
            VarValue VV(wkBnd[i].first,wkBnd[i].second);
            assert(ty == VV.getType());
            switch (ty)
            {
            case ValueType::INT:
            case ValueType::UINT:
            case ValueType::LONG:
                ins.setValueAt(i,VV.getRandomValue());
                break;
            
            case ValueType::FLT:
            case ValueType::DBL:
                if(wkBnd[i].first== stgBnd[i].first 
                    || wkBnd[i].second == stgBnd[i].second){
					double sp = genRandomDouble(0.0,1.0);
				    if(sp<=0.01){
		            	if(wkBnd[i].first==stgBnd[i].first)
		    	    		ins.setValueAt(i,wkBnd[i].first);
		    	    	else
	        	    		ins.setValueAt(i,wkBnd[i].second);
				    }
                    else
                        ins.setValueAt(i,VV.getRandomValue());
				}
                else
					ins.setValueAt(i,VV.getRandomValue());

                break;

            default:
                assert(0);
                break;
            }
		}
        else
            ins.setValueAt(i,wkBnd[i].first);
    }
	return ins;
}

bool RACOSOptimizer::isInstanceUnique(const Instance& ins){
    for(auto ano : sam)
        if(ano == ins)
            return false;
    for(auto ano : nextSam)
        if(ano == ins)
            return false;
    return true;
}

float RACOSOptimizer::queryObj(const Instance& ins){
    numQuery++;
    return (float)obj.getValue(ins);
}

void RACOSOptimizer::resetWkBounds(){
    wkBnd.clear();
    if(stgBnd.empty()){
        for(unsigned i=0;i<numAttr;i++)
            wkBnd.push_back(attrInfo.getBoundAt(i));
    }
    else
        wkBnd = stgBnd;
}

void RACOSOptimizer::updateWkBounds(const Instance& center){
	
	for(unsigned i=0; i<numAttr; i++){
        Value lower(1),upper(1);
		auto width = wkBnd[i].second - wkBnd[i].first;
        auto val = center.getValueAt(i);
        auto ty = attrInfo.getTypeAt(i);
        switch (ty)
        {
        case ValueType::INT:
            lower = Value((int)ceil(1.0*val.convertToInt()-shrinkScale*width.convertToInt()/2.0));
            upper = Value((int)floor(1.0*val.convertToInt()+shrinkScale*width.convertToInt()/2.0));
            break;
        case ValueType::UINT:
            lower = Value((unsigned)ceil(1.0*val.convertToUnsignedInt()-shrinkScale*width.convertToUnsignedInt()/2.0));
            upper = Value((unsigned)floor(1.0*val.convertToUnsignedInt()+shrinkScale*width.convertToUnsignedInt()/2.0));
            break;
        case ValueType::LONG:
            lower = Value((long)ceil(1.0*val.convertToLongInt()-shrinkScale*width.convertToLongInt()/2.0));
            upper = Value((long)floor(1.0*val.convertToLongInt()+shrinkScale*width.convertToLongInt()/2.0));
            break;

        case ValueType::FLT:
            lower = Value((float)(1.0*val.convertToFloat()-shrinkScale*width.convertToFloat()/2.0));
            upper = Value((float)(1.0*val.convertToFloat()+shrinkScale*width.convertToFloat()/2.0));
            break;
        
        case ValueType::DBL:
            lower = Value(1.0*val.convertToDouble()-shrinkScale*width.convertToDouble()/2.0);
            upper = Value(1.0*val.convertToDouble()+shrinkScale*width.convertToDouble()/2.0);
            break;

        default:
            assert(0);
            break;
        }
		if(lower<=stgBnd[i].first)
			wkBnd[i].first = stgBnd[i].first;
        else if(lower < wkBnd[i].first)
            wkBnd[i].first = lower;        

		if(upper >= stgBnd[i].second)
			wkBnd[i].second = stgBnd[i].second;
        else if(upper > wkBnd[i].second)
            wkBnd[i].second = upper;
	}
	return ;
}

void RACOSOptimizer::resetIsAttrBest(){
	for(unsigned i=0; i<numAttr; i++)
		isAttrBest[i] = false;
}

vector<double> RACOSOptimizer::calProbDistr(bool type, const vector<unsigned>& mask, unsigned num,const vector<int>& weight){
	vector<double> prob;
	for(unsigned i=0; i<num; i++){
		if(type)
			prob.push_back(weight[mask[i]]);
		else
			prob.push_back(1.0/weight[mask[i]]);
	}

	double sum = 0;
	for(unsigned i=0; i<num; i++)
	    sum += prob[i];

	for(unsigned i=0; i<num; i++)
		prob[i]/=sum;

	sum = 0;
	for(unsigned i=0; i<num; i++){
		prob[i] = pow(prob[i], probabilityScale);
		sum += prob[i];
	}
	
    for(unsigned i=0; i<num; i++)
		prob[i] /= sum;

	return prob;
}

unsigned RACOSOptimizer::selectRandomly(const vector<double>& distr){
        double prob = genRandomDouble(0.0,1.0);
		double sum = 0.0;
		for(unsigned i=0; i<distr.size(); i++){
			if(prob <= sum + distr[i])
				return i;				
			sum += distr[i];
		}
		return distr.size()-1;
}

vector<unsigned> RACOSOptimizer::getNegativesInRegion(){
	vector<unsigned> inRegion;

	for(unsigned i = numPositives;i<sampleSize; i++){
        unsigned j;
		for(j=0; j<numAttr; j++){			
			if(sam[i].getValueAt(j)<wkBnd[j].first ||sam[i].getValueAt(j)>=wkBnd[j].second)
				break;
        }
		if(j == numAttr)
			inRegion.push_back(i);
    }
	return inRegion;
}

/*
    边界值未考虑！！！
*/
void RACOSOptimizer::shrinkWkBounds(const Instance& pos,const Instance& neg,int attrChosen){
    // cout<<"["<<weakBnd[0].first.getFloat()<<","<<weakBnd[0].second.getFloat()<<"] "<<endl;
    if(isAttrBest[attrChosen])
        return;
    auto ty = attrInfo.getTypeAt(attrChosen);
    auto posVal = pos.getValueAt(attrChosen),negVal = neg.getValueAt(attrChosen);
    if(posVal < negVal){
        auto lower = posVal;
        auto upper = negVal;
        if(ty ==ValueType::LONG || ty==ValueType::INT)
            upper = upper - Value(1);
        else if(ValueType::UINT == ty && upper > Value(0u))
            upper = upper - Value(1u);

        if(lower < upper){
            Value newUpper(1);
            switch (ty)
            {
            case ValueType::INT:
                newUpper = Value(genRandomInt(lower.convertToInt(),upper.convertToInt()));
                break;

            case ValueType::UINT:
                newUpper = Value(genRandomUInt(lower.convertToUnsignedInt(),upper.convertToUnsignedInt()));
                break;
            
            case ValueType::LONG:
                newUpper = Value(genRandomLong(lower.convertToLongInt(),upper.convertToLongInt()));
                break;
            
            case ValueType::FLT:{
                auto nextAfterUpper = nextafterf(upper.convertToFloat(),lower.convertToFloat());
                if(nextAfterUpper == lower.convertToFloat())
                    newUpper = lower;
                else
                    newUpper = Value(genRandomFloat(lower.convertToFloat(),nextAfterUpper));   
                break;
            }
                
            
            case ValueType::DBL:{
                auto nextAfterUpper = nextafter(upper.convertToDouble(),lower.convertToDouble());
                if(nextAfterUpper == lower.convertToDouble())
                    newUpper = lower;
                else
                    newUpper = Value(genRandomDouble(lower.convertToDouble(),nextAfterUpper));   
                break;
                assert(0);
                break;
            }
                        
            default:assert(0);break;
            }
            assert(newUpper.getType()==ty);
            if (newUpper < wkBnd[attrChosen].second)
				wkBnd[attrChosen].second = newUpper;
        }
        else if(ty==ValueType::LONG || ty == ValueType::UINT ||ty==ValueType::INT)
            wkBnd[attrChosen].second = upper;
        else
           assert(0);
    }
    else if(posVal == negVal)
        return;
    else{
        auto upper = posVal;
        auto lower = negVal;   
        if(ty ==ValueType::LONG || ty==ValueType::INT || ty == ValueType::UINT)
            lower = lower + Value(1);

        if(lower < upper){
            Value newLower(1);
            switch (ty)
            {
            case ValueType::INT:
                newLower = Value(genRandomInt(lower.convertToInt(),upper.convertToInt()));
                break;
            
            case ValueType::UINT:
                newLower = Value(genRandomUInt(lower.convertToUnsignedInt(),upper.convertToUnsignedInt()));
                break;

            case ValueType::LONG:
                newLower = Value(genRandomLong(lower.convertToLongInt(),upper.convertToLongInt()));
                break;
            
            case ValueType::FLT:{
                auto nextAferLower = nextafterf(lower.convertToFloat(),upper.convertToFloat());
                if(nextAferLower==upper.convertToFloat())
                    newLower = upper;
                else
                    newLower = Value(genRandomFloat(nextAferLower,upper.convertToFloat()));  
                break;
            }
                
            
            case ValueType::DBL:{
                auto nextAferLower = nextafter(lower.convertToDouble(),upper.convertToDouble());
                if(nextAferLower==upper.convertToDouble())
                    newLower = upper;
                else
                    newLower = Value(genRandomDouble(nextAferLower,upper.convertToDouble()));  
                break;
            }

                        
            default:assert(0);break;
            }
            assert(newLower.getType()==ty);
            if (newLower > wkBnd[attrChosen].first)
			    wkBnd[attrChosen].first = newLower;
        }         
        else if(ty==ValueType::LONG || ty == ValueType::UINT ||ty==ValueType::INT)
                wkBnd[attrChosen].first = lower;
        else
            assert(0);
            
    }
    auto bnd = wkBnd[attrChosen];
    if(bnd.first == bnd.second)
        isAttrBest[attrChosen] = true;
    
    // cout<<pos.getValueAt(0).getFloat()<<", "<<neg.getValueAt(0).getFloat()<<" "<<
    // "["<<weakBnd[0].first.getFloat()<<","<<weakBnd[0].second.getFloat()<<"]"<<endl;
    
}
