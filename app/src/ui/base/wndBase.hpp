#pragma once

#include <concepts>
#include <functional>
#include <set>
#include <string>
#include <vector>

#include "app.h"

namespace CC
{
    template<class T> class WndBase;
    template<class T> class WndDataBase;
    class WndBaseHolder;
    class WndDataBaseHolder;

    class WndDataBaseHolder
    {
        friend class WndMgr;
        template<class T> friend class WndDataBase;
    public:
        template<class T>
        requires std::is_base_of<WndDataBase<T>, T>::value
        T* tryGetWndData()
        {
            if (!this || typeid(T).hash_code() != this->classId) return nullptr;
            return static_cast<T*>(this);
        }
        std::vector<std::function<void(WndBaseHolder&, WndDataBaseHolder&)>> onShowCB;
        std::vector<std::function<void(WndBaseHolder&)>> onHideCB;

    protected:
        WndDataBaseHolder(size_t classId) : classId(classId) {}
        virtual ~WndDataBaseHolder() {}

    private:
        const size_t classId;           // 窗口数据类型 id
    };

    class WndBaseHolder : LoopUnit
    {
        friend class WndMgr;
        template<class T> friend class WndBase;
    public:
        template<class T>
        requires std::is_base_of<WndBase<T>, T>::value
        T* tryGetWnd()
        {
            if (typeid(T).hash_code() != this->classId) return nullptr;
            return static_cast<T*>(this);
        }
        const char* getNameId() const { return _nameWithId.c_str(); }

    protected:
        WndBaseHolder(size_t classId) : LoopUnit(LoopLayer::Wnd), classId(classId) {}
        virtual ~WndBaseHolder()
        {
            for (auto& hashcode : _events)
                StaticEventMgr::unregisterEvent(hashcode, (std::ptrdiff_t) this);
            onDestroy();
        }

        virtual void onInit() {}
        virtual void onShow(WndDataBaseHolder* wndData) {}
        virtual bool onWndBegin() { return true; }
        virtual void onRefresh() = 0;
        virtual void onWndEnd() {}
        virtual void onHide() {}
        virtual void onDestroy() {}

        virtual void start() override final {}
        virtual void update() override final
        {
            if (_showingWndId)
            {
                if (onWndBegin()) onRefresh();
                onWndEnd();
            }
        }
        virtual void closeSelf() final;
        virtual void lateHide() final;

        template<typename EType>
        requires EType::__cc_isEventType::value && std::is_same<typename EType::__cc_eType, EType>::value
        void registerEvent(EType::__cc_fType cb)
        {
            auto hashcode = StaticEventMgr::registerEvent<EType>((std::ptrdiff_t)this, cb);
            if (hashcode) _events.insert(hashcode.value());
        }

        template<typename EType>
        requires EType::__cc_isEventType::value && std::is_same<typename EType::__cc_eType, EType>::value
        void unregisterEvent()
        {
            auto hashcode = StaticEventMgr::unregisterEvent<EType>((std::ptrdiff_t)this);
            if (hashcode) _events.erase(hashcode.value());
        }

    private:
        const size_t classId;           // 窗口类型 id
        size_t _showingWndId = 0;       // 隐藏时这个 id 会被设置为 0
        std::string _nameWithId;        // 窗口唯一 str
        std::set<std::size_t> _events;  // 窗口已注册的事件集
        std::vector<std::function<void(WndBaseHolder&)>> onHideCB;  // 关闭窗口时回调
    };

    template<class T>
    class WndDataBase : public WndDataBaseHolder
    {
    protected:
        WndDataBase() : WndDataBaseHolder(typeid(T).hash_code()) {};
        virtual ~WndDataBase() {}
    };

    /// @brief 默认 data
    class WndDataDefault : public WndDataBase<WndDataDefault>
    {
    public:
        WndDataDefault() {}
    };

    template<class T>
    class WndBase : public WndBaseHolder
    {
    protected:
        WndBase() : WndBaseHolder(typeid(T).hash_code()) {}
        virtual ~WndBase() { onDestroy(); }
    };
}