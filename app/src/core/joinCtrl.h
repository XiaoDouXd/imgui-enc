#pragma once

#include <glm.hpp>

#include "clipCtrl.h"
#include "entrance.h"
#include "ui/util/rectTest.hpp"

namespace CC
{
    struct Tile
    {
    public:
        struct TileVData { glm::ivec2 pLU; glm::vec2 pLD; glm::vec2 pRD; glm::vec2 pRU;};

        TileVData getVDataLocal()
        {
            if (_dirty) updateTileVData();
            return _vDataCacheLocal;
        }

        bool test(glm::ivec2 min, glm::ivec2 size)
        {
            if (_dirty) updateTileVData();
            if (!twoRectTestFloat(min, size, _aabbMin, _aabbSize)) return false;
            auto v1 = _vDataCacheLocal.pRD - _vDataCacheLocal.pLD,
                v2 = glm::vec2(_vDataCacheLocal.pLU) - _vDataCacheLocal.pLD;
        }

        void setPos(glm::ivec2 p) noexcept { _pos = p; _dirty = true; }
        void setScale(glm::vec2 s) noexcept { _scale = s; _dirty = true; }
        void setRota(float r) noexcept { _rota = r; _dirty = true; }

    private:
        void updateTileVData()
        {
            if (_clipId >= ClipCtrl::getCurClips().size() || ClipCtrl::getCurClips()[_clipId].empty)
            {
                _vDataCacheLocal = {_pos, _pos, _pos, _pos};
                _aabbMin = _pos;
                _aabbSize = glm::vec2(0, 0);
                _dirty = false;
                return;
            }
            _vDataCacheLocal.pLU = _pos;
            auto clipSize = ClipCtrl::getCurClips()[_clipId].size;
            _vDataCacheLocal.pLD = glm::ivec2(0, clipSize.y);
            _vDataCacheLocal.pRD = clipSize;
            _vDataCacheLocal.pRU = glm::ivec2(clipSize.x, 0);

            auto mR = glm::mat2x2(
                glm::cos(_rota) * _scale.x, glm::sin(_rota) * _scale.x,
                -glm::sin(_rota) * _scale.y, glm::cos(_rota) * _scale.y);
            _vDataCacheLocal.pLD = mR * _vDataCacheLocal.pLD;
            _vDataCacheLocal.pRD = mR * _vDataCacheLocal.pRD;
            _vDataCacheLocal.pRU = mR * _vDataCacheLocal.pRU;

            _vDataCacheLocal.pLD += _pos;
            _vDataCacheLocal.pRD += _pos;
            _vDataCacheLocal.pRU += _pos;

            _aabbMin = glm::min(
                _vDataCacheLocal.pLD, glm::min(
                _vDataCacheLocal.pRD, glm::min(
                _vDataCacheLocal.pRU,
                glm::vec2(_vDataCacheLocal.pLU))));
            _aabbSize = glm::max(
                _vDataCacheLocal.pLD, glm::max(
                _vDataCacheLocal.pRD, glm::max(
                _vDataCacheLocal.pRU,
                glm::vec2(_vDataCacheLocal.pLU))));
            _aabbSize -= _aabbMin;

            _dirty = false;
        }

        TileVData _vDataCacheLocal;
        glm::vec2 _aabbMin;
        glm::vec2 _aabbSize;
        glm::ivec2 _pos;
        glm::vec2 _scale;
        size_t _clipId;
        float _rota;
        bool _dirty;
    };
}