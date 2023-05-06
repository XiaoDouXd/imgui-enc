#pragma once

#include <concepts>
#include <functional>
#include <set>
#include <string>
#include <vector>

#include "app.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

namespace XD::App {
    /// @brief class T must be final class,
    /// or you can ensure that T wouldn't be inherited!
    template<class T> class WndBase;

    /// @brief class T must be final class,
    /// or you can ensure that T wouldn't be inherited!
    template<class T> class WndDataBase;

    class WndBaseHolder;
    class WndDataBaseHolder;

    class WndDataBaseHolder {
        friend class WndMgr;
        template<class T> friend class WndDataBase;
    public:
        template<class T>
        requires std::is_base_of<WndDataBase<T>, T>::value
        T* tryGetWndData() {
            if (!this || typeid(T).hash_code() != this->classId) return nullptr;
            return static_cast<T*>(this);
        }
        std::vector<std::function<void(WndBaseHolder&, WndDataBaseHolder&)>> onShowCB;
        std::vector<std::function<void(WndBaseHolder&)>> onHideCB;

    protected:
        explicit WndDataBaseHolder(size_t classId) : classId(classId) {}
        virtual ~WndDataBaseHolder() = default;

    private:
        const size_t classId; // 窗口数据类型 id
    };

    class WndBaseHolder : LoopUnit {
        friend class WndMgr;
        template<class T> friend class WndBase;
    public:
        template<class T>
        requires std::is_base_of<WndBase<T>, T>::value
        T* tryGetWnd() {
            if (typeid(T).hash_code() != this->classId) return nullptr;
            return static_cast<T*>(this);
        }
        [[nodiscard]] const char* getNameId() const { return _nameWithId.c_str(); }

    protected:
        WndBaseHolder(size_t classId, LoopLayer loopLayer) :
            LoopUnit(loopLayer),
            classId(classId),
            _uuid(UUID::gen()) {}
        ~WndBaseHolder() override { unregisterAll(); }

        virtual void onInit() {}
        virtual void onShow(WndDataBaseHolder* wndData) {}
        virtual bool onWndBegin() { return true; }
        virtual void onRefresh() = 0;
        virtual void onWndEnd() {}
        virtual void onHide() {}

        void start() final {}
        void update() final {
            if (_showingWndId) {
                if (onWndBegin()) onRefresh();
                onWndEnd();
            }
        }
        virtual void closeSelf() final;
        virtual void lateHide() final;

        template<typename EType>
        requires EType::_cc_isEventType::value && std::is_same<typename EType::_cc_eType, EType>::value
        void registerEvent(EType::_cc_fType cb) {
            auto hashcode = StaticEventMgr::registerEvent<EType>(_uuid, cb);
            if (hashcode) _events.insert(hashcode.value());
        }

        template<typename EType>
        requires EType::_cc_isEventType::value && std::is_same<typename EType::_cc_eType, EType>::value
        void unregisterEvent() {
            auto hashcode = StaticEventMgr::unregisterEvent<EType>(_uuid);
            if (hashcode) _events.erase(hashcode.value());
        }

    private:
        void unregisterAll() {
            for (auto& hashcode : _events)
                StaticEventMgr::unregisterEvent(hashcode, _uuid);
        }

        const size_t classId;           // 窗口类型 id
        size_t _showingWndId = 0;       // 隐藏时这个 id 会被设置为 0
        uuids::uuid _uuid;              // 窗口唯一 id
        std::string _nameWithId;        // 窗口唯一 str
        std::set<std::size_t> _events;  // 窗口已注册的事件集
        std::vector<std::function<void(WndBaseHolder&)>> onHideCB;  // 关闭窗口时回调
    };

    template<class T>
    class WndDataBase : public WndDataBaseHolder {
    protected:
        WndDataBase() : WndDataBaseHolder(typeid(T).hash_code()) {};
        ~WndDataBase() override = default;
    };

    /// @brief 默认 data
    class WndDataDefault : public WndDataBase<WndDataDefault> {
    public:
        WndDataDefault() = default;
    };

    template<class T>
    class WndBase : public WndBaseHolder {
    protected:
        explicit WndBase(LoopLayer loopLayer = LoopLayer::WndNormal) :
            WndBaseHolder(typeid(T).hash_code(), loopLayer) {}
        ~WndBase() override = default;
    };
}
#pragma clang diagnostic pop