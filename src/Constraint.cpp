#include "Constraint.h"
#include "Utils.h"
using namespace std;

#include <map>

static map<string,ArithOperator> StrToArithOp = {
    {"",NONE},
    {"+",ADD},{"-",SUB},{"*",MUL},{"/",DIV},
    {"Sin",SIN},{"Cos",COS}
};

static map<ArithOperator,string> ArithOpToStr = {
    {NONE,""},
    {ADD,"+"},{SUB,"-"},{MUL,"*"},{DIV,"/"},
    {SIN,"Sin"},{COS,"Cos"}
};

static map<string,RelOperator> StrToRelOp = {
    {"=",EQ},{":=",EQ},{"!=",NE},
    {"<",LT},{"<=",LE},
    {">",GT},{">=",GE}
};

static map<RelOperator,string> RelOpToStr = {
    {EQ,"="},{NE,"!="},
    {LT,"<"},{LE,"<="},
    {GT,">"},{GE,">="}
};

Variable::Variable(string name, ValueType ty):name(name),type(ty),domain(ty){}

Variable::Variable(string name,VarValue dom):domain(dom){
    if(dom.isConstant())
        this->name = "";
    else
        this->name = name;
    type = dom.getType();
}

Variable::Variable(Value dom):name(""),type(dom.getType()),domain(dom){}

Variable Variable::parse(string str, ValueType type){
    if(isInteger(str,true)){
        long num = stol(str);
        switch (type)
        {
        case INT:return Value((int)num);
        case UINT:return Value((unsigned)num);
        case LONG:return Value((long int)num);
        case FLT:return Value((float)num);
        case DBL:return Value((double)num);
        default:assert(0);break;
        }
    }
    else if(isReal(str)){
        double num = stod(str);
        switch (type)
        {
        case INT:return Value((int)num);
        case UINT:return Value((unsigned)num);
        case LONG:return Value((long int)num);
        case FLT:return Value((float)num);
        case DBL:return Value((double)num);
        default:assert(0);break;
        }
    }
    else{
        return Variable(str,type);
    }   
}

ostream& operator<<(ostream& out,const Variable& var){
    if(var.isConstant())
        out<<var.domain;
    else
        out<<var.name;
    return out;
}

Expression::Expression(string str){
    auto wsPos0 = str.find(" ",0UL);
    if(wsPos0 == string::npos){
        numOperands = 1u;
        op = NONE;
        operands.push_back(Variable::parse(str,FLT));
    }
    else{
        string head = str.substr(0,wsPos0);
        auto wsPos1 = str.find(" ",wsPos0+1);
        if(head == "Sin("){
            auto oper =str.substr(wsPos0+1,wsPos1 - wsPos0 -1);
            numOperands = 1u;
            Variable var = Variable::parse(oper,FLT);
            if(var.isConstant()){
                op = NONE;
                float f = var.getMinimum().convertToFloat();
                operands.push_back(Value(sin(f)));
            }
            else{
                op = SIN;
                operands.push_back(var);
            }
        }
        else if(head == "Cos("){
            auto oper =str.substr(wsPos0+1,wsPos1 - wsPos0 -1);
            numOperands = 1u;
            Variable var = Variable::parse(oper,FLT);
            if(var.isConstant()){
                op = NONE;
                float f = var.getMinimum().convertToFloat();
                operands.push_back(Value(cos(f)));
            }
            else{
                op = COS;
                operands.push_back(var);
            }
        }
        else{
            auto oper0 = str.substr(0,wsPos0);
            auto op = str.substr(wsPos0+1,wsPos1-wsPos0-1);
            auto oper1 = str.substr(wsPos1+1);
            numOperands = 2;
            this->op = StrToArithOp[op];
            operands.push_back(Variable::parse(oper0,FLT));
            operands.push_back(Variable::parse(oper1,FLT));

        }
    }
    simplify();
}

ostream& operator<<(ostream& out,const Expression& expr){
    assert(expr.numOperands == expr.operands.size());
    switch (expr.op)
    {
    case NONE:
        out<<expr.operands[0];
        break;
    case ADD:case SUB:case MUL:case DIV:
        out<<expr.operands[0]<<ArithOpToStr[expr.op]<<expr.operands[1];
        break;
    case SIN:case COS:
        out<<ArithOpToStr[expr.op]<<"("<<expr.operands[0]<<")";
        break;
    default:
        break;
    }
    return out;
}

Expression Expression::calculate(const map<string,Value>& assigns){
    Expression result(*this);
    unsigned num = 0;
    for(auto oper : this->operands){
        if(!oper.isConstant()){
            auto name = oper.getName();
            auto domain = oper.getDomain();
            auto it = assigns.find(name);
            if(it!=assigns.end()){
                if(domain.contains(it->second)){
                    result.operands[num] = it->second;
                }
            }
        }
        num ++;
    }
    result.simplify();
    return result;
}

void Expression::simplify(){
    if(1 == numOperands) {
        auto oper = operands[0];
        if(oper.isConstant()){
            double result = 0;
            switch (op)
            {
            case NONE:
                break;
            case SIN:
                result = sin(oper.getMinimum().convertToDouble());
                break;
            case COS:
                result = cos(oper.getMinimum().convertToDouble());
                break;
            default:
                assert(0);
                break;
            }
            if(NONE!=op){
                operands.clear();
                switch (oper.getType())
                {
                case INT:operands.push_back(Value((int)result));break;
                case UINT:operands.push_back(Value((unsigned int)result));break;
                case LONG:operands.push_back(Value((long int)result));break;
                case FLT:operands.push_back(Value((float)result));break;
                case DBL:operands.push_back(Value((double)result));break;
                default:break;
                }
                op = NONE;
            }
        }
    }
    else if(2 == numOperands){
        auto oper0 = operands[0],oper1 = operands[1];
        if(oper0.isConstant()&& oper1.isConstant()){
            operands.clear();
            
            switch (op)
            {
            case ADD:
                operands.push_back(oper0.getMinimum()+oper1.getMinimum());
                break;
            case SUB:
                operands.push_back(oper0.getMinimum()-oper1.getMinimum());
                break;
            case MUL:
                operands.push_back(oper0.getMinimum()*oper1.getMinimum());
                break;
            case DIV:
                operands.push_back(oper0.getMinimum()/oper1.getMinimum());
                break;
            
            default:
                assert(0);
                break;
            }
            numOperands = 1u;
            op = NONE;
        }
    }
    assert(numOperands == operands.size());
}

Constraint::Constraint(string str){
    size_t lhsEnd,opEnd,pos=0UL;
    bool inOp = false;
    for(auto ch : str){
        switch (ch)
        {
        case ':':
        case '=':
        case '<':
        case '>':
            if(inOp){
                opEnd ++;
            }
            else{
                lhsEnd = pos - 1;
                opEnd = pos + 1;
                inOp = true;
            }
            break;
        
        default:
            break;
        }
        pos++;
    }
    string sLHS = str.substr(0,lhsEnd);
    string sOp = str.substr(lhsEnd + 1, opEnd - lhsEnd-1);
    string sRHS = str.substr(opEnd+1);

    LHS = new Expression(sLHS);
    op = StrToRelOp[sOp];
    RHS = new Expression(sRHS);
}

Constraint::Constraint(Expression LHS,RelOperator op,Expression RHS){
    this->LHS = new Expression(LHS);
    this->op = op;
    this->RHS = new Expression(RHS);
}

ostream& operator<<(ostream& out, const Constraint& cons){
    out<<*cons.LHS<<" "<<RelOpToStr[cons.op]<<" "<<*cons.RHS;
    return out;
}