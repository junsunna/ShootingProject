#pragma once
#include <string>
#include <vector>

// 스테이지별 적 스탯을 저장할 구조체
struct EnemyStats {
	int normalHp;
	int eliteHp;
	int bossHp;
};

struct StageEvent {
	int frame;    // 이 프레임에 도달하면
	int command;  // 이 커맨드를 실행
};

// ... 함수 선언 ...
bool LoadGameData();
void FreeGameData();
bool LoadStagePattern(const std::string& filename);
// --- 적 정보 ---
extern std::vector<EnemyStats> g_enemyStatsPerStage;

// --- 스테이지 정보 ---
extern int g_stageCount;
extern char** g_stageFileNames;
extern std::vector<StageEvent> g_currentStagePattern;