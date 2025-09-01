#pragma once
#include <string>
#include <vector>

// ���������� �� ������ ������ ����ü
struct EnemyStats {
	int normalHp;
	int eliteHp;
	int bossHp;
};

struct StageEvent {
	int frame;    // �� �����ӿ� �����ϸ�
	int command;  // �� Ŀ�ǵ带 ����
};

// ... �Լ� ���� ...
bool LoadGameData();
void FreeGameData();
bool LoadStagePattern(const std::string& filename);
// --- �� ���� ---
extern std::vector<EnemyStats> g_enemyStatsPerStage;

// --- �������� ���� ---
extern int g_stageCount;
extern char** g_stageFileNames;
extern std::vector<StageEvent> g_currentStagePattern;