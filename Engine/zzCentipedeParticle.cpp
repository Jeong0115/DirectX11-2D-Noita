#include "zzCentipedeParticle.h"
#include "zzParticleSystem.h"
#include "zzResourceManager.h"
#include "zzTime.h"
#include "zzTransform.h"
#include "zzLight.h"
#include "zzAudioSource.h"
#include "zzCollider.h"

namespace zz
{
    CentipedeParticle::CentipedeParticle()
        : mShareData{}
        , mParticle(nullptr)
        , mbCreate(false)
        , mTime(0.f)
        , mAudio(nullptr)
    {
    }
    CentipedeParticle::~CentipedeParticle()
    {
    }

    void CentipedeParticle::Initialize()
    {
        AddComponent<Collider>()->SetScale(180.f, 180.f, 1.0f);
        mParticle = AddComponent<ParticleSystem>();
        mParticle->SetMaterial(ResourceManager::Find<Material>(L"m_ParticleCirceShader"));
        mParticle->SetMesh(ResourceManager::Find<Mesh>(L"PointMesh"));
        mParticle->SetParticleShader(ResourceManager::Find<ParticleShader>(L"ParticleMakeCircleCS"));
        mParticle->SetActive(false);

        auto particles = std::make_shared<Particle[]>(32400);
        mParticle->CreateStructedBuffer(sizeof(Particle), 32400, eViewType::UAV, particles.get(), true, 0, 14, 0);
        mParticle->CreateStructedBuffer(sizeof(ParticleCircleShared), 1, eViewType::UAV, nullptr, true, 6, 14, 1);

        mShareData.scale = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
        mShareData.color_min = mShareData.color_max = Vector4(47.f / 255.f, 93.0f / 255.f, 100.f / 255.f, 1.0f);
        mShareData.randVelocityMax = Vector2(15.0f, 15.0f);
        mShareData.randVelocityMin = Vector2(-15.0f, -15.0f);
        mShareData.randLifeTime = Vector2(4.5f, 2.0f);
        mShareData.radius = 90.f;
        mShareData.createTime = 0.5f;
        mShareData.particleCnt = 32400;
        mShareData.createRate = 0.3f;
        mShareData.lightScale = Vector4(8.0f, 8.0f, 1.0f, 0.0f);

        MeshRenderer* particleLight = new MeshRenderer();
        particleLight->SetMaterial(ResourceManager::Find<Material>(L"m_explosion_stain_particleLight"));
        particleLight->SetMesh(ResourceManager::Find<Mesh>(L"PointMesh"));
        mParticle->SetParticleLight(particleLight);

        mAudio = AddComponent<AudioSource>();
        mAudio->SetClip(ResourceManager::LoadAudioClip(L"centipede_shoot_clean_01", L"..\\Resources\\Audio\\Enemy\\centipede_shoot_clean_01.wav"));
        mAudio->SetLoop(false);
        GameObject::Initialize();
    }

    void CentipedeParticle::Update()
    {
        GameObject::Update();
    }
    void CentipedeParticle::LateUpdate()
    {
        if (mbCreate)
        {
            mTime += (float)Time::DeltaTime();
            mParticle->SetStructedBufferData(&mShareData, 1, 1);
            mShareData.create = false;
        }

        if (mTime >= 5.0f)
        {
            mbCreate = false;
            mTime = 0.f;
            mParticle->SetActive(false);
            SetState(eState::LastUpdate);
        }

        GameObject::LateUpdate();
    }
    void CentipedeParticle::Render()
    {
        GameObject::Render();
    }

    void CentipedeParticle::Play()
    {
        if (mTime > 0.f)
        {
            return;
        }
        mAudio->Play();
        SetState(eState::Active);
        mParticle->SetActive(true);
        mShareData.create = true;
        mShareData.curPosition = GetComponent<Transform>()->GetPosition() + 0.0f;
        mbCreate = true;
    }
}