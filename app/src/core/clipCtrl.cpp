#include "clipCtrl.h"

namespace CC
{
    std::unique_ptr<ClipCtrl::ClipCtrlData> ClipCtrl::_inst = nullptr;

    static const std::vector<Clip> emptyClips = {};

    void ClipCtrl::init()
    {
        if (!_inst)
            _inst = std::make_unique<ClipCtrlData>();
    }

    void ClipCtrl::push(const std::list<Clip>& clips)
    {
        if (!_inst) return;
        if (clips.empty()) return;
        ClipDoData data;
        for (auto& clip : clips)
            data.dObjs.push_back({0, clip});
        if (data.dObjs.empty()) return;
        _inst->prstInst.todo(Op::PushClip, std::move(data));
    }

    void ClipCtrl::del(const std::list<size_t>& idx)
    {
        if (!_inst) return;
        if (idx.empty()) return;
        ClipDoData data;
        for (auto& i : idx)
        {
            if (i >= _inst->clips.size()) continue;
            auto itr = data.dObjs.begin();
            while(itr != data.dObjs.end())
                if (itr->first > i)
                {
                    data.dObjs.insert(itr, {i, _inst->clips[i]});
                    break;
                }
                else itr++;
            if (itr == data.dObjs.end()) data.dObjs.insert(itr, {i, _inst->clips[i]});
        }
        if (data.dObjs.empty()) return;
        _inst->prstInst.todo(Op::DeleteClip, std::move(data));
    }

    void ClipCtrl::swap(size_t a, size_t b)
    {
        if (!_inst) return;
        if (a == b) return;
        if (a >= _inst->clips.size() || b >= _inst->clips.size()) return;
        ClipDoData data;
        data.dObjs.push_back({a, Clip()});
        data.dObjs.push_back({b, Clip()});
        _inst->prstInst.todo(Op::SwapClip, std::move(data));
    }

    void ClipCtrl::merge(const std::list<size_t>& idx)
    {
        if (!_inst) return;
        if (idx.size() <= 1) return;
        ClipDoData data;
        for (auto& i : idx)
        {
            if (i >= _inst->clips.size()) continue;
            data.dObjs.push_back({i, Clip()});
        }
        if (data.dObjs.size() <= 1) return;
        _inst->prstInst.todo(Op::MergeClip, std::move(data));
    }

    void ClipCtrl::erase(const std::list<size_t>& idx)
    {
        if (!_inst) return;
        if (idx.empty()) return;
        ClipDoData data;
        for (auto& i : idx)
        {
            if (i >= _inst->clips.size()) continue;
            data.dObjs.push_back({i, Clip()});
        }
        if (data.dObjs.empty()) return;
        _inst->prstInst.todo(Op::EraseClip, std::move(data));
    }

    void ClipCtrl::set(size_t idx, const Clip& clip)
    {
        if (!_inst) return;
        if (idx >= _inst->clips.size()) return;
        ClipDoData data;
        data.dObjs.push_back({idx, _inst->clips[idx]});
        data.dObjs.push_back({idx, clip});
        _inst->prstInst.todo(Op::ChangeClip, std::move(data));
    }

    void ClipCtrl::devid(size_t idx)
    {
        if (!_inst) return;
        if (idx >= _inst->clips.size()) return;
        ClipDoData data;
        data.dObjs.push_back({idx, Clip()});
        auto childCount = _inst->clips[idx].mergedRects.size();
        if (childCount == 0) return;
        data.dObjs.push_back({childCount, Clip()});
        _inst->prstInst.todo(Op::DevidClip, std::move(data));
    }

    void ClipCtrl::undo()
    {
        if (!_inst) return;
        _inst->prstInst.undo();
    }

    void ClipCtrl::redo()
    {
        if (!_inst) return;
        _inst->prstInst.redo();
    }

    bool ClipCtrl::isCanRedo()
    {
        if (!_inst) return false;
        return _inst->prstInst.isCanRedo();
    }

    bool ClipCtrl::isCanUndo()
    {
        if (!_inst) return false;
        return _inst->prstInst.isCanUndo();
    }

    const std::vector<Clip>& ClipCtrl::getCurClips()
    {
        if (!_inst) return emptyClips;
        return _inst->clips;
    }
}