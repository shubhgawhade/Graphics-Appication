//
// Game.cpp
//

#include "pch.h"
#include "Game.h"


//toreorganise
#include <fstream>

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
#ifdef DXTK_AUDIO
    if (m_audEngine)
    {
        m_audEngine->Suspend();
    }
#endif
}

Vector3 position;
Vector3 rotation;

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
	Width = width;
	Height = height;

	m_input.Initialise(window);

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	//setup light
	m_Light.setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	m_Light.setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light.setPosition(2.0f, 1.0f, 1.0f);
	m_Light.setDirection(-1.0f, -1.0f, 0.0f);

	//setup camera
	m_Camera01.setPosition(Vector3(0.0f, 0.0f, 4.0f));
	m_Camera01.setRotation(Vector3(-90.0f, 180.0f, 0.0f));	//orientation is -90 becuase zero will be looking up at the sky straight up.
	position = m_Camera01.getPosition(); //get the position
	rotation = m_Camera01.getRotation(); //get the rotation
	//m_Camera01.active = true;

    // m_Camera02.setPosition(Vector3(0.0f, 0.0f, -4.0f));
    // m_Camera02.setRotation(Vector3(-90.0f, -180.0f, 0.0f)); 
    //m_Camera02.active = false;
	
#ifdef DXTK_AUDIO
    // Create DirectXTK for Audio objects
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif

    m_audEngine = std::make_unique<AudioEngine>(eflags);

    m_audioEvent = 0;
    m_audioTimerAcc = 10.f;
    m_retryDefault = false;

    m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"adpcmdroid.xwb");

    m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"MusicMono_adpcm.wav");
    m_effect1 = m_soundEffect->CreateInstance();
    m_effect2 = m_waveBank->CreateInstance(10);

    m_effect1->Play(true);
    m_effect2->Play();
#endif
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
	// if(m_deviceResources->GetScreenViewport().Height != Height || m_deviceResources->GetScreenViewport().Width != Width)
	// {
	// 	Height = m_deviceResources->GetScreenViewport().Height;
	// 	Width = m_deviceResources->GetScreenViewport().Width;
	// }
	
	//take in input
	m_input.Update();								//update the hardware
	m_gameInputCommands = m_input.getGameInput();	//retrieve the input for our game
	
	//Update all game objects
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

	//Render all game content. 
    Render();

#ifdef DXTK_AUDIO
    // Only update audio engine once per frame
    if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
#endif

	
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
	//note that currently.  Delta-time is not considered in the game object movement. 
	// if (m_gameInputCommands.left)
	// {
	// 	Vector3 rotation = m_Camera01.getRotation();
	// 	rotation.y = rotation.y += m_Camera01.getRotationSpeed() * m_timer.GetElapsedSeconds();
	// 	m_Camera01.setRotation(rotation);
	// }
	// if (m_gameInputCommands.right)
	// {
	// 	Vector3 rotation = m_Camera01.getRotation();
	// 	rotation.y = rotation.y -= m_Camera01.getRotationSpeed() * m_timer.GetElapsedSeconds();
	// 	m_Camera01.setRotation(rotation);
	// }

	Vector3 dir;
	
	if (m_gameInputCommands.left)
	{
		dir -= (m_Camera01.getRight() * m_timer.GetElapsedSeconds()); //add the left vector
		// m_Camera01.setPosition(position);
	}
	if (m_gameInputCommands.right)
	{
		dir += (m_Camera01.getRight() * m_timer.GetElapsedSeconds()); //add the right vector
		// m_Camera01.setPosition(position);
	}
	if(m_gameInputCommands.up)
	{
		dir += (m_Camera01.getRight().Cross(m_Camera01.getForward()) * m_timer.GetElapsedSeconds()); //add the up vector
		// m_Camera01.setPosition(position);
	}
	if(m_gameInputCommands.down)
	{
		dir -= (m_Camera01.getRight().Cross(m_Camera01.getForward()) * m_timer.GetElapsedSeconds()); //add the down vector
		// m_Camera01.setPosition(position);
	}
	if (m_gameInputCommands.forward)
	{
        dir += (m_Camera01.getForward() * m_timer.GetElapsedSeconds()); //add the forward vector
		// m_Camera01.setPosition(position);
	}
	if (m_gameInputCommands.back)
	{
		dir -= (m_Camera01.getForward()* m_timer.GetElapsedSeconds()); //add the back vector
		// m_Camera01.setPosition(position);
	}

	// Normalize movement direction to prevent faster diagonal speed
	dir.Normalize();
	
	if(m_gameInputCommands.boost)
	{
		dir*=m_Camera01.getBoostSpeed();
	}

	dir*=m_Camera01.getMoveSpeed();
	
	position+=dir;
	m_Camera01.setSmoothPosition(position);
	
    // if (m_gameInputCommands.rotLeft)
    // {
    //     Vector3 rotation = m_Camera01.getRotation();
    //     rotation.z = rotation.z += m_Camera01.getRotationSpeed() * m_timer.GetElapsedSeconds();
    //     m_Camera01.setRotation(rotation);
    // }
    // if (m_gameInputCommands.rotRight)
    // {
    //     Vector3 rotation = m_Camera01.getRotation();
    //     rotation.z = rotation.z -= m_Camera01.getRotationSpeed() * m_timer.GetElapsedSeconds();
    //     m_Camera01.setRotation(rotation);
    // }

	// float screenRatioY = m_gameInputCommands.mouseY / static_cast<float>(Height);
	// float screenRatioX = m_gameInputCommands.mouseX / static_cast<float>(Width);
	// m_gameInputCommands.mouseY = -1 + screenRatioY * 2;
	// m_gameInputCommands.mouseX = -1 + screenRatioX * 2;

	rotation += Vector3(0, -m_gameInputCommands.mouseX * m_Camera01.getRotationSpeed(),-m_gameInputCommands.mouseY * m_Camera01.getRotationSpeed());

	rotation.z = std::min(90.0f, rotation.z);
	rotation.z = std::max(-90.0f, rotation.z);

	m_Camera01.setSmoothRotation(rotation);
    
    if (m_gameInputCommands.camera2)
    {
        m_Camera02.Update();	//camera update.

        m_view = m_Camera02.getCameraMatrix();
    }
    else
    {
        m_Camera01.Update();	//camera update.

        m_view = m_Camera01.getCameraMatrix();
    }

	
	m_world = Matrix::Identity;

#ifdef DXTK_AUDIO
    m_audioTimerAcc -= (float)timer.GetElapsedSeconds();
    if (m_audioTimerAcc < 0)
    {
        if (m_retryDefault)
        {
            m_retryDefault = false;
            if (m_audEngine->Reset())
            {
                // Restart looping audio
                m_effect1->Play(true);
            }
        }
        else
        {
            m_audioTimerAcc = 4.f;

            m_waveBank->Play(m_audioEvent++);

            if (m_audioEvent >= 11)
                m_audioEvent = 0;
        }
    }
#endif

  
	if (m_input.Quit())
	{
		ExitGame();
	}
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{	
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    // Draw Text to the screen
    m_deviceResources->PIXBeginEvent(L"Draw sprite");
    m_sprites->Begin();
		m_font->DrawString(m_sprites.get(), L"DirectXTK Demo Window", XMFLOAT2(10, 10), Colors::Yellow);
    m_sprites->End();
    m_deviceResources->PIXEndEvent();
	
	//Set Rendering states. 
	context->OMSetBlendState(m_states->AlphaBlend(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	context->RSSetState(m_states->CullClockwise());
//	context->RSSetState(m_states->Wireframe());



	// Turn our shaders on, set parameters
	m_BasicShaderPair.EnableShader(context);
	m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_texture1.Get());

	//render our astronaut model
	m_astronaut.Render(context);
	
	//prepare transform for object.
	m_world = SimpleMath::Matrix::Identity; //set world back to identity
	SimpleMath::Matrix astroPos = SimpleMath::Matrix::CreateTranslation(-5.0f, 0.0f, 0.0f);
	m_world = m_world * astroPos;
	

	// Turn our shaders on, set parameters
	m_BasicShaderPair.EnableShader(context);
	m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_texture1.Get());//, grassNormal.Get());

	//render our model
	m_BasicModel.Render(context);

	//prepare transform for second object.
	m_world = SimpleMath::Matrix::Identity; //set world back to identity
	SimpleMath::Matrix newPosition = SimpleMath::Matrix::CreateTranslation(2.0f, 0.0f, 0.0f);
	m_world = m_world * newPosition;


	//setup and draw sphere
	m_BasicShaderPair.EnableShader(context);
	m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_texture2.Get());
	m_BasicModel2.Render(context);

	//prepare transform for floor object. 
	m_world = SimpleMath::Matrix::Identity; //set world back to identity
	SimpleMath::Matrix newPosition2 = SimpleMath::Matrix::CreateTranslation(0.0f, -0.6f, 0.0f);
	m_world = m_world * newPosition2;

	//setup and draw cube
	m_BasicShaderPair.EnableShader(context);
	m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, grassAlbedo.Get());//, grassNormal.Get());
	m_BasicModel3.Render(context);

	// SKYBOX
	for (int i =0; i< 2; i++)
	{
		m_world = SimpleMath::Matrix::Identity; //set world back to identity
		SimpleMath::Matrix skyBoxRotation;
		SimpleMath::Matrix skyBoxScale;
		if(i%2==0)
		{
			skyBoxRotation = Matrix::CreateRotationY(m_timer.GetTotalSeconds()/50);
			skyBoxScale = SimpleMath::Matrix::CreateScale(50.0f, 50.0f, 50.0f);
	
		}
		else
		{
			skyBoxRotation = Matrix::CreateRotationY(-m_timer.GetTotalSeconds()/50);
			skyBoxScale = SimpleMath::Matrix::CreateScale(20.0f, 20.0f, 20.0f);
		}
		SimpleMath::Matrix skyBoxTranslation = Matrix::CreateTranslation(m_Camera01.getPosition());
		
		m_world = m_world * skyBoxRotation * skyBoxScale * skyBoxTranslation;
	
		m_BasicShaderPair.EnableShader(context);
		m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, starsTexture.Get());
		m_skyBox.Render(context);
	}

	// PARTICLE SYSTEM
	
	m_world = SimpleMath::Matrix::Identity; //set world back to identity
	SimpleMath::Matrix newPosition3 = SimpleMath::Matrix::CreateTranslation(0.0f, 0.0f, 2.0f);
	// SimpleMath::Matrix lookAt = Matrix::CreateLookAt(Vector3(0,0,2), m_Camera01.getForward(), m_Camera01.getForward().Cross(m_Camera01.getRight()));
	// SimpleMath::Matrix particleRotation = Matrix::CreateRotationY(XMConvertToRadians(180));
	m_ParticleSystem.Render(context, m_Camera01);
	m_world = m_world * newPosition3;
	m_ParticleSystem.Frame(5, m_deviceResources->GetD3DDeviceContext());
	m_ParticleShader.Render(context, m_ParticleSystem.GetIndexCount(), &m_world, &m_view, &m_projection, 
					  fxtexture.Get());
	

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::Black);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
#ifdef DXTK_AUDIO
    m_audEngine->Suspend();
#endif
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

#ifdef DXTK_AUDIO
    m_audEngine->Resume();
#endif
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
	Height = height;
	Width = width;

	if (!m_deviceResources->WindowSizeChanged(width, height))
		return;

    CreateWindowSizeDependentResources();
}

#ifdef DXTK_AUDIO
void Game::NewAudioDevice()
{
    if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
}
#endif

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(device);
    m_fxFactory = std::make_unique<EffectFactory>(device);
    m_sprites = std::make_unique<SpriteBatch>(context);
    m_font = std::make_unique<SpriteFont>(device, L"SegoeUI_18.spritefont");
	m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

	//setup our test model
	m_BasicModel.InitializeSphere(device);

	m_BasicModel2.InitializeModel(device,"drone.obj");
	m_BasicModel3.InitializeBox(device, 10.0f, 0.1f, 10.0f);	//box includes dimensions

	m_skyBox.InitializeModel(device, "Models/skybox.obj");
	m_astronaut.InitializeModel(device, "Models/astronaut.obj");

	//load and set up our Vertex and Pixel Shaders
	m_BasicShaderPair.InitStandard(device, L"light_vs.cso", L"light_ps.cso");
	// m_BasicShaderPair1.InitStandard(device, L"colour_vs.cso", L"colour_ps.cso");

	
	// m_ParticleShader = new ParticleShaderClass;
	m_ParticleShader.InitializeShader(device, L"particle_vs.cso", L"particle_ps.cso");
	
	// Create the particle system object.
	// m_ParticleSystem = new ParticleSystemClass;
	
	// Initialize the particle system object.
	m_ParticleSystem.Initialize(device);
	
	//load Textures
	m_fxFactory->CreateTexture(L"Textures/Grass Albedo.png",context, grassAlbedo.ReleaseAndGetAddressOf());
	m_fxFactory->CreateTexture(L"Textures/Grass Normal.png",context, grassNormal.ReleaseAndGetAddressOf());
	m_fxFactory->CreateTexture(L"Textures/EvilDrone_Diff.png",context, m_texture2.ReleaseAndGetAddressOf());
	m_fxFactory->CreateTexture(L"Textures/starsTexture2.png",context, starsTexture.ReleaseAndGetAddressOf());
	m_fxFactory->CreateTexture(L"Textures/particles.png",context, fxtexture.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"Textures/seafloor.dds", nullptr,	m_texture1.ReleaseAndGetAddressOf());
	// CreateDDSTextureFromFile(device, L"Textures/particles.dds", nullptr,	fxtexture.ReleaseAndGetAddressOf());
	// CreateDDSTextureFromFile(device, L"EvilDrone_Diff.dds", nullptr,	m_texture2.ReleaseAndGetAddressOf());
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    float aspectRatio = float(size.right) / float(size.bottom);
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // This sample makes use of a right-handed coordinate system using row-major matrices.
    m_projection = Matrix::CreatePerspectiveFieldOfView(
        fovAngleY,
        aspectRatio,
        0.01f,
        100.0f
    );
}


void Game::OnDeviceLost()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_font.reset();
	m_batch.reset();
	m_testmodel.reset();
    m_batchInputLayout.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
