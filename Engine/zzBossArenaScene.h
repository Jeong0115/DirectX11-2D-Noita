#pragma once

#include "zzScene.h"

namespace zz
{
    class BossArenaScene : public Scene
    {
    public:
        BossArenaScene();
        virtual ~BossArenaScene();

        virtual void Initialize()   override;
        virtual void Update()       override;
        virtual void LateUpdate()   override;
        virtual void Render()       override;

        virtual void Exit()         override;
        virtual void MovePlayer()   override;

    private:
        std::shared_ptr<class AudioClip> mAudio;
    };
}