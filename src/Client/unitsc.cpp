#include "unitsc.h"


UnitsC::UnitsC()
{
    init();
}

void UnitsC::init()
{
    //质量单位
    _units["mg"]={MassUnit,1};
    _units["g"]={MassUnit,1000};
    _units["kg"]={MassUnit,1000000};
    _units["t"]={MassUnit,1000000000};
    _units["μg"]={MassUnit,0.001};
    _units["ng"]={MassUnit,0.000001};
    _units["pg"]={MassUnit,0.000000001};
    //体积单位
    _units["L"]={VolumeUnit,1};
    _units["mL"]={VolumeUnit,0.001};
    _units["μL"]={VolumeUnit,0.000001};
    _units["100mL"]={VolumeUnit,0.1};
    _units["m3"]={VolumeUnit,1000};
    //面积单位
    _units["cm2"]={AreaUnit,1};
    _units["mm2"]={AreaUnit,0.01};
    _units["m2"]={AreaUnit,100};
    //长度单位
    _units["km"]={LengthUnit,1000000};
    _units["m"]={LengthUnit,1000};
    _units["dm"]={LengthUnit,100};
    _units["cm"]={LengthUnit,10};
    _units["mm"]={LengthUnit,1};
    _units["μm"]={LengthUnit,0.001};
    _units["nm"]={LengthUnit,0.000001};
    //时间单位
    _units["s"]={TimeUnit,1};
    _units["ms"]={TimeUnit,0.001};
    _units["μs"]={TimeUnit,0.000001};
    _units["min"]={TimeUnit,60};
    _units["h"]={TimeUnit,3600};
    _units["d"]={TimeUnit,3600*24};
    _units["a"]={TimeUnit,3600*24*365};
    //气压单位
    _units["Pa"   ]={PressureUnit,1};
    _units["kPa"]={PressureUnit,1000};
    _units["bar"]={PressureUnit,100000};
    _units["MPa"]={PressureUnit,1000000};
    _units["atm"]={PressureUnit,101325};
    _units["mmHg"]={PressureUnit,133.3};
    //摩尔单位
    _units["mol"]={MolUnit,1000};
    _units["mmol"]={MolUnit,1};
    _units["cmol"]={MolUnit,10};
    _units["μmol"]={MolUnit,0.001};
    //温度单位
    _units["℃"]={TempUnit,1};
    //声级单位
    _units["db"]={SoundUnit,1};
    //计数单位
    _units["个"]={CountUnit,1};
    _units["MPN"]={CountUnit,1};
    _units["CPU"]={CountUnit,1};
    _units["次"]={CountUnit,1};
    _units["倍"]={CountUnit,1};
    //比例单位
    _units["%"]={PerUnit,0.01};
    _units["‰"]={PerUnit,0.001};
    _units["ppm"]={PerUnit,0.000001};
    _units["ppb"]={PerUnit,0.000000001};
    //辐射剂量单位（待添加）
    //电磁场强度单位（待添加）
    //电导单位
    _units["S"]={ConductanceUnit,1};
    _units["mS"]={ConductanceUnit,0.001};
    _units["μS"]={ConductanceUnit,0.000001};
    //能力单位
    _units["J"]={EnergeUnit,1};
    _units["kJ"]={EnergeUnit,1000};
    //电位单位
    _units["V"]={PotentialUnit,1};
    _units["mV"]={PotentialUnit,0.001};
    //放射性单位
    _units["Bq"]={RadiationUnit,1};


}

QStringList UnitsC::allUnits()
{
    return _units.keys();
}

bool UnitsC::transBaseUnit(const QString &unit1, const QString &unit2, double &x)
{
    if(_units.find(unit1)==_units.end()||_units.find(unit2)==_units.end()) return false;
    Unit U1,U2;
    U1=_units.value(unit1);
    U2=_units.value(unit2);
    if(U1.unitType!=U2.unitType) return false;
    x=U1.coefficient/U2.coefficient;
    return true;
}

bool UnitsC::transUnit(const QString &fromUnit1, const QString &toUnit2, double &x)
{
    if(fromUnit1.indexOf("/")>0&&toUnit2.indexOf("/")>0){
        double x1,x2;
        if(!transBaseUnit(fromUnit1.split("/").at(0),toUnit2.split("/").at(0),x1)) return false;
        if(!transBaseUnit(fromUnit1.split("/").at(1),toUnit2.split("/").at(1),x2)) return false;
        x=x1/x2;
        return true;
    }
    if(fromUnit1.indexOf("/")>0) {
        Unit U2=_units.value(toUnit2);
        if(U2.unitType!=PerUnit) return false;
        double x1;
        if(!transBaseUnit(fromUnit1.split("/").at(0),fromUnit1.split("/").at(1),x1)) return  false;
        x=x1/U2.coefficient;
        return true;
    }
    if(toUnit2.indexOf("/")>0) {
        Unit U1=_units.value(fromUnit1);
        if(U1.unitType!=PerUnit) return false;
        double x1;
        if(!transBaseUnit(toUnit2.split("/").at(0),toUnit2.split("/").at(1),x1)) return  false;
        x=U1.coefficient/x1;
        return true;
    }
    return transBaseUnit(fromUnit1,toUnit2,x);
}


