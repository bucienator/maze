// maze.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include <random>
#include <iostream>
#include <fstream>
#include <list>
#include <string>

struct Node {
	unsigned row;
	unsigned col;
	Node() : row(0), col(0) {}
	Node(unsigned row, unsigned col) : row(row), col(col) {}
};

using DistanceMap = std::vector<std::vector<int>>;

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

	Node entryNode;
	Node exitNode;

	DistanceMap visualizedDMap;
};


DistanceMap generateDistanceMap(const Maze & maze, const Node & startNode)
{
	DistanceMap map(maze.height, std::vector<int>(maze.width, -1));
	map[startNode.row][startNode.col] = 0;

	std::list<Node> queue;
	queue.emplace_back(startNode);

	while (!queue.empty()) {
		const Node n = queue.front();
		queue.pop_front();

		const int distance = map[n.row][n.col];

		const bool openToTop = n.row > 0 && !maze.horizontalWalls[n.row][n.col];
		const bool openToBottom = n.row < maze.height - 1 && !maze.horizontalWalls[n.row + 1][n.col];
		const bool openToLeft = n.col > 0 && !maze.verticalWalls[n.col][n.row];
		const bool openToRight = n.col < maze.width - 1 && !maze.verticalWalls[n.col + 1][n.row];

		if (openToTop && map[n.row - 1][n.col] == -1) {
			queue.emplace_back(n.row - 1, n.col);
			map[n.row - 1][n.col] = distance + 1;
		}
		if (openToBottom && map[n.row + 1][n.col] == -1) {
			queue.emplace_back(n.row + 1, n.col);
			map[n.row + 1][n.col] = distance + 1;
		}
		if (openToLeft && map[n.row][n.col - 1] == -1) {
			queue.emplace_back(n.row, n.col - 1);
			map[n.row][n.col - 1] = distance + 1;
		}
		if (openToRight && map[n.row][n.col + 1] == -1) {
			queue.emplace_back(n.row, n.col + 1);
			map[n.row][n.col + 1] = distance + 1;
		}
	}

	return map;
}

DistanceMap minMap(const Maze & maze, const DistanceMap & map1, const DistanceMap & map2)
{
	DistanceMap map(maze.height, std::vector<int>(maze.width, -1));

	for (unsigned row = 0; row < maze.height; ++row) {
		for (unsigned col = 0; col < maze.width; ++col) {
			map[row][col] = std::min(map1[row][col], map2[row][col]);
		}
	}

	return map;
}

std::vector<Node> searchMaximums(const DistanceMap & map)
{
	std::vector<Node> ret;
	unsigned maxValue = 0;

	for (unsigned row = 0; row < map.size(); ++row)
	{
		for (unsigned col = 0; col < map[row].size(); ++col)
		{
			if (map[row][col] > maxValue) {
				maxValue = map[row][col];
				ret.clear();
			}
			if (map[row][col] == maxValue) {
				ret.push_back(Node(row, col));
			}
		}
	}

	return ret;
}

std::vector<unsigned> searchMaximumsAtEdge(const Maze & maze, const DistanceMap & map)
{
	std::vector<unsigned> ret;
	unsigned maxValue = 0;

	unsigned index = 0;
	for (unsigned col = 0; col < maze.width; col++, index++) {
		if (map[0][col] > maxValue) {
			maxValue = map[0][col];
			ret.clear();
		}
		if (map[0][col] == maxValue) {
			ret.push_back(index);
		}
	}
	for (unsigned row = 0; row < maze.height; row++, index++) {
		if (map[row][maze.width-1] > maxValue) {
			maxValue = map[row][maze.width - 1];
			ret.clear();
		}
		if (map[row][maze.width - 1] == maxValue) {
			ret.push_back(index);
		}
	}
	for (unsigned col = maze.width-1; col < maze.width; col--, index++) {
		if (map[maze.height-1][col] > maxValue) {
			maxValue = map[maze.height - 1][col];
			ret.clear();
		}
		if (map[maze.height - 1][col] == maxValue) {
			ret.push_back(index);
		}
	}
	for (unsigned row = maze.height-1; row < maze.height; row--, index++) {
		if (map[row][0] > maxValue) {
			maxValue = map[row][0];
			ret.clear();
		}
		if (map[row][0] == maxValue) {
			ret.push_back(index);
		}
	}

	return ret;
}

std::random_device rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()

Node placeXX(Maze & maze, const unsigned entrySelection)
{
	if (entrySelection < maze.width) {
		// entry on top
		const unsigned entry = entrySelection;
		maze.horizontalWalls[0][entry] = false;
		return Node(0, entry);
	}
	else if (entrySelection - maze.width < maze.height) {
		// entry on right
		const unsigned entry = entrySelection - maze.width;
		maze.verticalWalls[maze.width][entry] = false;
		return Node(entry, maze.width - 1);
	}
	else if (entrySelection - maze.width - maze.height < maze.width) {
		// entry on bottom
		const unsigned entry = entrySelection - maze.width - maze.height;
		maze.horizontalWalls[maze.height][maze.width - 1 - entry] = false;
		return Node(maze.height - 1, maze.width - 1 - entry);
	}
	else if (entrySelection - maze.width - maze.height - maze.width < maze.height) {
		// entry on left
		const unsigned entry = entrySelection - maze.width - maze.height - maze.width;
		maze.verticalWalls[0][maze.height - 1 - entry] = false;
		return Node(maze.height - 1 - entry, 0);
	}

}

void placeEntryExit(Maze & maze)
{
	const unsigned entryOptions = maze.width * 2 + maze.height * 2;
	std::uniform_int_distribution<> dis(0, entryOptions - 1);
	const unsigned entrySelection = dis(gen);

	maze.entryNode = placeXX(maze, entrySelection);

	const DistanceMap entryMap = generateDistanceMap(maze, maze.entryNode);

	std::vector<unsigned> maximumsAtEdge = searchMaximumsAtEdge(maze, entryMap);
	maze.exitNode = placeXX(maze, maximumsAtEdge[0]);

	const DistanceMap exitMap = generateDistanceMap(maze, maze.exitNode);

	const DistanceMap treasure1Map = minMap(maze, entryMap, exitMap);

	maze.visualizedDMap = treasure1Map;
}

void openTheWall(std::vector<bool> & wall, const unsigned firstElement, const unsigned lastElement, const unsigned fullLength)
{
	std::uniform_int_distribution<> dis(1, fullLength);
	const unsigned isThereADoor = dis(gen) <= (lastElement - firstElement + 1);

	if (!isThereADoor)
		return;

	std::uniform_int_distribution<> dis2(firstElement, lastElement);
	const unsigned doorAt = dis2(gen);

	wall[doorAt] = false;

	if (doorAt - firstElement >= 2) {
		openTheWall(wall, firstElement, doorAt - 2, fullLength);
	}
	if (lastElement - doorAt >= 2) {
		openTheWall(wall, doorAt + 2, lastElement, fullLength);
	}
}

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

		openTheWall(maze.verticalWalls[leftWall + splitAt + 1], topWall, bottomWall - 1, bottomWall - topWall);

		generateArea(maze, leftWall, topWall, leftWall + splitAt + 1, bottomWall);
		generateArea(maze, leftWall + splitAt + 1, topWall, rightWall, bottomWall);
	}
	else { // horizontal split
		const unsigned splitAt = splitSelection - verticalSplitOptions;

		openTheWall(maze.horizontalWalls[topWall + splitAt + 1], leftWall, rightWall - 1, rightWall - leftWall);

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

void printMaze2(const Maze & maze)
{
	for (unsigned row = 0; row < maze.height * 2 + 1; ++row) {
		for (unsigned col = 0; col < maze.width * 2 + 1; ++col) {
			if (row % 2 == 0) {
				if (col % 2 == 0) {
					std::cout << char(219);
				}
				else {
					std::cout << (maze.horizontalWalls[row / 2][col / 2] ? char(219) : ' ');
				}
			}
			else {
				if (col % 2 == 0) {
					std::cout << (maze.verticalWalls[col / 2][row / 2] ? char(219) : ' ');
				}
				else {
					std::cout << ' ';
				}
			}
		}
		std::cout << std::endl;
	}
}

void printMazeSVG(const Maze & maze, const std::string & svgName, const bool addDistanceMap)
{
	std::ofstream f;
	f.open(svgName);

	f << "<?xml version='1.0' standalone='no'?>" << std::endl;
	f << "<svg width='210mm' height='297mm' version='1.1' xmlns='http://www.w3.org/2000/svg' viewBox='0 0 210 297'>" << std::endl;

	f << "<rect x='0' y='0' width='210' height='297' fill='white'/>" << std::endl;

	f << "<g stroke='black' stroke-linecap='round' stroke-width='0.5' >" << std::endl;

	const float w = 210;
	const float h = 297;
	const float margin = 20;

	const float maxStep = 10;
	const float fitStepW = (w - 2 * margin) / maze.width;
	const float fitStepH = (h - 2 * margin) / maze.height;
	const float fitStep = std::min(fitStepH, fitStepW);
	const float step = std::min(fitStep, maxStep);

	const float mazeWidth = step * maze.width;
	const float mazeHeight = step * maze.height;

	const float left = (w - mazeWidth) / 2;
	const float top = (h - mazeHeight) / 2;

	for (unsigned row = 0; row < maze.height + 1; ++row) {
		for (unsigned col = 0; col < maze.width + 1; ++col) {
			const bool closedTop = col < maze.width && maze.horizontalWalls[row][col];
			const bool closedLeft = row < maze.height && maze.verticalWalls[col][row];

			if (closedTop) {
				const float x = left + step * col;
				const float y = top + step * row;
				f << "<line x1='" << x << "' y1='" << y << "' x2='" << (x + step) << "' y2='" << y << "' />" << std::endl;
			}
			if (closedLeft) {
				const float x = left + step * col;
				const float y = top + step * row;
				f << "<line x1='" << x << "' y1='" << y << "' x2='" << x << "' y2='" << (y + step) << "' />" << std::endl;
			}

		}
	}

	if (addDistanceMap)
	{
		for (unsigned row = 0; row < maze.height; ++row) {
			for (unsigned col = 0; col < maze.width; ++col) {
				const float x = left + step * col;
				const float y = top + step * row;
				f << "<text x='" << (x + step / 4) << "' y='" << (y + step / 2) << "' style='fill: black; font: 5px sans-serif '>" << std::endl;
				f << std::to_string(maze.visualizedDMap[row][col]) << std::endl;
				f << "</text>" << std::endl;
			}
		}
	}


	f << "</g>" << std::endl;
	f << "</svg>" << std::endl;
}

void generateMaze(const unsigned width, const unsigned height, const std::string & svgName)
{
	Maze m(width, height);
	generateArea(m, 0, 0, width, height);
	placeEntryExit(m);

	printMazeSVG(m, svgName, false);
}

int main()
{
	std::uniform_int_distribution<> dis(16, 32);

	for (int i = 0; i < 30; ++i) {
		const unsigned width = dis(gen);
		const unsigned height = dis(gen);
		generateMaze(width, height, "maze" + std::to_string(100 + i) + ".svg");
	}
}
