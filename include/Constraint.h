#ifndef CONSTRAINT_H
#define CONSTRAINT_H
#include<iostream>
#include<vector>
#include<map>

#include "Value.h"

// Arithmetic Operator
enum ArithOperator{
    NONE,
    ADD,SUB,MUL,DIV,
    SIN,COS,
};

// Relational Operator
enum RelOperator{
    EQ,NE,GT,GE,LT,LE,
};

class Variable{
public:
    explicit Variable(std::string, ValueType);
    explicit Variable(std::string, VarValue);
    Variable(Value);
    inline ValueType getType() const{
        return type;
    }
    inline VarValue getDomain() const{
        return domain;
    }
    inline Value getMinimum() const {
        return domain.getMinimum();
    }
    inline Value getMaximum() const {
        return domain.getMaximum();
    }
    inline bool isConstant() const {
        return domain.isConstant();
    }
    inline std::string getName() const {
        return name;
    }
    static Variable parse(std::string, ValueType);

    friend std::ostream& operator<<(std::ostream&,const Variable&);
private:
    std::string name;
    ValueType type;
    VarValue domain;
};

class Expression{
public:
    explicit Expression(std::string);

    explicit Expression(const Variable& var){
        numOperands = 1u;
        operands.push_back(var);
        op = ArithOperator::NONE;
    }
    explicit Expression(const Variable& LHS, const ArithOperator op, const Variable& RHS){
        numOperands = 2u;
        operands.push_back(LHS);
        operands.push_back(RHS);
        this->op = op;
    }
    
    inline bool isSingleValue() const {
        return (1==numOperands) && (operands[0].isConstant());
    }
    inline bool isSingleVariable() const {
        return (1==numOperands) && (!operands[0].isConstant());
    }
    Expression calculate(const std::map<std::string,Value>&);
    inline Variable getOperandAt(unsigned ith) const{
        assert(ith < numOperands);
        return operands[ith];
    }
    inline std::vector<Variable> getOperands() const {
        return operands;
    }
    inline ArithOperator getOperator() const {
        return op;
    }
    friend std::ostream& operator<<(std::ostream&,const Expression&);

private:
    void simplify();
private:
    unsigned numOperands;
    std::vector<Variable> operands;
    ArithOperator op;
};

class Constraint{
public:
    Constraint(const Constraint& ano){
        op = ano.op;
        LHS = new Expression(*ano.LHS);
        RHS = new Expression(*ano.RHS);
    }
    Constraint(Constraint&& ano){
        LHS = ano.LHS;
        op = ano.op;
        RHS = ano.RHS;

        ano.LHS = nullptr;
        ano.RHS = nullptr;
    }
    explicit Constraint(std::string);
    explicit Constraint(Expression,RelOperator,Expression);
    ~Constraint(){
        if(LHS)
            delete LHS;
        if(RHS)
            delete RHS;
        LHS =nullptr;
        RHS = nullptr;
    }
    
    Constraint& operator=(const Constraint& ano){
        if(this == &ano)
            return *this;

        op = ano.op;
        if(LHS)
            delete LHS;
        if(RHS)
            delete RHS;
        LHS = new Expression(*ano.LHS);
        RHS = new Expression(*ano.RHS);

        return *this;
    }
    Constraint& operator=(Constraint&& ano){
        if(this == &ano)
            return *this;
            
        LHS = ano.LHS;
        op = ano.op;
        if(LHS)
            delete LHS;
        if(RHS)
            delete RHS;
        LHS = ano.LHS;
        RHS = ano.RHS;

        ano.LHS = nullptr;
        ano.RHS = nullptr;

        return *this;
    }

    inline Expression getLHS() const {
        assert(LHS);
        return *LHS;
    }
    inline Expression getRHS() const {
        assert(RHS);
        return *RHS;
    }
    inline RelOperator getOperator() const {
        return op;
    }

    
    friend std::ostream& operator<<(std::ostream&, const Constraint&);

private:
    RelOperator op;
    Expression *LHS,*RHS;
};


#endif