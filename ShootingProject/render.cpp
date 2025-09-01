#include "game.h"

// ���� �Լ� ������

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
void EndScene_Render(void) {
	if (g_GameClear) {
		// --- �¸����� �� ȭ�� ---
		char text[6][50] = {
			" * * * V I C T O R Y * * * ",
			"",
			"",
			" (N)ext Stage ",
			" (S)tart Menu ",
			" (Q)uit "
		};

		int startY = 8;
		for (int i = 0; i < 6; i++) {
			int startX = (dfSCREEN_WIDTH / 2) - (strlen(text[i]) / 2);
			for (int j = 0; text[i][j] != '\0'; j++) {
				Sprite_Draw(startX + j, startY + i, text[i][j]);
			}
		}
	}
	else {
		// --- �й����� �� ȭ�� ---
		char text[5][50] = {
			" G A M E   O V E R ",
			"",
			"",
			" (R)estart ",
			" (Q)uit "
		};

		int startY = 8;
		for (int i = 0; i < 5; i++) {
			int startX = (dfSCREEN_WIDTH / 2) - (strlen(text[i]) / 2);
			for (int j = 0; text[i][j] != '\0'; j++) {
				Sprite_Draw(startX + j, startY + i, text[i][j]);
			}
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
void drawPlayer() {
	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (g_Players[i].active) {
			Sprite_Draw(g_Players[i].x, PLAYER_YPOS - 1, 'A');
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
void LoadingScene_Render(void) {
	const char* loadingText = "Loading...";
	int startX = (dfSCREEN_WIDTH / 2) - (strlen(loadingText) / 2);
	int startY = dfSCREEN_HEIGHT / 2;
	for (int i = 0; loadingText[i] != '\0'; ++i) {
		Sprite_Draw(startX + i, startY, loadingText[i]);
	}
}