#include "DissatRate.h"
#include "Utils.h"
using namespace std;

DissatRateCalculator::DissatRateCalculator(const vector<Constraint>& constraints){
    initConstraints(constraints);
    initVaribales();
    initVarBounds();
    initAttrInfo();


    /*
    for(auto cons : this->constraints)
        cout<< cons<<endl;

    */

    /*
    for(auto it : vars){
        cout<<it.first<<" "<<it.second<<endl;
    }
    */
    initMaxRates();
}

double DissatRateCalculator::getValue(const Instance& ins){
    double val = 0.0;
	auto rates = compute(ins);
	for(unsigned i=0;i<rates.size();i++)
		val += rates[i]/maxRates[i];

    return val;
}

Instance DissatRateCalculator::getBestNeighbor(const Instance& src,const Instance& dest){
	unsigned idx = genRandomInt(0,constraints.size()-1);

    auto srcRate = compute(src)[idx];
    auto destRate = compute(dest)[idx];


	double t;
	if(srcRate == destRate)
		t = genRandomDouble(-1.0,1.0);
	else
		t = -destRate/(srcRate - destRate);
	
	Instance neighbor = Instance::genRandomInstance(attrInfo);
	for(unsigned i=0;i<attrInfo.getNumAttributes();i++){
		
		Value val(1);
		auto srcVal = src.getValueAt(i),dstVal = dest.getValueAt(i);
		switch (attrInfo.getTypeAt(i))
		{
		case ValueType::INT:
        case ValueType::UINT:
		case ValueType::LONG :
			neighbor.setValueAt(i,srcVal);
			break;
		case ValueType::FLT:
			val = Value(((float)t) * (srcVal.convertToFloat() - dstVal.convertToFloat()) + dstVal.convertToFloat());
			assert(val.getType()== ValueType::FLT);
			if (val >= attrInfo.getUpperBoundAt(i))
				neighbor.setValueAt(i,attrInfo.getUpperBoundAt(i));
			else if(val <= attrInfo.getLowerBoundAt(i))
				neighbor.setValueAt(i,attrInfo.getLowerBoundAt(i));
			else
				neighbor.setValueAt(i,val);
			break;
		case ValueType::DBL:
			val = Value(t * (srcVal.convertToDouble() - dstVal.convertToDouble()) + dstVal.convertToDouble());
			assert(val.getType()== ValueType::DBL);
			if (val >= attrInfo.getUpperBoundAt(i))
				neighbor.setValueAt(i,attrInfo.getUpperBoundAt(i));
			else if(val <= attrInfo.getLowerBoundAt(i))
				neighbor.setValueAt(i,attrInfo.getLowerBoundAt(i));
			else
				neighbor.setValueAt(i,val);
			
			break;
		default:
			assert(0);
			break;
		}
		
	}
	neighbor.setFnValue(getValue(neighbor));
	if(neighbor.getFnValue()<src.getFnValue())
		return neighbor;
	else
		return src;
}


void DissatRateCalculator::initConstraints(const vector<Constraint>& constraints){
    map<string, Value> assigns;
    for(auto cons : constraints){
        auto LHS = cons.getLHS().calculate(assigns);
        auto RHS = cons.getRHS().calculate(assigns);
        auto op = cons.getOperator();

        if(RelOperator::EQ == op){
            if(LHS.isSingleVariable() && RHS.isSingleValue()){
                assigns.insert({LHS.getOperandAt(0).getName(),RHS.getOperandAt(0).getMinimum()});
            }
            else if(RHS.isSingleVariable() && LHS.isSingleValue()){
                assigns.insert({RHS.getOperandAt(0).getName(),LHS.getOperandAt(0).getMinimum()});
            }
            else
                this->constraints.push_back(Constraint(LHS,op,RHS));
        }
        else{
            this->constraints.push_back(Constraint(LHS,op,RHS));
        }
    }
}

void DissatRateCalculator::initVaribales(){
    set<string> defVars;
    for(auto cons:constraints){
        // cout<<cons<<endl;
        auto LHS = cons.getLHS();
        auto RHS = cons.getRHS();
        for(auto oper : RHS.getOperands()){
            if(!oper.isConstant()){
                auto name = oper.getName();
                if(0 == defVars.count(name))
                    vars.insert({name,VarValue(Value(-1000.0f),Value(1000.0f))});
            }
        }
        if(RelOperator::EQ == cons.getOperator() && LHS.isSingleVariable()){
            // assert(0==defVars.count(LHS.getOperandAt(0).getName()));
            auto name  = LHS.getOperandAt(0).getName();
            if(0 == vars.count(name))
                defVars.insert(name);
        }
        else {
            for(auto oper : LHS.getOperands()){
                if(!oper.isConstant()){
                    auto name = oper.getName();
                    if(0 == defVars.count(name))
                        vars.insert({name,VarValue(Value(-1000.0f),Value(1000.0f))});
                }    
            }
        }
    }
}

void DissatRateCalculator::initVarBounds(){
    auto copy = constraints;
    constraints.clear();
    cout<<"Warning: same operations on '<' and '<='."<<endl;
    for(auto cons : copy){
        auto LHS = cons.getLHS();
        auto RHS = cons.getRHS();
        if(LHS.isSingleVariable() && RHS.isSingleValue()){
            auto name = LHS.getOperandAt(0).getName();
            auto val = RHS.getOperandAt(0).getMinimum();
            auto it = vars.find(name);
            if(it == vars.end()){
                constraints.push_back(cons);
                continue;
            }
            auto domain = it->second;
            // assert(domain == vars.find(name)->second);
            
            switch (cons.getOperator())
            {
            case RelOperator::GT:
            case RelOperator::GE:
                if(domain.getMinimum() < val  && val <= domain.getMaximum()){
                    assert(domain.setMinimum(val));
                    vars.erase(it);
                    vars.insert({name,domain});
                }
                else
                    constraints.push_back(cons);
                break;
            case RelOperator::LT:
            case RelOperator::LE:
                if(domain.getMinimum() <= val  && val < domain.getMaximum()){
                    assert(domain.setMaximum(val));
                    vars.erase(it);
                    vars.insert({name,domain});

                }
                else
                    constraints.push_back(cons);
                break;
            
            default:
                constraints.push_back(cons);
                break;
            }

        }
        else if(LHS.isSingleValue() && RHS.isSingleVariable()){
            auto name = RHS.getOperandAt(0).getName();
            auto val = LHS.getOperandAt(0).getMinimum();
            auto it = vars.find(name);
            if(it == vars.end()){
                constraints.push_back(cons);
                continue;
            }
            auto domain = it->second;
            
            switch (cons.getOperator())
            {
            case RelOperator::GT:
            case RelOperator::GE:
                if(domain.getMinimum() <= val  && val < domain.getMaximum()){
                    assert(domain.setMaximum(val));
                    vars.erase(it);
                    vars.insert({name,domain});
                }
                else
                    constraints.push_back(cons);
                break;
            case RelOperator::LT:
            case RelOperator::LE:
                // cout<<(domain.getMinimum() < val) <<" "<<val<<" "<<domain.getMaximum()<<endl;
                if(domain.getMinimum() < val  && val <= domain.getMaximum()){
                    assert(domain.setMinimum(val));
                    vars.erase(it);
                    vars.insert({name,domain});
                }
                else
                    constraints.push_back(cons);
                break;
            
            default:
                constraints.push_back(cons);
                break;
            }
        }
        else
            constraints.push_back(cons);
    }
}

void DissatRateCalculator::initAttrInfo(){
    size_t i=0;

    for(auto it : vars){
        varIDs.insert({it.first,i});
        attrInfo.addAttribute(it.second);
        i++;
    }   
}

void DissatRateCalculator::initMaxRates(){

	vector<vector<double>> multiRates;

    for(unsigned i=0;i<sampleSize;i++){
		Instance ins = Instance::genRandomInstance(attrInfo);
        multiRates.push_back(compute(ins));
    }
    for(unsigned i=0;i<constraints.size();i++){
        double max_ = multiRates[0][i];
        for(unsigned j = 1;j<sampleSize;j++)
            if(max_ < multiRates[j][i])
                max_ = multiRates[j][i];
        if(max_!=0)
            maxRates.push_back(max_);
        else
            maxRates.push_back(warningPunishment);

    }
}

vector<double> DissatRateCalculator::compute(const Instance& ins){
    map<string,Value> varVals;
    vector<double> rates;
    for(auto it : varIDs)
        varVals.insert({it.first,ins.getValueAt(it.second)});
    
    for(auto cons : constraints){
        // cout<<cons<<endl;
        auto LHS = cons.getLHS().calculate(varVals);
        auto RHS = cons.getRHS().calculate(varVals);
        auto op = cons.getOperator();

        assert(RHS.isSingleValue());

        auto RHSVal = RHS.getOperandAt(0).getMinimum();
        if(RelOperator::EQ == op && LHS.isSingleVariable()){
            varVals.insert({LHS.getOperandAt(0).getName(),RHSVal});
            rates.push_back(0.0);
        }
        else{
            assert(LHS.isSingleValue());
            auto LHSVal = LHS.getOperandAt(0).getMinimum();
            double rate = abs((LHSVal - RHSVal).convertToDouble());
            switch (op)
            {
            case RelOperator::EQ :
                if(LHSVal == RHSVal)
                    rate = 0.0;
                break;
            case RelOperator:: NE:
                if(LHSVal != RHSVal)
                    rate = 0.0;
                else
                   rate = inequalityPunishment;
                break;
            case RelOperator::GT :
                if(LHSVal > RHSVal)
                    rate = 0.0;
                else if(LHSVal == RHSVal)
                    rate = 0.00001;
                break;
            case RelOperator::GE :
                if(LHSVal >= RHSVal)
                    rate = 0.0;
                break;

            case RelOperator::LT :
                if(LHSVal < RHSVal)
                    rate = 0.0;
                else if(LHSVal == RHSVal)
                    rate = 0.00001;
                break;
            case RelOperator::LE :
                if(LHSVal <= RHSVal)
                    rate = 0.0;
                break;
            
            default:
                assert(0);
                break;
            }
            rates.push_back(rate);
        }
    }
    assert(rates.size() == constraints.size());
    return rates;
}
/*
map<Variable*,double> DissatRateCalculator::getVariableValueMap(Instance& ins){
	map<Variable*,double> varValMap;
	for(auto it : symVars){
		auto var = it.first;
		auto attrVal = ins.getValueAt(symIDs[{var,0u}]);
		switch (attrVal.getType())
		{
		case AttrValue::Type::INT :
			varValMap[var] = attrVal.getInt();
			break;
		
		case AttrValue::Type::LONG :
			varValMap[var] = attrVal.getLong();
			break;
		
		case AttrValue::Type::FLT :
			varValMap[var] = attrVal.getFloat();
			break;

		case AttrValue::Type::DBL :
			varValMap[var] = attrVal.getDouble();
			break;
		
		default:
			break;
		}
	}
	return varValMap;
}
*/