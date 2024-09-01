#include "zzGraphicsDevice.h"
#include "zzApplication.h"
#include "zzRenderer.h"

#include "zzInput.h"
#include "zzTime.h"

using namespace zz::renderer;
namespace zz::graphics
{
	GraphicsDevice::GraphicsDevice()
	{
        Application& application = Application::GetInst();

        HWND hWnd = application.GetHwnd();
        UINT deviceFlag = D3D11_CREATE_DEVICE_DEBUG;
        D3D_FEATURE_LEVEL featureLevel = (D3D_FEATURE_LEVEL)0;

        D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr
            , deviceFlag, nullptr, 0
            , D3D11_SDK_VERSION
            , mDevice.GetAddressOf(), &featureLevel
            , mContext.GetAddressOf());

        // SwapChain
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        swapChainDesc.BufferCount = 2;
        swapChainDesc.BufferDesc.Width = application.GetWidth();
        swapChainDesc.BufferDesc.Height = application.GetHeight();

        if (!CreateSwapChain(&swapChainDesc, hWnd))
            return;

        // get rendertarget by swapchain
        if (FAILED(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D)
            , (void**)mRenderTarget.GetAddressOf())))
            return;

        // create rendertarget view

        mDevice->CreateRenderTargetView((ID3D11Resource*)mRenderTarget.Get()
            , nullptr, mRenderTargetView.GetAddressOf());

        D3D11_TEXTURE2D_DESC depthStencilDesc = {};
        depthStencilDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
        depthStencilDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
        depthStencilDesc.CPUAccessFlags = 0;
        depthStencilDesc.Width = application.GetWidth();
        depthStencilDesc.Height = application.GetHeight();
        depthStencilDesc.ArraySize = 1;
        depthStencilDesc.SampleDesc.Count = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
        depthStencilDesc.MiscFlags = 0;
        depthStencilDesc.MipLevels = 0;

        D3D11_SUBRESOURCE_DATA data;

        if (!CreateTexture(&depthStencilDesc, &data))
            return;

        RECT winRect = {};
        GetClientRect(hWnd, &winRect);
        mViewPort =
        {
            0.0f, 0.0f
            , (float)(winRect.right - winRect.left)
            , (float)(winRect.bottom - winRect.top)
            , 0.0f, 1.0f
        };

        //CreatePixelGrid();
        CreateLightMap();
        CreateBloomRenderTarget();
        CreateFinalLightTarget();

        BindViewPort(&mViewPort);
        mContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());
	}

	GraphicsDevice::~GraphicsDevice()
	{
	}

    void GraphicsDevice::CreatePixelGrid()
    {
        //D3D11_TEXTURE2D_DESC desc;
        //ZeroMemory(&desc, sizeof(desc));
        //desc.Width = 500;  // Width of the texture
        //desc.Height = 500; // Height of the texture
        //desc.MipLevels = 1;
        //desc.ArraySize = 1;
        //desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Pixel format
        //desc.SampleDesc.Count = 1;
        //desc.Usage = D3D11_USAGE_DEFAULT;
        //desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;// D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        //// Create the texture
        //ID3D11Texture2D* texture = nullptr;
        //mDevice->CreateTexture2D(&desc, nullptr, &texture);

        //// Create a render target view
        //ID3D11RenderTargetView* rtv = nullptr;
        //mDevice->CreateRenderTargetView(texture, nullptr, &rtv);

        //// Bind the render target view
        //mContext->OMSetRenderTargets(1, &rtv, nullptr);
    }

	bool GraphicsDevice::CreateSwapChain(const DXGI_SWAP_CHAIN_DESC* desc, HWND hWnd)
	{
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

        swapChainDesc.OutputWindow = hWnd;
        swapChainDesc.Windowed = true;
        swapChainDesc.BufferCount = desc->BufferCount;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;

        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferDesc.Width = desc->BufferDesc.Width;
        swapChainDesc.BufferDesc.Height = desc->BufferDesc.Height;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 240;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
        swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;

        Microsoft::WRL::ComPtr<IDXGIDevice> pDXGIDevice = nullptr;
        Microsoft::WRL::ComPtr<IDXGIAdapter> pAdapter = nullptr;
        Microsoft::WRL::ComPtr<IDXGIFactory> pFactory = nullptr;

        if (FAILED(mDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)pDXGIDevice.GetAddressOf())))
            return false;

        if (FAILED(pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)pAdapter.GetAddressOf())))
            return false;

        if (FAILED(pAdapter->GetParent(__uuidof(IDXGIFactory), (void**)pFactory.GetAddressOf())))
            return false;

        if (FAILED(pFactory->CreateSwapChain(mDevice.Get(), &swapChainDesc, mSwapChain.GetAddressOf())))
            return false;

        return true;
	}

    bool GraphicsDevice::CreateTexture(const D3D11_TEXTURE2D_DESC* desc, void* data)
    {
        D3D11_TEXTURE2D_DESC dxgiDesc = {};
        dxgiDesc.BindFlags = desc->BindFlags;
        dxgiDesc.Usage = desc->Usage;
        dxgiDesc.CPUAccessFlags = 0;

        dxgiDesc.Format = desc->Format;
        dxgiDesc.Width = desc->Width;
        dxgiDesc.Height = desc->Height;
        dxgiDesc.ArraySize = desc->ArraySize;

        dxgiDesc.SampleDesc.Count = desc->SampleDesc.Count;
        dxgiDesc.SampleDesc.Quality = 0;

        dxgiDesc.MipLevels = desc->MipLevels;
        dxgiDesc.MiscFlags = desc->MiscFlags;

        if (FAILED(mDevice->CreateTexture2D(&dxgiDesc, nullptr, mDepthStencilBuffer.ReleaseAndGetAddressOf())))
            return false;

        if (FAILED(mDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), nullptr, mDepthStencilView.GetAddressOf())))
            return false;

        return true;
    }

    bool GraphicsDevice::CreateTexture2d(const D3D11_TEXTURE2D_DESC* pDesc, ID3D11Texture2D** ppTexture2D)
    {
        if (FAILED(mDevice->CreateTexture2D(pDesc, nullptr, ppTexture2D)))
            return false;
        return true;    
    }

    bool GraphicsDevice::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs, UINT NumElements, ID3DBlob* byteCode, ID3D11InputLayout** ppInputLayout)
    {
        if (FAILED(mDevice->CreateInputLayout(pInputElementDescs, NumElements 
            , byteCode->GetBufferPointer(), byteCode->GetBufferSize(), ppInputLayout)))
            return false;

        return true;
    }

    bool GraphicsDevice::CreateBuffer(ID3D11Buffer** buffer, D3D11_BUFFER_DESC* bufferDesc, D3D11_SUBRESOURCE_DATA* data)
    {
        if (FAILED(mDevice->CreateBuffer(bufferDesc, data, buffer)))
            return false;

        return true;    
    }

    bool GraphicsDevice::CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState)
    {
        if (FAILED(mDevice->CreateSamplerState(pSamplerDesc, ppSamplerState)))
            return false;

        return false;
    }

    bool GraphicsDevice::CreateRasterizeState(const D3D11_RASTERIZER_DESC* pRasterizerDesc, ID3D11RasterizerState** ppRasterizerState)
    {
        if (FAILED(mDevice->CreateRasterizerState(pRasterizerDesc, ppRasterizerState)))
            return false;

        return true;
    }

    bool GraphicsDevice::CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState)
    {
        if (FAILED(mDevice->CreateDepthStencilState(pDepthStencilDesc, ppDepthStencilState)))
            return false;

        return true;
    }

    bool GraphicsDevice::CreateBlendState(const D3D11_BLEND_DESC* pBlendStateDesc, ID3D11BlendState** ppBlendState)
    {
        if (FAILED(mDevice->CreateBlendState(pBlendStateDesc, ppBlendState)))
            return false;

        return true;
    }

    bool GraphicsDevice::CreateShaderResourceView(ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D11ShaderResourceView** ppSRView)
    {
        if (FAILED(mDevice->CreateShaderResourceView(pResource, pDesc, ppSRView)))
            return false;

        return true;
    }

    bool GraphicsDevice::CreateRenderTargetView(ID3D11Resource* pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc, ID3D11RenderTargetView** ppRTView)
    {
        if (FAILED(mDevice->CreateRenderTargetView(pResource, pDesc, ppRTView)))
            return false;

        return true;
    }

    bool GraphicsDevice::CreateUnorderedAccessView(ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAView)
    {
        if (FAILED(mDevice->CreateUnorderedAccessView(pResource, pDesc, ppUAView)))
            return false;

        return true;
    }
    
    bool GraphicsDevice::CompileFromfile(const std::wstring& fileName, const std::string& funcName, const std::string& version, ID3DBlob** ppCode)
    {
        ID3DBlob* errorBlob = nullptr;
        D3DCompileFromFile(fileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, funcName.c_str(), version.c_str(), 0, 0, ppCode, &errorBlob);
    
        if (errorBlob)
        {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
            errorBlob = nullptr;

            return false;
        }

        return true;
    }

    bool GraphicsDevice::CreateVertexShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11VertexShader** ppVertexShader)
    {
        if (FAILED(mDevice->CreateVertexShader(pShaderBytecode, BytecodeLength, nullptr, ppVertexShader)))
        {
            return false;
        }

        return true;
    }

    bool GraphicsDevice::CreateGeometryShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11GeometryShader** ppGeometryShader)
    {
        if (FAILED(mDevice->CreateGeometryShader(pShaderBytecode, BytecodeLength, nullptr, ppGeometryShader)))
            return false;

        return true;
    }

    bool GraphicsDevice::CreatePixelShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11PixelShader** ppPixelShader)
    {
        if (FAILED(mDevice->CreatePixelShader(pShaderBytecode, BytecodeLength, nullptr, ppPixelShader)))
            return false;

        return true;
    }

    bool GraphicsDevice::CreateComputeShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ComputeShader** ppComputeShader)
    {
        if (FAILED(mDevice->CreateComputeShader(pShaderBytecode, BytecodeLength, nullptr, ppComputeShader)))
            return false;

        return true;
    }

    void GraphicsDevice::BindInputLayout(ID3D11InputLayout* pInputLayout)
    {
        mContext->IASetInputLayout(pInputLayout);
    }

    void GraphicsDevice::BindPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology)
    {
        mContext->IASetPrimitiveTopology(Topology);
    }  

    void GraphicsDevice::BindVertexBuffer(UINT StartSlot, ID3D11Buffer* const* ppVertexBuffers, const UINT* pStrides, const UINT* pOffsets)
    {
        mContext->IASetVertexBuffers(StartSlot, 1, ppVertexBuffers, pStrides, pOffsets);
    }

    void GraphicsDevice::BindIndexBuffer(ID3D11Buffer* pIndexBuffer, DXGI_FORMAT Format, UINT Offset)
    {
        mContext->IASetIndexBuffer(pIndexBuffer, Format, Offset);
    }

    void GraphicsDevice::BindVertexShader(ID3D11VertexShader* pVetexShader)
    {
        mContext->VSSetShader(pVetexShader, 0, 0);
    }

    void GraphicsDevice::BindHullShader(ID3D11HullShader* pHullShader)
    {
        mContext->HSSetShader(pHullShader, 0, 0);
    }

    void GraphicsDevice::BindDomainShader(ID3D11DomainShader* pDomainShader)
    {
        mContext->DSSetShader(pDomainShader, 0, 0);
    }

    void GraphicsDevice::BindGeometryShader(ID3D11GeometryShader* pGeometryShader)
    {
        mContext->GSSetShader(pGeometryShader, 0, 0);
    }

    void GraphicsDevice::BindPixelShader(ID3D11PixelShader* pPixelShader)
    {
        mContext->PSSetShader(pPixelShader, 0, 0);
    }

    void GraphicsDevice::BindComputeShader(ID3D11ComputeShader* pComputeShader)
    {
        mContext->CSSetShader(pComputeShader, 0, 0);
    }

    void GraphicsDevice::Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
    {
        mContext->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
    }

    void GraphicsDevice::SetConstantBuffer(ID3D11Buffer* buffer, void* data, UINT size)
    {
        D3D11_MAPPED_SUBRESOURCE subRes = {};
        mContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subRes);
        memcpy(subRes.pData, data, size);
        mContext->Unmap(buffer, 0);
    }

    void GraphicsDevice::BindConstantBuffer(eShaderStage stage, eCBType type, ID3D11Buffer* buffer)
    {
        switch (stage)
        {
        case eShaderStage::VS:
            mContext->VSSetConstantBuffers((UINT)type, 1, &buffer);
            break;

        case eShaderStage::HS:
            mContext->HSSetConstantBuffers((UINT)type, 1, &buffer);
            break;

        case eShaderStage::DS:
            mContext->DSSetConstantBuffers((UINT)type, 1, &buffer);
            break;

        case eShaderStage::GS:
            mContext->GSSetConstantBuffers((UINT)type, 1, &buffer);
            break;

        case eShaderStage::PS:
            mContext->PSSetConstantBuffers((UINT)type, 1, &buffer);
            break;

        case eShaderStage::CS:
            mContext->CSSetConstantBuffers((UINT)type, 1, &buffer);
            break;

        default:
            break;
        }
    }

    void GraphicsDevice::BindsConstantBuffer(eShaderStage stage, eCBType type, ID3D11Buffer* buffer)
    {
        mContext->VSSetConstantBuffers((UINT)type, 1, &buffer);
        mContext->HSSetConstantBuffers((UINT)type, 1, &buffer);
        mContext->DSSetConstantBuffers((UINT)type, 1, &buffer);
        mContext->GSSetConstantBuffers((UINT)type, 1, &buffer);
        mContext->PSSetConstantBuffers((UINT)type, 1, &buffer);
        mContext->CSSetConstantBuffers((UINT)type, 1, &buffer);
    }

    void GraphicsDevice::BindBuffer(ID3D11Buffer* buffer, void* data, UINT size)
    {
        D3D11_MAPPED_SUBRESOURCE sub = {};
        mContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
        memcpy(sub.pData, data, size);
        mContext->Unmap(buffer, 0);
    }

    void GraphicsDevice::ReadBuffer(ID3D11Buffer* buffer, void* data, UINT size)
    {
        D3D11_MAPPED_SUBRESOURCE sub = {};
        mContext->Map(buffer, 0, D3D11_MAP_READ, 0, &sub);
        memcpy(data, sub.pData, size); 
        mContext->Unmap(buffer, 0);
    }

    void GraphicsDevice::BindStagingBuffer(ID3D11Buffer* buffer, void* data, UINT size)
    {
        D3D11_MAPPED_SUBRESOURCE sub = {};
        mContext->Map(buffer, 0, D3D11_MAP_WRITE, 0, &sub);
        memcpy(sub.pData, data, size);
        mContext->Unmap(buffer, 0);
    }

    void GraphicsDevice::BindSamplerState(eShaderStage stage, UINT StartSlot, ID3D11SamplerState** ppSamplers)
    {
        switch (stage)
        {
        case eShaderStage::VS:
            mContext->VSSetSamplers(StartSlot, 1, ppSamplers);
            break;

        case eShaderStage::HS:
            mContext->HSSetSamplers(StartSlot, 1, ppSamplers);
            break;

        case eShaderStage::DS:
            mContext->DSSetSamplers(StartSlot, 1, ppSamplers);
            break;

        case eShaderStage::GS:
            mContext->GSSetSamplers(StartSlot, 1, ppSamplers);
            break;

        case eShaderStage::PS:
            mContext->PSSetSamplers(StartSlot, 1, ppSamplers);
            break;

        case eShaderStage::CS:
            mContext->CSSetSamplers(StartSlot, 1, ppSamplers);
            break;

        default:
            break;
        }
    }

    void GraphicsDevice::BindShaderResource(eShaderStage stage, UINT startSlot, ID3D11ShaderResourceView** ppSRV)
    {
        switch (stage)
        {
        case eShaderStage::VS:
            mContext->VSSetShaderResources(startSlot, 1, ppSRV);
            break;

        case eShaderStage::HS:
            mContext->HSSetShaderResources(startSlot, 1, ppSRV);
            break;

        case eShaderStage::DS:
            mContext->DSSetShaderResources(startSlot, 1, ppSRV);
            break;

        case eShaderStage::GS:
            mContext->GSSetShaderResources(startSlot, 1, ppSRV);
            break;

        case eShaderStage::PS:
            mContext->PSSetShaderResources(startSlot, 1, ppSRV);
            break;

        case eShaderStage::CS:
            mContext->CSSetShaderResources(startSlot, 1, ppSRV);
            break;

        default:
            break;
        }
    }

    void GraphicsDevice::BindUnorderedAccess(UINT slot, ID3D11UnorderedAccessView** ppUnorderedAccessViews, const UINT* pUAVInitialCounts)
    {
        mContext->CSSetUnorderedAccessViews(slot, 1, ppUnorderedAccessViews, pUAVInitialCounts);
    }

    void GraphicsDevice::BindViewPort(D3D11_VIEWPORT* viewPort)
    {
        mContext->RSSetViewports(1, viewPort);
    }

    void GraphicsDevice::BindRasterizeState(ID3D11RasterizerState* pRasterizerState)
    {
        mContext->RSSetState(pRasterizerState);
    }

    void GraphicsDevice::BindDepthStencilState(ID3D11DepthStencilState* pDepthStencilState)
    {
        mContext->OMSetDepthStencilState(pDepthStencilState, 0);
    }

    void GraphicsDevice::BindBlendState(ID3D11BlendState* pBlendState)
    {
        mContext->OMSetBlendState(pBlendState, nullptr, 0xffffffff);
    }

    void GraphicsDevice::CopyResource(ID3D11Resource* pDstResource, ID3D11Resource* pSrcResource)
    {
        mContext->CopyResource(pDstResource, pSrcResource);
    }

    void GraphicsDevice::CreateLightMap()
    {
        Application& application = Application::GetInst();

        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = application.GetWidth();
        texDesc.Height = application.GetHeight();
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;

        HRESULT hr = mDevice->CreateTexture2D(&texDesc, nullptr, &mLightRenderTarget);

        if (FAILED(hr))
        {
            return;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = texDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        hr = mDevice->CreateShaderResourceView(mLightRenderTarget.Get(), &srvDesc, &mBloomLightSRV);
        if (FAILED(hr))
        {
            return;
        }

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = texDesc.Format;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;

        hr = mDevice->CreateRenderTargetView(mLightRenderTarget.Get(), &rtvDesc, &mLightRenderTargetView);
        if (FAILED(hr))
        {
            return;
        }

        
    }

    void GraphicsDevice::CreateBloomRenderTarget()
    {
        Application& application = Application::GetInst();

        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = application.GetWidth() / 4;
        texDesc.Height = application.GetHeight() / 4;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;

        HRESULT hr = mDevice->CreateTexture2D(&texDesc, nullptr, &mBloomRenderTarget);

        if (FAILED(hr))
        {
            return;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = texDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        hr = mDevice->CreateShaderResourceView(mBloomRenderTarget.Get(), &srvDesc, &mBloomSRV);
        if (FAILED(hr))
        {
            return;
        }

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = texDesc.Format;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;

        hr = mDevice->CreateRenderTargetView(mBloomRenderTarget.Get(), &rtvDesc, &mBloomRenderTargetView);
        if (FAILED(hr))
        {
            return;
        }

        mBloomViewPort = { 0.0f, 0.0f , 400.f , 225.f, 0.0f, 1.0f };
    }

    void GraphicsDevice::CreateFinalLightTarget()
    {
        Application& application = Application::GetInst();

        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = application.GetWidth();
        texDesc.Height = application.GetHeight();
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;

        HRESULT hr = mDevice->CreateTexture2D(&texDesc, nullptr, &mPlayerVisibilityRenderTarget);

        if (FAILED(hr))
        {
            return;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = texDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        hr = mDevice->CreateShaderResourceView(mPlayerVisibilityRenderTarget.Get(), &srvDesc, &mPlayerVisibilitySRV);
        if (FAILED(hr))
        {
            return;
        }

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = texDesc.Format;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;

        hr = mDevice->CreateRenderTargetView(mPlayerVisibilityRenderTarget.Get(), &rtvDesc, &mPlayerVisibilityRenderTargetView);
        if (FAILED(hr))
        {
            return;
        }
    }
    void GraphicsDevice::CreateVisibility(UINT width, UINT height)
    {
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;

        HRESULT hr = mDevice->CreateTexture2D(&texDesc, nullptr, &mViewMapRenderTarget);

        if (FAILED(hr))
        {
            return;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = texDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        hr = mDevice->CreateShaderResourceView(mViewMapRenderTarget.Get(), &srvDesc, &mViewMap);
        if (FAILED(hr))
        {
            return;
        }

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = texDesc.Format;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;

        hr = mDevice->CreateRenderTargetView(mViewMapRenderTarget.Get(), &rtvDesc, &mViewMapRenderTargetView);
        if (FAILED(hr))
        {
            return;
        }

        mVisibilityViewPort = { 0.0f, 0.0f , (float)width , (float)height, 0.0f, 1.0f };
    }

    void GraphicsDevice::DrawIndexed(UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
    {
        mContext->DrawIndexed(IndexCount, StartIndexLocation, BaseVertexLocation);
    }

    void GraphicsDevice::DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
    {
        mContext->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
    }

    void GraphicsDevice::SetLightMapRenderTarget(bool bClear)
    {
        if(bClear)
        {
            FLOAT bgColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            mContext->ClearRenderTargetView(mLightRenderTargetView.Get(), bgColor);
        }
        mContext->OMSetRenderTargets(1, mLightRenderTargetView.GetAddressOf(), nullptr);
        BindViewPort(&mViewPort);
    }

    void GraphicsDevice::SetBloomRenderTarget()
    {
        FLOAT bgColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        mContext->ClearRenderTargetView(mBloomRenderTargetView.Get(), bgColor);
        mContext->OMSetRenderTargets(1, mBloomRenderTargetView.GetAddressOf(), nullptr);
        BindViewPort(&mBloomViewPort);
    }

    void GraphicsDevice::SetPlayerVisibilityRenderTarget()
    {
        FLOAT bgColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        mContext->ClearRenderTargetView(mPlayerVisibilityRenderTargetView.Get(), bgColor);
        mContext->OMSetRenderTargets(1, mPlayerVisibilityRenderTargetView.GetAddressOf(), nullptr);
        BindViewPort(&mViewPort);
    }

    void GraphicsDevice::SetVisibilityRenderTarget()
    {
        mContext->OMSetRenderTargets(1, mViewMapRenderTargetView.GetAddressOf(), nullptr);
        BindViewPort(&mVisibilityViewPort);
    }

    void GraphicsDevice::ClearRenderTarget()
    {
        FLOAT bgColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        mContext->ClearRenderTargetView(mRenderTargetView.Get(), bgColor);
        mContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
        mContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());
    }

    void GraphicsDevice::UpdateViewPort()
    {
        Application& application = Application::GetInst();
        HWND hWnd = application.GetHwnd();
        RECT winRect = {};

        GetClientRect(hWnd, &winRect);
        mViewPort =
        {
            0.0f, 0.0f
            , 1600
            , 900
            , 0.0f, 1.0f
        };

        BindViewPort(&mViewPort);
    }

    void GraphicsDevice::Draw()
    {
        
        // viewport update
        
       
    }

    void GraphicsDevice::Present()
    {
        mSwapChain->Present(0, 0);
    }

}
