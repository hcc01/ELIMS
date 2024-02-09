#ifndef GLOBAL_H
#define GLOBAL_H
#include<QString>
void toStdParameterName(QString& parameterName){
    parameterName.remove(' ').replace("，",",").replace("’","'").replace("（","(").replace("）",")").replace("′","'");
}

#endif // GLOBAL_H
