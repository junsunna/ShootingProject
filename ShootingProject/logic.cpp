#include "game.h"
#include "data.h"

// 키 입력
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

void EndScene_Update(void) {
	if (g_GameClear) {
		// --- 승리했을 때 로직 ---
		if (GetAsyncKeyState('N') & 0x8000) {
			// 다음 스테이지로
			g_currentStageIndex++;
			// 마지막 스테이지까지 클리어했다면 처음으로 돌아감
			if (g_currentStageIndex >= g_stageCount) {
				g_currentStageIndex = 0;
			}
			g_CurrentScene = SCENE_LOADING; // 로딩 씬으로 전환
		}
		else if (GetAsyncKeyState('S') & 0x8000) {
			// 시작 메뉴로
			g_currentStageIndex = 0; // 스테이지 인덱스 초기화
			g_CurrentScene = SCENE_START;
		}
		else if (GetAsyncKeyState('Q') & 0x8000) {
			// 게임 종료
			g_CurrentScene = SCENE_EXIT;
		}
	}
	else {
		// --- 패배했을 때 로직 ---
		if (GetAsyncKeyState('R') & 0x8000) {
			// 처음부터 다시 시작
			g_currentStageIndex = 0; // 스테이지 인덱스 초기화
			g_CurrentScene = SCENE_START;
		}
		else if (GetAsyncKeyState('Q') & 0x8000) {
			// 게임 종료
			g_CurrentScene = SCENE_EXIT;
		}
	}
}

// 화면
void LoadingScene_Update(void) {
	// 1. 게임 전체 데이터를 아직 로드하지 않았다면 로드합니다.
	srand(0); // 버프도 패턴으로
	static bool isGameDataLoaded = false;
	initPlayer();
	initEnemy();
	initBullet();
	if (!isGameDataLoaded) {
		if (!LoadGameData()) {
			// 데이터 로딩 실패 시 게임 종료 (에러 처리는 나중에 추가 가능)
			g_CurrentScene = SCENE_EXIT;
			return;
		}
		isGameDataLoaded = true;
	}

	// 2. 현재 스테이지의 패턴 파일을 로드합니다. (우선 1단계만 로드)
	if (g_stageCount > g_currentStageIndex) {
		LoadStagePattern(g_stageFileNames[g_currentStageIndex]);
	}

	// 3. 게임 시작을 위한 변수들을 초기화합니다.
	g_FrameCounter = 0;
	g_stageEventIndex = 0;
	g_CurrentPattern = PATTERN_NONE;
	g_BossSpawned = false;
	g_GameClear = false;
	initPlayer();
	initEnemy();
	initBullet();

	// 4. 모든 준비가 끝나면 게임 씬으로 전환합니다.
	g_CurrentScene = SCENE_GAME;
}

void StartScene_Update(void) {
	// Enter 키가 눌리면 게임 씬으로 변경
	if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
		g_CurrentScene = SCENE_LOADING;
		g_PlayerCount = 0; // 현재 활성화된 플레이어 수 - 생명
		g_FrameCounter = 0;      // 프레임 카운터 (이동, 공격 속도 조절용)
		g_PlayerMoveSpeed = 2;   // 플레이어는 2프레임당 1번 움직임
		g_EnemyMoveSpeed = 10;   // 적은 10프레임당 1번 움직임
		g_AttackCooldown = 0;    // 공격 딜레이 (0일 때만 발사 가능)
		g_AttackSpeed = 3;      // 3프레임당 1발 발사
		g_EnemyCooldown = 0;    // 적 생성 주기
		g_BossSpawned = false; // 보스 스폰
		g_GameClear = false; // 보스 죽이면 클리어
		g_CurrentWeaponLevel = LEVEL_NORMAL;
		g_PatternCounter = 0;                         // 패턴 진행도를 나타내는 카운터
		g_CurrentPattern = PATTERN_NONE; // 현재 진행중인 패턴
	}
}


// 충돌 생성 이동
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
							g_GameClear = true;
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


void createEnemy(EnemyType pattern, int x, int y) {
	const EnemyStats& currentStageStats = g_enemyStatsPerStage[g_currentStageIndex];
	switch (pattern) {
	case normal:
		for (int i = 0; i < MAX_ENEMY; i++) {
			if (g_Enemy[i].visible == false) {
				g_Enemy[i].visible = true;
				g_Enemy[i].x = x;
				g_Enemy[i].y = y;
				g_Enemy[i].hp = currentStageStats.normalHp;
				g_Enemy[i].type = normal;
				g_Enemy[i].width = 1;
				g_Enemy[i].height = 1;
				break;
			}
		}
		break;
	case elite:
		for (int i = 0; i < MAX_ENEMY; i++) {
			if (g_Enemy[i].visible == false) {
				g_Enemy[i].visible = true;
				g_Enemy[i].x = x;
				g_Enemy[i].y = y;
				g_Enemy[i].hp = currentStageStats.eliteHp;
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
				g_Enemy[i].x = x;
				g_Enemy[i].y = y;
				g_Enemy[i].hp = currentStageStats.bossHp;
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
	g_CurrentWeaponLevel = LEVEL_NORMAL;
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


// 패턴
void StartPattern(enum EnemyPattern newPattern) {
	g_CurrentPattern = newPattern;
	g_PatternCounter = 0; // 패턴 카운터 초기화
}

void UpdatePattern() {
	// 현재 진행중인 패턴이 없으면 아무것도 하지 않음
	if (g_CurrentPattern == PATTERN_NONE) {
		return;
	}

	switch (g_CurrentPattern) {
	case PATTERN_V_SHAPE:
	{
		int centerX = dfSCREEN_WIDTH / 4 - 1; // V자의 중심 X좌표

		// g_PatternCounter 값에 따라 다른 위치에 적 생성
		if (g_PatternCounter == 0) {
			createEnemy(normal, centerX, 2);
		}
		else if (g_PatternCounter == 1) {
			createEnemy(normal, centerX - 1, 2);
			createEnemy(normal, centerX + 1, 2);
		}
		else if (g_PatternCounter == 2) {
			createEnemy(normal, centerX - 2, 2);
			createEnemy(normal, centerX + 2, 2);
		}
		else if (g_PatternCounter == 3) {
			createEnemy(normal, centerX - 3, 2);
			createEnemy(normal, centerX + 3, 2);
		}
		else if (g_PatternCounter == 4) {
			createEnemy(elite, 1, 2);
		}

		// 다음 단계를 위해 카운터 증가
		g_PatternCounter++;

		// 마지막 단계까지 끝나면 패턴 종료
		if (g_PatternCounter > 4) {
			g_PatternCounter = 0;
		}
	}
	break;
	case PATTERN_SIDE_WAVE: {
		int centerX = dfSCREEN_WIDTH / 4 - 1; // V자의 중심 X좌표

		// g_PatternCounter 값에 따라 다른 위치에 적 생성
		if (g_PatternCounter == 0) {
			createEnemy(normal, centerX - 3, 2);
			createEnemy(normal, centerX - 2, 2);
			createEnemy(normal, centerX + 3, 2);
			createEnemy(normal, centerX + 4, 2);
		}
		else if (g_PatternCounter == 1) {
			createEnemy(normal, centerX, 2);
			createEnemy(normal, centerX + 1, 2);
		}
		else if (g_PatternCounter == 2) {
			createEnemy(normal, centerX - 3, 2);
			createEnemy(normal, centerX, 2);
			createEnemy(normal, centerX + 1, 2);
			createEnemy(normal, centerX + 4, 2);

		}
		else if (g_PatternCounter == 3) {
			createEnemy(normal, centerX - 1, 2);
			createEnemy(normal, centerX - 2, 2);
			createEnemy(normal, centerX - 3, 2);
			createEnemy(normal, centerX, 2);
			createEnemy(normal, centerX + 1, 2);
			createEnemy(normal, centerX + 2, 2);
			createEnemy(normal, centerX + 3, 2);
			createEnemy(normal, centerX + 4, 2);
		}
		else if (g_PatternCounter == 4) {
			createEnemy(elite, 1, 2);
		}

		// 다음 단계를 위해 카운터 증가
		g_PatternCounter++;

		// 마지막 단계까지 끝나면 패턴 종료
		if (g_PatternCounter > 4) {
			g_PatternCounter = 0;
		}
	}
	break;

	case PATTERN_LINE_DROP:
		g_PatternCounter++;
		if (g_PatternCounter % 4 == 0) {
			createEnemy(elite, 1, 2);
		}
		else {
			for (int i = 1; i <= dfSCREEN_WIDTH / 2 - 1; i++) {
				createEnemy(normal, i, 2);
			}
		}
		break;
	}
}