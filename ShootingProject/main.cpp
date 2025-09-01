#include <stdio.h>
#include <memory.h>
#include <Windows.h>
#include <mmsystem.h>
#include <time.h>
#include <stdbool.h>
#include "Console.h"
#include "game.h"
#include "data.h"
#pragma comment(lib, "winmm.lib")


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

char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];
struct Player g_Players[MAX_PLAYERS];
struct Bullet g_Bullet[MAX_BULLET];
struct Enemy g_Enemy[MAX_ENEMY];
int g_PlayerCount = 0; // 현재 활성화된 플레이어 수 - 생명

int g_FrameCounter = 0;      // 프레임 카운터 (이동, 공격 속도 조절용)
int g_PlayerMoveSpeed = 2;   // 플레이어는 2프레임당 1번 움직임
int g_EnemyMoveSpeed = 10;   // 적은 10프레임당 1번 움직임
int g_AttackCooldown = 0;    // 공격 딜레이 (0일 때만 발사 가능)
int g_AttackSpeed = 4;      // 프레임당 1발 발사
int g_EnemyCooldown = 0;    // 적 생성 주기
bool g_BossSpawned = false; // 보스 스폰
bool g_GameClear = false; // 보스 죽이면 클리어
int g_PatternCounter = 0;                         // 패턴 진행도를 나타내는 카운터
int g_stageEventIndex = 0; // 현재 스테이지 인덱스
int g_currentStageIndex = 0;

enum EnemyPattern g_CurrentPattern = PATTERN_NONE; // 현재 진행중인 패턴
enum GameScene g_CurrentScene = SCENE_START; // 현재 게임 씬을 저장하는 변수 (초기값은 시작 화면)
enum WeaponLevel g_CurrentWeaponLevel = LEVEL_NORMAL;

void main(void)
{
	timeBeginPeriod(1);
	unsigned long fpsTick = timeGetTime();
	unsigned long startTick, endTick;

	cs_Initial();
	srand(0);
	startTick = timeGetTime();
	while (g_CurrentScene != SCENE_EXIT)
	{
		switch (g_CurrentScene) {
		case SCENE_START:
			// 시작 화면
			StartScene_Update();
			break;
		case SCENE_LOADING:
			// 로딩
			LoadingScene_Update();
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

			if (g_FrameCounter == 900 && !g_BossSpawned) { // 보스 생성
				createEnemy(boss, 1, 2);
				g_BossSpawned = true;
			}
			// --- 데이터 기반 스테이지 이벤트 실행 ---
			if (g_stageEventIndex < g_currentStagePattern.size()) {
				if (g_FrameCounter >= g_currentStagePattern[g_stageEventIndex].frame) {
					int command = g_currentStagePattern[g_stageEventIndex].command;
					switch (command) {
					case 0: StartPattern(PATTERN_V_SHAPE); break;
					case 1: StartPattern(PATTERN_SIDE_WAVE); break;
					case 2: StartPattern(PATTERN_LINE_DROP); break;
						
					}
					g_stageEventIndex++;
				}
			}

			// 실행 중인 패턴의 다음 단계를 진행
			if (g_CurrentPattern != PATTERN_NONE && g_FrameCounter % ENEMY_CREATE_INTERVAL == 0) {
				UpdatePattern();
			}
			if (g_FrameCounter != 0 && g_FrameCounter % 250 == 0) {
				// 랜덤하게 버프 종류 결정
				if (rand() % 2 == 0) {
					createBuff(BUFF_WEAPON_UPGRADE);
				}
				else {
					createBuff(BUFF_DUPLICATE);
				}
			}

			checkCollision();
			
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
		if (useTick > 30)
		{
			startTick += 30;
			continue;
		}
		// 3. 랜더부
		Buffer_Clear();
		switch (g_CurrentScene) {
		case SCENE_START:StartScene_Render();		break;
		case SCENE_LOADING: LoadingScene_Render();	break;
		case SCENE_GAME:
			drawBox();
			drawUI();
			drawPlayer();
			drawBullet();
			drawEnemy();
			break;
		case SCENE_END:	EndScene_Render();			break;
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

	FreeGameData();
	timeEndPeriod(1);

}         