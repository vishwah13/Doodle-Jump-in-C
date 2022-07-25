#include"raylib.h"

#define GRAVITY 400
#define PLAYER_JUMP_SPEED 350.0f
#define PLAYER_HORIZONTAL_SPEED 200.0f
#define PLATFORMCOUNT 99


typedef struct Player {
	Vector2 position;
	Rectangle PlayerRect;
	Texture2D playerTexture;
	float speed;
	bool canJump;
} Player;

typedef struct Platform {
	Rectangle rect;
	int blocking;
} Platform;


Platform platforms[PLATFORMCOUNT];
Texture2D platformTexture;
int GeneratedPlatformYaxisValue = 800;
int PlatformsLength;
int Score;
bool bGameOver = false;
bool bPaused = false;

void InitGame(Player* player,Camera2D* camera);
void UpdatePlayer(Player* player, Platform* platform, int platformsLength, float delta);
void UpdateCameraEvenOutOnLanding(Camera2D* camera, Player* player, Platform* platform, int platformsLength, float delta, int width, int height);
void RandomPlatformGenerator();
void DestroyPlatformAfterCrossedCameraBorder(Platform* platform,int platformsLength,Camera2D* camera);
void CheckForGameOver(Player* player,Camera2D* camera);
void UpdateScore(Player* player);



int main()
{
	const int ScreenWidth = 400;
	const int ScreenHeight = 600;
	InitWindow(ScreenWidth, ScreenHeight, "Doodle Jump Clone");

	Player player = { 0 };
	Camera2D camera = { 0 };
	player.playerTexture = LoadTexture("resources/character_0001.png");
	platformTexture = LoadTexture("resources/CloudPlatform.png");
	InitGame(&player, &camera);
	
	while (!WindowShouldClose())
	{
		float deltaTime = GetFrameTime();

		if (!bGameOver)
		{
			if (IsKeyPressed('P')) bPaused = !bPaused;
			
			if (!bPaused)
			{
				UpdatePlayer(&player, platforms, PlatformsLength, deltaTime);

				UpdateCameraEvenOutOnLanding(&camera, &player, platforms, PlatformsLength, deltaTime, ScreenWidth, ScreenHeight);

				UpdateScore(&player);

				DestroyPlatformAfterCrossedCameraBorder(platforms, PlatformsLength, &camera);
				CheckForGameOver(&player, &camera);
			}
		}
		else if(bGameOver)
		{
			if (IsKeyPressed(KEY_ENTER))
			{
				InitGame(&player, &camera);
				bGameOver = false;
			}
		}

		player.PlayerRect.x = player.position.x - 20;
		player.PlayerRect.y = player.position.y - 40;
		/*player.PlayerRect.width = 40;
		player.PlayerRect.height = 40;*/
		player.playerTexture.width = 40;
		player.playerTexture.height = 40;


		if (player.position.y < GeneratedPlatformYaxisValue)
		{
			GeneratedPlatformYaxisValue += 500;
			RandomPlatformGenerator();
		}
		//Drawing Things
		BeginDrawing();
		ClearBackground(LIGHTGRAY);
		BeginMode2D(camera);
		//Drawing Platforms
		for (int i = 0; i < PlatformsLength; i++)
		{
			//DrawRectangleRec(platforms[i].rect, platforms[i].color);
			DrawTexture(platformTexture, platforms[i].rect.x, platforms[i].rect.y, WHITE);
		}
		//Drawing Player
		//DrawRectangleRec(player.PlayerRect, RED);
		DrawTexture(player.playerTexture, player.PlayerRect.x, player.PlayerRect.y, WHITE);
		EndMode2D();
		//Drawing Score Text
		DrawText(TextFormat("%08i", Score), 5, 5, 20, RED);
		if (bGameOver)
		{
			DrawText(TextFormat("Press ENTER to Play again !"), 50, 250, 20, RED);
		}
		EndDrawing();
	}
	CloseWindow();
	return 0;
}

void InitGame(Player* player, Camera2D* camera)
{
	GeneratedPlatformYaxisValue = 800;
	Score = 0;

	player->position = (Vector2){ 300, 400 };
	player->speed = 0;
	player->canJump = false;

	camera->target = player->position;
	camera->offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
	camera->rotation = 0.0f;
	camera->zoom = 1.0f;
	RandomPlatformGenerator();
}

void UpdatePlayer(Player* player, Platform* platform, int platformsLength, float delta)
{
	if (IsKeyDown(KEY_LEFT))
	{
		player->position.x -= PLAYER_HORIZONTAL_SPEED * delta;
	}
	if (IsKeyDown(KEY_RIGHT))
	{
		player->position.x += PLAYER_HORIZONTAL_SPEED * delta;
	}
	if (player->canJump)
	{
		player->speed = -PLAYER_JUMP_SPEED;
		player->canJump = false;
	}

	int hitObstacle = 0;
	for (int i = 0; i < platformsLength; i++)
	{
		Platform* ei = platform + i;
		Vector2* p = &(player->position);
		if (ei->blocking &&
			ei->rect.x <= p->x &&
			ei->rect.x + ei->rect.width >= p->x &&
			ei->rect.y >= p->y &&
			ei->rect.y < p->y + player->speed * delta)
		{
			hitObstacle = 1;
			player->speed = 0.0f;
			p->y = ei->rect.y;
		}
	}

	if (!hitObstacle)
	{
		player->position.y += player->speed * delta;
		player->speed += GRAVITY * delta;
		player->canJump = false;
	}
	else
	{
		player->canJump = true;
	}

	//Invert the Player postion on X Axis While player cross the border on X Axis
	//still needs tweeking
	if (player->position.x > GetScreenWidth()+100 ) player->position.x = 40;
	if (player->position.x < 0) player->position.x = GetScreenWidth() +80;

}

void UpdateCameraEvenOutOnLanding(Camera2D* camera, Player* player, Platform* platform, int platformsLength, float delta, int width, int height)
{
	static float evenOutSpeed = 700;
	static int eveningOut = false;
	static float evenOutTarget;

	camera->offset = (Vector2){ width / 2.0f, height / 2.0f };

	if (eveningOut)
	{
		if (evenOutTarget > camera->target.y)
		{
			camera->target.y += evenOutSpeed * delta;

			if (camera->target.y > evenOutTarget)
			{
				camera->target.y = evenOutTarget;
				eveningOut = 0;
			}
		}
		else
		{
			camera->target.y -= evenOutSpeed * delta;

			if (camera->target.y < evenOutTarget)
			{
				camera->target.y = evenOutTarget;
				eveningOut = 0;
			}
		}
	}
	else
	{
		if (player->canJump && (player->speed == 0) && (player->position.y != camera->target.y))
		{
			eveningOut = 1;
			evenOutTarget = player->position.y;
		}
	}
}

void RandomPlatformGenerator()
{
	for (int i = 0;i < PLATFORMCOUNT;i++)
	{
		Rectangle RandomRec[PLATFORMCOUNT];
		Rectangle tempRandomRec = { GetRandomValue(0,400), GeneratedPlatformYaxisValue -= 50,50,10 };
		RandomRec[i] = tempRandomRec;
		
		platforms[i].rect = RandomRec[i];
		platforms[i].blocking = 1;
	}

	PlatformsLength = sizeof(platforms) / sizeof(platforms[0]);
}

void DestroyPlatformAfterCrossedCameraBorder(Platform* platform, int platformsLength, Camera2D* camera)
{
	for (int i = 0; i < platformsLength; i++)
	{
		if (camera->target.y + 200< platform[i].rect.y)
		{
			platform[i] = platform[i + 1];
		}
	}
}

void CheckForGameOver(Player* player, Camera2D* camera)
{
	if (player->speed > 600.f)
	{
		bGameOver = true;
	}
}

void UpdateScore(Player* player)
{
	if (player->canJump && (player->speed == 0))
	{
		Score = (player->position.y - 600) * -1;
	}
}
