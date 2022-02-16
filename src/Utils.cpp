#include "Utils.h"
#include <random>
#include <iostream>

using namespace std;

#define DefRandomIntegerGenerator(Name,Type,Engine) \
Type Name(const Type lower,const Type upper){\
    assert(lower <= upper);\
    if(lower == upper)\
        return lower;\
    uniform_int_distribution<Type> distr(lower,upper);\
    auto rand = distr(Engine);\
    assert(lower<=rand && rand <=upper);\
    return rand;\
}

#define DefRandomRealGenerator(Name,Type,Engine) \
Type Name(const Type lower,const Type upper){\
    assert(lower <= upper);\
    if(lower == upper)\
        return lower;\
    uniform_real_distribution<Type> distr(lower,upper);\
    auto rand = distr(Engine);\
    assert(lower<=rand && rand <=upper);\
    return rand;\
}

static random_device RDevice;

static default_random_engine REngine(RDevice());

using namespace std;
bool isInteger(const string& str, bool sign){
    if(str == "")
        return false;

    auto it=str.begin();
    if(sign){
        if('+' == *it || '-' == *it)
            it++;
    }
    if(it == str.end())
        return false;

    while(it!=str.end()){
        char chr = *it;
        if(chr < '0' || chr >'9')
            return false;
        it++;
    }
    return true;
}

bool isReal(const string& str){
    auto ptPos = str.find('.');
    if(string::npos == ptPos)
        return false;
    if(ptPos == 0)
        return isInteger(str.substr(1),false);
    else if(ptPos == str.size()-1)
        return isInteger(str.substr(0,ptPos),true);
    else
        return isInteger(str.substr(0,ptPos),true) && isInteger(str.substr(ptPos+1),false);
}

DefRandomIntegerGenerator(genRandomInt,int,REngine);
DefRandomIntegerGenerator(genRandomUInt,unsigned,REngine);
DefRandomIntegerGenerator(genRandomLong,long,REngine);

DefRandomRealGenerator(genRandomFloat,float,REngine);
DefRandomRealGenerator(genRandomDouble,double,REngine);