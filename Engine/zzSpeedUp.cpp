#include "zzSpeedUp.h"
#include "zzProjectileSpell.h"

namespace zz
{
    SpeedUp::SpeedUp()
    {
    }
    SpeedUp::~SpeedUp()
    {
    }

    void SpeedUp::ModifierProjectile(ProjectileSpell* projectile)
    {
        projectile->SetSpeedFactor(2);
    }
}