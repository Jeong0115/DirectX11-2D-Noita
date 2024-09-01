#include "zzExplosion_128.h"
#include "zzResourceManager.h"
#include "zzTexture.h"
#include "zzAnimator.h"
#include "zzMeshRenderer.h"
#include "zzMesh.h"
#include "zzMaterial.h"
#include "zzTransform.h"
#include "zzPixelWorld.h"
#include "zzParticleSystem.h"
#include "zzSmoke_Orange.h"
#include "zzObjectPoolManager.h"
#include "zzLight.h"
#include "zzAudioSource.h"

namespace zz
{
    Explosion_128::Explosion_128()
        : mbFirstFrame(true)
        , mParticle(nullptr)
    {
    }

    Explosion_128::~Explosion_128()
    {
    }

    void Explosion_128::Initialize()
    {
        std::shared_ptr<Texture> texture = ResourceManager::Find<Texture>(L"explosion_128");

        MeshRenderer* meshRender = AddComponent<MeshRenderer>();
        meshRender->SetMaterial(ResourceManager::Find<Material>(L"m_SpriteAnimation"));
        meshRender->SetMesh(ResourceManager::Find<Mesh>(L"RectMesh"));

        Animator* animator = AddComponent<Animator>();
        animator->Create(L"explosion_128_play", texture, Vector2(0.0f, 0.0f), Vector2(128.8f, 129.f), 5, Vector2::Zero, 0.03f);
        animator->PlayAnimation(L"explosion_128_play", false);
        animator->EndEvent(L"explosion_128_play") = std::bind(&Explosion_128::EndAnimation, this);

        GetComponent<Transform>()->SetScale(129.f, 129.f, 1.0f);
        AddComponent<Light>()->SetLightScale(200.f, 200.f, 1.0f);

        AudioSource* audio = AddComponent<AudioSource>();
        audio->SetClip(ResourceManager::LoadAudioClip(L"tnt_explode", L"..\\Resources\\Audio\\Projectiles\\tnt_explode.wav"));
        audio->SetLoop(false);

        mParticle = AddComponent<ParticleSystem>();
        mParticle->SetMaterial(ResourceManager::Find<Material>(L"m_fire_falling"));
        mParticle->SetMesh(ResourceManager::Find<Mesh>(L"PointMesh"));
        mParticle->SetParticleShader(ResourceManager::Find<ParticleShader>(L"ParticleAnimationCS"));

        ParticleAnimation particles[10] = {};
        mParticle->CreateStructedBuffer(sizeof(ParticleAnimation), 10, eViewType::UAV, particles, true, 2, 15, 0);
        mParticle->CreateStructedBuffer(sizeof(AnimationShared), 1, eViewType::UAV, nullptr, true, 3, 15, 1);

        Vector3 pos = GetComponent<Transform>()->GetPosition();

        AnimationShared shareData = {};
        shareData.curPosition = Vector4((int)pos.x, (int)pos.y, (int)pos.z, 0.0f);
        shareData.gravity = Vector4(0.0f, -5.0f, 0.0f, 0.0f);
        shareData.activeCount = 10;
        shareData.durtaion = 0.06f;
        shareData.maxAnimationCnt = 12;

        mParticle->SetStructedBufferData(&shareData, 1, 1);

        GameObject::Initialize();
    }

    void Explosion_128::Update()
    {
        if (mbFirstFrame)
        {
            Vector3 pos = GetComponent<Transform>()->GetPosition();

            int posX = static_cast<int>(pos.x);
            int posY = static_cast<int>(pos.y);

            for (int x = posX - 60; x <= posX + 60; x++)
            {
                int y = static_cast<int>(std::round(std::sqrt(60 * 60 - (x - posX) * (x - posX))));
                for (int j = posY - y; j <= posY + y; j++)
                {
                    PixelWorld::InsertElement(x, -j, FIRE);
                }
            }
            GetComponent<AudioSource>()->Play();
            mbFirstFrame = false;
        }
        GameObject::Update();
    }

    void Explosion_128::LateUpdate()
    {
        GameObject::LateUpdate();
    }

    void Explosion_128::Render()
    {
        GameObject::Render();
    }

    void Explosion_128::EndAnimation()
    {
        if (!IsDead())
        {
            DeleteObject(this, eLayerType::Effect);

            int rand = randi(12) + 12;
            for (int i = 0; i < rand; i++)
            {
                Smoke_Orange* smoke = ObjectPoolManager::GetObjectInPool<Smoke_Orange>();
                smoke->SetPosition(GetComponent<Transform>()->GetPosition());
            }
        }

    }
}