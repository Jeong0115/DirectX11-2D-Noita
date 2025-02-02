#pragma once

#include "zzComponent.h"
#include "zzAnimation.h"

namespace zz
{
    class Animator : public Component
    {
    public:
        struct Event
        {
            void operator=(std::function<void()> func)
            {
                mEvent = std::move(func);
            }
            void operator()()
            {
                if (mEvent)
                    mEvent();
            }

            std::function<void()> mEvent;
        };

        struct Events
        {
            Event startEvent;
            Event completeEvent;
            Event endEvent;
        };

        Animator();
        virtual ~Animator();

        virtual void Initialize()   override;
        virtual void Update()       override;
        virtual void LateUpdate()   override;
        virtual void Render()       override;

        void Create(const std::wstring& name, std::shared_ptr<Texture> atlas, Vector2 leftTop
            , Vector2 size, UINT columnLength, Vector2 offset = Vector2::Zero, float duration = 0.1f);

        Animation* FindAnimation(const std::wstring& name);
        Events* FindEvents(const std::wstring& name);
        void PlayAnimation(const std::wstring& name, bool loop);
        void Binds();

        std::function<void()>& StartEvent(const std::wstring key);
        std::function<void()>& CompleteEvent(const std::wstring key);
        std::function<void()>& EndEvent(const std::wstring key);

    private:
        std::map<std::wstring, Animation*> mAnimations;
        std::map<std::wstring, Events*> mEvents;
        Animation* mActiveAnimation;

        bool mbLoop;
    };
}
