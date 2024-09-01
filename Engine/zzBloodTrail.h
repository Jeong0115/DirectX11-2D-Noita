#pragma once

#include "zzModifierSpell.h"

namespace zz
{
    class ProjectileSpell;
    class BloodTrail : public ModifierSpell
    {
    public:
        BloodTrail();
        virtual ~BloodTrail();

        virtual void ModifierProjectile(ProjectileSpell* projectile) override;
    };
}

