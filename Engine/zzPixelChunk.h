#pragma once

#include "zzElement.h"
#include "../External/Box2d/include/box2d.h"
#include <array>
#include <bitset>
#include <mutex>

namespace zz
{
    class PixelChunk
    {
    public:
        PixelChunk(size_t width, size_t height, int x, int y, int worldWidth, int worldHeight);
        ~PixelChunk();

        void Update();
        void UpdateElement(int x, int y);

        void SwapElement(Element& dstElement, int dstX, int dstY, Element& srcElement, int srcX, int srcY);

        void InsertElement(int x, int y, const Element& element);
        void InsertElementInOrOut(int x, int y, const Element& element);

        void InsertElementIfEmpty(int x, int y, const Element& element);

        void RegisterElement(int x, int y, const Element& element) { RegisterElement(GetIndex(x, y), element); }
        void RegisterElement(size_t index, const Element& element);

        void DelteElement(int x, int y) { DelteElement(GetIndex(x, y)); }
        void DelteElement(size_t index);

        bool TakeElement(int x, int y) { return TakeElement(GetIndex(x, y)); }
        bool TakeElement(size_t index);
            
        // �ӽ� ���߿� ����
        void KeepAlive(int x, int y);
        void KeepAlive(size_t index) { KeepAlive(index % mWidth + mStartX, index / mWidth + mStartY); }

        void KeepAliveBoundary(int x, int y);

        void UpdateRect();

        Element& GetElement(int x, int y);
        Element& GetElementInOrOut(int x, int y);

        Element& GetInChunkElement(size_t index) { return mElements[index]; }

        void UpdateSand(int x, int y, Element& element);
        void UpdateWater(int x, int y, Element& element);
        void UpdateBlood(int x, int y, Element& element);
        void UpdateLava(int x, int y, Element& element);
        void UpdateFire(int x, int y, Element& element);
        void UpdateGrass(int x, int y, Element& element);
        void UpdateSmoke(int x, int y, Element& element);
        void UpdateParticle(int x, int y, Element& element);

        void HeatNeighbors(int x, int y, Element& element);
        void DecreaseLifeTime(int x, int y, Element& element);

        bool InBounds(int x, int y) { return x >= mStartX && x < mStartX + mWidth && y >= mStartY && y < mStartY + mHeight;}
        bool IsEmpty(int x, int y) { return IsEmpty(GetIndex(x, y)); }
        bool IsEmpty(size_t index) { return GetInChunkElement(index).Type == eElementType::EMPTY; }

        size_t GetIndex(int x, int y) { return (x - mStartX) + (y - mStartY) * mWidth; };

        void RenewalBody();

    public:
        const int mWidth;
        const int mHeight;
        const int mStartX;
        const int mStartY;

        int mMinX;
        int mMinY;
        int mMaxX; 
        int mMaxY;

        int mWorldWidth;
        int mWorldHeight;

        size_t mElementCount;

        uint8_t mStaticCount[64* 64];

   
    private:

        std::vector<std::tuple<PixelChunk*, size_t, size_t>> mChanges;
        std::mutex mDirtyBoxMutex;
        Element* mElements;

        std::bitset<1> mUpdateDir;

        std::array<std::bitset<64>, 64> mStaticSolidElements;
        std::vector<b2Body*> mBodies;

        bool mbChange;

    private:
        int m_minXw, m_minYw,
            m_maxXw, m_maxYw;
    };
}
