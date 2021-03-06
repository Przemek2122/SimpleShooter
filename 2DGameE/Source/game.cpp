#include "game.h"
#include "ErrorCodes.h"

#include "Sockets/Sockets.h"
#include "Sockets/SocketClientTCP.h"
#include "Sockets/SocketServerTCP.h"
#include "Sockets/SocketDataParser.h"

#include "Components/CameraManager.h"

#include "Util.h"
#include "AudioManager.h"
#include "TextureManager.h"

#include "Keyboard.h"
#include "Mouse.h"
#include "EventsHandler.h"

#include "Components/ECS.h"
#include "Components/PlayerComponent.h"
#include "Components/EnemyComponent.h"

#include "UI/UI.h"
#include "UI/TextWidget.h"
#include "UI/ButtonWidget.h"

#include "CustomShapes.h"
#include "Map.h"



SDL_Renderer* Game::renderer = nullptr;
SDL_Event Game::event;

Manager manager;
auto& cameraManager(manager.addEntity());
auto& Player(manager.addEntity());
auto& Enemy(manager.addEntity());
auto& Enemy2(manager.addEntity());
auto& Enemy3(manager.addEntity());
auto& Enemy4(manager.addEntity());

AssetsManager* Game::assets = new AssetsManager(&manager);
SDL_Window* Game::window = nullptr;


Game::Game()
{
	server = false;
}
Game::~Game(){}

void Game::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen)
{
	int flags = 0;
	if (fullscreen)
	{
		flags = SDL_WINDOW_FULLSCREEN;
	}

	// Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		Util::Info("Subsystem Initialised!");

		window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
		if (window)
		{
			Util::Info("Window created!");
		} 
		else 
		{
			Util::Error("Window creating: " + (std::string)SDL_GetError());
			exit(ErrorCode_WindowCreateFail);
		}

		renderer = SDL_CreateRenderer(window, -1, 0);
		if (renderer) 
		{
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			Util::Info("Renderer created!");
		} 
		else 
		{
			Util::Error("Renderer creating: " + (std::string)SDL_GetError());
			exit(ErrorCode_RendererCreateFail);
		}

		running = true;
	} 
	else
	{
		running = false;
		Util::Error("SDL_INIT_EVERYTHING error: " + (std::string)SDL_GetError());
		exit(ErrorCode_SDLInitAllFail);
	}

	// Initialize SDL TTF 
	if (/*TTF_Init() > 0*/ TTF_Init() != 0)
	{
		Util::Error("TTF_Init: " + (std::string)TTF_GetError());
		exit(ErrorCode_TTFInitFail);
	} 

	// Load support for the OGG and MOD sample/music formats
	int mixFlags = MIX_INIT_OGG | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_FLAC;
	int initted = Mix_Init(mixFlags);
	if (initted & mixFlags != mixFlags)
	{
		Util::Error("Mix_Init: " + (std::string)Mix_GetError());
		exit(ErrorCode_MixInitFail);
	}

	// Initialize sdl mixer
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		Util::Error("SDL_mixer: " + (std::string)Mix_GetError() + (std::string)SDL_GetError());
		exit(ErrorCode_SDLMixInitFail);
	}
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

	// Entity - component system manager
	manager.game = this;
	ECSManager = &manager;

	// UI
	UiManager = new UIManager(this);

	// Audio 
	audioManager = new AudioManager;

	// Map (will generate anything too...)
	map = new Map(this);

	// Sockets
	sockets = new SocketsManager(this);
	if (server)
	{
		if (sockets->Listen("", 62000, true))
			Util::Debug("Listening succesfull!");
		else
			Util::Debug("Listening failed!");
	}
	else
	{
		if (sockets->Connect("127.0.0.1", 62000))
			Util::Debug("Connection succesfull!");
		else
			Util::Debug("Connection failed!");
	}

	// Camera
	cameraManager.addComponent<CameraManagerComponent>();
	camera = &cameraManager;

	// Player
	Player.addComponent<PlayerComponent>();
	player = &Player;
	//Player.considerNetwork = true;

	// Test enemy(s)
	Enemy.addComponent<EnemyComponent>();

	Enemy2.addComponent<EnemyComponent>();
	Enemy2.getComponent<TransformComponent>().Location = Vector2D<>(200, 140);
	Enemy3.addComponent<EnemyComponent>();
	Enemy3.getComponent<TransformComponent>().Location = Vector2D<>(200, 180);
	Enemy4.addComponent<EnemyComponent>();
	Enemy4.getComponent<TransformComponent>().Location = Vector2D<>(800, 640);

	// UI
	assets->addTexture("ui_button_default", "assets/ui/button_default.png");
	assets->addFont("OpenSans_16", "font/OpenSans-Regular.ttf", 16);
	SDL_Color color = { 255, 255, 255, 255 };
	UiManager->addWidget<TextWidget>(
		"FPS",								// Text name
		UIUtil::getScreenPercentW(97),		// Width
		0,									// Height
		"FPS",								// Default value
		"OpenSans_16",						// Font
		color								// Color
	);
	UiManager->addWidget<TextWidget>(
		"PlayerX",							// Text name
		UIUtil::getScreenPercentW(90),		// Width
		UIUtil::getScreenPercentH(96),		// Height
		"0", 								// Default value
		"OpenSans_16", 						// Font
		color 								// Color
	);
	UiManager->addWidget<TextWidget>(
		"PlayerY",							// Text name
		UIUtil::getScreenPercentW(95),		// Width
		UIUtil::getScreenPercentH(96),		// Height
		"0",								// Default value
		"OpenSans_16",  					// Font
		color 								// Color
	);
	//UiManager->addWidget<ButtonWidget>("testbutton", 100, 100, 200, 50);
}

void Game::update()
{
	map->updateCollision();
	manager.update();
	UiManager->update();
	sockets->update();

	// FPS Counter
	if (checkFPS)
	{
		time_t rawtime;
		time(&rawtime);
		struct tm timeinfo;
		time_t time = localtime_s(&timeinfo, &rawtime);

		if (rawtime != time1) 
		{
			UiManager->getWidget<TextWidget>("FPS").setText(std::to_string(cnt), "OpenSans_16");
			cnt = 0;
			time1 = rawtime;
		} 
		else 
		{
			cnt++;
		}
	}

	// Log player position at left bottom corner of the screen
	char buffer[33];
	_itoa_s(Player.getComponent<TransformComponent>().Location.X, buffer, 10);
	UiManager->getWidget<TextWidget>("PlayerX").setText(buffer, "OpenSans_16");
	_itoa_s(Player.getComponent<TransformComponent>().Location.Y, buffer, 10);
	UiManager->getWidget<TextWidget>("PlayerY").setText(buffer, "OpenSans_16");
}

void Game::render()
{
	SDL_RenderClear(renderer);

	map->DrawMap(); // Map render

	manager.render(); // Entity & Components render

	UiManager->render(); // UI render

	SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // Background color
	SDL_RenderPresent(renderer);
}

void Game::clean()
{
	SDL_DestroyWindow(window);
	//SDL_DestroyRenderer(renderer); // Causes infinite loop / freez when destroying ?! @TODO FIX
	TTF_Quit();
	Mix_CloseAudio();
	SDL_Quit();

	Util::Info("Game cleaned.");
}

void Game::setDeltaTime(double time)
{
	deltaTime = time;
}

double Game::getDeltaTime()
{
	return deltaTime;
}
