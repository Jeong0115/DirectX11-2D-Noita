#include "zzInput.h"
#include "zzApplication.h"
#include "zzCamera.h"
#include "zzRenderer.h"
#include "zzGameObject.h"
#include "zzUICamera.h"
#include "zzTransform.h"
namespace zz
{
    int ASCII[(UINT)eKeyCode::END] =
    {
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
        'Z', 'X', 'C', 'V', 'B', 'N', 'M',

        '0', '1', '2', '3', '4',
        '5', '6', '7', '8', '9',

        VK_UP, VK_DOWN, VK_LEFT ,VK_RIGHT, VK_SPACE,
        VK_LBUTTON, VK_RBUTTON, VK_TAB, VK_CONTROL,

        VK_F5, VK_F6, VK_F8
    };

    std::vector<Input::Key> Input::mKeys;
    Vector3 Input::mMouseWorldPos = Vector3::Zero;
    Vector3 Input::mMouseUIPos = Vector3::Zero;
    Transform* Input::mPlayer = nullptr;

    void Input::Initialize()
    {
        for (UINT i = 0; i < (UINT)eKeyCode::END; i++)
        {
            Key keyInfo;
            keyInfo.key = (eKeyCode)i;
            keyInfo.state = eKeyState::None;
            keyInfo.bPressed = false;

            mKeys.push_back(keyInfo);
        }
    }

    void Input::Update()
    {
        if (GetFocus())
        {

            for (UINT i = 0; i < (UINT)eKeyCode::END; i++)
            {
                if (GetAsyncKeyState(ASCII[i]) & 0x8000)
                {
                    // ���� �����ӿ��� ���� �־���
                    if (mKeys[i].bPressed)
                        mKeys[i].state = eKeyState::Pressed;
                    else
                        mKeys[i].state = eKeyState::Down;

                    mKeys[i].bPressed = true;
                }
                else // ���� �����ӿ� Ű�� �������� �ʴ�.
                {
                    if (mKeys[i].bPressed)
                        mKeys[i].state = eKeyState::Up;
                    else
                        mKeys[i].state = eKeyState::None;

                    mKeys[i].bPressed = false;
                }
            }

            POINT mousePos = {};
            GetCursorPos(&mousePos);
            Application& application = Application::GetInst();

            ScreenToClient(application.GetHwnd(), &mousePos);

            Vector3 pos((float)mousePos.x, (float)mousePos.y, 0.0f);
            Viewport viewport;
            viewport.width = 1600.0f;
            viewport.height = 900.0f;
            viewport.x = 0;
            viewport.y = 0;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;  

            mMouseUIPos = Vector3((float)pos.x * 0.4f, 360.f - ((float)pos.y * 0.4f), 1.0f);
            mMouseWorldPos = viewport.Unproject(pos, Camera::GetGpuProjectionMatrix(), Camera::GetGpuViewMatrix(), Matrix::Identity);
        }
        else
        {
            for (UINT i = 0; i < (UINT)eKeyCode::END; i++)
            {
                if (eKeyState::Down == mKeys[i].state
                    || eKeyState::Pressed == mKeys[i].state)
                {
                    mKeys[i].state = eKeyState::Up;
                }
                else if (eKeyState::Up == mKeys[i].state)
                {
                    mKeys[i].state = eKeyState::None;
                }

                mKeys[i].bPressed = false;
            }
        }
    }

    UINT Input::IsInputNumberKey()
    {
        for (UINT i = (UINT)eKeyCode::Num1; i < (UINT)eKeyCode::Num9; i++)
        {
            if (mKeys[i].state == eKeyState::Down)
            {
                return i - (UINT)eKeyCode::Num0;
            }
        }
        return 0;
    }

    float Input::IsFlip()
    {
        if (mPlayer == nullptr)
        {
            return 0.f;
        }

        if (mMouseWorldPos.x - mPlayer->GetPosition().x >= 0)
        {
            return 0.f;
        }
        else
        {
            return 1.f;
        }
    }
}
