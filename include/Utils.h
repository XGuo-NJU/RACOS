#ifndef UTILS_H
#define UTILS_H
#include <string>

#define DeclRandomGenerator(Name,Type) Type Name(const Type,const Type);

bool isInteger(const std::string&, bool);
bool isReal(const std::string&);

DeclRandomGenerator(genRandomInt,int);
DeclRandomGenerator(genRandomUInt,unsigned);
DeclRandomGenerator(genRandomLong,long);
DeclRandomGenerator(genRandomFloat,float);
DeclRandomGenerator(genRandomDouble,double);

#endif