#include "game.h"
#include "data.h"

// Ű �Է�
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

void EndScene_Update(void) {
	if (g_GameClear) {
		// --- �¸����� �� ���� ---
		if (GetAsyncKeyState('N') & 0x8000) {
			// ���� ����������
			g_currentStageIndex++;
			// ������ ������������ Ŭ�����ߴٸ� ó������ ���ư�
			if (g_currentStageIndex >= g_stageCount) {
				g_currentStageIndex = 0;
			}
			g_CurrentScene = SCENE_LOADING; // �ε� ������ ��ȯ
		}
		else if (GetAsyncKeyState('S') & 0x8000) {
			// ���� �޴���
			g_currentStageIndex = 0; // �������� �ε��� �ʱ�ȭ
			g_CurrentScene = SCENE_START;
		}
		else if (GetAsyncKeyState('Q') & 0x8000) {
			// ���� ����
			g_CurrentScene = SCENE_EXIT;
		}
	}
	else {
		// --- �й����� �� ���� ---
		if (GetAsyncKeyState('R') & 0x8000) {
			// ó������ �ٽ� ����
			g_currentStageIndex = 0; // �������� �ε��� �ʱ�ȭ
			g_CurrentScene = SCENE_START;
		}
		else if (GetAsyncKeyState('Q') & 0x8000) {
			// ���� ����
			g_CurrentScene = SCENE_EXIT;
		}
	}
}

// ȭ��
void LoadingScene_Update(void) {
	// 1. ���� ��ü �����͸� ���� �ε����� �ʾҴٸ� �ε��մϴ�.
	srand(0); // ������ ��������
	static bool isGameDataLoaded = false;
	initPlayer();
	initEnemy();
	initBullet();
	if (!isGameDataLoaded) {
		if (!LoadGameData()) {
			// ������ �ε� ���� �� ���� ���� (���� ó���� ���߿� �߰� ����)
			g_CurrentScene = SCENE_EXIT;
			return;
		}
		isGameDataLoaded = true;
	}

	// 2. ���� ���������� ���� ������ �ε��մϴ�. (�켱 1�ܰ踸 �ε�)
	if (g_stageCount > g_currentStageIndex) {
		LoadStagePattern(g_stageFileNames[g_currentStageIndex]);
	}

	// 3. ���� ������ ���� �������� �ʱ�ȭ�մϴ�.
	g_FrameCounter = 0;
	g_stageEventIndex = 0;
	g_CurrentPattern = PATTERN_NONE;
	g_BossSpawned = false;
	g_GameClear = false;
	initPlayer();
	initEnemy();
	initBullet();

	// 4. ��� �غ� ������ ���� ������ ��ȯ�մϴ�.
	g_CurrentScene = SCENE_GAME;
}

void StartScene_Update(void) {
	// Enter Ű�� ������ ���� ������ ����
	if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
		g_CurrentScene = SCENE_LOADING;
		g_PlayerCount = 0; // ���� Ȱ��ȭ�� �÷��̾� �� - ����
		g_FrameCounter = 0;      // ������ ī���� (�̵�, ���� �ӵ� ������)
		g_PlayerMoveSpeed = 2;   // �÷��̾�� 2�����Ӵ� 1�� ������
		g_EnemyMoveSpeed = 10;   // ���� 10�����Ӵ� 1�� ������
		g_AttackCooldown = 0;    // ���� ������ (0�� ���� �߻� ����)
		g_AttackSpeed = 3;      // 3�����Ӵ� 1�� �߻�
		g_EnemyCooldown = 0;    // �� ���� �ֱ�
		g_BossSpawned = false; // ���� ����
		g_GameClear = false; // ���� ���̸� Ŭ����
		g_CurrentWeaponLevel = LEVEL_NORMAL;
		g_PatternCounter = 0;                         // ���� ���൵�� ��Ÿ���� ī����
		g_CurrentPattern = PATTERN_NONE; // ���� �������� ����
	}
}


// �浹 ���� �̵�
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
							g_GameClear = true;
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


// ����
void StartPattern(enum EnemyPattern newPattern) {
	g_CurrentPattern = newPattern;
	g_PatternCounter = 0; // ���� ī���� �ʱ�ȭ
}

void UpdatePattern() {
	// ���� �������� ������ ������ �ƹ��͵� ���� ����
	if (g_CurrentPattern == PATTERN_NONE) {
		return;
	}

	switch (g_CurrentPattern) {
	case PATTERN_V_SHAPE:
	{
		int centerX = dfSCREEN_WIDTH / 4 - 1; // V���� �߽� X��ǥ

		// g_PatternCounter ���� ���� �ٸ� ��ġ�� �� ����
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

		// ���� �ܰ踦 ���� ī���� ����
		g_PatternCounter++;

		// ������ �ܰ���� ������ ���� ����
		if (g_PatternCounter > 4) {
			g_PatternCounter = 0;
		}
	}
	break;
	case PATTERN_SIDE_WAVE: {
		int centerX = dfSCREEN_WIDTH / 4 - 1; // V���� �߽� X��ǥ

		// g_PatternCounter ���� ���� �ٸ� ��ġ�� �� ����
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

		// ���� �ܰ踦 ���� ī���� ����
		g_PatternCounter++;

		// ������ �ܰ���� ������ ���� ����
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