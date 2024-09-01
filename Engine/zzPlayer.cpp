#include "zzPlayer.h"
#include "zzTexture.h"
#include "zzAnimator.h"
#include "zzResourceManager.h"
#include "zzPixelWorld.h"
#include "zzInput.h"
#include "zzTime.h"
#include "zzTransform.h"
#include "zzRenderer.h"
#include "zzApplication.h"
#include "zzMeshRenderer.h"
#include "zzMaterial.h"
#include "zzUIManager.h"
#include "zzExplosion_128.h"
#include "zzCollider.h"
#include "zzPixelCollider.h"
#include "zzLevitation.h"
#include "zzRigidBody.h"
#include "zzLight.h"
#include "zzDetectPlayer.h"
#include "zzEventManager.h"
#include "zzHealthPoint.h"
#include "zzAudioListener.h"
#include "zzGraphicsDevice.h"
#include "zzPlayerView.h"

#include "zzAudioSource.h"
#include "zzAudioClip.h"

using namespace zz::graphics;
namespace zz
{
    Player::Player()
        : mEquipment(nullptr)
        , mCamera(nullptr)
        , mAnimator(nullptr)
        , vel(0.f)
        , mEquipmentInitialPos(Vector3::Zero)
        , mPlayerArm(nullptr)
        , mRigid(nullptr)
    {
    }

    Player::~Player()
    {
        if (mPlayerArm != nullptr)
        {
            delete mPlayerArm;
            mPlayerArm = nullptr;
        }
        delete mPlayerView;
        mPlayerView = nullptr;
    }

    void Player::Initialize()
    {
        Input::SetPlayer(GetComponent<Transform>());

        std::shared_ptr<Texture> texture
            = ResourceManager::Load<Texture>(L"Player", L"..\\Resources\\Texture\\Player\\Player.png");

        MeshRenderer* mesh = AddComponent<MeshRenderer>();
        mesh->SetMaterial(ResourceManager::Find<Material>(L"m_SpriteAnimation"));
        mesh->SetMesh(ResourceManager::Find<Mesh>(L"RectMesh"));


        AddComponent<Levitation>()->SetMaxEnergy(3.0f);
        AddComponent<AudioListener>();
        PixelCollider* pxCollier = AddComponent<PixelCollider>();
        pxCollier->SetCollision(Vector3(0.0f, -4.0f, 0.0f), Vector3(6.f, 8.f, 0.0f));
        pxCollier->SetClimbY(3.0f);

        mPlayerView = new PlayerView();
        mPlayerView->SetPlayer(this);
        mPlayerView->Initialize();

        mRigid = AddComponent<RigidBody>();
        // mRigid->SetGravity(0.f);
        AddComponent<Collider>()->SetScale(6.0f, 10.f, 1.0f);

        mAnimator = AddComponent<Animator>();

        mAnimator->Create(L"Player_Idle", texture, Vector2(0.0f, 1.0f), Vector2(12.0f, 19.0f), 6, Vector2::Zero, 0.1f);
        mAnimator->Create(L"Player_Walk", texture, Vector2(0.0f, 21.f), Vector2(12.0f, 19.0f), 6, Vector2::Zero, 0.105f);
        mAnimator->Create(L"Player_Jump_Up", texture, Vector2(0.0f, 41.f), Vector2(12.0f, 19.0f), 3, Vector2::Zero, 0.082f);
        mAnimator->Create(L"Player_Jump_Fall", texture, Vector2(0.0f, 61.f), Vector2(12.0f, 19.0f), 3, Vector2::Zero, 0.082f);
        mAnimator->Create(L"Player_Land", texture, Vector2(0.0f, 81.f), Vector2(12.0f, 19.0f), 3, Vector2::Zero, 0.075f);

        mAnimator->PlayAnimation(L"Player_Idle", true);     

        mPlayerArm->Initialize();

        mHealthPoint = AddComponent<HealthPoint>();
		mHealthPoint->SetMaxHP(100.f);
		mHealthPoint->SetHitEvent([this]() { HitEvent(); });
        mHealthPoint->SetHpZeroEvent([this]() { DeathEvent(); });
        GameObject::Initialize();
    }

    void Player::Update()
    {
        Transform* tr = GetComponent<Transform>();
        Vector3 pos = tr->GetPosition();

        if (Input::GetKeyDown(eKeyCode::A))
        {
            mAnimator->PlayAnimation(L"Player_Walk", true);
        }
        if (Input::GetKey(eKeyCode::A))
        {
            mRigid->SetVelocityX(-50.f);
        }
        else if (Input::GetKeyUp(eKeyCode::A))
        {
            mAnimator->PlayAnimation(L"Player_Idle", true);
            mRigid->SetVelocityX(0.0f);
        }
        if (Input::GetKeyDown(eKeyCode::A))
        {
            mAnimator->PlayAnimation(L"Player_Walk", true);
        }
        if (Input::GetKey(eKeyCode::D))
        {
            mRigid->SetVelocityX(50.f);
        }
        else if (Input::GetKeyUp(eKeyCode::D))
        {
            mAnimator->PlayAnimation(L"Player_Idle", true);
            mRigid->SetVelocityX(0.0f);
        }
        

        //if (Input::GetKey(eKeyCode::D))
        //{
        //    pos.x += Time::DeltaTime() * 100.f;
        //}
        //if (Input::GetKey(eKeyCode::A))
        //{
        //    pos.x -= Time::DeltaTime() * 100.f;
        //}
        //if (Input::GetKey(eKeyCode::W))
        //{
        //    pos.y += Time::DeltaTime() * 100.f;
        //}
        //if (Input::GetKey(eKeyCode::S))
        //{
        //    pos.y -= Time::DeltaTime() * 100.f;
        //}
        tr->SetPosition(pos);

        DetectPlayer::PlayerPos = pos;

        GameObject::Update();

        mPlayerArm->Update();

        if(mEquipment != nullptr)
        {
            mEquipment->Update();
            if (Input::GetKey(eKeyCode::LBUTTON) || Input::GetKeyDown(eKeyCode::LBUTTON))
            {
                mEquipment->UseEquipment();
            }

            EvenetData data;
            data.eventType = eEvent::Mana_Change;
            data.mana = mEquipment->GetEquipmentRate();

            EventManager::RegisterEvent(data);
            
        }
    }

    void Player::LateUpdate()
    {
        if (mCamera != nullptr)
        {
            mCamera->GetComponent<Transform>()->SetPosition(GetComponent<Transform>()->GetPosition());
        }

        GameObject::LateUpdate();
        mPlayerArm->LateUpdate();
        if (mEquipment != nullptr)
            mEquipment->LateUpdate();

        mPlayerView->LateUpdate();
    }

    void Player::Render()
    {

        renderer::FlipCB flipCB = {};

        flipCB.flip.x = Input::IsFlip();
        
        ConstantBuffer* cb = renderer::constantBuffer[(UINT)eCBType::Flip];
        cb->SetBufferData(&flipCB);
        cb->BindConstantBuffer(eShaderStage::PS);

        GameObject::Render();

        flipCB.flip.y = flipCB.flip.x;

        cb->SetBufferData(&flipCB);
        cb->BindConstantBuffer(eShaderStage::PS);

        mPlayerArm->Render();

        flipCB = {};
        cb->SetBufferData(&flipCB);
        cb->BindConstantBuffer(eShaderStage::PS);

        
        if (mEquipment != nullptr)
        {
            Transform* equipmentTr = mEquipment->GetComponent<Transform>();

            if (Input::IsFlip() == 1)
            {
                equipmentTr->SetPosition(mEquipmentInitialPos.x, mEquipmentInitialPos.y, mEquipmentInitialPos.z);
            }
            else
            {
                equipmentTr->SetPosition(mEquipmentInitialPos.x, -mEquipmentInitialPos.y, mEquipmentInitialPos.z);
            }

            mEquipment->Render();
        }
    }
     
    void Player::OnCollisionEnter(GameObject* other)
    {
    }
    void Player::OnCollisionStay(GameObject* other)
    {
    }
    void Player::OnCollisionExit(GameObject* other)
    {
    }

	void Player::HitEvent()
	{
		EvenetData data;
		data.eventType = eEvent::Health_Change;

		data.health = mHealthPoint->GetCurHP() / mHealthPoint->GetMaxHP();

		EventManager::RegisterEvent(data);
	}

    void Player::DeathEvent()
    {
        mHealthPoint->ChangeCurHP(mHealthPoint->GetMaxHP());

        EvenetData data;
        data.eventType = eEvent::Health_Change;

        data.health = 1.0f;

        EventManager::RegisterEvent(data);
    }

}