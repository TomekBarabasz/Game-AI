// gra_w_pana.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "GameRules.h"

IGameState* createGraWPanaGameState();

void testGraWPanaRules();

//int main()
//int _tmain(int argc, char32_t* argv[])
int wmain(int argc, wchar_t *argv[], wchar_t *envp[])
{
	auto & s = *createGraWPanaGameState();
	s.initialize(2);

	std::wcout << L"Initial state is" << std::endl;
	std::wcout << s.toString() << std::endl;
	
	testGraWPanaRules();

	auto mv1 = s.getPlayerLegalMoves(0);
	auto mv2 = s.getPlayerLegalMoves(1);
	std::wcout << L"Player 1 legal moves : " << std::endl;
	for (auto * mv : mv1) {
		std::wcout << mv->toString() << std::endl;
		mv->release();
	}
	std::wcout << L"Player 2 legal moves : " << std::endl;
	for (auto * mv : mv2) {
		std::wcout << mv->toString() << std::endl;
		mv->release();
	}

	s.release();

	return 0;
}
