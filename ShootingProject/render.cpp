#include "game.h"

// 렌더 함수 구현부

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
		// --- 승리했을 때 화면 ---
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
		// --- 패배했을 때 화면 ---
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
void LoadingScene_Render(void) {
	const char* loadingText = "Loading...";
	int startX = (dfSCREEN_WIDTH / 2) - (strlen(loadingText) / 2);
	int startY = dfSCREEN_HEIGHT / 2;
	for (int i = 0; loadingText[i] != '\0'; ++i) {
		Sprite_Draw(startX + i, startY, loadingText[i]);
	}
}