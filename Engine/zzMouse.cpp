#include "zzMouse.h"
#include "zzCollider.h"
#include "zzMeshRenderer.h"
#include "zzResourceManager.h"
#include "zzInput.h"
#include "zzMaterial.h"
#include "zzItemTexture.h"
#include "zzItemSlot.h"
#include "zzSpellSlot.h"

#include "zzUIManager.h"

namespace zz
{
    Mouse::Mouse(eUIType type)
        : UI(type)
        , mControllUI(nullptr)
    {
    }
    Mouse::~Mouse()
    {
    }
    void Mouse::Initialize()
    {
        MeshRenderer* mesh = AddComponent<MeshRenderer>();
        mesh->SetMaterial(ResourceManager::Find<Material>(L"m_mouse_cursor"));
        mesh->SetMesh(ResourceManager::Find<Mesh>(L"RectMesh"));

        Vector3 pos = Input::GetMouseUIPos();
        GetComponent<Transform>()->SetPosition(pos.x, pos.y, 0.0f);
        GetComponent<Transform>()->SetScale(13.f, 13.f, 1.f);

        Collider* collider = AddComponent<Collider>();
        collider->SetScale(Vector3(1.f, 1.f, 1.f));

        GameObject::Initialize();
    }

    void Mouse::Update()
    {
        Vector3 pos = Input::GetMouseUIPos();
        GetComponent<Transform>()->SetPosition(pos.x, pos.y, 0.0f);

        if (mControllUI != nullptr)
        {
            if (Input::GetKeyUp(eKeyCode::LBUTTON))
            {
                mControllUI->MoveOriginSlot();
                mControllUI = nullptr;
            }
            else
            {
                mControllUI->GetComponent<Transform>()->SetPosition(pos.x + 10.f, pos.y - 10.f, 0.0f);
            }
        }

        GameObject::Update();
    }

    void Mouse::LateUpdate()
    {
        GameObject::LateUpdate();
    }
    void Mouse::Render()
    {
        GameObject::Render();
    }

    void Mouse::OnCollisionEnter(GameObject* other)
    {
    }

    void Mouse::OnCollisionStay(GameObject* other)
    {
        UI* uiObject = dynamic_cast<UI*>(other);

        if(uiObject != nullptr)
        {
            if (uiObject->GetUIType() == eUIType::ItemSlot && Input::GetKeyDown(eKeyCode::LBUTTON))
            {       
                mControllUI = dynamic_cast<ItemSlot*>(uiObject)->GetItemTexture();
            }
            else if (uiObject->GetUIType() == eUIType::ItemSlot && Input::GetKeyUp(eKeyCode::LBUTTON))
            {
                if (mControllUI != nullptr && mControllUI->GetTextureType() == eTextureType::Wand)
                {
                    UINT prevSlotIndex = mControllUI->GetSlotIndex();
                    UINT moveSlotIndex = dynamic_cast<ItemSlot*>(uiObject)->GetSlotIndex();

                    UIManager::MoveItemToSlot(prevSlotIndex, moveSlotIndex);

                    mControllUI = nullptr;
                }
            }

            if ((uiObject->GetUIType() == eUIType::SpellSlot || uiObject->GetUIType() == eUIType::WandSlot) && Input::GetKeyDown(eKeyCode::LBUTTON))
            {
                mControllUI = dynamic_cast<SpellSlot*>(uiObject)->GetItemTexture();
                if (mControllUI != nullptr)
                {
                    mControllUI->DisconnectParent();
                }
            }
            else if ((uiObject->GetUIType() == eUIType::SpellSlot || uiObject->GetUIType() == eUIType::WandSlot) && Input::GetKeyUp(eKeyCode::LBUTTON))
            {
                if (mControllUI != nullptr && mControllUI->GetTextureType() == eTextureType::Spell)
                {
                    SpellSlot* srcSlot = mControllUI->GetOwner();
                    SpellSlot* dstSlot = dynamic_cast<SpellSlot*>(uiObject);

                    ItemTexture* srcItemTexture = srcSlot->GetItemTexture();
                    SpellUI* srcSpell = srcSlot->GetSpell();

                    if (dstSlot->GetItemTexture())
                    {
                        dstSlot->GetItemTexture()->DisconnectParent();
                    }
                    srcSlot->SetSpell(dstSlot->GetSpell(), dstSlot->GetItemTexture());
                    dstSlot->SetSpell(srcSpell, srcItemTexture);

                    mControllUI = nullptr;
                }
            }
        }
    }

    void Mouse::OnCollisionExit(GameObject* other)
    {
    }
}