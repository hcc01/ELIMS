#ifndef TABFACTORY_H
#define TABFACTORY_H
#include<QWidget>
#include<QMap>
#include"cclient.h"
typedef  void * (*CREATE_FUNC)( QWidget* );
class TabFactory//今后会不断有新功能新页面拓展，使用这个工厂方便统一添加和管理。
{
public:
    static void *CreateObject( const QString &name, QWidget*parent=nullptr)
    {
        QMap<QString, CREATE_FUNC>::const_iterator it;
        it = mapCls_.find(name);
        if (it == mapCls_.end())
            return  0;
        else{
           CREATE_FUNC f= it.value();
            return f(parent);  //func();
        }
    }

    static void Register( const QString &name, CREATE_FUNC func)
    {
        mapCls_[name] = func;
    }
    static QStringList tabClasses(){ return mapCls_.keys();}
private:
    static QMap<QString, CREATE_FUNC> mapCls_;
};

//头文件被包含多次，也只定义一次mapCls_;
__declspec(selectany) QMap<QString, CREATE_FUNC> TabFactory::mapCls_;


class Register
{
public:
    Register( const QString &name, CREATE_FUNC func)
    {
        TabFactory::Register(name, func);
    }
};

#define REGISTER_TAB(class_name) \
    class class_name##Register { \
    public: \
    static  void* NewInstance(QWidget*parent=nullptr) \
    {  return  new class_name( parent);  } \
    private: \
    static Register reg_; \
    }; \
    Register class_name##Register::reg_(class_name::tabText(), class_name##Register::NewInstance)




#endif // TABFACTORY_H
