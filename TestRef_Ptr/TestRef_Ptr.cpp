// TestRef_Ptr.cpp: 定义应用程序的入口点。
//

#include "TestRef_Ptr.h"
#include "../ref_ptr.hpp"
#include <string>
#include <list>
#include <vector>

using namespace std;

class  Person : public osg::Referenced
{
    int* p;
public:
    osg::ref_ptr<Person>  m_Father;
    osg::ref_ptr<Person>  m_Mather;
    vector<osg::observer_ptr<Person>> m_Childen;
    string  Name;
    Person()
    {
        p = new int[10];
    }

    virtual ~Person()
    {
        delete[]p;
        static int i = 1;
        std::cout << "我被析构了" << i++ << std::endl;
    }
    void SetName(const string _Name)
    {
        Name = _Name;
    }
    void SetFather(osg::ref_ptr<Person>& father)
    {
        m_Father = father;
    }
    void SetMather(osg::ref_ptr<Person>& mather)
    {
        m_Mather = mather;
    }
    void AddChild(osg::observer_ptr<Person>& child)
    {
        m_Childen.push_back(child);
    }
    void AddChild(osg::ref_ptr<Person>& child)
    {
        osg::observer_ptr<Person> child_ptr = child;
        m_Childen.emplace_back(child_ptr);
    }
};
std::list<osg::ref_ptr<Person>>  g_PersonList;

int main()
{
    osg::ref_ptr<Person>  person = new Person();
    person->SetName("张三");
    

    osg::ref_ptr<Person>  farther = new Person();
    farther->SetName("张三爸爸");
    

    osg::ref_ptr<Person>  mather = new Person();
    mather->SetName("张三妈妈");
    

    osg::ref_ptr<Person>  brother = new Person();
    brother->SetName("张三弟弟");
    brother->m_Father = farther;
    brother->m_Mather = mather;

    osg::ref_ptr<Person>  brother1 = new Person();
    brother1->SetName("张三妹妹");
    brother1->m_Father = farther;
    brother1->m_Mather = mather;

    person->m_Father = farther;
    person->m_Mather = mather;

    farther->AddChild(person);
    farther->AddChild(brother);
    farther->AddChild(brother1);

    mather->AddChild(person);
    mather->AddChild(brother);
    mather->AddChild(brother1);

    g_PersonList.push_back(person);
    g_PersonList.push_back(farther);
    g_PersonList.push_back(mather);
    g_PersonList.push_back(brother);
    g_PersonList.push_back(brother1);

    auto pIt = g_PersonList.begin();

    while(pIt != g_PersonList.end() )
    {
        cout << (*pIt)->Name << endl;
        osg::ref_ptr<Person> Ref_Person = *pIt;
        if (Ref_Person->m_Father != nullptr)
        {
            cout << "父亲为:"<< Ref_Person->m_Father->Name << endl;
        }
        if (Ref_Person->m_Mather != nullptr)
        {
            cout << "父亲为:" << Ref_Person->m_Mather->Name << endl;
        }
        auto  pIt1 = Ref_Person->m_Childen.begin();
        while (pIt1 != Ref_Person->m_Childen.end())
        {
            osg::observer_ptr<Person> ObserverPerson_Ptr = *pIt1;

            osg::ref_ptr< Person >  tempChild = ObserverPerson_Ptr.lock();
            //if (ObserverPerson_Ptr.lock(tempChild) == true)
            if(tempChild != nullptr)
            {
                cout << "还有一个子女为:" << tempChild->Name << endl;
            }
            pIt1++;
        }
        cout << "-------------------" << endl;
        pIt++;
    }

	return 0;
}
