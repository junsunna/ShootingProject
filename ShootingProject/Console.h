//////////////////////////////////////////////////////////////
// Windows 의 콘솔 화면에서 커서 제어.
//
//////////////////////////////////////////////////////////////
#ifndef __CONSOLE__
#define __CONSOLE__

#define dfSCREEN_WIDTH		21		// 콘솔 가로 80칸 + NULL
#define dfSCREEN_HEIGHT		21		// 콘솔 세로 24칸
#define PLAYER_SPRITE   'A'
#define NORMAL_BULLET_SPRITE        '*'
#define RARE_BULLET_SPRITE   '#'
#define BOMB_BULLET_SPRITE   '!'
#define NORMAL_ENEMY_SPRITE    '@'
#define ELITE_ENEMY_SPRITE     'E'

//-------------------------------------------------------------
// 콘솔 제어를 위한 준비 작업.
//
//-------------------------------------------------------------
void cs_Initial(void);


//-------------------------------------------------------------
// 콘솔 화면의 커서를 X, Y 좌표로 이동시킨다.
//
//-------------------------------------------------------------
void cs_MoveCursor(char iPosX, char iPosY);

//-------------------------------------------------------------
// 콘솔 화면을 조기화 한다.
//
//-------------------------------------------------------------
void cs_ClearScreen(void);


#endif

