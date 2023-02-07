#pragma once

#include <ctime>
#include <list>
#include <memory>
#include <stack>
#include <unordered_map>
#include <unordered_set>

#include "wndBase.hpp"

namespace CC
{
    class WndMgr
    {
        friend class App;
    private:
        struct WndHolderItr
        {
            std::list<WndBaseHolder*>::iterator itr;
            size_t      wndId = 0;
            clock_t     hideTime = 0;
        };
        class WndMgrData
        {
        public:
            std::unordered_map<size_t, std::list<WndBaseHolder*>> wndClassPool;
            std::unordered_map<size_t, std::list<WndHolderItr>> hidenWndPool;
            std::unordered_map<size_t, std::list<WndBaseHolder*>::iterator> shownWndPool;
            std::unordered_set<size_t> allWndPool;
            size_t curId = 0;
            size_t lastGCtime = 0;
        };
        static std::unique_ptr<WndMgrData> _inst;
        static const clock_t intervalGCTime = 100000; // 1.6 min

    public:
        template<class T>
        requires std::is_base_of<WndBase<T>, T>::value
        static size_t open(WndDataBaseHolder* data = nullptr)
        {
            auto classId = typeid(T).hash_code();
            if (_inst->hidenWndPool.find(classId) == _inst->hidenWndPool.end() ||
                _inst->hidenWndPool[classId].empty()
            )
            {
                auto wndId = newId();
                WndBaseHolder* newWnd = new T();
                newWnd->onInit();
                newWnd->onShow(data);
                if (newWnd->_showingWndId)
                {
                    newWnd->onHide();
                    delete newWnd;
                    return 0;
                }

                auto& l = _inst->wndClassPool[classId];
                l.push_back(newWnd);
                auto itr = --l.end();
                if (_inst->shownWndPool.find(wndId) != _inst->shownWndPool.end())
                    throw Exce(__LINE__, __FILE__, "CC::WndMgr Exce: 窗口实例碰撞");
                _inst->shownWndPool[wndId] = itr;

                newWnd->_showingWndId = wndId;
                _inst->allWndPool.insert(wndId);
                return wndId;
            }
            else
            {
                auto& l = _inst->hidenWndPool[classId];
                auto itr = l.begin();
                (*(itr->itr))->onShow(data);
                if ((*(itr->itr))->_showingWndId)
                {
                    (*(itr->itr))->onHide();
                    (*(itr->itr))->_showingWndId = 0;
                    return 0;
                }

                if (_inst->shownWndPool.find(itr->wndId) != _inst->shownWndPool.end())
                    throw Exce(__LINE__, __FILE__, "CC::WndMgr Exce: 窗口实例碰撞");
                _inst->shownWndPool[itr->wndId] = itr->itr;

                auto wndId = itr->wndId;
                (*(itr->itr))->_showingWndId = wndId;
                l.erase(itr);
                return wndId;
            }
        }

        template<class T>
        requires std::is_base_of<WndBase<T>, T>::value
        static T* getWnd(size_t wndId)
        {
            auto classId = typeid(T).hash_code();
            if (_inst->wndClassPool.find(classId) == _inst->wndClassPool.end())
                return nullptr;
            if (_inst->wndClassPool[classId].empty()) return nullptr;
            if (_inst->shownWndPool.find(wndId) == _inst->shownWndPool.end()) return nullptr;
            auto itr = _inst->shownWndPool[wndId];
            return (*itr)->tryGetWnd<T>();
        }

        template<class T>
        requires std::is_base_of<WndBase<T>, T>::value
        static std::list<T*> getWnds()
        {
            auto classId = typeid(T).hash_code();
            if (_inst->wndClassPool.find(classId) == _inst->wndClassPool.end())
                return std::list<T*>();
            if (_inst->wndClassPool[classId].empty()) return std::list<T*>();

            auto wndList = std::list<T*>();
            for (auto& wnd : _inst->wndClassPool[classId])
            {
                if (!(wnd->_showingWndId)) continue;
                auto ptr = wnd->tryGetWnd<T>();
                if (ptr) wndList.emplace_back(ptr);
            }
            return wndList;
        }

        static void close(size_t wndId);

    private:
        static void init();
        static void destory();
        static void wndGC();
        static void update();
        static size_t newId();
    };
}