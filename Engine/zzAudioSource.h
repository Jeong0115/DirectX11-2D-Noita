#pragma once
#include "zzComponent.h"
#include "zzAudioClip.h"

namespace zz
{
	class AudioSource : public Component
	{
	public:
		AudioSource();
		~AudioSource();

		virtual void Initialize() override;
		virtual void Update() override;
		virtual void LateUpdate() override;
		virtual void Render() override;

		void Play();
		void Stop();
		void SetLoop(bool loop);

		void SetClip(std::shared_ptr<AudioClip> clip) { mAudioClip = clip; }
        void SetClip(const std::wstring audio_name);

        void SetVolume(float volume) { mAudioClip->SetVolume(volume); }

		std::shared_ptr<AudioClip> GetClip() { return mAudioClip; }

	private:
		std::shared_ptr<AudioClip> mAudioClip;
	};
}
