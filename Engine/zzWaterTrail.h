#pragma once

#include "zzModifierSpell.h"

namespace zz
{
    class ProjectileSpell;
    class WaterTrail : public ModifierSpell
    {
    public:
        WaterTrail();
        virtual ~WaterTrail();

        virtual void ModifierProjectile(ProjectileSpell* projectile) override;
    };
}

