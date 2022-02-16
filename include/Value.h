#ifndef VALUE_H
#define VALUE_H

#include<iostream>
#define DeclValueToBase(Name,Type) Type Name() const;
#define DeclBiOp(Op,RetType) friend RetType operator Op(const Value&, const Value&);

enum ValueType{
    INT,
    UINT,
    LONG,
    FLT,
    DBL,
};

class Value{
public:
    explicit Value(int i):int_(i),type(INT){}
    explicit Value(unsigned ui):uint_(ui),type(UINT){}
    explicit Value(long l):lg(l),type(LONG){}
    explicit Value(float f):flt(f),type(FLT){}
    explicit Value(double d):dbl(d),type(DBL){}
    inline ValueType getType() const {
        return type;
    }
    inline void setValue(int i){
        int_ = i;
        type = INT;
    }
    inline void setValue(unsigned ui){
        uint_ = ui;
        type = UINT;
    }
    inline void setValue(long l){
        lg = l;
        type = LONG;
    }
    inline void setValue(float f){
        flt = f;
        type = FLT;
    }
    inline void setValue(double d){
        dbl = d;
        type = DBL;
    }
    DeclValueToBase(convertToInt,int)
    DeclValueToBase(convertToUnsignedInt,unsigned int)
    DeclValueToBase(convertToLongInt,long int)
    DeclValueToBase(convertToFloat,float)
    DeclValueToBase(convertToDouble,double)

    DeclBiOp(+,Value)
    DeclBiOp(-,Value)
    DeclBiOp(*,Value)
    DeclBiOp(/,Value)
    DeclBiOp(==,bool)
    DeclBiOp(!=,bool)
    DeclBiOp(<,bool)
    DeclBiOp(<=,bool)
    DeclBiOp(>,bool)
    DeclBiOp(>=,bool)
    
    friend std::ostream& operator<<(std::ostream&, const Value&);
private:
    union{
        /* data */
        int int_;
        unsigned uint_;
        long lg;
        float flt;
        double dbl;
    };
    ValueType type;
};

class VarValue{
public:
    VarValue(const VarValue& ano);
    VarValue(VarValue&& ano);
    explicit VarValue(ValueType ty);
    explicit VarValue(Value min_,Value max_);
    explicit VarValue(Value);
    ~VarValue(){
        if(min_)
            delete min_;
        if(max_)
            delete max_;
        min_ = nullptr;
        max_ = nullptr;
    }

    VarValue& operator=(const VarValue& ano);
    VarValue& operator=(VarValue&& ano);
    
    inline bool isConstant() const {
        return isConst;
    }
    inline Value getMinimum() const{
        return *min_;
    }
    inline Value getMaximum() const {
        return *max_;
    }
    inline ValueType getType() const {
        return type;
    }
    inline bool contains(Value v) const {
        return *min_ <= v && v <= *max_;
    }
    inline bool contains(VarValue VV) const{
        return this->contains(VV.getMinimum()) && this->contains(VV.getMaximum());
    }
    bool setMinimum(Value v);
    bool setMaximum(Value v);
    
    Value getRandomValue() const;

    friend bool operator ==(const VarValue&, const VarValue&);
    friend std::ostream& operator<<(std::ostream&, const VarValue&);
private:
    ValueType type;
    Value *min_=nullptr, *max_=nullptr;
    bool isConst;
};



#endif