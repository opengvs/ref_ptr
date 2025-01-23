/*    FileName: References.hpp  ���ļ��Ǹ���OSG�е�Referenced���������µģ�ȥ����ԭ�ļ���������ж�OpenThread��������
*      ������߳����������㰲ȫ�Ե�����ָ�룬���ʱ��OSG�¿ٳ����ģ��õ��˶��������Ŀ�У�
                    ʹ����std::scoped_lock lock(_ObserverMutex);��������Ҫc++17��֧��
      2024��12������û�£�ͻȻ��������ǰ�Ĵ��룬�뵽ʹ��ԭ�Ӳ���ʵ�֣����Ǹ�����Refencenced.hpp�ļ�
      ���θ�����Ҫ�޸��˼���������atomicԭ�Ӽ�������������_USE_ATIMIC_REFCOUNT_�꣬�����Ƿ�ʹ��ԭ�Ӽ�����
                        �����̻߳���Mutex����ɻ���ԭ��atomic������������SpinLock��ʽ
      1�����ļ���ҪC++11��֧��----��ӭͬѧ�ǽ��в���
      2����ָ��Ϊģ���࣬ģ��T����Ҫʵ��T->ref()/T->unref()/T->unref_nodelete()���������������û���osg::Referenced������
      ��Ȩ���У�������     2024��12��  ����˫���ɼ���****************************/


#ifndef OSG_REFERENCED
#define OSG_REFERENCED 1


#include <mutex>
#include <atomic>
#include <set>

namespace osg {

    // forward declare, declared after Referenced below.
    //class DeleteHandler;
    class Observer;
    class ObserverSet;

#define   _USE_ATIMIC_REFCOUNT_  1
    /** Base class for providing reference counted objects.*/
    class  Referenced
    {
    public:
        Referenced() :
            //_refCount(0),
            _observerSet(0)
        {
            setRefCount(0);
        };
        //  Referenced(const Referenced&) = delete;
        // inline Referenced& operator=(const Referenced&) = delete;

        inline Referenced(const Referenced& _Ref)
        {

            setRefCount(_Ref._refCount); //_refCount = _Ref._refCount;
            _observerSet = _Ref._observerSet;

        }

        //2024��11��������ֵ��ֵ���� 
        Referenced(/*const*/ Referenced&& _Ref)noexcept
        {
            //_refMutex = std::move_if_noexcept(_Ref._refMutex);
            setRefCount(_Ref._refCount); //_refCount = _Ref._refCount;
            _observerSet = std::move_if_noexcept(_Ref._observerSet);

            //_Ref._refMutex = nullptr;
#ifndef _USE_ATIMIC_REFCOUNT_
            _Ref._refCount = 0;
#else
            _Ref._refCount.store(0); //_Ref._refCount = 0;
#endif

            _Ref._observerSet = nullptr;
        }
        Referenced& operator = (Referenced&& _Ref)noexcept
        {
            if ((&_Ref) == this)
            {
                return *this;
            }

            if (this->_refCount > 0)
                this->unref();
            //_refMutex = std::move_if_noexcept(_Ref._refMutex);
            setRefCount(_Ref._refCount); //_refCount = _Ref._refCount;
            _observerSet = std::move_if_noexcept(_Ref._observerSet);

            //_Ref._refMutex = nullptr;
#ifndef _USE_ATIMIC_REFCOUNT_
            _Ref._refCount = 0;
#else
            _Ref._refCount.store(0); //_Ref._refCount = 0;
#endif 


            _Ref._observerSet = nullptr;
            return *this;
        }

        inline Referenced& operator = (const Referenced& _Ref)
        {
            if (this == &_Ref)
                return *this;
            if (this->_refCount > 0)
                this->unref();

            //_refMutex = _Ref._refMutex;
            setRefCount(_Ref._refCount); //_refCount = _Ref._refCount;


            _observerSet = _Ref._observerSet;
            return *this;
        }

        /** Increment the reference count by one, indicating that
           this object has another pointer which is referencing it.*/
        inline int ref() const;

        /** Decrement the reference count by one, indicating that
            a pointer to this object is no longer referencing it.  If the
            reference count goes to zero, it is assumed that this object
            is no longer referenced and is automatically deleted.*/
        inline int unref() const;

        /** Decrement the reference count by one, indicating that
            a pointer to this object is no longer referencing it.  However, do
            not delete it, even if ref count goes to 0.  Warning, unref_nodelete()
            should only be called if the user knows exactly who will
            be responsible for, one should prefer unref() over unref_nodelete()
            as the latter can lead to memory leaks.*/
        int unref_nodelete() const;

        /** Return the number of pointers currently referencing this object. */
        inline int referenceCount() const { return _refCount; }


        /** Get the ObserverSet if one is attached, otherwise return NULL.*/

        ObserverSet* getObserverSet() const
        {
            return static_cast<ObserverSet*>(_observerSet);
        }

        /** Get the ObserverSet if one is attached, otherwise create an ObserverSet, attach it, then return this newly created ObserverSet.*/
        ObserverSet* getOrCreateObserverSet() const;

        /** Add a Observer that is observing this object, notify the Observer when this object gets deleted.*/
        void addObserver(Observer* observer) const;

        /** Remove Observer that is observing this object.*/
        void removeObserver(Observer* observer) const;

    public:
        // friend class  ObserverSet;
        // friend void ObserverAtomicLock_Begin(const Referenced* _ref);
        // friend void ObserverAtomicLock_End(const Referenced* _ref);
    protected:

        virtual ~Referenced();

        //  void signalObserversAndDelete(bool signalDelete, bool doDelete) const;

          //���û�ж������ԭ�Ӳ���ʵ�ֵ������������廥����

         // atomic_flag  _refAtomicLock = ATOMIC_FLAG_INIT;       //��ʼ��ԭ�Ӳ�������
        mutable std::atomic_flag _refAtomicLock = ATOMIC_FLAG_INIT;
#define  refAtomicLockLOCK_BEGIN    while (_refAtomicLock.test_and_set(std::memory_order_acquire)) ;
#define  refAtomicLockLOCK_END    _refAtomicLock.clear(std::memory_order_release);

#ifndef _USE_ATIMIC_REFCOUNT_
        mutable unsigned long    _refCount = 0;
#else
        mutable    std::atomic<unsigned long>                 _refCount; // ����һ��ԭ������������
#endif


        //  mutable void* _observerSet;
        mutable ObserverSet* _observerSet;

        void  setRefCount(unsigned long _count)
        {
#ifndef _USE_ATIMIC_REFCOUNT_
            _refCount = _count;
#else
            _refCount.store(_count);
#endif
        }
    };
    /** Observer base class for tracking when objects are unreferenced (their reference count goes to 0) and are being deleted.*/
    class  Observer
    {
    public:
        Observer() {};
        /* virtual*/ ~Observer() {};

        /** objectDeleted is called when the observed object is about to be deleted.  The observer will be automatically
        * removed from the observed object's observer set so there is no need for the objectDeleted implementation
        * to call removeObserver() on the observed object. */
        /* virtual*/ void objectDeleted(void*) {}

    };


#define  observerAtomicLockLOCK_BEGIN    while (_refAtomicLock.test_and_set(std::memory_order_acquire)) ;
#define  observerAtomicLockLOCK_END    _refAtomicLock.clear(std::memory_order_release);




    /** Class used by osg::Referenced to track the observers associated with it.*/
    class  ObserverSet
    {
    public:
        /** Increment the reference count by one, indicating that
           this object has another pointer which is referencing it.*/
        inline int ref() const
        {
#ifndef _USE_ATIMIC_REFCOUNT_
            observerAtomicLockLOCK_BEGIN
#endif
                ++_refCount;

#ifndef _USE_ATIMIC_REFCOUNT_
            observerAtomicLockLOCK_END
#endif
                return _refCount;
        }

        inline int unref() const
        {
            int newRef;

            bool needDelete = false;


            observerAtomicLockLOCK_BEGIN


                --_refCount;
            newRef = _refCount;
            needDelete = newRef == 0;


            observerAtomicLockLOCK_END


                if (needDelete)
                {

                    if (_refCount == 0)
                        delete this;
                }


            return newRef;
        }
        inline int unref_nodelete() const
        {
            unsigned long tempRet = 0;
#ifndef _USE_ATIMIC_REFCOUNT_
            observerAtomicLockLOCK_BEGIN
#endif
                tempRet = --_refCount;

#ifndef _USE_ATIMIC_REFCOUNT_
            observerAtomicLockLOCK_END
#endif

                return  tempRet;
        }
    protected:
        //���û�ж������ԭ�Ӳ���ʵ�ֵ������������廥����
        mutable std::atomic_flag _refAtomicLock = ATOMIC_FLAG_INIT;  //��ʼ��ԭ�Ӳ�������

#ifndef _USE_ATIMIC_REFCOUNT_
        mutable unsigned long    _refCount = 0;
#else
        mutable    std::atomic<unsigned long>        _refCount = 0; // ����һ��ԭ������������
#endif
        void  setRefCount(unsigned long _count)
        {
#ifndef _USE_ATIMIC_REFCOUNT_
            _refCount = _count;
#else
            _refCount.store(_count);
#endif
        }

    public:
        ObserverSet() = delete;
        // ObserverSet() :osg::Referenced() {};

        ObserverSet(const Referenced* observedObject) :_observedObject(const_cast<Referenced*>(observedObject))
        {
            setRefCount(0);
        };

        Referenced* getObserverdObject() { return _observedObject; }
        const Referenced* getObserverdObject() const { return _observedObject; }

        /** "Lock" a Referenced object i.e., protect it from being deleted
          *  by incrementing its reference count.
          *
          * returns null if object doesn't exist anymore. */
        Referenced* addRefLock();

        // inline std::mutex* getObserverSetMutex() const { return &_ObserverMutex; }

        void addObserver(Observer* observer);
        void removeObserver(Observer* observer);

        void signalObjectDeleted(void* ptr);

        std::set<Observer*>& getObservers() { return _observers; }
        const std::set<Observer*>& getObservers() const { return _observers; }

    protected:
#pragma warning(disable : 26495)    
        ObserverSet(const ObserverSet& rhs) /* : osg::ObserverSetReferenced(rhs) */
        {
            setRefCount(rhs._refCount);
        }
#pragma warning(suppress : 26495)  

        ObserverSet& operator = (const ObserverSet& rhs)
        {
            // if (this == &rhs)
            //     return *this;
            // if (this->_refCount > 0)
            //     this->unref();
             //
            // setRefCount(rhs._refCount);


            return *this;
        }


        /*virtual*/ ~ObserverSet()
        {
            _observedObject = nullptr;
        }




        Referenced* _observedObject;
        std::set<Observer*>                       _observers;

    };



    inline void ObserverSet::addObserver(Observer* observer)
    {

        observerAtomicLockLOCK_BEGIN

            _observers.insert(observer);


        observerAtomicLockLOCK_END

    }

    inline void ObserverSet::removeObserver(Observer* observer)
    {

        observerAtomicLockLOCK_BEGIN


            _observers.erase(observer);


        observerAtomicLockLOCK_END

    }

    inline Referenced* ObserverSet::addRefLock()
    {
        if (_observedObject == nullptr)
            return 0;


        observerAtomicLockLOCK_BEGIN


            int refCount = _observedObject->ref();
        if (refCount == 1)
        {
            observerAtomicLockLOCK_END
                // The object is in the process of being deleted, but our
                // objectDeleted() method hasn't been run yet (and we're
                // blocking it -- and the final destruction -- with our lock).
                _observedObject->unref_nodelete();


            return nullptr;
        }


        observerAtomicLockLOCK_END


            return _observedObject;
    }


    inline void ObserverSet::signalObjectDeleted(void* ptr)
    {
        // const Referenced* _pRef = getObserverdObject();

        observerAtomicLockLOCK_BEGIN
            // ObserverAtomicLock_Begin(_pRef);

            for (std::set<Observer*>::iterator itr = _observers.begin();
                itr != _observers.end(); ++itr)
        {
            (*itr)->objectDeleted(ptr);
        }
        _observers.clear();


        // reset the observed object so that we know that it's now detached.
        _observedObject = nullptr;

        observerAtomicLockLOCK_END
            // ObserverAtomicLock_End(_pRef);
    }


    inline int Referenced::ref() const
    {
#ifndef _USE_ATIMIC_REFCOUNT_
        refAtomicLockLOCK_BEGIN
#endif
            _MT_INCR(_refCount); // ++_refCount;

#ifndef _USE_ATIMIC_REFCOUNT_
        refAtomicLockLOCK_END
#endif

            return _refCount;
    }

    inline int Referenced::unref() const
    {
        int newRef;

        bool needDelete = false;

        {  //������Ų���ɾ��

            refAtomicLockLOCK_BEGIN

                --_refCount;
            newRef = _refCount;
            needDelete = newRef == 0;

            refAtomicLockLOCK_END

        }
        if (needDelete)
        {
            if (_observerSet)
            {
                _observerSet->signalObjectDeleted(const_cast<Referenced*>(this));
            }
            if (_refCount == 0)
                delete this;
        }

        return newRef;
    }

    inline Referenced::~Referenced()
    {
        if (_refCount > 0)
        {
            // OSG_WARN<<"Warning: deleting still referenced object "<<this<<" of type '"<<typeid(this).name()<<"'"<<std::endl;
             //OSG_WARN<<"         the final reference count was "<<_refCount<<", memory corruption possible."<<std::endl;
        }

        if (_observerSet)
        {
            _observerSet->signalObjectDeleted(const_cast<Referenced*>(this));
            _observerSet->unref();
        }
    }

    inline ObserverSet* Referenced::getOrCreateObserverSet() const
    {

        refAtomicLockLOCK_BEGIN

            if (!_observerSet)
            {
                _observerSet = new ObserverSet(this);
                static_cast<ObserverSet*>(_observerSet)->ref();
            }


        refAtomicLockLOCK_END


            return static_cast<ObserverSet*>(_observerSet);
    }

    inline void Referenced::addObserver(Observer* observer) const
    {
        getOrCreateObserverSet()->addObserver(observer);
    }

    inline void Referenced::removeObserver(Observer* observer) const
    {
        getOrCreateObserverSet()->removeObserver(observer);
    }

    inline int Referenced::unref_nodelete() const
    {
        unsigned long tempRet = 0;

#ifndef _USE_ATIMIC_REFCOUNT_
        refAtomicLockLOCK_BEGIN
#endif
            tempRet = --_refCount;

#ifndef _USE_ATIMIC_REFCOUNT_
        refAtomicLockLOCK_END
#endif

            return  tempRet;
    }

    // intrusive_ptr_add_ref and intrusive_ptr_release allow
    // use of osg Referenced classes with boost::intrusive_ptr
    inline void intrusive_ptr_add_ref(Referenced* p) { p->ref(); }
    inline void intrusive_ptr_release(Referenced* p) { p->unref(); }


}

#endif