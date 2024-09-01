#include "zzSpellUI.h"
#include "zzCollider.h"
#include "zzTextBox.h"

namespace zz
{
    SpellUI::SpellUI()
        : UI(eUIType::Spell)
        , mSpell(nullptr)
        , mTextBox(nullptr)
    {
    }

    SpellUI::~SpellUI()
    {
        if (mSpell != nullptr)
        {
            delete mSpell;
        }
        delete mTextBox;
    }

    void SpellUI::Initialize()
    {
        AddComponent<Collider>()->SetScale(10.f, 10.f, 1.0f);

        GameObject::Initialize();
    }

    void SpellUI::Update()
    {
        GameObject::Update();
    }

    void SpellUI::LateUpdate()
    {
        GameObject::LateUpdate();
    }

    void SpellUI::Render()
    {
        GameObject::Render();
    }

    void SpellUI::OnCollisionEnter(GameObject* other)
    {
    }
    void SpellUI::OnCollisionStay(GameObject* other)
    {
    }
    void SpellUI::OnCollisionExit(GameObject* other)
    {
    }
}