#include "wndMgr.h"
#include "app.h"

namespace XD
{
    std::unique_ptr<WndMgr::WndMgrData> WndMgr::_inst = nullptr;

    size_t WndMgr::newId()
    {
        if (_inst->curId == 0 || _inst->curId == SIZE_MAX)
            _inst->curId = 1;
        while (_inst->allWndPool.find(_inst->curId) != _inst->allWndPool.end())
            _inst->curId++;
        return _inst->curId;
    }

    void WndMgr::close(size_t wndId)
    {
        if (_inst->shownWndPool.find(wndId) == _inst->shownWndPool.end())
            return;

        auto itr = _inst->shownWndPool[wndId];
        (*itr)->onHide();
        (*itr)->lateHide();
        (*itr)->_showingWndId = 0;
        _inst->hidenWndPool[(*itr)->classId].push_front({itr, wndId, TimeMgr::now()});
        _inst->shownWndPool.erase(wndId);
    }

    void WndMgr::wndGC()
    {
        static std::stack<size_t> delCache = std::stack<size_t>();
        if (_inst->hidenWndPool.empty()) return;
        for (auto& wndList : _inst->hidenWndPool)
        {
            while (!wndList.second.empty() &&
                (TimeMgr::now() - wndList.second.back().hideTime) > intervalGCTime)
            {
                auto& i = wndList.second.back();
                auto& l =_inst->wndClassPool[wndList.first];
                delete (*(i.itr));
                l.erase(i.itr);
                _inst->allWndPool.erase(i.wndId);
                wndList.second.pop_back();

                if (l.empty())
                {
                    _inst->wndClassPool.erase(wndList.first);
                    delCache.push(wndList.first);
                }
            }
        }
        while (!delCache.empty())
        {
            _inst->hidenWndPool.erase(delCache.top());
            delCache.pop();
        }
    }

    void WndMgr::update()
    {
        if (TimeMgr::now() - _inst->lastGCtime > intervalGCTime)
        {
            wndGC();
            _inst->lastGCtime = TimeMgr::now();
        }
    }

    void WndMgr::init()
    {
        _inst = std::make_unique<WndMgrData>();
    }

    void WndMgr::destory()
    {
        for (auto& wndCls : _inst->wndClassPool)
            for (auto& wnd : wndCls.second)
                delete wnd;
        _inst.reset();
    }

    void WndBaseHolder::closeSelf()
    {
        onHide();
        lateHide();
        for (auto& hashcode : _events)
            StaticEventMgr::unregisterEvent(hashcode, (std::ptrdiff_t) this);

        if (!_showingWndId)
            _showingWndId = SIZE_MAX;
        else
            WndMgr::close(_showingWndId);
    }

    void WndBaseHolder::lateHide()
    {
        for (auto& hideCB : onHideCB) hideCB(*this);
    }
}