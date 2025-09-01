#include <stdio.h>
#include <memory.h>
#include <Windows.h>
#include <mmsystem.h>
#include <time.h>
#include <stdbool.h>
#include "Console.h"
#pragma comment(lib, "winmm.lib")

char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];

#define PLAYER_YPOS 20
#define MAX_PLAYERS 3
#define MAX_BULLET 500
#define MAX_ENEMY 500
#define ENEMY_CREATE_INTERVAL 20
#define GUN_UPGRADE_RARE 10
#define GUN_UPGRADE_UNIQUE 30
#define NORMAL_GUN_DAMAGE 1
#define RARE_GUN_DAMAGE 1
#define BOMB_GUN_DAMAGE 1
#define NORMAL_ENEMY_HP 1
#define ELITE_ENEMY_HP 10
#define BOSS_ENEMY_HP 50
#define BUFF_HP 20

enum GameScene {
	SCENE_START, // 시작 화면
	SCENE_GAME,  // 게임 진행 화면
	SCENE_END,    // 종료 화면
	SCENE_EXIT    // 종료 상태
};

enum WeaponLevel {
	LEVEL_NORMAL,
	LEVEL_RARE,
	LEVEL_BOMB
};

struct Player {
	bool active;
	char x;
};

enum BulletType {
	normalBullet,
	rareBullet,
	bombBullet
};

struct Bullet {
	bool visible;
	char x;
	char y;
	bool isExplosive;
	int damage;
	BulletType type;
};

enum EnemyType {
	normal,
	elite,
	boss,
	buff
};

enum EnemyPattern {
	PATTERN_NONE,
	PATTERN_V_SHAPE,   // V자 형태로 내려오는 패턴
	PATTERN_SIDE_WAVE, // 양쪽에서 번갈아 나오는 패턴
	PATTERN_LINE_DROP  // 한 줄로 떨어지는 패턴
};

enum BuffType {
	BUFF_NONE,
	BUFF_DUPLICATE,
	BUFF_WEAPON_UPGRADE
};

struct Enemy {
	bool visible;
	char x;
	char y;
	char width;
	char height;
	int hp;
	EnemyType type;
	BuffType buffType;
};

struct Player g_Players[MAX_PLAYERS];
struct Bullet g_Bullet[MAX_BULLET];
struct Enemy g_Enemy[MAX_ENEMY];
int g_PlayerCount = 0; // 현재 활성화된 플레이어 수 - 생명

int g_FrameCounter = 0;      // 프레임 카운터 (이동, 공격 속도 조절용)
int g_PlayerMoveSpeed = 2;   // 플레이어는 2프레임당 1번 움직임
int g_EnemyMoveSpeed = 10;   // 적은 10프레임당 1번 움직임
int g_AttackCooldown = 0;    // 공격 딜레이 (0일 때만 발사 가능)
int g_AttackSpeed = 2;      // 1프레임당 1발 발사
int g_EnemyCooldown = 0;    // 적 생성 주기
bool g_BossSpawned = false; // 보스 스폰
bool g_GameClear = false; // 보스 죽이면 클리어
int g_PatternCounter = 0;                         // 패턴 진행도를 나타내는 카운터
int g_LastPatternFrame = 0;                       // 마지막 패턴이 끝난 시간

enum EnemyPattern g_CurrentPattern = PATTERN_NONE; // 현재 진행중인 패턴
enum GameScene g_CurrentScene = SCENE_START; // 현재 게임 씬을 저장하는 변수 (초기값은 시작 화면)
enum WeaponLevel g_CurrentWeaponLevel = LEVEL_NORMAL;

void Buffer_Flip(void);
void Buffer_Clear(void);
void Sprite_Draw(int iX, int iY, char chSprite);

void StartScene_Update(void);
void StartScene_Render(void);
void EndScene_Update(void);
void EndScene_Render(void);

void initPlayer(void);
void drawPlayer(void);
void movePlayer(int);

void inputKeyboard(void);

void initBullet(void);
void normalGun(int);
void rareGun(int);
void bombGun(int);
void moveBullet(void);
void drawBullet(void);

void initEnemy(void);
void createEnemy(EnemyType);
void createBuff(BuffType);
void moveEnemy(void);
void drawEnemy(void);

void checkCollision(void);
void fireGun(void);
void drawUI(void);
void drawBox(void);

void StartPattern(enum EnemyPattern newPattern);
void UpdatePattern();

void main(void)
{
	timeBeginPeriod(1);
	unsigned long fpsTick = timeGetTime();
	unsigned long startTick, endTick;


	srand((unsigned int)time(NULL));
	cs_Initial();

	startTick = timeGetTime();
	while (1)
	{
		switch (g_CurrentScene) {
		case SCENE_START:
			StartScene_Update();
			break;

		case SCENE_GAME:
			// 1. 키보드 입력부
			inputKeyboard();
			if (g_AttackCooldown > 0) {
				g_AttackCooldown--;
			}

			// 2. 로직부 
			moveBullet();
			if (g_FrameCounter % g_EnemyMoveSpeed == 0) {
				moveEnemy();
			}
			if (g_FrameCounter > 0 && g_FrameCounter % 200 == 0) {
				createBuff(BUFF_WEAPON_UPGRADE); // 200 프레임마다 무기 업그레이드 버프 생성
			}

			if (g_FrameCounter == 1800 && !g_BossSpawned) { // 보스 생성
				createEnemy(boss);
				g_BossSpawned = true;
			}


			checkCollision();
			if (g_FrameCounter > 0) {
				if (g_FrameCounter % 250 == 0) { // 250프레임마다 버프 생성
					// 랜덤하게 버프 종류 결정
					if (rand() % 2 == 0) {
						createBuff(BUFF_WEAPON_UPGRADE);
					}
					else {
						createBuff(BUFF_DUPLICATE);
					}
				}
				if (g_FrameCounter % ENEMY_CREATE_INTERVAL == 0 && !g_BossSpawned) {
					UpdatePattern();
				}
			}
			if (g_PlayerCount <= 0) {
				g_CurrentScene = SCENE_END;
			}
			// FPS 카운팅
			g_FrameCounter++;
			break;
		case SCENE_END:
			EndScene_Update();
			break;
		}
		endTick = timeGetTime();
		// 프레임 제어 대기 코드
		long useTick = (long)(endTick - startTick);
		//if (useTick > 30) continue;
		if (useTick > 30)
		{
			startTick += 30;
			continue;
		}
		// 3. 랜더부
		Buffer_Clear();
		switch (g_CurrentScene) {
		case SCENE_START:
			StartScene_Render();
			break;
		case SCENE_GAME:
			drawBox();
			drawUI();
			drawPlayer();
			drawBullet();
			drawEnemy();
			break;
		case SCENE_END:
			EndScene_Render();
			break;
		case SCENE_EXIT:
			break;
		}
		Buffer_Flip();

		// 대기 코드
		endTick = timeGetTime();
		useTick = (long)(endTick - startTick);
		if (30 - useTick > 0)
		{
			Sleep(30 - useTick);
		}
		startTick += 30;

		if (g_CurrentScene == SCENE_EXIT) break;
	}
	timeEndPeriod(1);

}



//--------------------------------------------------------------------
// 버퍼의 내용을 화면으로 찍어주는 함수.
//
// 적군,아군,총알 등을 szScreenBuffer 에 넣어주고, 
// 1 프레임이 끝나는 마지막에 본 함수를 호출하여 버퍼 -> 화면 으로 그린다.
//--------------------------------------------------------------------
void Buffer_Flip(void)
{
	cs_MoveCursor(0, 0);

	for (int i = 0; i < dfSCREEN_HEIGHT - 1; i++)
	{
		printf("%s\n", szScreenBuffer[i]);
	}
	printf("%s", szScreenBuffer[dfSCREEN_HEIGHT - 1]);
}


//--------------------------------------------------------------------
// 화면 버퍼를 지워주는 함수
//
// 매 프레임 그림을 그리기 직전에 버퍼를 지워 준다. 
// 안그러면 이전 프레임의 잔상이 남으니까
//--------------------------------------------------------------------
void Buffer_Clear(void)
{
	memset(szScreenBuffer, ' ', dfSCREEN_WIDTH * dfSCREEN_HEIGHT);

	for (int i = 0; i < dfSCREEN_HEIGHT; i++)
	{
		szScreenBuffer[i][dfSCREEN_WIDTH - 1] = '\0';
	}

}

//--------------------------------------------------------------------
// 버퍼의 특정 위치에 원하는 문자를 출력.
//
// 입력 받은 X,Y 좌표에 아스키코드 하나를 출력한다. (버퍼에 그림)
//--------------------------------------------------------------------
void Sprite_Draw(int iX, int iY, char chSprite)
{
	if (iX < 0 || iY < 0 || iX >= dfSCREEN_WIDTH - 1 || iY >= dfSCREEN_HEIGHT)
		return;

	szScreenBuffer[iY][iX] = chSprite;
}

enum Key {
	LEFT,  // 0
	RIGHT, // 1
	ATTACK // 2
};

void StartScene_Update(void) {
	// Enter 키가 눌리면 게임 씬으로 변경
	if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
		g_CurrentScene = SCENE_GAME;
		g_PlayerCount = 0; // 현재 활성화된 플레이어 수 - 생명
		g_FrameCounter = 0;      // 프레임 카운터 (이동, 공격 속도 조절용)
		g_PlayerMoveSpeed = 2;   // 플레이어는 2프레임당 1번 움직임
		g_EnemyMoveSpeed = 10;   // 적은 10프레임당 1번 움직임
		g_AttackCooldown = 0;    // 공격 딜레이 (0일 때만 발사 가능)
		g_AttackSpeed = 2;      // 1프레임당 1발 발사
		g_EnemyCooldown = 0;    // 적 생성 주기
		g_BossSpawned = false; // 보스 스폰
		g_GameClear = false; // 보스 죽이면 클리어
		g_CurrentWeaponLevel = LEVEL_NORMAL;
		g_PatternCounter = 0;                         // 패턴 진행도를 나타내는 카운터
		g_LastPatternFrame = 0;                       // 마지막 패턴이 끝난 시간
		g_CurrentPattern = PATTERN_NONE; // 현재 진행중인 패턴
		initPlayer();
		initEnemy();
		initBullet();
	}
}

void StartScene_Render(void) {
	char title[7][50] = {
		" S H O O T I N G ",
		" G A M E ",
		"",
		"",
		" PRESS ENTER TO START "
	};

	int startY = 5;
	for (int i = 0; i < 5; i++) {
		int startX = (dfSCREEN_WIDTH / 2) - (strlen(title[i]) / 2);
		for (int j = 0; title[i][j] != '\0'; j++) {
			Sprite_Draw(startX + j, startY + i, title[i][j]);
		}
	}
}

void EndScene_Update(void) {
	// 'R' 키를 누르면 시작 화면으로 돌아가 재시작
	if (GetAsyncKeyState('R') & 0x8000) {
		g_CurrentScene = SCENE_START;
	}
	// 'Q' 키를 누르면 게임 종료 상태로 변경
	else if (GetAsyncKeyState('Q') & 0x8000) {
		g_CurrentScene = SCENE_EXIT;
	}
}

void EndScene_Render(void) {
	char text[5][50] = {
		" G A M E   O V E R ",
		"",
		"",
		" PRESS 'R' TO RESTART ",
		" PRESS 'Q' TO QUIT "
	};

	int startY = 8;
	for (int i = 0; i < 5; i++) {
		int startX = (dfSCREEN_WIDTH / 2) - (strlen(text[i]) / 2);
		for (int j = 0; text[i][j] != '\0'; j++) {
			Sprite_Draw(startX + j, startY + i, text[i][j]);
		}
	}
}

void drawUI(void) {
	char szUibuffer[100];
	char weaponName[10];
	switch (g_CurrentWeaponLevel) {
	case LEVEL_NORMAL: sprintf_s(weaponName, sizeof(weaponName), "Normal"); break;
	case LEVEL_RARE:   sprintf_s(weaponName, sizeof(weaponName), "Rare"); break;
	case LEVEL_BOMB:   sprintf_s(weaponName, sizeof(weaponName), "Bomb"); break;
	}
	sprintf_s(szUibuffer, sizeof(szUibuffer),
		"Ships: %d | Gun: %s", g_PlayerCount, weaponName);

	for (int i = 0; szUibuffer[i] != '\0'; i++) {
		Sprite_Draw(i, 0, szUibuffer[i]);
	}
}

void drawBox(void)
{
	int i;
	// 위쪽과 아래쪽 벽 그리기
	for (i = 0; i < dfSCREEN_WIDTH - 1; i++)
	{
		Sprite_Draw(i, 1, '-'); // 맨 윗줄(Y=0)은 UI를 위해 비워두고 Y=1에 그립니다.
		Sprite_Draw(i, dfSCREEN_HEIGHT - 1, '-');
	}

	// 왼쪽과 오른쪽 벽 그리기
	for (i = 2; i < dfSCREEN_HEIGHT - 1; i++)
	{
		Sprite_Draw(0, i, '|');
		Sprite_Draw(dfSCREEN_WIDTH - 2, i, '|');
	}

	// 모서리 그리기
	Sprite_Draw(0, 1, '+');
	Sprite_Draw(dfSCREEN_WIDTH - 2, 1, '+');
	Sprite_Draw(0, dfSCREEN_HEIGHT - 1, '+');
	Sprite_Draw(dfSCREEN_WIDTH - 2, dfSCREEN_HEIGHT - 1, '+');

	// 가운데 세로 벽 그리기
	int middleX = dfSCREEN_WIDTH / 2;
	for (i = 2; i <= dfSCREEN_HEIGHT - 5; i++) // y=2부터 시작해서 위쪽 벽과 겹치지 않게 함
	{
		Sprite_Draw(middleX, i, '|');
	}
}

void checkCollision(void) {
	for (int i = 0; i < MAX_BULLET; i++) {
		if (g_Bullet[i].visible) {
			int middleX = dfSCREEN_WIDTH / 2;

			if (g_Bullet[i].x == middleX &&
				g_Bullet[i].y >= 2 && g_Bullet[i].y <= dfSCREEN_HEIGHT - 5)
			{
				g_Bullet[i].visible = false; // 가운데 벽에 닿으면 총알을 비활성화
				continue;
			}
			// 기존의 바깥쪽 벽 충돌 확인
			else if (g_Bullet[i].y <= 1 || g_Bullet[i].y >= dfSCREEN_HEIGHT - 1 ||
				g_Bullet[i].x <= 0 || g_Bullet[i].x >= dfSCREEN_WIDTH - 2)
			{
				g_Bullet[i].visible = false; // 바깥쪽 벽에 닿아도 비활성화
				continue;
			}

			for (int j = 0; j < MAX_ENEMY; j++) {
				if (!g_Enemy[j].visible) continue;

				if (g_Bullet[i].x >= g_Enemy[j].x && g_Bullet[i].x < g_Enemy[j].x + g_Enemy[j].width &&
					g_Bullet[i].y >= g_Enemy[j].y && g_Bullet[i].y < g_Enemy[j].y + g_Enemy[j].height)
				{
					int impactX = g_Bullet[i].x; // 명중한 X좌표 저장
					int impactY = g_Bullet[i].y; // 명중한 Y좌표 저장
					g_Bullet[i].visible = false;
					g_Enemy[j].hp -= g_Bullet[i].damage; // 적의 HP를 총알의 데미지만큼 감소

					if (g_Enemy[j].hp <= 0) {
						g_Enemy[j].visible = false;
						if (g_Enemy[j].type == boss) {
							g_CurrentScene = SCENE_END; // 보스를 죽이면 종료 씬으로 전환
							break; // 더 이상 충돌 검사를 할 필요가 없으므로 루프 탈출
						}
						if (g_Enemy[j].type == buff) {

							switch (g_Enemy[j].buffType) {
							case BUFF_DUPLICATE: // 플레이어 수를 늘리는 버프 로직
								if (g_PlayerCount < MAX_PLAYERS) {
									int newPlayerIdx = -1;
									// 비활성된 플레이어 탐색
									for (int i = 0; i < MAX_PLAYERS; i++) {
										if (!g_Players[i].active) {
											newPlayerIdx = i;
											break;
										}
									}

									if (newPlayerIdx != -1) {
										int newX = -1;
										bool positionFound = false; // 위치를 찾았는지 확인하는 플래그

										// 모든 기존 플레이어를 순회
										// 기존 플레이어 기준 왼쪽 혹은 오른쪽에 정확히 들어가기 위해서
										for (int i = 0; i < MAX_PLAYERS; i++) {
											if (g_Players[i].active) {
												int potential_positions[] = { g_Players[i].x + 1, g_Players[i].x - 1 };

												for (int p_idx = 0; p_idx < 2; p_idx++) {
													int potentialX = potential_positions[p_idx];
													bool isOccupied = false;

													// 벽에 막히면 건너뜀
													if (potentialX < 1 || potentialX > dfSCREEN_WIDTH - 3) {
														continue;
													}

													// 찾은 위치에 기존 플레이어 있는지 체크
													for (int k = 0; k < MAX_PLAYERS; k++) {
														if (g_Players[k].active && g_Players[k].x == potentialX) {
															isOccupied = true;
															break;
														}
													}

													// 빈 자리면 위치 확정
													if (!isOccupied) {
														newX = potentialX;
														positionFound = true;
														break; // 안쪽 for문 탈출
													}
												}
											}
											if (positionFound) {
												break; // 바깥쪽 for문 탈출
											}
										}

										// 빈자리 찾았으면 활성화
										if (newX != -1) {
											g_Players[newPlayerIdx].active = true;
											g_Players[newPlayerIdx].x = newX;
											g_PlayerCount++;
										}
									}
								}
								break;
							case BUFF_WEAPON_UPGRADE:
								if (g_CurrentWeaponLevel == LEVEL_NORMAL) g_CurrentWeaponLevel = LEVEL_RARE;
								else if (g_CurrentWeaponLevel == LEVEL_RARE) g_CurrentWeaponLevel = LEVEL_BOMB;
								break;
							}
						}
						if (g_Bullet[i].isExplosive == TRUE) {
							for (int k = 0; k < MAX_ENEMY; k++) {
								if (g_Enemy[k].visible == TRUE) {
									if (abs(g_Enemy[k].x - impactX) <= 2 && abs(g_Enemy[k].y - impactY) <= 2) {
										g_Enemy[k].hp -= g_Bullet[i].damage;
										if (g_Enemy[k].hp <= 0) {
											g_Enemy[k].visible = false;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	for (int i = 0; i < MAX_ENEMY; i++) {
		if (!g_Enemy[i].visible) {
			continue;
		}

		for (int p = 0; p < MAX_PLAYERS; p++) {
			if (!g_Players[p].active) {
				continue;
			}

			int playerX = g_Players[p].x;
			int playerY = PLAYER_YPOS - 1;

			if (playerX >= g_Enemy[i].x && playerX < g_Enemy[i].x + g_Enemy[i].width &&
				playerY >= g_Enemy[i].y && playerY < g_Enemy[i].y + g_Enemy[i].height)
			{
				g_Enemy[i].visible = false;
				g_Players[p].active = false;
				g_PlayerCount--;
			}
		}
	}
}

void initEnemy() {
	for (int i = 0; i < MAX_ENEMY; i++) {
		g_Enemy[i].visible = FALSE;
	}
}

void moveEnemy() {
	for (int i = 0; i < MAX_ENEMY; i++) {
		if (g_Enemy[i].visible) {
			g_Enemy[i].y++;
			if (g_Enemy[i].y >= dfSCREEN_HEIGHT - 1)
				g_Enemy[i].visible = false;
		}
	}
}

void drawEnemy() {
	for (int i = 0; i < MAX_ENEMY; i++) {
		if (g_Enemy[i].visible) {
			char hpStr[20];
			switch (g_Enemy[i].type) {
			case normal:
				Sprite_Draw(g_Enemy[i].x, g_Enemy[i].y, NORMAL_ENEMY_SPRITE);
				break;
			case elite:
				Sprite_Draw(g_Enemy[i].x, g_Enemy[i].y, ELITE_ENEMY_SPRITE);
				sprintf_s(hpStr, sizeof(hpStr), "---%d---", g_Enemy[i].hp);
				for (int j = 0; hpStr[j] != '\0'; j++) Sprite_Draw(g_Enemy[i].x + 1 + j, g_Enemy[i].y, hpStr[j]);
				break;
			case boss:
			{ // C에서 case 내 지역 변수 사용을 위한 중괄호
				// 5x11 크기의 보스 아스키 아트
				char bossSprite[4][10] = {
					" <====>  ",
					"| o  o | ",
					" ( -- )  ",
					"^ '  ' ^ "
				};

				for (int row = 0; row < 4; row++) {
					for (int col = 0; col < 10; col++) {
						Sprite_Draw(g_Enemy[i].x + col, g_Enemy[i].y + row, bossSprite[row][col]);
					}
				}

				// 보스 HP 바 그리기
				sprintf_s(hpStr, sizeof(hpStr), "-HP: %d -", g_Enemy[i].hp);
				for (int j = 0; hpStr[j] != '\0'; j++) {
					Sprite_Draw(g_Enemy[i].x + j, g_Enemy[i].y + 4, hpStr[j]);
				}
			}
			break;
			case buff:
				char buffSprite = '?';
				if (g_Enemy[i].buffType == BUFF_DUPLICATE) buffSprite = 'D';
				else if (g_Enemy[i].buffType == BUFF_WEAPON_UPGRADE) buffSprite = 'U';
				Sprite_Draw(g_Enemy[i].x, g_Enemy[i].y, buffSprite);
				sprintf_s(hpStr, sizeof(hpStr), "---%d---", g_Enemy[i].hp);
				for (int j = 0; hpStr[j] != '\0'; j++) Sprite_Draw(g_Enemy[i].x + 1 + j, g_Enemy[i].y, hpStr[j]);
				break;
			}
		}
	}
}

void createEnemy(EnemyType pattern) {
	const int enemyCount = dfSCREEN_WIDTH / 2 - 1;
	int xCount = 1;
	switch (pattern) {
	case normal:
		for (int i = 0; i < MAX_ENEMY; i++) {
			if (g_Enemy[i].visible == false) {
				g_Enemy[i].visible = true;
				g_Enemy[i].x = xCount++;
				g_Enemy[i].y = 2;
				g_Enemy[i].hp = NORMAL_ENEMY_HP;
				g_Enemy[i].type = normal;
				g_Enemy[i].width = 1;
				g_Enemy[i].height = 1;
				if (xCount > enemyCount) break;
			}
		}
		break;
	case elite:
		for (int i = 0; i < MAX_ENEMY; i++) {
			if (g_Enemy[i].visible == false) {
				g_Enemy[i].visible = true;
				g_Enemy[i].x = 1;
				g_Enemy[i].y = 2;
				g_Enemy[i].hp = ELITE_ENEMY_HP;
				g_Enemy[i].type = elite;
				g_Enemy[i].width = 10;
				g_Enemy[i].height = 1;
				break;
			}
		}
		break;
	case boss:
		for (int i = 0; i < MAX_ENEMY; i++) {
			if (g_Enemy[i].visible == false) {
				g_Enemy[i].visible = true;
				g_Enemy[i].x = 1;
				g_Enemy[i].y = 2;
				g_Enemy[i].hp = BOSS_ENEMY_HP;
				g_Enemy[i].type = boss;
				g_Enemy[i].width = 9;
				g_Enemy[i].height = 5;
				break;
			}
		}
		break;
	}
}

void createBuff(BuffType type) {
	for (int i = 0; i < MAX_ENEMY; i++) {
		if (g_Enemy[i].visible == false) {
			g_Enemy[i].visible = true;
			g_Enemy[i].x = dfSCREEN_WIDTH / 2 + 1;
			g_Enemy[i].y = 2;
			g_Enemy[i].hp = BUFF_HP;
			g_Enemy[i].type = buff; // 이 객체는 'buff' 타입
			g_Enemy[i].buffType = type; // 어떤 버프를 줄지 저장
			g_Enemy[i].width = 8;
			g_Enemy[i].height = 1;
			break;
		}
	}
}

void initBullet() {
	for (int i = 0; i < MAX_BULLET; i++) {
		g_Bullet[i].visible = FALSE;
	}
}

void moveBullet() {
	for (int i = 0; i < MAX_BULLET; i++) {
		if (g_Bullet[i].visible) {
			g_Bullet[i].y--;
			if (g_Bullet[i].y <= 1 || g_Bullet[i].y >= dfSCREEN_HEIGHT - 1 ||
				g_Bullet[i].x <= 0 || g_Bullet[i].x >= dfSCREEN_WIDTH - 2)
			{
				g_Bullet[i].visible = false; // 벽에 닿으면 총알을 비활성화
			}
		}
	}
}

void drawBullet() {
	for (int i = 0; i < MAX_BULLET; i++) {
		if (g_Bullet[i].visible) {
			switch (g_Bullet[i].type) {
			case normalBullet:
				Sprite_Draw(g_Bullet[i].x, g_Bullet[i].y, NORMAL_BULLET_SPRITE);
				break;
			case rareBullet:
				Sprite_Draw(g_Bullet[i].x, g_Bullet[i].y, RARE_BULLET_SPRITE);
				break;
			case bombBullet:
				Sprite_Draw(g_Bullet[i].x, g_Bullet[i].y, BOMB_BULLET_SPRITE);
				break;
			}

		}
	}
}

void normalGun(int playerX) {
	for (int i = 0; i < MAX_BULLET; i++) {
		if (g_Bullet[i].visible == false) {
			g_Bullet[i].visible = true;
			g_Bullet[i].x = playerX;
			g_Bullet[i].y = PLAYER_YPOS - 1;
			g_Bullet[i].damage = NORMAL_GUN_DAMAGE;
			g_Bullet[i].type = normalBullet;
			break;
		}
	}
}

void rareGun(int playerX) {
	int bulletsFired = 0;
	for (int i = 0; i < MAX_BULLET; i++) {
		if (g_Bullet[i].visible == false) {
			if (bulletsFired == 0) {
				g_Bullet[i].x = playerX + 1;
			}
			else if (bulletsFired == 1) {
				g_Bullet[i].x = playerX - 1;
			}
			else if (bulletsFired == 2) {
				g_Bullet[i].x = playerX;

			}
			if (g_Bullet[i].x >= 0 && g_Bullet[i].x <= dfSCREEN_WIDTH) {
				g_Bullet[i].visible = true;
				g_Bullet[i].damage = RARE_GUN_DAMAGE;
			}

			g_Bullet[i].y = PLAYER_YPOS - 1;
			g_Bullet[i].type = rareBullet;
			bulletsFired++;
			if (bulletsFired % 3 == 0) break;

		}
	}
}

void bombGun(int playerX) {
	for (int i = 0; i < MAX_BULLET; i++) {
		if (g_Bullet[i].visible == false) {
			g_Bullet[i].visible = true;
			g_Bullet[i].x = playerX;
			g_Bullet[i].y = PLAYER_YPOS - 1;
			g_Bullet[i].damage = BOMB_GUN_DAMAGE;
			g_Bullet[i].isExplosive = true;
			g_Bullet[i].type = bombBullet;
			break;
		}
	}
}

void initPlayer() {
	for (int i = 0; i < MAX_PLAYERS; i++) {
		g_Players[i].active = false;
	}
	g_Players[0].active = true;
	g_Players[0].x = dfSCREEN_WIDTH / 2;
	g_PlayerCount = 1;
}

void drawPlayer() {
	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (g_Players[i].active) {
			Sprite_Draw(g_Players[i].x, PLAYER_YPOS - 1, 'A');
		}
	}
}

void movePlayer(int dir) {
	bool canMove = true;
	if (dir == LEFT) {
		for (int i = 0; i < MAX_PLAYERS; i++) {
			if (g_Players[i].active && g_Players[i].x <= 1) canMove = false;
		}
	}
	else if (dir == RIGHT) {
		for (int i = 0; i < MAX_PLAYERS; i++) {
			if (g_Players[i].active && g_Players[i].x >= dfSCREEN_WIDTH - 3) canMove = false;
		}
	}

	if (canMove) {
		for (int i = 0; i < MAX_PLAYERS; i++) {
			if (g_Players[i].active) {
				if (dir == LEFT) g_Players[i].x--;
				else if (dir == RIGHT) g_Players[i].x++;
			}
		}
	}
}

void fireGun(void) {
	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (g_Players[i].active) {
			switch (g_CurrentWeaponLevel) {
			case LEVEL_NORMAL:
				normalGun(g_Players[i].x);
				break;
			case LEVEL_RARE:
				rareGun(g_Players[i].x);
				break;
			case LEVEL_BOMB:
				bombGun(g_Players[i].x);
				break;
			}
		}
	}
}

void inputKeyboard() {
	// 이동 (g_FrameCounter를 이용해 속도 제한)
	if (g_FrameCounter % g_PlayerMoveSpeed == 0) {
		if (GetAsyncKeyState(VK_LEFT)) {
			movePlayer(LEFT);
		}
		if (GetAsyncKeyState(VK_RIGHT)) {
			movePlayer(RIGHT);
		}
	}

	// 공격 (g_AttackCooldown을 이용해 연사 속도 제한)
	if (GetAsyncKeyState('A')) {
		if (g_AttackCooldown == 0) {
			fireGun();
			if (g_CurrentWeaponLevel == LEVEL_BOMB)
				g_AttackCooldown = g_AttackSpeed * 2;
			else
				g_AttackCooldown = g_AttackSpeed;

		}
	}
}

void StartPattern(enum EnemyPattern newPattern) {
	g_CurrentPattern = newPattern;
	g_PatternCounter = 0; // 패턴 카운터 초기화
}

void UpdatePattern() {
	// 현재 진행중인 패턴이 없으면 아무것도 하지 않음
	if (g_CurrentPattern == PATTERN_NONE) {
		return;
	}

	// 패턴 카운터를 1 증가
	g_PatternCounter++;

	// switch 문으로 각 패턴의 로직을 처리
	switch (g_CurrentPattern) {
	case PATTERN_V_SHAPE:
		break;

	case PATTERN_SIDE_WAVE:
		// (여기에 새로운 패턴 로직 추가)
		break;

	case PATTERN_LINE_DROP:
		// (여기에 새로운 패턴 로직 추가)
		break;
	}
}