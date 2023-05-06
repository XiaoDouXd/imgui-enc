#pragma once

#include <concepts>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <stack>
#include <type_traits>
#include <unordered_map>

#include "uuidGen.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

namespace XD
{
    /// @brief base class of event
    /// @tparam EType event type which is inherit this class
    /// @tparam ArgTypes the params of event callback
    template<class EType, typename... ArgTypes>
    class EventTypeBase
    {
        friend class StaticEventMgr;
    public:
        using _cc_eType [[maybe_unused]] = EType;
        using _cc_fType [[maybe_unused]] = std::function<void(ArgTypes...)>;
        using _cc_isEventType [[maybe_unused]] = std::true_type;
    };


    class _xd_staticEvent_BaseFunc { public: virtual ~_xd_staticEvent_BaseFunc() = default; };
    template<class EType>
    class _xd_staticEvent_Func : public _xd_staticEvent_BaseFunc
    {
    public:
        explicit _xd_staticEvent_Func(typename EType::_cc_fType func): func(func) {}
        typename EType::_cc_fType func;
    };

    class StaticEventMgr
    {
    private:
        struct EventAsyncHandler;
        struct EventHandler
        {
        public:
            EventHandler(uuids::uuid objId, std::unique_ptr<_xd_staticEvent_BaseFunc>&& cb)
                    :objId(objId), cb(std::move(cb)), waiting(std::queue<std::list<EventAsyncHandler>::const_iterator>()) {}
            uuids::uuid objId = uuids::uuid();
            std::unique_ptr<_xd_staticEvent_BaseFunc> cb = nullptr;
            std::queue<std::list<EventAsyncHandler>::const_iterator> waiting;
        };

        struct EventAsyncHandler
        {
        public:
            EventAsyncHandler(EventHandler* event, std::function<void()> invoke)
                    :event(event), invoke(std::move(invoke)) {}
            EventHandler* event = nullptr;
            std::function<void()> invoke;
        };

        class EventMgrData
        {
        public:
            /// @brief 静态事件 <事件id, <事件监听成员, EventHandler>>
            std::unordered_map<std::size_t, std::map<uuids::uuid, EventHandler>> staticEvents;
            /// @brief 异步事件的等待队列
            std::list<EventAsyncHandler>                                         waitingQueue;
        };
        static std::unique_ptr<EventMgrData> _inst;

    public:
        /// @brief register event
        /// @tparam EType event type
        /// @param obj obj uuid
        /// @param cb callback of event
        /// @return the event inst of register action (you can unregister specific event using this)
        template<class EType>
        requires EType::_cc_isEventType::value && std::is_same<typename EType::_cc_eType, EType>::value
        static std::optional<std::size_t> registerEvent(uuids::uuid obj, typename EType::_cc_fType cb) {
            std::size_t hashCode = typeid(EType).hash_code();
            auto& eDic = _inst->staticEvents;
            if (eDic.find(hashCode) == eDic.end())
                eDic.insert({hashCode, std::map<uuids::uuid, EventHandler>()});
            auto& lDic = eDic[hashCode];
            if (lDic.find(obj) != lDic.end()) return std::nullopt;
            lDic.insert(std::pair(obj, EventHandler(
                    obj,
                    std::unique_ptr<_xd_staticEvent_BaseFunc>
                            (reinterpret_cast<_xd_staticEvent_BaseFunc*>(new _xd_staticEvent_Func<EType>(cb)))
            )));
            return std::make_optional<std::size_t>(hashCode);
        }

        /// @brief unregister,
        /// this function would unregister all event registered by this obj uuid
        /// @tparam EType event type
        /// @param obj obj uuid
        template<class EType>
        requires EType::_cc_isEventType::value && std::is_same<typename EType::_cc_eType, EType>::value
        static std::optional<std::size_t> unregisterEvent(const uuids::uuid& obj) {
            std::size_t hashCode = typeid(EType).hash_code();
            auto& eDic = _inst->staticEvents;
            if (eDic.find(hashCode) == eDic.end()) return std::nullopt;
            auto& lDic = eDic[hashCode];
            if (lDic.find(obj) == lDic.end()) return std::nullopt;
            auto eH = lDic.find(obj);

            while (!eH->second.waiting.empty())
            {
                _inst->waitingQueue.erase(eH->second.waiting.front());
                eH->second.waiting.pop();
            }

            lDic.erase(eH);
            return std::make_optional<std::size_t>(hashCode);
        }

        /// @brief unregister event
        /// @param hashCode event register action
        /// @param obj obj uuid
        static void unregisterEvent(const std::size_t& hashCode, uuids::uuid obj);

        /// @brief unregister event
        /// @param hashCodeOpt event register action
        /// @param obj obj uuid
        static void unregisterEvent(const std::optional<std::size_t>& hashCodeOpt, uuids::uuid obj);

        /// @brief clear all listener of this event
        /// @tparam EType event type
        template<class EType>
        requires EType::_cc_isEventType::value && std::is_same<typename EType::_cc_eType, EType>::value
        static void clearEvent() {
            std::size_t hashCode = typeid(EType).hash_code();
            auto& eDic = _inst->staticEvents;
            if (eDic.find(hashCode) == eDic.end()) return;
            for (auto& lDic : eDic[hashCode]) {
                while (!lDic.second.waiting.empty()) {
                    _inst->waitingQueue.erase(lDic.second.waiting.front());
                    lDic.second.waiting.pop();
                }
            }
            eDic.erase(hashCode);
        }

        /// @brief broadcast
        /// @tparam EType event type
        /// @tparam ...ArgTypes param package
        /// @param ...args callback params
        template<class EType, typename... ArgTypes>
        requires EType::_cc_isEventType::value && std::is_same<typename EType::_cc_eType, EType>::value
        && std::is_same<typename EType::_cc_fType, std::function<void(ArgTypes...)>>::value
        static void broadcast(ArgTypes... args) {
            std::size_t hashCode = typeid(EType).hash_code();
            auto& eDic = _inst->staticEvents;
            if (eDic.find(hashCode) == eDic.end()) return;
            for (auto& lDic : eDic[hashCode])
            {
                reinterpret_cast<_xd_staticEvent_Func<EType>*>(lDic.second.cb.get())->
                        func(std::forward<ArgTypes>(args)...);
            }
        }

        /// @brief broadcast async
        /// @tparam EType event type
        /// @tparam ...ArgTypes param package
        /// @param ...args callback params
        template<class EType, typename... ArgTypes>
        requires EType::_cc_isEventType::value && std::is_same<typename EType::_cc_eType, EType>::value
        && std::is_same<typename EType::_cc_fType, std::function<void(ArgTypes...)>>::value
        static void broadcastAsync(ArgTypes... args) {
            std::size_t hashCode = typeid(EType).hash_code();
            auto& eDic = _inst->staticEvents;
            if (eDic.find(hashCode) == eDic.end()) return;
            for (auto& lDic : eDic[hashCode]) {
                auto cbPtr = reinterpret_cast<_xd_staticEvent_Func<EType>*>(lDic.second.cb.get());
                _inst->waitingQueue.emplace_back(
                        EventAsyncHandler(
                                &lDic.second,
                                [cbPtr, args...](){cbPtr->func(args...);}
                        ));
                lDic.second.waiting.emplace(--(_inst->waitingQueue.end()));
            }
        }

    public:

        /// @brief init
        static void init();

        /// @brief update a frame
        static void update();

        /// @brief destroy static event mgr
        static void destroy();
    };
}
#pragma clang diagnostic pop