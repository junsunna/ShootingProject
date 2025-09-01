#include "data.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

// ... 전역 변수 정의 ...
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
        // 각 파일명 문자열에 대한 메모리 해제
        for (int i = 0; i < g_stageCount; ++i) {
            delete[] g_stageFileNames[i];
        }
        // 파일명 포인터 배열 자체에 대한 메모리 해제
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
    // 바이너리 모드
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // 첫 4바이트는 스테이지 개수
    int totalStages = 0;
    file.read(reinterpret_cast<char*>(&totalStages), sizeof(int));

    // 벡터의 크기를 조절
    g_enemyStatsPerStage.resize(totalStages);

    // 파일 내용을 통으로 읽기
    file.read(reinterpret_cast<char*>(g_enemyStatsPerStage.data()), totalStages * sizeof(EnemyStats));

    file.close();
    return true;
}

