#include "zzModifierSpell.h"
#include "zzProjectileSpell.h"

namespace zz
{
    ModifierSpell::ModifierSpell()
    {
        mUtilityType = eUtilityType::Modifier;
    }

    ModifierSpell::~ModifierSpell()
    {
    }

    void ModifierSpell::ModifierProjectile(ProjectileSpell* projectile)
    {
    }
}