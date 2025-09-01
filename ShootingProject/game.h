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
// 상수, 열거형, 구조체 정의
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
	SCENE_START, // 시작 화면
	SCENE_LOADING,
	SCENE_GAME,  // 게임 진행 화면
	SCENE_END,    // 종료 화면
	SCENE_EXIT    // 종료 상태
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
	PATTERN_V_SHAPE,   // V자 형태로 내려오는 패턴
	PATTERN_SIDE_WAVE, // 양쪽에서 번갈아 나오는 패턴
	PATTERN_LINE_DROP  // 한 줄로 떨어지는 패턴
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
// 전역 변수 선언 (extern 키워드)
// ====================================================================
extern char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];
extern struct Player g_Players[MAX_PLAYERS];
extern struct Bullet g_Bullet[MAX_BULLET];
extern struct Enemy g_Enemy[MAX_ENEMY];
extern int g_PlayerCount; // 현재 활성화된 플레이어 수 - 생명

extern int g_FrameCounter;      // 프레임 카운터 (이동, 공격 속도 조절용)
extern int g_PlayerMoveSpeed;   // 플레이어는 2프레임당 1번 움직임
extern int g_EnemyMoveSpeed;   // 적은 10프레임당 1번 움직임
extern int g_AttackCooldown;    // 공격 딜레이 (0일 때만 발사 가능)
extern int g_AttackSpeed;      // 1프레임당 1발 발사
extern int g_EnemyCooldown;    // 적 생성 주기
extern bool g_BossSpawned; // 보스 스폰
extern bool g_GameClear; // 보스 죽이면 클리어
extern int g_PatternCounter;                         // 패턴 진행도를 나타내는 카운터
extern int g_stageEventIndex;
extern int g_currentStageIndex; // 현 스테이지
extern int g_stageCount;

extern enum EnemyPattern g_CurrentPattern; // 현재 진행중인 패턴
extern enum GameScene g_CurrentScene; // 현재 게임 씬을 저장하는 변수 (초기값은 시작 화면)
extern enum WeaponLevel g_CurrentWeaponLevel;



// ====================================================================
// 함수 원형 선언
// ====================================================================
// 입력
void inputKeyboard(void);

// 씬 관련
void StartScene_Update(void);
void StartScene_Render(void);
void EndScene_Update(void);
void EndScene_Render(void);

// 로직
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

// 렌더링
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