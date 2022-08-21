#include"raylib.h"
#include"raymath.h"
#include"stdio.h"

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

typedef struct Enemy {
	Vector2 position;
}Enemy;


Platform platforms[PLATFORMCOUNT];
Enemy enemy[5];
Texture2D platformTexture;
Texture2D enemyTexture;
int GeneratedPlatformYaxisValue = 0;
int GeneratedEnemyYaxisValue = 0;
int PlatformsLength;
int enemyLength;
int Score;
int EveryOffNumber = 0;
bool bGameOver = false;
bool benemyCollided = false;
bool bPaused = false;
bool bGenerateEnemy = false;

void InitGame(Player* player,Camera2D* camera,Enemy* enemy);
void UpdatePlayer(Player* player, Platform* platform, int platformsLength, float delta);
void UpdateCameraEvenOutOnLanding(Camera2D* camera, Player* player, Platform* platform, int platformsLength, float delta, int width, int height);
void RandomPlatformGenerator();
void RandomEnemyGenerator();
void DestroyPlatformAfterCrossedCameraBorder(Platform* platform,int platformsLength,Camera2D* camera);
void CheckForGameOver(Player* player,Camera2D* camera);
void UpdateScore(Player* player);
void CalculateDistance(Player* player,Enemy* enemy);



int main()
{
	const int ScreenWidth = 400;
	const int ScreenHeight = 600;
	InitWindow(ScreenWidth, ScreenHeight, "Doodle Jump Clone");

	Player player = { 0 };
	Camera2D camera = { 0 };
	player.playerTexture = LoadTexture("resources/character_0001.png");
	platformTexture = LoadTexture("resources/CloudPlatform.png");
	enemyTexture = LoadTexture("resources/Enemy_1.png");
	InitGame(&player, &camera,&enemy);

	/*player.PlayerRect.width = 40;
	player.PlayerRect.height = 40;*/
	player.playerTexture.width = 40;
	player.playerTexture.height = 40;
	enemyTexture.width = 40;
	enemyTexture.height = 40;
	
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
				player.PlayerRect.x = player.position.x - 20;
				player.PlayerRect.y = player.position.y - 40;
				CalculateDistance(&player, &enemy);
				
			}
		}
		else if(bGameOver)
		{
			if (IsKeyPressed(KEY_ENTER))
			{
				InitGame(&player, &camera,&enemy);
				bGameOver = false;
				benemyCollided = false;
			}
		}

		if (player.position.y < GeneratedPlatformYaxisValue)
		{
			GeneratedPlatformYaxisValue += 500;
			RandomPlatformGenerator();
		}	
		/*if (player.position.y < GeneratedEnemyYaxisValue)
		{
			GeneratedEnemyYaxisValue += 500;
			RandomEnemyGenerator();
		}*/
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
		for (int i = 0; i < 5; i++)
		{
			//DrawRectangleRec(platforms[i].rect, platforms[i].color);
			DrawTexture(enemyTexture, enemy[i].position.x, enemy[i].position.y, WHITE);
			//printf("%f,%f\n", enemy[i].position.x, enemy[i].position.y);
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

void InitGame(Player* player, Camera2D* camera,Enemy* enemy)
{
	GeneratedPlatformYaxisValue = 600;
	GeneratedEnemyYaxisValue = 300;
	Score = 0;

	player->position = (Vector2){ 300, 400 };
	player->speed = 0;
	player->canJump = false;

	camera->target = player->position;
	camera->offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
	camera->rotation = 0.0f;
	camera->zoom = 1.0f;
	enemyLength = sizeof(enemy) / sizeof(enemy[0]);
	RandomPlatformGenerator();
	RandomEnemyGenerator();
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
		//printf("GeneratedPlatformYaxisValue: %i\n", GeneratedPlatformYaxisValue);

		//if (GeneratedPlatformYaxisValue < EveryOffNumber - 300)
		//{
		//	EveryOffNumber = GeneratedPlatformYaxisValue;
		//	//printf("%f\n",player->position.y);
		//	enemy[i].position.x = GetRandomValue(20,380);
		//	enemy[i].position.y = GeneratedPlatformYaxisValue;
		//	//printf("%f,%f\n", enemy->position.x, enemy->position.y);

		//}
	}

	PlatformsLength = sizeof(platforms) / sizeof(platforms[0]);
	
}

void RandomEnemyGenerator()
{
	for (int i = 0; i < 5; i++)
	{
		//enemy[i].position = (Vector2){ GetRandomValue(20,380),GeneratedEnemyYaxisValue -= 300 };
		enemy[i].position.x =  GetRandomValue(20,380);
		enemy[i].position.y = GeneratedEnemyYaxisValue -= 300;
	}
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
	if (player->speed > 600.f || benemyCollided)
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

void CalculateDistance(Player* player, Enemy* enemy)
{
	for (int i = 0; i < enemyLength; i++)
	{
		float distance = Vector2Distance(player->position, enemy[i].position);
		if (distance < 5.0f)
		{
			benemyCollided = true;
			printf("GAME OVER");
		}
	}
	
}

//void GenerateEnemies(Player* player, Enemy* enemy)
//{
//	if (GeneratedPlatformYaxisValue > EveryOffNumber + 200)
//	{
//		EveryOffNumber = GeneratedPlatformYaxisValue;
//		//printf("%f\n",player->position.y);
//		enemy->position.x = 100;
//		enemy->position.y = GeneratedPlatformYaxisValue - 100.f;
//		printf("%f,%f", enemy->position.x, enemy->position.y);
//		bGenerateEnemy = true;
//
//	}
//}
