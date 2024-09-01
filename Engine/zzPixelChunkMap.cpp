#include "zzPixelChunkMap.h"
#include "zzRenderer.h"
#include "zzPixelWorld.h"

// ====

#include "zzTextObject.h"
#include "zzTime.h"
namespace zz
{
    PixelChunkMap::PixelChunkMap(UINT mapWidth, UINT mapHeight, int x, int y)
        : mHeight(mapHeight)
        , mWidth(mapWidth)
        , mStartX(x * mapWidth)
        , mStartY(y * mapHeight)
        , mChunkWidth(64)
        , mChunkHeight(64)
        , mYChunkCnt(mHeight / mChunkHeight)
        , mXChunkCnt(mWidth / mChunkWidth)
        , mDebugMeshColorType(0)
    {
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                mChunks.push_back(new PixelChunk(mChunkWidth, mChunkHeight
                    , mStartX + mChunkWidth * j, mStartY + mChunkHeight * i, PixelWorld::mWorldWidth, PixelWorld::mWorldHeight));
            }
        }
    }
    PixelChunkMap::~PixelChunkMap()
    {
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                delete mChunks[j + i * mXChunkCnt];
            }
        }
    }

    void PixelChunkMap::UpdateStep(int index)
    {
        // index값에 따라 매 프레임 업데이트 되는 청크의 순서를 변경
        int startI = mXChunkCnt - 1;
        int startJ = mYChunkCnt - 1;

        switch (index)
        {
        case 1: --startJ; break;
        case 2: --startI; break;
        case 3: --startI; --startJ; break;
        default: break;
        }

        for (int i = startI; i >= 0; i -= 2)
        {
            for (int j = startJ; j >= 0; j -= 2)
            {
                PixelWorld::threadPool.enqueue([=]() {mChunks[j * mXChunkCnt + i]->Update(); });

                if (PixelWorld::IsDebugDrawing())
                {
                    DebugMesh mesh = {};
                    mesh.temp = 1;
                    mesh.position = math::Vector3(mChunks[j * mXChunkCnt + i]->mStartX + 32, -mChunks[j * mXChunkCnt + i]->mStartY - 32, 0.01f);
                    mesh.scale = math::Vector3(64, 64, 1.0f);
                    mesh.rotation = math::Vector3::Zero;
                    renderer::PushDebugMeshAttribute(mesh);
                }
            }
        }
    }


    void PixelChunkMap::UpdateRect()
    {
        time += Time::DeltaTime();
        if (time >= 10.0f)
        {
            b = true;
            time -= 10.0f;
        }
        else
        {
            b = false;
        }
        for (int i = 0; i < 8; i ++)
        {
            for (int j = 0; j < 8; j ++)
            {
                PixelWorld::threadPool.enqueue([=]() {mChunks[j * mXChunkCnt + i]->UpdateRect(); });
                //mChunks[j * mXChunkCnt + i]->UpdateRect();
            }
        }
    }

    PixelChunk* PixelChunkMap::GetChunk(int x, int y)
    {
        x -= mStartX;
        y -= mStartY;
        return mChunks[(y / mChunkHeight * mXChunkCnt) + x / mChunkWidth];
    }
}