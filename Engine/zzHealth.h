#pragma once

#include "zzUI.h"

namespace zz
{
    class Health : public UI
    {
    public:
        Health();
        virtual ~Health();

        virtual void Initialize();
        virtual void Update();
        virtual void LateUpdate();
        virtual void Render();

        virtual void OnCollisionEnter(GameObject* other)    override;
        virtual void OnCollisionStay(GameObject* other)     override;
        virtual void OnCollisionExit(GameObject* other)     override;

        void OnEvent(const struct EvenetData& data);

    private:
        void createIcon();
        void createBar();

        class BarBackGround* mBar;

        float mHealthRate;
    };
}

