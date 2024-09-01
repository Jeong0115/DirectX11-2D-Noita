#include "zzMaterial.h"

namespace zz
{
    Material::Material()
        : Resource(eResourceType::Material)
        , mShader(nullptr)
        , mTexture(nullptr)
        , mMode(eRenderingMode::Transparent)
    {
    }

    Material::~Material()
    {
    }

    HRESULT Material::Load(const std::wstring& path)
    {
        return E_NOTIMPL;
    }

    void Material::Binds()
    {
        if(mTexture != nullptr)
            mTexture->BindShader(eShaderStage::PS, 0); // ���߿� �Լ� �̸� �ٲ���

        mShader->BindShaders();
    }

    void Material::Clear()
    {
        if (mTexture != nullptr)
            mTexture->Clear();
    }
}