#include "EmulatorRenderer.h"
#include "DDSTextureLoader.h"
#include "EmulatorFileHandler.h"
#include "EmulatorSettings.h"
#include <string>
#include <sstream>

#define CROSS_TEXTURE_FILE_NAME						L"Assets/pad_cross.dds"
#define BUTTONS_TEXTURE_FILE_NAME					L"Assets/pad_buttons.dds"
#define BUTTONS_GRAY_TEXTURE_FILE_NAME				L"Assets/pad_buttons_gray.dds"
#define SS_TEXTURE_FILE_NAME						L"Assets/pad_start_select.dds"
#define L_TEXTURE_FILE_NAME							L"Assets/pad_l_button.dds"
#define R_TEXTURE_FILE_NAME							L"Assets/pad_r_button.dds"
#define STICK_TEXTURE_FILE_NAME						L"Assets/ThumbStick.dds"
#define STICK_CENTER_TEXTURE_FILE_NAME				L"Assets/ThumbStickCenter.dds"

#define AUTOSAVE_INTERVAL			10.0f

using namespace Windows::Foundation;

namespace Emulator
{
	//extern bool timeMeasured;
	extern bool autosaving;

	int framesNotRendered = 0;
	bool monitorTypeSkipped = false;

	EmulatorRenderer ^EmulatorRenderer::renderer = nullptr;

	EmulatorRenderer ^EmulatorRenderer::GetInstance(void)
	{
		return renderer;
	}

	EmulatorRenderer::EmulatorRenderer()
	: emulator(EmulatorGame::GetInstance()),
	frontbuffer(0), controller(nullptr), 
	elapsedTime(0.0f), frames(0), autosaveElapsed(0.0f)
	{ 
		this->gameTime = ref new GameTime();
		renderer = this;

		/*this->stopThread = false;
		this->autosaveDoneEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
		this->autosaveEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
		this->threadAction = ThreadPool::RunAsync(ref new WorkItemHandler([this](IAsyncAction ^action)
		{
			this->AutosaveAsync();
		}), WorkItemPriority::High, WorkItemOptions::None);*/
	}

	EmulatorRenderer::~EmulatorRenderer(void)
	{
		if(this->m_d3dContext)
		{
			this->m_d3dContext->Unmap(this->buffers[(this->frontbuffer + 1) % 2].Get(), 0);
		}

		/*this->stopThread = true;
		SetEvent(this->autosaveEvent);
		WaitForSingleObjectEx(this->autosaveDoneEvent, INFINITE, false);*/
		
		delete this->spriteBatch;
		this->spriteBatch = nullptr;

		delete this->commonStates;
		this->commonStates = nullptr;

		delete this->font;
		this->font = nullptr;
	}

	void EmulatorRenderer::Initialize(SwapChainBackgroundPanel ^swapChainPanel) 
	{
		Direct3DBase::Initialize(swapChainPanel);

		this->emulator->ResizeBuffer(this->m_renderTargetSize.Width, this->m_renderTargetSize.Height);
		this->controller = this->emulator->GetVirtualController();

		this->width = this->m_renderTargetSize.Width;
		this->height = this->m_renderTargetSize.Height;
	}

	GameTime ^EmulatorRenderer::GetGameTime(void)
	{
		return this->gameTime;
	}

	void EmulatorRenderer::HandleDeviceLost(void)
	{

	}

	void EmulatorRenderer::CreateDeviceResources()
	{
		Direct3DBase::CreateDeviceResources();

		if(this->spriteBatch)
		{
			delete this->spriteBatch;
			this->spriteBatch = nullptr;
		}
		if(this->font)
		{
			delete this->font;
			this->font = nullptr;
		}
		if(this->commonStates)
		{
			delete this->commonStates;
			this->commonStates = nullptr;
		}
		this->m_d3dDevice->GetImmediateContext1(this->m_d3dContext.GetAddressOf());
		this->spriteBatch = new SpriteBatch(this->m_d3dContext.Get());
		this->font = new SpriteFont(this->m_d3dDevice.Get(), L"Assets/arial24.spritefont");
		this->commonStates = new CommonStates(this->m_d3dDevice.Get());
		
		DX::ThrowIfFailed(
			CreateDDSTextureFromFile(
			this->m_d3dDevice.Get(), STICK_TEXTURE_FILE_NAME,
			this->stickResource.GetAddressOf(),
			this->stickSRV.GetAddressOf())
			);
		DX::ThrowIfFailed(
			CreateDDSTextureFromFile(
			this->m_d3dDevice.Get(), STICK_CENTER_TEXTURE_FILE_NAME,
			this->stickCenterResource.GetAddressOf(),
			this->stickCenterSRV.GetAddressOf())
			);
		DX::ThrowIfFailed(
			CreateDDSTextureFromFile(
			this->m_d3dDevice.Get(), CROSS_TEXTURE_FILE_NAME,
			this->crossResource.GetAddressOf(),
			this->crossSRV.GetAddressOf())
			);
		DX::ThrowIfFailed(
			CreateDDSTextureFromFile(
			this->m_d3dDevice.Get(), BUTTONS_TEXTURE_FILE_NAME,
			this->buttonsResource.GetAddressOf(),
			this->buttonsSRV.GetAddressOf())
			);
		DX::ThrowIfFailed(
			CreateDDSTextureFromFile(
			this->m_d3dDevice.Get(), BUTTONS_GRAY_TEXTURE_FILE_NAME,
			this->buttonsGrayResource.GetAddressOf(),
			this->buttonsGraySRV.GetAddressOf())
			);
		DX::ThrowIfFailed(
			CreateDDSTextureFromFile(
			this->m_d3dDevice.Get(), SS_TEXTURE_FILE_NAME,
			this->startSelectResource.GetAddressOf(),
			this->startSelectSRV.GetAddressOf())
			);
		DX::ThrowIfFailed(
			CreateDDSTextureFromFile(
			this->m_d3dDevice.Get(), L_TEXTURE_FILE_NAME,
			this->lButtonResource.GetAddressOf(),
			this->lButtonSRV.GetAddressOf())
			);
		DX::ThrowIfFailed(
			CreateDDSTextureFromFile(
			this->m_d3dDevice.Get(), R_TEXTURE_FILE_NAME,
			this->rButtonResource.GetAddressOf(),
			this->rButtonSRV.GetAddressOf())
			);

		// Create Textures and SRVs for front and backbuffer
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.Format = DXGI_FORMAT_B5G6R5_UNORM;
		desc.Width = EXT_WIDTH;
		desc.Height = EXT_HEIGHT;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DYNAMIC;

		DX::ThrowIfFailed(
			this->m_d3dDevice->CreateTexture2D(&desc, nullptr, this->buffers[0].GetAddressOf())
			);
		DX::ThrowIfFailed(
			this->m_d3dDevice->CreateTexture2D(&desc, nullptr, this->buffers[1].GetAddressOf())
			);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

		DX::ThrowIfFailed(
			this->m_d3dDevice->CreateShaderResourceView(this->buffers[0].Get(), &srvDesc, this->bufferSRVs[0].GetAddressOf())
			);
		DX::ThrowIfFailed(
			this->m_d3dDevice->CreateShaderResourceView(this->buffers[1].Get(), &srvDesc, this->bufferSRVs[1].GetAddressOf())
			);

		// Map backbuffer so it can be unmapped on first update
		size_t pitch;
		int backbuffer = (this->frontbuffer + 1) % 2;
		this->MapBuffer(backbuffer, &pitch);

		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));

		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		DX::ThrowIfFailed(
			this->m_d3dDevice->CreateBlendState(&blendDesc, this->alphablend.GetAddressOf())
			);
	}

	void EmulatorRenderer::UpdateForWindowSizeChange()
	{
		Direct3DBase::UpdateForWindowSizeChange();

		//float scale = ((int)Windows::Graphics::Display::DisplayProperties::ResolutionScale) / 100.0f;
		this->width = m_renderTargetSize.Width;// width * scale;
		this->height = m_renderTargetSize.Height;//height * scale;

		//this->emulator->ResizeBuffer(this->width, this->height);
	}
	
	void EmulatorRenderer::FPSCounter(void)
	{
		float lastElapsed = this->gameTime->GetLastFrameElapsed();
		this->fpsElapsedTime += lastElapsed;
		if(this->fpsElapsedTime >= 1.0)
		{
			this->fps = this->frames;
			this->frames = 0;
			this->fpsElapsedTime -= 1.0;
		}
	}

	/*void EmulatorRenderer::MeasureTime(void)
	{
		float lastElapsed = this->gameTime->GetLastFrameElapsed();
		this->elapsedTimeMeasure += lastElapsed;
		if(this->elapsedTimeMeasure >= 3.0)
		{
			timeMeasured = true;
			if(this->fps < 34)
			{
				EnableLowDisplayRefreshMode(true);
			}
		}
	}*/

	void EmulatorRenderer::AutosaveAsync(void)
	{
		/*WaitForSingleObjectEx(this->autosaveEvent, INFINITE, false);
		while(!this->stopThread)
		{
			this->emulator->Pause();
			SaveSRAMAsync().wait();
			int oldSlot = SavestateSlot;
			SavestateSlot = AUTOSAVESTATE_SLOT;
			SaveStateAsync().wait();
			SavestateSlot = oldSlot;
			Settings.Mute = !SoundEnabled();

			SetEvent(this->autosaveDoneEvent);
			WaitForSingleObjectEx(this->autosaveEvent, INFINITE, false);
		}
		SetEvent(this->autosaveDoneEvent);*/
	}

	void EmulatorRenderer::Autosave(void)
	{
		if(AutosavingEnabled() && !this->emulator->IsPaused() && IsROMLoaded())
		{
			float lastElapsed = this->gameTime->GetLastFrameElapsed();
			this->autosaveElapsed += lastElapsed;
			if(this->autosaveElapsed >= AUTOSAVE_INTERVAL)
			{
				this->autosaveElapsed -= AUTOSAVE_INTERVAL;

				create_task([this]()
				{
					SaveSRAMCopyAsync();
				});

		//		SetEvent(this->autosaveEvent);

		//		WaitForSingleObjectEx(this->autosaveDoneEvent, INFINITE, false);

		//		//create_task([this]()
		//		//{
		//		//	autosaving = true;
		//		//	this->emulator->Pause();
		//		//	SaveSRAMAsync().then([this]()
		//		//	{
		//		//		int oldSlot = SavestateSlot;
		//		//		SavestateSlot = AUTOSAVESTATE_SLOT;
		//		//		SaveStateAsync().wait();
		//		//		SavestateSlot = oldSlot;
		//		//		Settings.Mute = !SoundEnabled();
		//		//		autosaving = false;
		//		//		//this->emulator->Unpause();
		//		//	});
		//		//});
			}
		}
	}

	void EmulatorRenderer::Update(void)
	{
		this->gameTime->Update();

		float timeDelta = this->gameTime->GetLastFrameElapsed();

		/*if(!timeMeasured)
		{
			this->MeasureTime();
		}*/

		this->Autosave();

		this->FPSCounter();
		
		if(!emulator->IsPaused())
		{
			this->elapsedTime += timeDelta;
		}
	}
	void EmulatorRenderer::Render()
	{
		float timeDelta = this->gameTime->GetLastFrameElapsed();

		m_d3dContext->OMSetRenderTargets(
			1,
			m_renderTargetView.GetAddressOf(),
			m_depthStencilView.Get()
			);

		const float black[] = { 0.0f, 0.0f, 0.0f, 1.000f };
		m_d3dContext->ClearRenderTargetView(
			m_renderTargetView.Get(),
			black
			);

		m_d3dContext->ClearDepthStencilView(
			m_depthStencilView.Get(),
			D3D11_CLEAR_DEPTH,
			1.0f,
			0
			);

		if(!this->emulator->IsPaused()/* || autosaving*/)
		{
			if((GetMonitorType() != MONITOR_120HZ) || monitorTypeSkipped)
			{
				monitorTypeSkipped = false;
				if(framesNotRendered >= GetPowerFrameSkip())
				{
					framesNotRendered = 0;
					if(!this->emulator->LastFrameSkipped())
					{
						// Swap buffers for background emulation
						int backbuffer = this->frontbuffer;
						this->frontbuffer = (this->frontbuffer + 1) % 2;
						uint16 *buffer = (uint16 *) this->MapBuffer(backbuffer, &this->pitch);
						this->backbufferPtr = buffer;
						//memset (buffer, 0, EXT_PITCH * EXT_HEIGHT);

						this->emulator->Update((void *) buffer, pitch, timeDelta);
						this->m_d3dContext->Unmap(this->buffers[this->frontbuffer].Get(), 0);
					}else
					{
						this->emulator->Update((void *) this->backbufferPtr, pitch, timeDelta);
					}
				}else
				{
					framesNotRendered++;
				}
			}else
			{
				monitorTypeSkipped = true;
			}
		}

		int height = this->height;
		int width = (int)(height * ((float) SNES_WIDTH) / (float) SNES_HEIGHT);
		int leftOffset = (this->width - width) / 2;
		RECT rect;
		rect.left = leftOffset;
		rect.right = width + leftOffset;
		rect.top = 0;
		rect.bottom = height;

		RECT source;
		source.left = 0;
		source.right = EmulatorGame::SnesImageWidth - 1;
		source.top = 0;
		source.bottom = EmulatorGame::SnesImageHeight;
			
		this->controller->GetButtonsRectangle(&buttonsRectangle);
		this->controller->GetCrossRectangle(&crossRectangle);
		this->controller->GetStartSelectRectangle(&startSelectRectangle);
		this->controller->GetLRectangle(&lRectangle);
		this->controller->GetRRectangle(&rRectangle);

		XMFLOAT4A colorf = XMFLOAT4A(1.0f, 1.0f, 1.0f, GetControllerOpacity() / 100.0f);
		XMFLOAT4A colorf2 = XMFLOAT4A(1.0f, 1.0f, 1.0f, (GetControllerOpacity() / 100.0f) + 0.2f);
		XMVECTOR colorv = XMLoadFloat4A(&colorf);
		XMVECTOR colorv2 = XMLoadFloat4A(&colorf2);
	
		// Render last frame to screen
		this->spriteBatch->Begin(
			DirectX::SpriteSortMode::SpriteSortMode_Deferred, 
			this->alphablend.Get());
		this->spriteBatch->Draw(this->bufferSRVs[this->frontbuffer].Get(), rect, &source);
		if(TouchControlsEnabled())
		{
			if(ButtonsGray())
			{
				this->spriteBatch->Draw(this->buttonsGraySRV.Get(), this->buttonsRectangle, nullptr, colorv);
			}else
			{
				this->spriteBatch->Draw(this->buttonsSRV.Get(), this->buttonsRectangle, nullptr, colorv);
			}
			int dpad = GetDPadStyle();
			if(dpad == 0)
			{
				this->spriteBatch->Draw(this->crossSRV.Get(), this->crossRectangle, nullptr, colorv);
			}
			else if(dpad == 1 || (dpad == 2 && this->controller->StickFingerDown()))
			{
				RECT centerRect;
				RECT stickRect;
				this->controller->GetStickRectangle(&stickRect);
				this->controller->GetStickCenterRectangle(&centerRect);

				this->spriteBatch->Draw(this->stickCenterSRV.Get(), centerRect, nullptr, colorv2);
				this->spriteBatch->Draw(this->stickSRV.Get(), stickRect, nullptr, colorv);
			}
			this->spriteBatch->Draw(this->startSelectSRV.Get(), this->startSelectRectangle, nullptr, colorv);
			this->spriteBatch->Draw(this->lButtonSRV.Get(), this->lRectangle, nullptr, colorv);
			this->spriteBatch->Draw(this->rButtonSRV.Get(), this->rRectangle, nullptr, colorv);
		}

		if(ShowingFPS())
		{
			wstringstream wss;
			wss << L"FPS: ";
			wss << this->fps;

			XMFLOAT4A fpscolor = XMFLOAT4A(1.0f, 1.0f, 1.0f, 1.0f);
			XMVECTOR fpscolorA = XMLoadFloat4A(&fpscolor);

			XMFLOAT2A pos = XMFLOAT2A(10.0f, 10.0f);
			XMVECTOR posA = XMLoadFloat2A(&pos);

			XMFLOAT2A origin = XMFLOAT2A(0.0f, 0.0f);
			XMVECTOR originA = XMLoadFloat2A(&origin);

			this->font->DrawString(this->spriteBatch, wss.str().c_str(), posA, fpscolorA, 0.0f, originA, 1.0f);
		}

		this->spriteBatch->End();

		this->Present();

		frames++;
	}

	void *EmulatorRenderer::MapBuffer(int index, size_t *rowPitch)
	{
		D3D11_MAPPED_SUBRESOURCE map;
		ZeroMemory(&map, sizeof(D3D11_MAPPED_SUBRESOURCE));

		DX::ThrowIfFailed(
			this->m_d3dContext->Map(this->buffers[index].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map)
			);

		*rowPitch = map.RowPitch;
		return map.pData;
	}

}