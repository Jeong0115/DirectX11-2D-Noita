#include "zzWaterTrail.h"
#include "zzProjectileSpell.h"
#include "zzElement.h"
namespace zz
{
    WaterTrail::WaterTrail()
    {
    }
    WaterTrail::~WaterTrail()
    {
    }

    void WaterTrail::ModifierProjectile(ProjectileSpell* projectile)
    {
        projectile->AddTrailElement().push_back(WATER);

    }
}