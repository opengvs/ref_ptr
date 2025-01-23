// TestRef_PtrWithThread.cpp: 定义应用程序的入口点。
//

#include "TestRef_PtrWithThread.h"
#include "../ref_ptr.hpp"
#include <iostream>
#include <memory>
#include <thread>
#include <list>
#include <vector>
#include <atomic>
#include <ctime>
#include <cstdlib>  //主要是为了使用随机数

using namespace std;

void TestPrintPerson(void* p);

static int Index = 0;
class  Person : public osg::Referenced
{
    int index;
    int* p;
public:
    osg::ref_ptr<Person>  m_Father;
    osg::ref_ptr<Person>  m_Mather;
    vector<osg::observer_ptr<Person>> m_Childen;
    string  Name;
    Person()
    {
        p = new int[10];
        index = ++Index;
        for (int i = 0; i < 10; i++)
            p[i] = rand();
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
    }

    int Add(int i)
    {
        if ((i >= 0) && (i < 10))
        {
            p[i]++;
        }
        int total = 0;
        for (int j = 0; j < 10; j++)
        {
            total += p[j];
        }
        return total;
    }
    thread   ComputeWithThread;

    void  JoinComputeThread()
    {
        if(ComputeWithThread.joinable() == false)
                ComputeWithThread = thread(TestPrintPerson, this);
        ComputeWithThread.join();

    }
    static osg::ref_ptr <Person>   MakePersonPtr()
    {
        osg::ref_ptr <Person>  ptr(new Person());
        return std::move(ptr);
    }
};
std::vector<osg::ref_ptr<Person>>  g_PersonList;

void TestPrintPerson(void* p)
{
    osg::ref_ptr<Person>  pPerson = (Person*)p;
    for (int i = 0; i < 10; i++)
    {
        pPerson->Add(i);
    }
    for (int j = 0; j < 100; j++)
    {
        for (int i = 0; i < 10; i++)
        {
            printf("%d***%d***%d***\n", pPerson->Add(i), j, i);
        }
        ::_sleep(1);
    }
}

int main()
{
    srand(time(NULL));
    osg::ref_ptr<Person>  person = Person::MakePersonPtr();
    person->SetName("张三");


    osg::ref_ptr<Person>  farther = Person::MakePersonPtr();
    farther->SetName("张三爸爸");


    osg::ref_ptr<Person>  mather = Person::MakePersonPtr();
    mather->SetName("张三妈妈");


    osg::ref_ptr<Person>  brother = Person::MakePersonPtr();
    brother->SetName("张三弟弟");


    osg::ref_ptr<Person>  brother1 = Person::MakePersonPtr();
    brother1->SetName("张三妹妹");


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

    person = Person::MakePersonPtr();
    person->SetName("王五");
    farther = Person::MakePersonPtr();
    farther->SetName("王五爸爸");
    mather = Person::MakePersonPtr();
    mather->SetName("王五妈妈");
    brother = Person::MakePersonPtr();
    brother->SetName("王五弟弟");
    brother1 = Person::MakePersonPtr();
    brother1->SetName("王五妹妹");

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

    std::vector<osg::ref_ptr<Person>>::iterator pIt = g_PersonList.begin();
    while (pIt != g_PersonList.end())
    {
        osg::ref_ptr<Person> p = *pIt;

        p->JoinComputeThread();
        pIt++;
    }

	cout << "Hello CMake." << endl;
	return 0;
}
