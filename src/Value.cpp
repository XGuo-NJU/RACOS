#include "Value.h"
#include "Utils.h"

#include <limits>

using namespace std;

#define DefValueToBase(Name,Type) \
Type Value::Name() const{\
    switch (type)\
    {\
    case INT:return int_;\
    case UINT:return uint_;\
    case LONG:return lg;\
    case FLT:return flt;\
    case DBL:return dbl;\
    default:assert(0);break;\
    }\
}

#define DefBiOp(Op,RetType) \
RetType operator Op(const Value& LHS, const Value& RHS){\
    assert(LHS.type == RHS.type);\
    switch (LHS.type)\
    {\
    case INT:\
        return RetType(LHS.int_ Op RHS.int_);\
    case UINT:\
        return RetType(LHS.uint_ Op RHS.uint_);\
    case LONG:\
        return RetType(LHS.lg Op RHS.lg);\
    case FLT:\
        return RetType(LHS.flt Op RHS.flt);\
    case DBL:\
        return RetType(LHS.dbl Op RHS.dbl);\
    default:assert(0);\
    }\
}



DefValueToBase(convertToInt,int)
DefValueToBase(convertToUnsignedInt,unsigned int)
DefValueToBase(convertToLongInt,long int)
DefValueToBase(convertToFloat,float)
DefValueToBase(convertToDouble,double)


DefBiOp(+,Value)
DefBiOp(-,Value)
DefBiOp(*,Value)
DefBiOp(/,Value)
DefBiOp(==,bool)
DefBiOp(!=,bool)
DefBiOp(<,bool)
DefBiOp(<=,bool)
DefBiOp(>,bool)
DefBiOp(>=,bool)

ostream& operator<<(ostream& out, const Value& val){
    switch (val.type)
    {
    case INT:out<<val.int_;break;
    case UINT:out<<val.uint_;break;
    case LONG:out<<val.lg;break;
    case FLT:out<<val.flt;break;
    case DBL:out<<val.dbl;break;
    default:assert(0);break;
    }
    return out;
}

VarValue::VarValue(const VarValue& ano){
    type = ano.type;
    min_ = new Value(*ano.min_);
    max_ = new Value(*ano.max_);
    isConst = ano.isConst;
}

VarValue::VarValue(VarValue&& ano){
    type = ano.type;
    min_ = ano.min_;
    max_ = ano.max_;
    isConst = ano.isConst;

    ano.min_ = nullptr;
    ano.max_ = nullptr;
}

VarValue& VarValue::operator=(const VarValue& ano){
    if(this == &ano)
        return *this;
    type = ano.type;
    if(min_)
        delete min_;
    min_ = new Value(*ano.min_);
    if(max_)
        delete max_;
    max_ = new Value(*ano.max_);
    isConst = ano.isConst;
    return *this;
}

VarValue& VarValue::operator=(VarValue&& ano){
    if(this == &ano)
        return *this;
    type = ano.type;
    if(min_)
        delete min_;
    min_ = ano.min_;
    if(max_)
        delete max_;
    max_ = ano.max_;
    isConst = ano.isConst;

    ano.min_ = nullptr;
    ano.max_ = nullptr;
    return *this;
}
    
VarValue::VarValue(ValueType ty):type(ty){
    switch (ty)
    {
    case INT:
        min_ = new Value(INT_MIN);
        max_ = new Value(INT_MAX);
        break;
    case UINT:
        min_ = new Value(0u);
        max_ = new Value(UINT32_MAX);
        break;
    case LONG:
        min_ = new Value(LONG_MIN);
        max_ = new Value(LONG_MAX);
        break;
    case FLT:
        min_ = new Value(-__FLT_MAX__);
        max_ = new Value(__FLT_MAX__);
        break;
    case DBL:
        min_ = new Value(-__DBL_MAX__);
        max_ = new Value(__DBL_MAX__);
        break;
    default:
        assert(0);
        break;
    }
    isConst = false;
}

VarValue::VarValue(Value min_,Value max_){
    type = min_.getType();
    assert(type == max_.getType());
    assert(min_<=max_);

    this->min_ = new Value(min_);
    this->max_ = new Value(max_);
    isConst = min_== max_;
}

VarValue::VarValue(Value val){
    type = val.getType();
    isConst = true;
    min_ = new Value(val);
    max_ = new Value(val);
}

bool VarValue::setMinimum(Value v) {
    if(v > *max_ || v.getType() != type)
        return false;
    *min_ = v;
    isConst = *min_ == *max_;
    return true;
}

bool VarValue::setMaximum(Value v) {
    if(v < *min_ || v.getType() != type)
        return false;
    *max_ = v;
    isConst = *min_ == *max_;
    return true;
}

Value VarValue::getRandomValue() const{
    if(isConst)
        return *min_;
    assert(*min_ < *max_);
    switch (type)
    {
    case INT:return Value(genRandomInt(min_->convertToInt(),max_->convertToInt()));
    case UINT:return Value(genRandomUInt(min_->convertToUnsignedInt(),max_->convertToUnsignedInt()));
    case LONG:return Value(genRandomLong(min_->convertToLongInt(),max_->convertToLongInt()));
    case FLT:return Value(genRandomFloat(min_->convertToFloat(),max_->convertToFloat()));
    case DBL:return Value(genRandomDouble(min_->convertToDouble(),max_->convertToDouble()));
    default:assert(0);break;
    }
}

bool operator ==(const VarValue& LHS, const VarValue& RHS){
    return LHS.type == RHS.type && *LHS.min_ == *RHS.min_ && *LHS.max_ == *RHS.max_;
}

ostream& operator<<(ostream& out, const VarValue& varVal){
    
    if(varVal.isConst)
        out<<*varVal.min_;
    else
        out<<"["<<*varVal.min_<<","<<*varVal.max_<<"]";
    return out;
}
