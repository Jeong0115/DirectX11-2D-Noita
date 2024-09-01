#pragma once

#include "zzGameObject.h"

namespace zz
{
    class ShotGunner_Weak : public GameObject
    {
    public:
        enum class eMonsterState
        {
            Freedom,
            FollowPlayer,
            Battle,
        };

        ShotGunner_Weak();
        virtual ~ShotGunner_Weak();

        virtual void Initialize()   override;
        virtual void Update()       override;
        virtual void LateUpdate()   override;
        virtual void Render()       override;

        virtual void OnCollisionEnter(GameObject* other) override;
        virtual void OnCollisionStay(GameObject* other)  override;
        virtual void OnCollisionExit(GameObject* other)  override;

        void DeadEvent();
        void HitEvent();

    private:
        void playIdleAnimation();
        void shootAnimationEnd();
        void shoot();

        void freedom();
        void followPlayer();
        void battle();

        bool shootRay();

        void createBody();

        std::vector<class MonsterBody*> mBodies;

        class PixelCollider*    mPxCollider;
        class MuzzleEffect*     mMuzzleEffect;
        class HealthPoint*      mHealPoint;
        class DetectPlayer*     mDetectPlayer;
        class Animator*         mAnimator;
        class RigidBody*        mRigid;
        class AudioSource*      mAudio;

        eMonsterState mState;

        Vector3 mPxColliderScale;
        Vector3 mTip;

        float   mChoiceNextAction;
        float   mDirection;
        float   mHitFlashTime;
        float   mDetectPlayerTime;
        float   mReloadTime;

        int     mActionIndex;

        bool    mbEnterAction;
        bool    mbJump;
    };
}

