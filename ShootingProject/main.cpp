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
int g_PlayerCount = 0; // ���� Ȱ��ȭ�� �÷��̾� �� - ����

int g_FrameCounter = 0;      // ������ ī���� (�̵�, ���� �ӵ� ������)
int g_PlayerMoveSpeed = 2;   // �÷��̾�� 2�����Ӵ� 1�� ������
int g_EnemyMoveSpeed = 10;   // ���� 10�����Ӵ� 1�� ������
int g_AttackCooldown = 0;    // ���� ������ (0�� ���� �߻� ����)
int g_AttackSpeed = 4;      // �����Ӵ� 1�� �߻�
int g_EnemyCooldown = 0;    // �� ���� �ֱ�
bool g_BossSpawned = false; // ���� ����
bool g_GameClear = false; // ���� ���̸� Ŭ����
int g_PatternCounter = 0;                         // ���� ���൵�� ��Ÿ���� ī����
int g_stageEventIndex = 0; // ���� �������� �ε���
int g_currentStageIndex = 0;

enum EnemyPattern g_CurrentPattern = PATTERN_NONE; // ���� �������� ����
enum GameScene g_CurrentScene = SCENE_START; // ���� ���� ���� �����ϴ� ���� (�ʱⰪ�� ���� ȭ��)
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
			// ���� ȭ��
			StartScene_Update();
			break;
		case SCENE_LOADING:
			// �ε�
			LoadingScene_Update();
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

			if (g_FrameCounter == 900 && !g_BossSpawned) { // ���� ����
				createEnemy(boss, 1, 2);
				g_BossSpawned = true;
			}
			// --- ������ ��� �������� �̺�Ʈ ���� ---
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

			// ���� ���� ������ ���� �ܰ踦 ����
			if (g_CurrentPattern != PATTERN_NONE && g_FrameCounter % ENEMY_CREATE_INTERVAL == 0) {
				UpdatePattern();
			}
			if (g_FrameCounter != 0 && g_FrameCounter % 250 == 0) {
				// �����ϰ� ���� ���� ����
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
		if (useTick > 30)
		{
			startTick += 30;
			continue;
		}
		// 3. ������
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

	FreeGameData();
	timeEndPeriod(1);

}         