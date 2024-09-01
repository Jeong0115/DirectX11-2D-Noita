#pragma once

#include "zzSpell.h"

namespace zz
{
    enum class eUtilityType
    {
        MultiCast,
        Modifier,
        None
    };

    class UtilitySpell : public Spell
    {
    public:
        UtilitySpell();
        ~UtilitySpell();

        virtual void Initialize();
        virtual void Update();
        virtual void LateUpdate();
        virtual void Render();

        eUtilityType GetUtilityType() { return mUtilityType; }

    protected:
        eUtilityType mUtilityType;
    };
}

