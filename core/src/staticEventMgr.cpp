#include "staticEventMgr.h"
#include "timeMgr.h"

namespace XD
{
    std::unique_ptr<StaticEventMgr::EventMgrData> StaticEventMgr::_inst = nullptr;
    static clock_t startTime;
    static const clock_t maxWaitTime = 160;

    void StaticEventMgr::init() {
        _inst = std::make_unique<StaticEventMgr::EventMgrData>();
    }

    void StaticEventMgr::update() {
        startTime = TimeMgr::now();
        while (!_inst->waitingQueue.empty() && TimeMgr::now() - startTime <= maxWaitTime) {
            _inst->waitingQueue.front().invoke();
            _inst->waitingQueue.front().event->waiting.pop();
            _inst->waitingQueue.pop_front();
        }
    }

    void StaticEventMgr::unregisterEvent(const std::size_t& hashCode, uuids::uuid obj) {
        auto& eDic = _inst->staticEvents;
        if (eDic.find(hashCode) == eDic.end()) return;
        auto& lDic = eDic[hashCode];
        if (lDic.find(obj) == lDic.end()) return;
        auto eH = lDic.find(obj);

        while (!eH->second.waiting.empty()) {
            _inst->waitingQueue.erase(eH->second.waiting.front());
            eH->second.waiting.pop();
        }

        lDic.erase(eH);
    }

    void StaticEventMgr::unregisterEvent(const std::optional<std::size_t>& hashCodeOpt, uuids::uuid obj) {
        if (!hashCodeOpt) return;
        const auto& hashCode = hashCodeOpt.value();
        auto& eDic = _inst->staticEvents;
        if (eDic.find(hashCode) == eDic.end()) return;
        auto& lDic = eDic[hashCode];
        if (lDic.find(obj) == lDic.end()) return;
        auto eH = lDic.find(obj);

        while (!eH->second.waiting.empty()) {
            _inst->waitingQueue.erase(eH->second.waiting.front());
            eH->second.waiting.pop();
        }

        lDic.erase(eH);
    }

    void StaticEventMgr::destroy() {
        _inst.reset();
    }
}