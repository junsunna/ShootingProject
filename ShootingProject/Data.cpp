#include "data.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

// ... ���� ���� ���� ...
std::vector<EnemyStats> g_enemyStatsPerStage;
std::vector<StageEvent> g_currentStagePattern;
int g_stageCount = 0;
char** g_stageFileNames = nullptr;

bool LoadEnemyData(const std::string& filename);
bool LoadStageData(const std::string& filename);

bool LoadGameData() {
    if (!LoadEnemyData("enemies.bin")) {
        return false;
    }

    if (!LoadStageData("stages.txt")) {
        return false;
    }

    return true;
}

void FreeGameData() {
    if (g_stageFileNames != nullptr) {
        // �� ���ϸ� ���ڿ��� ���� �޸� ����
        for (int i = 0; i < g_stageCount; ++i) {
            delete[] g_stageFileNames[i];
        }
        // ���ϸ� ������ �迭 ��ü�� ���� �޸� ����
        delete[] g_stageFileNames;
        g_stageFileNames = nullptr;
    }
}

bool LoadStagePattern(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		return false;
	}

	g_currentStagePattern.clear();

	std::string line;
	while (std::getline(file, line)) {
		if (line.empty()) {
			continue;
		}

		int frame, command;

		if (sscanf_s(line.c_str(), "%d %d", &frame, &command) == 2) {
			g_currentStagePattern.push_back({ frame, command });
		}
	}

	file.close();
	return true;
}

bool LoadStageData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    file >> g_stageCount;
    g_stageFileNames = new char* [g_stageCount];

    for (int i = 0; i < g_stageCount; ++i) {
        g_stageFileNames[i] = new char[20];
        file >> g_stageFileNames[i];
    }

    file.close();
    return true;
}

bool LoadEnemyData(const std::string& filename) {
    // ���̳ʸ� ���
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // ù 4����Ʈ�� �������� ����
    int totalStages = 0;
    file.read(reinterpret_cast<char*>(&totalStages), sizeof(int));

    // ������ ũ�⸦ ����
    g_enemyStatsPerStage.resize(totalStages);

    // ���� ������ ������ �б�
    file.read(reinterpret_cast<char*>(g_enemyStatsPerStage.data()), totalStages * sizeof(EnemyStats));

    file.close();
    return true;
}

