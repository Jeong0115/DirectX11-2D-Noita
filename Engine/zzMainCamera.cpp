#include "zzMainCamera.h"
#include "zzTransform.h"
#include "zzGameObject.h"
#include "zzRenderer.h"
#include "zzScene.h"
#include "zzSceneManager.h"
#include "zzMeshRenderer.h"

#include "zzGraphicsDevice.h"
#include "zzResourceManager.h"
#include "zzApplication.h"
#include "zzBloomManager.h"
#include "zzInput.h"
#include "zzTransform.h"
#include "zzPixelWorld.h"
#include "zzTime.h"
#include "zzDamaged.h"

namespace zz
{
    bool CompareZSort(GameObject* a, GameObject* b)
    {
        if (a->GetComponent<Transform>()->GetPosition().z
            <= b->GetComponent<Transform>()->GetPosition().z)
            return false;

        return true;
    }


    MainCamera::MainCamera()
        : mLayerMask{}
        , mOpaqueGameObjects{}
        , mCutOutGameObjects{}
        , mTransparentGameObjects{}
        , mTarget(nullptr)
        , mDamaged(nullptr)
    {
        EnableLayerMasks();
    }

    MainCamera::~MainCamera()
    {
        if (mDamaged)
        {
            delete mDamaged;
            mDamaged = nullptr;
        }
    }

    void MainCamera::Initialize()
    {
        mDamaged = new Damaged();
        mDamaged->Initialize();
    }

    void MainCamera::Update()
    {
    }

    void MainCamera::LateUpdate()
    {
        CreateViewMatrix();
        CreateProjectionMatrix(mType);
        RegisterCameraInRenderer();
    }

    void MainCamera::Render()
    {
        View = mView;
        Projection = mProjection;

        DisableDepthStencilState();
        RenderPlayerView();

        graphics::GetDevice()->ClearRenderTarget();
        SortGameObjects();
        RenderOpaque();

        DisableDepthStencilState();
        RenderCutOut();     
        RenderTransparent();
        EnableDepthStencilState();

        if (Application::LightDisabled) return;

        DisableDepthStencilState();
        ResourceManager::Find<Mesh>(L"LightMesh")->BindBuffer();
        ResourceManager::Find<Shader>(L"AddDarkShader")->BindShaders();

        ID3D11ShaderResourceView* lightSrv = graphics::GetDevice()->GetPlayerVisibilityResource();
        ID3D11ShaderResourceView* null = nullptr;

        graphics::GetDevice()->BindShaderResource(eShaderStage::PS, 2, &lightSrv);
        graphics::GetDevice()->DrawIndexed(6, 0, 0);
        graphics::GetDevice()->BindShaderResource(eShaderStage::PS, 2, &null);

        ResourceManager::Find<Mesh>(L"LightMesh")->BindBuffer();
        ResourceManager::Find<Shader>(L"BloomShader")->BindShaders();

        BloomManager::Render();      

        Vector3 targetPos = mTarget->GetComponent<Transform>()->GetPosition();
        Vector3 curPos = GetOwner()->GetComponent<Transform>()->GetPosition();
        Vector3 mousePos = Input::GetMouseWorldPos();

        float disX = std::clamp(fabs(targetPos.x - mousePos.x), 1.f, 20.f);
        float disY = std::clamp(fabs(targetPos.y - mousePos.y), 1.f, 20.f);

        Vector3 cameraPos;
        cameraPos.x = std::lerp(curPos.x, mousePos.x, (20.f / disX) * 0.05f * (float)Time::DeltaTime());
        cameraPos.y = std::lerp(curPos.y, mousePos.y, (20.f / disY) * 0.05f * (float)Time::DeltaTime());

        float clampedX = std::clamp(cameraPos.x, targetPos.x - 10.f, targetPos.x + 10.f);
        float clampedY = std::clamp(cameraPos.y, targetPos.y - 10.f, targetPos.y + 10.f);

        GetOwner()->GetComponent<Transform>()->SetPositionXY(clampedX, clampedY);
        mDamaged->GetComponent<Transform>()->SetPosition(clampedX, clampedY, 0.0f);
        mDamaged->Update();
        mDamaged->LateUpdate();
        mDamaged->Render();
    }


    void MainCamera::TurnLayerMask(eLayerType type, bool enable)
    {
        mLayerMask.set((UINT)type, enable);
    }

    void MainCamera::SortGameObjects()
    {
        mOpaqueGameObjects.clear();
        mCutOutGameObjects.clear();  
        mTransparentGameObjects.clear(); 
        mTp.clear();

        Scene* scene = SceneManager::GetActiveScene();
        for (size_t i = 0; i < (UINT)eLayerType::End; i++)
        {
            if (mLayerMask[i] == true)
            {
                Layer& layer = scene->GetLayer((eLayerType)i);
                std::vector<GameObject*>& gameObjs = layer.GetGameObjects();
                auto iter = gameObjs.begin();

                for (; iter != gameObjs.end();)
                {
                    if ((*iter)->IsAllowDelete())
                    {
                        iter = gameObjs.erase(iter);
                        continue;
                    }
                    else if ((*iter)->GetActive())
                    {
                        MeshRenderer* mr = (*iter)->GetComponent<MeshRenderer>();
                        if (mr == nullptr)
                        {
                            iter++;
                            continue;
                        }

                        std::shared_ptr<Material> mt = mr->GetMaterial();
                        eRenderingMode mode = mt->GetRenderingMode();
                        switch (mode)
                        {
                        case eRenderingMode::Opaque:
                            mOpaqueGameObjects.push_back((*iter));
                            break;
                        case eRenderingMode::CutOut:
                            mCutOutGameObjects.push_back((*iter));
                            break;
                        case eRenderingMode::Transparent:
                            mTransparentGameObjects.push_back((*iter));
                            break;
                        default:
                            break;
                        }
                        iter++;
                    }
                    else
                    {
                        iter++;
                    }
                }
            }
        }

        std::sort(mCutOutGameObjects.begin(), mCutOutGameObjects.end(), CompareZSort);
        std::sort(mTransparentGameObjects.begin(), mTransparentGameObjects.end(), CompareZSort);
    }
    void MainCamera::RenderOpaque()
    {
        for (GameObject* gameObj : mOpaqueGameObjects)
        {
            if (gameObj == nullptr)
                continue;

            gameObj->Render();
        }
    }
    void MainCamera::RenderCutOut()
    {
        for (GameObject* gameObj : mCutOutGameObjects)
        {
            if (gameObj == nullptr)
                continue;

            gameObj->Render();
        }
    }
    void MainCamera::RenderTransparent()
    {
        for (GameObject* gameObj : mTransparentGameObjects)
        {
            if (gameObj == nullptr)
                continue;
                
            gameObj->Render();
        }
    }

    void MainCamera::RenderPlayerView()
    {
        graphics::GetDevice()->SetPlayerVisibilityRenderTarget();

        Vector3 pos = Vector3(PixelWorld::mWorldWidth / 2.f, -PixelWorld::mWorldHeight / 2.f, 0.0f);
        Matrix position = Matrix::CreateTranslation(pos);
        Matrix scale = Matrix::CreateScale(Vector3(PixelWorld::mWorldWidth, PixelWorld::mWorldHeight, 1.0f));
        Matrix world = scale * position;
        
        renderer::TransformCB trCB = {};
        
        trCB.mView = Camera::GetGpuViewMatrix();
        trCB.mProjection = Camera::GetGpuProjectionMatrix();
        trCB.WorldViewProj = world * trCB.mView * trCB.mProjection;
        trCB.mWorld = world;

        ConstantBuffer* cb = renderer::constantBuffer[(UINT)eCBType::Transform];
        cb->SetBufferData(&trCB);
        cb->BindConstantBuffer(eShaderStage::VS);
        cb->BindConstantBuffer(eShaderStage::PS);
        
        ResourceManager::Find<Mesh>(L"RectMesh")->BindBuffer();
        ResourceManager::Find<Shader>(L"ViewShader")->BindShaders();
        
        ID3D11ShaderResourceView* viewMapSrv = graphics::GetDevice()->GetViewMapResource();
        ID3D11ShaderResourceView* null = nullptr;

        graphics::GetDevice()->BindShaderResource(eShaderStage::PS, 2, &viewMapSrv);
        graphics::GetDevice()->DrawIndexed(6, 0, 0);
        graphics::GetDevice()->BindShaderResource(eShaderStage::PS, 2, &null);
    }
}