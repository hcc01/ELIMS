#ifndef UNITSC_H
#define UNITSC_H
#include<math.h>
#include<QMap>
#include<QObject>
#define UNITSC UnitsC::instance()
enum UnitType{
    MassUnit,VolumeUnit,AreaUnit,LengthUnit,PressureUnit,MolUnit,SoundUnit,TimeUnit,TempUnit,CountUnit,PerUnit,CombiUnit,
    ConductanceUnit,EnergeUnit,PotentialUnit,RadiationUnit,
};
struct Unit{
    UnitType unitType;
    double coefficient;
};

class UnitsC
{
public:
    static UnitsC instance(){
        static UnitsC uc;
        return uc;
    }
    QStringList allUnits();
    bool transBaseUnit(const QString& fromUnit1,const QString& toUnit2,double& x);//将uinit1表示的数值转为unit2表示的数值，应当乘以的系数
    bool transUnit(const QString &fromUnit1,const QString& toUnit2,double& x);//将uinit1表示的数值转为unit2表示的数值，应当乘以的系数

private:
    UnitsC();
    void init();
private:
    QMap<QString, Unit> _units;
};

#endif // UNITSC_H
