#pragma once

#include "Direct3DBase.h"
#include "CommonStates.h"
#include "SpriteBatch.h"
#include "Emulator.h"
#include "VirtualControllerInput.h"
#include "SpriteFont.h"

using namespace DirectX;
using namespace Microsoft::WRL;

namespace Emulator
{
	ref class EmulatorRenderer sealed
		: public Direct3DBase
	{
	public:
		static EmulatorRenderer ^GetInstance(void);

		EmulatorRenderer();
		virtual ~EmulatorRenderer(void);

		// Direct3DBase methods.
		virtual void Initialize(SwapChainBackgroundPanel ^swapChainPanel) override;
		virtual void HandleDeviceLost() override;
		virtual void CreateDeviceResources() override;
		virtual void Render() override;
		virtual void UpdateForWindowSizeChange() override;
	
		void Update(void);
		GameTime ^GetGameTime(void);

	private:
		static EmulatorRenderer ^renderer;
		
		/*Windows::Foundation::IAsyncAction ^threadAction;
		HANDLE autosaveEvent;
		HANDLE autosaveDoneEvent;	
		bool stopThread;*/
		float autosaveElapsed;
		float elapsedTime;
		int frames;
		int fps;
		float fpsElapsedTime;
		float elapsedTimeMeasure;
		size_t pitch;
		uint16 *backbufferPtr;

		GameTime ^gameTime;
		VirtualControllerInput				*controller;
		RECT buttonsRectangle;
		RECT crossRectangle;
		RECT startSelectRectangle;
		RECT lRectangle;
		RECT rRectangle;

		int									frontbuffer;
		int									width, height;
		SpriteBatch							*spriteBatch;
		CommonStates						*commonStates;
		EmulatorGame						*emulator;
		SpriteFont							*font;

		ComPtr<ID3D11Texture2D>				buffers[2];
		ComPtr<ID3D11ShaderResourceView>	bufferSRVs[2];
		ComPtr<ID3D11BlendState>			alphablend;
		
		ComPtr<ID3D11Resource>				stickCenterResource;
		ComPtr<ID3D11ShaderResourceView>	stickCenterSRV;
		ComPtr<ID3D11Resource>				stickResource;
		ComPtr<ID3D11ShaderResourceView>	stickSRV;
		ComPtr<ID3D11Resource>				crossResource;
		ComPtr<ID3D11ShaderResourceView>	crossSRV;
		ComPtr<ID3D11Resource>				buttonsResource;
		ComPtr<ID3D11ShaderResourceView>	buttonsSRV;
		ComPtr<ID3D11Resource>				buttonsGrayResource;
		ComPtr<ID3D11ShaderResourceView>	buttonsGraySRV;
		ComPtr<ID3D11Resource>				startSelectResource;
		ComPtr<ID3D11ShaderResourceView>	startSelectSRV;
		ComPtr<ID3D11Resource>				lButtonResource;
		ComPtr<ID3D11ShaderResourceView>	lButtonSRV;
		ComPtr<ID3D11Resource>				rButtonResource;
		ComPtr<ID3D11ShaderResourceView>	rButtonSRV;
		
		void AutosaveAsync(void);
		void *MapBuffer(int index, size_t *rowPitch);
		void FPSCounter(void);
		//void MeasureTime(void);
		void Autosave(void);
	};
}