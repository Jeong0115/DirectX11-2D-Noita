#pragma once

#include "zzGameObject.h"
#include "zzEquipment.h"

namespace zz
{
    class Player : public GameObject
    {
    public:
        Player();
        ~Player();

        virtual void Initialize()   override;
        virtual void Update()       override;
        virtual void LateUpdate()   override;
        virtual void Render()       override;

        virtual void OnCollisionEnter(GameObject* other) override;
        virtual void OnCollisionStay(GameObject* other)  override;
        virtual void OnCollisionExit(GameObject* other)  override;

        void HitEvent();
        void DeathEvent();
        
        GameObject* GetPlayerArm() { return mPlayerArm; }

        void SetEquipment(Equipment* equipment)  { mEquipment = equipment; mEquipmentInitialPos = mEquipment->GetInitialPos(); }
        void SetCamera(GameObject* camera)          { mCamera = camera; }
        void SetArm(GameObject* arm)                { mPlayerArm = arm; }

    private:
        GameObject* mCamera;
        GameObject* mPlayerArm;
        Equipment*  mEquipment;
        
        class PlayerView*   mPlayerView;
        class Animator*     mAnimator;
        class RigidBody*    mRigid;
        class HealthPoint*  mHealthPoint;

        Vector3 mEquipmentInitialPos;
        float vel;

        bool jump = false;
        bool fall = false;
    };
}
