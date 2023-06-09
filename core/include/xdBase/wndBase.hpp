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
    /// @brief the base class of window
    /// @tparam T the class which inherit this, and it must be a final class,
    /// or you can ensure that it wouldn't be inherited!
    template<class T> class WndBase;

    /// @brief the base class of window init data
    /// @tparam T the class which inherit this, and it must be a final class,
    /// or you can ensure that it wouldn't be inherited!
    template<class T> class WndDataBase;

    class WndBaseHolder;
    class WndDataBaseHolder;

    class WndDataBaseHolder {
        friend class WndMgr;
        template<class T> friend class WndDataBase;
    public:
        /// @brief get window data
        /// @return the pointer of window data while the window data is the same type of T,
        /// otherwise return nullptr. the window data would deleted by wndMgr after onShow function called,
        /// so don't deleted this instance by yourself.
        template<class T>
        requires std::is_base_of<WndDataBase<T>, T>::value
        T* tryGetWndData() {
            if (!this || typeid(T).hash_code() != this->classId) return nullptr;
            return static_cast<T*>(this);
        }

        /// @brief callback on window shown
        std::vector<std::function<void(WndBaseHolder&, WndDataBaseHolder&)>> onShowCB;

        /// @brief callback on window hidden
        std::vector<std::function<void(WndBaseHolder&)>> onHideCB;

    protected:
        explicit WndDataBaseHolder(size_t classId) : classId(classId) {}
        virtual ~WndDataBaseHolder() = default;

    private:
        const size_t classId; // the type id of window
    };

    class WndBaseHolder : LoopUnit {
        friend class WndMgr;
        template<class T> friend class WndBase;
    public:
        /// @brief get window instance
        template<class T>
        requires std::is_base_of<WndBase<T>, T>::value
        T* tryGetWnd() {
            if (typeid(T).hash_code() != this->classId) return nullptr;
            return static_cast<T*>(this);
        }

        /// @brief get window nameId,
        /// which is an unique window identify generated by program
        [[nodiscard]] const char* getNameId() const { return _nameWithId.c_str(); }

    protected:
        WndBaseHolder(size_t classId, LoopLayer loopLayer) :
            LoopUnit(loopLayer),
            classId(classId),
            _uuid(UUID::gen()) {}
        ~WndBaseHolder() override { unregisterAll(); }

        /// @brief called while window instance create first
        virtual void onInit() {}

        /// @brief called while window is opened
        virtual void onShow(WndDataBaseHolder* wndData) {}

        /// @brief window refresh prefix
        /// @return call onRefresh function while return true
        virtual bool onWndBegin() { return true; }

        /// @brief window refresh, called while onWndBegin return true
        virtual void onRefresh() = 0;

        /// @brief window refresh suffix,
        /// called while onWndBegin or onRefresh return
        virtual void onWndEnd() {}

        /// @brief called while window is closing
        virtual void onHide() {}

        /// @brief close window
        virtual void closeSelf() final;

        /// @brief register window event,
        /// the events register by this function would be unregister by wndMgr while window is hidden
        /// @param cb event callback
        /// @tparam EType event type
        template<typename EType>
        requires EType::_cc_isEventType::value && std::is_base_of<typename EType::_cc_eType, EType>::value
        void registerEvent(EType::_cc_fType cb) {
            auto hashcode = StaticEventMgr::registerEvent<EType>(_uuid, cb);
            if (hashcode) _events.insert(hashcode.value());
        }

        /// @brief unregister window event
        /// @tparam EType event type
        template<typename EType>
        requires EType::_cc_isEventType::value && std::is_base_of<typename EType::_cc_eType, EType>::value
        void unregisterEvent() {
            auto hashcode = StaticEventMgr::unregisterEvent<EType>(_uuid);
            if (hashcode) _events.erase(hashcode.value());
        }

    private:
        void start() final {}
        void update() final {
            if (_showingWndId) {
                if (onWndBegin()) onRefresh();
                onWndEnd();
            }
        }
        void lateHide();

        void unregisterAll() {
            for (auto& hashcode : _events)
                StaticEventMgr::unregisterEvent(hashcode, _uuid);
        }

        const size_t classId;           // the type id of window
        size_t _showingWndId = 0;       // would be zero while window is hidden
        uuids::uuid _uuid;              // uuid
        std::string _nameWithId;        // unique str
        std::set<std::size_t> _events;  // window events
        std::vector<std::function<void(WndBaseHolder&)>> onHideCB;  // callback on window is hidden
    };

    template<class T>
    class WndDataBase : public WndDataBaseHolder {
    protected:
        WndDataBase() : WndDataBaseHolder(typeid(T).hash_code()) {};
        ~WndDataBase() override = default;
    };

    /// @brief default data
    class WndDataDefault : public WndDataBase<WndDataDefault> {
    public:
        WndDataDefault() = default;
    };

    template<class T>
    class WndBase : public WndBaseHolder {
    protected:
        /// @brief the base class of window
        /// @param loopLayer window layer, update first of others layer number is bigger
        explicit WndBase(LoopLayer loopLayer = LoopLayer::WndNormal) :
            WndBaseHolder(typeid(T).hash_code(), loopLayer) {}
        ~WndBase() override = default;
    };
}
#pragma clang diagnostic pop