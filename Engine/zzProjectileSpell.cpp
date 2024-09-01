#include "zzProjectileSpell.h"
#include "zzPixelWorld.h"
#include "zzElement.h"
#include "zzTime.h"
#include "zzTransform.h"
#include "zzRigidBody.h"
#include "zzHealthPoint.h"

namespace zz
{
    ProjectileSpell::ProjectileSpell()
        : mDirection(Vector3::Zero)
        , mSpeed(0.0f)
        , mSpread(0.0f)
        , mDamage(0.0f)
        , mMuzzleEffect(nullptr)
        , mTrailElement{}
        , mTime(0.0f)
        , mRigid(nullptr)
        , mPrevPos(Vector4::Zero)
        , mAudio(nullptr)
    {
        mType = eSpellType::Projectile;
    }

    ProjectileSpell::~ProjectileSpell()
    {
    }

    void ProjectileSpell::Initialize()
    {
        mTime = 0.f;
        GameObject::Initialize();
    }

    void ProjectileSpell::InitialSetting()
    {
    }

    void ProjectileSpell::Update()
    {
        mTime += (float)Time::DeltaTime();   
        mPrevPos = GetComponent<Transform>()->GetPosition() + 0.0f;

        GameObject::Update();
    }
    void ProjectileSpell::LateUpdate()
    {
        Vector3 curPos = GetComponent<Transform>()->GetPosition();
        float angle = GetComponent<Transform>()->GetRotation().z;

        if (!mTrailElement.empty())
        {
            int size = mTrailElement.size(); // ���߿� ������ ��������� ����?

            int dx = curPos.x - mPrevPos.x;
            int dy = curPos.y - mPrevPos.y;

            int steps = std::max(std::abs(dx), std::abs(dy));

            float xIncrement = dx / (float)steps;
            float yIncrement = dy / (float)steps;

            float x = mPrevPos.x;
            float y = mPrevPos.y;

            Vector2 offset = {};
            for (int i = 0; i < steps + 1; i++)
            {
                int count = randi(3);

                for (int j = 0; j < count; j++)
                {
                    int offsetY = randi(6) - 3;

                    offset.x = offsetY * sin(angle);
                    offset.y = offsetY * cos(angle);

                    PixelWorld::InsertElementIfEmpty(static_cast<int>(x + offset.x)
                        , -static_cast<int>(y + offset.y), mTrailElement[randi(size - 1)]);
                }
                x += xIncrement;
                y += yIncrement;
            }
        }

        GameObject::LateUpdate();
    }
    void ProjectileSpell::Render()
    {
        GameObject::Render();
    }

    void ProjectileSpell::OnCollisionEnter(GameObject* other)
    {
        if (other->GetLayerType() == eLayerType::Monster)
        {
            HealthPoint* hp = other->GetComponent<HealthPoint>();
            if (hp)
            {
                hp->Hit(mDamage);
            }
            Dead();
        }
    }

    void ProjectileSpell::OnCollisionStay(GameObject* other)
    {
    }
    void ProjectileSpell::OnCollisionExit(GameObject* other)
    {
    }

    void ProjectileSpell::Dead()
    {
    }

    ProjectileSpell* ProjectileSpell::Clone()
    {
        return nullptr;
    }
}