// maze.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include <random>
#include <iostream>

class Maze {
public:
	Maze(const unsigned width, const unsigned height)
		: width(width), height(height),
		horizontalWalls(height + 1, std::vector<bool>(width, true)),
		verticalWalls(width + 1, std::vector<bool>(height, true))
	{

	}

	const unsigned width;
	const unsigned height;
	std::vector<std::vector<bool>> horizontalWalls;
	std::vector<std::vector<bool>> verticalWalls;
};

std::random_device rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()

void generateArea(Maze & maze, const unsigned leftWall, const unsigned topWall, const unsigned rightWall, const unsigned bottomWall)
{
	const unsigned width = rightWall - leftWall;
	const unsigned height = bottomWall - topWall;

	if (width == 1 && height == 1) {
		return; // end of recursion, cannot split further
	}

	const unsigned verticalSplitOptions = width - 1;
	const unsigned horizontalSplitOptions = height - 1;
	const unsigned totalSplitOptions = verticalSplitOptions + horizontalSplitOptions;
	std::uniform_int_distribution<> dis(0, totalSplitOptions - 1);
	const unsigned splitSelection = dis(gen);

	const bool verticalSplit = splitSelection < verticalSplitOptions;

	if (verticalSplit) {
		const unsigned splitAt = splitSelection;
		std::uniform_int_distribution<> dis2(0, height - 1);
		const unsigned doorAt = dis2(gen);

		maze.verticalWalls[leftWall + splitAt + 1][topWall + doorAt] = false;

		generateArea(maze, leftWall, topWall, leftWall + splitAt + 1, bottomWall);
		generateArea(maze, leftWall + splitAt + 1, topWall, rightWall, bottomWall);
	}
	else { // horizontal split
		const unsigned splitAt = splitSelection - verticalSplitOptions;
		std::uniform_int_distribution<> dis2(0, width - 1);
		const unsigned doorAt = dis2(gen);

		maze.horizontalWalls[topWall + splitAt + 1][leftWall + doorAt] = false;

		generateArea(maze, leftWall, topWall, rightWall, topWall + splitAt + 1);
		generateArea(maze, leftWall, topWall + splitAt + 1, rightWall, bottomWall);
	}

}

void printCell(const Maze & maze, const unsigned row, const unsigned col)
{
	const bool openToTop = !maze.horizontalWalls[row][col]; // bit 0
	const bool openToBottom = !maze.horizontalWalls[row+1][col]; // bit 1
	const bool openToLeft = !maze.verticalWalls[col][row]; // bit 2
	const bool openToRight = !maze.verticalWalls[col+1][row]; // bit 3
	
	const unsigned value = openToTop + openToBottom * 2 + openToLeft * 4 + openToRight * 8;

	static const char elements[] = { 'X', 'V', 'A', 186, '>', 188, 187, 185, '<', 200, 201, 204, 205, 202, 203, 206 };

	std::cout << elements[value];
}

void printMaze(const Maze & maze)
{
	for (unsigned row = 0; row < maze.height; ++row) {
		for (unsigned col = 0; col < maze.width; ++col) {
			printCell(maze, row, col);
		}
		std::cout << std::endl;
	}
}

int main()
{
	const unsigned width = 110;
	const unsigned height = 25;
	Maze m(width, height);

	generateArea(m, 0, 0, width, height);

	printMaze(m);
}

