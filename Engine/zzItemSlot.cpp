#include "zzItemSlot.h"
#include "zzResourceManager.h"
#include "zzMeshRenderer.h"
#include "zzMaterial.h"
#include "zzTransform.h"
#include "zzCollider.h"
#include "zzUIManager.h"

namespace zz
{
    ItemSlot::ItemSlot(eUIType type)
        : UI(type)
        , mSlotIndex(9)
        , mItem(nullptr)
        , mItemTexture(nullptr)
        , mTextBox(nullptr)
        , mbMouseOn(false)
    {
    }

    ItemSlot::~ItemSlot()
    {
        if (mItem != nullptr)
        {
            delete mItem;
            mItem = nullptr;
        }

        if (mItemTexture != nullptr)
        {
            delete mItemTexture;
            mItemTexture = nullptr;
        }

        mTextBox = nullptr;
    }

    void ItemSlot::Initialize()
    {
        MeshRenderer* mesh = AddComponent<MeshRenderer>();
        mesh->SetMaterial(ResourceManager::Find<Material>(L"m_inventory_box"));
        mesh->SetMesh(ResourceManager::Find<Mesh>(L"RectMesh"));

        GetComponent<Transform>()->SetScale(Vector3(20.f, 20.f, 1.0f));

        Collider* collider = AddComponent<Collider>();
        collider->SetScale(Vector3(19.f, 19.f, 1.0f));

        GameObject::Initialize();
    }
    void ItemSlot::Update()
    {
        GameObject::Update();

        if (mItemTexture != nullptr)
        {
            mItemTexture->Update();

            if (mbMouseOn)
            {
                UIManager::RegisterRenderTextBox(mTextBox);
            }
        }
    }

    void ItemSlot::LateUpdate()
    {
        GameObject::LateUpdate();

        if (mItemTexture != nullptr)
        {
            mItemTexture->LateUpdate();
        }
    }

    void ItemSlot::Render()
    {
        GameObject::Render();

        if (mItemTexture != nullptr)
        {
            mItemTexture->Render();

        } 
    }

    void ItemSlot::OnCollisionEnter(GameObject* other)
    {
    }

    void ItemSlot::OnCollisionStay(GameObject* other)
    {
        if (dynamic_cast<UI*>(other)->GetUIType() == eUIType::Mouse)
        {
            MeshRenderer* mesh = GetComponent<MeshRenderer>();
            mesh->SetMaterial(ResourceManager::Find<Material>(L"m_inventory_box_highlihgt"));
            mbMouseOn = true;
        }
    }

    void ItemSlot::OnCollisionExit(GameObject* other)
    {
        if (dynamic_cast<UI*>(other)->GetUIType() == eUIType::Mouse)
        {
            MeshRenderer* mesh = GetComponent<MeshRenderer>();
            mesh->SetMaterial(ResourceManager::Find<Material>(L"m_inventory_box"));
            mbMouseOn = false;
        }
    }

    void ItemSlot::SetItem(Equipment* item, ItemTexture* tex)
    {
        mItem = item; 
        mItemTexture = tex;

        if(mItemTexture != nullptr)
        {
            Vector3 pos = GetComponent<Transform>()->GetWorldPosition();

            mItemTexture->MoveSlot(pos);
            mItemTexture->SetSlotIndex(mSlotIndex);

            mTextBox = mItem->GetTextBox();

            Transform* texTr = mTextBox->GetComponent<Transform>();
            Vector3 scale = texTr->GetScale();

            if (pos.x - scale.x / 2 - 15.f <= 0.f)
            {
                pos.x -= pos.x - scale.x / 2 - 15.f;
            }

            texTr->SetPosition(pos.x, pos.y - scale.y / 2 - 15.f, 0.0f);
        }
        else
        {
            mTextBox = nullptr;
        }
    }
}