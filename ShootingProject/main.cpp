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
	SCENE_START, // ���� ȭ��
	SCENE_GAME,  // ���� ���� ȭ��
	SCENE_END,    // ���� ȭ��
	SCENE_EXIT    // ���� ����
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
	PATTERN_V_SHAPE,   // V�� ���·� �������� ����
	PATTERN_SIDE_WAVE, // ���ʿ��� ������ ������ ����
	PATTERN_LINE_DROP  // �� �ٷ� �������� ����
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
int g_PlayerCount = 0; // ���� Ȱ��ȭ�� �÷��̾� �� - ����

int g_FrameCounter = 0;      // ������ ī���� (�̵�, ���� �ӵ� ������)
int g_PlayerMoveSpeed = 2;   // �÷��̾�� 2�����Ӵ� 1�� ������
int g_EnemyMoveSpeed = 10;   // ���� 10�����Ӵ� 1�� ������
int g_AttackCooldown = 0;    // ���� ������ (0�� ���� �߻� ����)
int g_AttackSpeed = 2;      // 1�����Ӵ� 1�� �߻�
int g_EnemyCooldown = 0;    // �� ���� �ֱ�
bool g_BossSpawned = false; // ���� ����
bool g_GameClear = false; // ���� ���̸� Ŭ����
int g_PatternCounter = 0;                         // ���� ���൵�� ��Ÿ���� ī����
int g_LastPatternFrame = 0;                       // ������ ������ ���� �ð�

enum EnemyPattern g_CurrentPattern = PATTERN_NONE; // ���� �������� ����
enum GameScene g_CurrentScene = SCENE_START; // ���� ���� ���� �����ϴ� ���� (�ʱⰪ�� ���� ȭ��)
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
			// 1. Ű���� �Էº�
			inputKeyboard();
			if (g_AttackCooldown > 0) {
				g_AttackCooldown--;
			}

			// 2. ������ 
			moveBullet();
			if (g_FrameCounter % g_EnemyMoveSpeed == 0) {
				moveEnemy();
			}
			if (g_FrameCounter > 0 && g_FrameCounter % 200 == 0) {
				createBuff(BUFF_WEAPON_UPGRADE); // 200 �����Ӹ��� ���� ���׷��̵� ���� ����
			}

			if (g_FrameCounter == 1800 && !g_BossSpawned) { // ���� ����
				createEnemy(boss);
				g_BossSpawned = true;
			}


			checkCollision();
			if (g_FrameCounter > 0) {
				if (g_FrameCounter % 250 == 0) { // 250�����Ӹ��� ���� ����
					// �����ϰ� ���� ���� ����
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
			// FPS ī����
			g_FrameCounter++;
			break;
		case SCENE_END:
			EndScene_Update();
			break;
		}
		endTick = timeGetTime();
		// ������ ���� ��� �ڵ�
		long useTick = (long)(endTick - startTick);
		//if (useTick > 30) continue;
		if (useTick > 30)
		{
			startTick += 30;
			continue;
		}
		// 3. ������
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

		// ��� �ڵ�
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
// ������ ������ ȭ������ ����ִ� �Լ�.
//
// ����,�Ʊ�,�Ѿ� ���� szScreenBuffer �� �־��ְ�, 
// 1 �������� ������ �������� �� �Լ��� ȣ���Ͽ� ���� -> ȭ�� ���� �׸���.
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
// ȭ�� ���۸� �����ִ� �Լ�
//
// �� ������ �׸��� �׸��� ������ ���۸� ���� �ش�. 
// �ȱ׷��� ���� �������� �ܻ��� �����ϱ�
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
// ������ Ư�� ��ġ�� ���ϴ� ���ڸ� ���.
//
// �Է� ���� X,Y ��ǥ�� �ƽ�Ű�ڵ� �ϳ��� ����Ѵ�. (���ۿ� �׸�)
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
	// Enter Ű�� ������ ���� ������ ����
	if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
		g_CurrentScene = SCENE_GAME;
		g_PlayerCount = 0; // ���� Ȱ��ȭ�� �÷��̾� �� - ����
		g_FrameCounter = 0;      // ������ ī���� (�̵�, ���� �ӵ� ������)
		g_PlayerMoveSpeed = 2;   // �÷��̾�� 2�����Ӵ� 1�� ������
		g_EnemyMoveSpeed = 10;   // ���� 10�����Ӵ� 1�� ������
		g_AttackCooldown = 0;    // ���� ������ (0�� ���� �߻� ����)
		g_AttackSpeed = 2;      // 1�����Ӵ� 1�� �߻�
		g_EnemyCooldown = 0;    // �� ���� �ֱ�
		g_BossSpawned = false; // ���� ����
		g_GameClear = false; // ���� ���̸� Ŭ����
		g_CurrentWeaponLevel = LEVEL_NORMAL;
		g_PatternCounter = 0;                         // ���� ���൵�� ��Ÿ���� ī����
		g_LastPatternFrame = 0;                       // ������ ������ ���� �ð�
		g_CurrentPattern = PATTERN_NONE; // ���� �������� ����
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
	// 'R' Ű�� ������ ���� ȭ������ ���ư� �����
	if (GetAsyncKeyState('R') & 0x8000) {
		g_CurrentScene = SCENE_START;
	}
	// 'Q' Ű�� ������ ���� ���� ���·� ����
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
	// ���ʰ� �Ʒ��� �� �׸���
	for (i = 0; i < dfSCREEN_WIDTH - 1; i++)
	{
		Sprite_Draw(i, 1, '-'); // �� ����(Y=0)�� UI�� ���� ����ΰ� Y=1�� �׸��ϴ�.
		Sprite_Draw(i, dfSCREEN_HEIGHT - 1, '-');
	}

	// ���ʰ� ������ �� �׸���
	for (i = 2; i < dfSCREEN_HEIGHT - 1; i++)
	{
		Sprite_Draw(0, i, '|');
		Sprite_Draw(dfSCREEN_WIDTH - 2, i, '|');
	}

	// �𼭸� �׸���
	Sprite_Draw(0, 1, '+');
	Sprite_Draw(dfSCREEN_WIDTH - 2, 1, '+');
	Sprite_Draw(0, dfSCREEN_HEIGHT - 1, '+');
	Sprite_Draw(dfSCREEN_WIDTH - 2, dfSCREEN_HEIGHT - 1, '+');

	// ��� ���� �� �׸���
	int middleX = dfSCREEN_WIDTH / 2;
	for (i = 2; i <= dfSCREEN_HEIGHT - 5; i++) // y=2���� �����ؼ� ���� ���� ��ġ�� �ʰ� ��
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
				g_Bullet[i].visible = false; // ��� ���� ������ �Ѿ��� ��Ȱ��ȭ
				continue;
			}
			// ������ �ٱ��� �� �浹 Ȯ��
			else if (g_Bullet[i].y <= 1 || g_Bullet[i].y >= dfSCREEN_HEIGHT - 1 ||
				g_Bullet[i].x <= 0 || g_Bullet[i].x >= dfSCREEN_WIDTH - 2)
			{
				g_Bullet[i].visible = false; // �ٱ��� ���� ��Ƶ� ��Ȱ��ȭ
				continue;
			}

			for (int j = 0; j < MAX_ENEMY; j++) {
				if (!g_Enemy[j].visible) continue;

				if (g_Bullet[i].x >= g_Enemy[j].x && g_Bullet[i].x < g_Enemy[j].x + g_Enemy[j].width &&
					g_Bullet[i].y >= g_Enemy[j].y && g_Bullet[i].y < g_Enemy[j].y + g_Enemy[j].height)
				{
					int impactX = g_Bullet[i].x; // ������ X��ǥ ����
					int impactY = g_Bullet[i].y; // ������ Y��ǥ ����
					g_Bullet[i].visible = false;
					g_Enemy[j].hp -= g_Bullet[i].damage; // ���� HP�� �Ѿ��� ��������ŭ ����

					if (g_Enemy[j].hp <= 0) {
						g_Enemy[j].visible = false;
						if (g_Enemy[j].type == boss) {
							g_CurrentScene = SCENE_END; // ������ ���̸� ���� ������ ��ȯ
							break; // �� �̻� �浹 �˻縦 �� �ʿ䰡 �����Ƿ� ���� Ż��
						}
						if (g_Enemy[j].type == buff) {

							switch (g_Enemy[j].buffType) {
							case BUFF_DUPLICATE: // �÷��̾� ���� �ø��� ���� ����
								if (g_PlayerCount < MAX_PLAYERS) {
									int newPlayerIdx = -1;
									// ��Ȱ���� �÷��̾� Ž��
									for (int i = 0; i < MAX_PLAYERS; i++) {
										if (!g_Players[i].active) {
											newPlayerIdx = i;
											break;
										}
									}

									if (newPlayerIdx != -1) {
										int newX = -1;
										bool positionFound = false; // ��ġ�� ã�Ҵ��� Ȯ���ϴ� �÷���

										// ��� ���� �÷��̾ ��ȸ
										// ���� �÷��̾� ���� ���� Ȥ�� �����ʿ� ��Ȯ�� ���� ���ؼ�
										for (int i = 0; i < MAX_PLAYERS; i++) {
											if (g_Players[i].active) {
												int potential_positions[] = { g_Players[i].x + 1, g_Players[i].x - 1 };

												for (int p_idx = 0; p_idx < 2; p_idx++) {
													int potentialX = potential_positions[p_idx];
													bool isOccupied = false;

													// ���� ������ �ǳʶ�
													if (potentialX < 1 || potentialX > dfSCREEN_WIDTH - 3) {
														continue;
													}

													// ã�� ��ġ�� ���� �÷��̾� �ִ��� üũ
													for (int k = 0; k < MAX_PLAYERS; k++) {
														if (g_Players[k].active && g_Players[k].x == potentialX) {
															isOccupied = true;
															break;
														}
													}

													// �� �ڸ��� ��ġ Ȯ��
													if (!isOccupied) {
														newX = potentialX;
														positionFound = true;
														break; // ���� for�� Ż��
													}
												}
											}
											if (positionFound) {
												break; // �ٱ��� for�� Ż��
											}
										}

										// ���ڸ� ã������ Ȱ��ȭ
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
			{ // C���� case �� ���� ���� ����� ���� �߰�ȣ
				// 5x11 ũ���� ���� �ƽ�Ű ��Ʈ
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

				// ���� HP �� �׸���
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
			g_Enemy[i].type = buff; // �� ��ü�� 'buff' Ÿ��
			g_Enemy[i].buffType = type; // � ������ ���� ����
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
				g_Bullet[i].visible = false; // ���� ������ �Ѿ��� ��Ȱ��ȭ
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
	// �̵� (g_FrameCounter�� �̿��� �ӵ� ����)
	if (g_FrameCounter % g_PlayerMoveSpeed == 0) {
		if (GetAsyncKeyState(VK_LEFT)) {
			movePlayer(LEFT);
		}
		if (GetAsyncKeyState(VK_RIGHT)) {
			movePlayer(RIGHT);
		}
	}

	// ���� (g_AttackCooldown�� �̿��� ���� �ӵ� ����)
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
	g_PatternCounter = 0; // ���� ī���� �ʱ�ȭ
}

void UpdatePattern() {
	// ���� �������� ������ ������ �ƹ��͵� ���� ����
	if (g_CurrentPattern == PATTERN_NONE) {
		return;
	}

	// ���� ī���͸� 1 ����
	g_PatternCounter++;

	// switch ������ �� ������ ������ ó��
	switch (g_CurrentPattern) {
	case PATTERN_V_SHAPE:
		break;

	case PATTERN_SIDE_WAVE:
		// (���⿡ ���ο� ���� ���� �߰�)
		break;

	case PATTERN_LINE_DROP:
		// (���⿡ ���ο� ���� ���� �߰�)
		break;
	}
}