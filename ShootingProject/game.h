#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <memory.h>
#include <Windows.h>
#include <mmsystem.h>
#include <time.h>
#include <stdbool.h>
#include "Console.h"

// ====================================================================
// ���, ������, ����ü ����
// ====================================================================
#define PLAYER_YPOS 20
#define MAX_PLAYERS 3
#define MAX_BULLET 500
#define MAX_ENEMY 500
#define ENEMY_CREATE_INTERVAL 10
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
	SCENE_LOADING,
	SCENE_GAME,  // ���� ���� ȭ��
	SCENE_END,    // ���� ȭ��
	SCENE_EXIT    // ���� ����
};

enum WeaponLevel {
	LEVEL_NORMAL,
	LEVEL_RARE,
	LEVEL_BOMB
};
enum BulletType {
	normalBullet,
	rareBullet,
	bombBullet
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

enum Key {
	LEFT,  // 0
	RIGHT, // 1
	ATTACK // 2
};


struct Player {
	bool active;
	char x;
};


struct Bullet {
	bool visible;
	char x;
	char y;
	bool isExplosive;
	int damage;
	BulletType type;
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

// ====================================================================
// ���� ���� ���� (extern Ű����)
// ====================================================================
extern char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];
extern struct Player g_Players[MAX_PLAYERS];
extern struct Bullet g_Bullet[MAX_BULLET];
extern struct Enemy g_Enemy[MAX_ENEMY];
extern int g_PlayerCount; // ���� Ȱ��ȭ�� �÷��̾� �� - ����

extern int g_FrameCounter;      // ������ ī���� (�̵�, ���� �ӵ� ������)
extern int g_PlayerMoveSpeed;   // �÷��̾�� 2�����Ӵ� 1�� ������
extern int g_EnemyMoveSpeed;   // ���� 10�����Ӵ� 1�� ������
extern int g_AttackCooldown;    // ���� ������ (0�� ���� �߻� ����)
extern int g_AttackSpeed;      // 1�����Ӵ� 1�� �߻�
extern int g_EnemyCooldown;    // �� ���� �ֱ�
extern bool g_BossSpawned; // ���� ����
extern bool g_GameClear; // ���� ���̸� Ŭ����
extern int g_PatternCounter;                         // ���� ���൵�� ��Ÿ���� ī����
extern int g_stageEventIndex;
extern int g_currentStageIndex; // �� ��������
extern int g_stageCount;

extern enum EnemyPattern g_CurrentPattern; // ���� �������� ����
extern enum GameScene g_CurrentScene; // ���� ���� ���� �����ϴ� ���� (�ʱⰪ�� ���� ȭ��)
extern enum WeaponLevel g_CurrentWeaponLevel;



// ====================================================================
// �Լ� ���� ����
// ====================================================================
// �Է�
void inputKeyboard(void);

// �� ����
void StartScene_Update(void);
void StartScene_Render(void);
void EndScene_Update(void);
void EndScene_Render(void);

// ����
void initPlayer(void);
void movePlayer(int);
void initBullet(void);
void normalGun(int);
void rareGun(int);
void bombGun(int);
void moveBullet(void);
void initEnemy(void);
void createEnemy(EnemyType, int, int);
void createBuff(BuffType);
void moveEnemy(void);
void checkCollision(void);
void StartPattern(enum EnemyPattern newPattern);
void UpdatePattern();
void LoadingScene_Update(void);

// ������
void drawUI(void);
void drawBox(void);
void Buffer_Flip(void);
void Buffer_Clear(void);
void Sprite_Draw(int iX, int iY, char chSprite);
void drawPlayer(void);
void drawBullet(void);
void drawEnemy(void);
void fireGun(void);
void LoadingScene_Render(void);


#endif // GAME_H