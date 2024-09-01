#pragma once

#include "zzEngine.h"
#include "zzPixelChunk.h"

namespace zz
{
    class PixelChunkMap
    {
    public:
        PixelChunkMap(UINT mapWidth, UINT mapHeight, int x, int y);
        ~PixelChunkMap();

        void UpdateStep(int index);
        void UpdateRect();

        PixelChunk* GetChunk(int x, int y);

        float time = 0;
        bool b = true;
    public:
        const int mStartX;
        const int mStartY;
        const int mChunkWidth;
        const int mChunkHeight;

        const UINT mHeight;
        const UINT mWidth;

        const int mYChunkCnt;
        const int mXChunkCnt;

        int mDebugMeshColorType;

    private:
        std::vector<PixelChunk*> mChunks;
    };
}
