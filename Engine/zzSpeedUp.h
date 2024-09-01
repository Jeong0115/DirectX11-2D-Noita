#pragma once

#include "zzModifierSpell.h"

namespace zz
{
    class ProjectileSpell;
    class SpeedUp : public ModifierSpell
    {
    public:
        SpeedUp();
        virtual ~SpeedUp();

        virtual void ModifierProjectile(ProjectileSpell* projectile) override;
    };
}

