// RTKNA.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace std;
vector<int> top;
vector<int> bot;
vector<vector<int>> col;


struct net {
	vector<int> indexes;
	vector<bool> directions;
	int netnum;

	int startind, endind;
};

net * first;
vector<net*> netlist;

void parse(string);
void arraytonet();

int main(int argc, char* argv[])
{
	parse(argv[0]);
	arraytonet();
    return 0;
}

void parse(string fileloc) {
	ifstream file;
	file.open(fileloc);

	string line = file.getline;
	uint64_t index = 0;
	uint64_t start = 0;

	while (index < line.length) {
		index = line.find(" ", (size_t)start);
		top.push_back(stoi(line.substr(start, index - start - 1)));
		start = index + 1;
	}

	line = file.getline;
	while (index < line.length) {
		index = line.find(" ", (size_t)start);
		bot.push_back(stoi(line.substr(start, index - start - 1)));
		start = index + 1;
	}
}

void arraytonet() {
	int previndex;
	//top part
	for (int i = 0; i < top.size; i++) {
		net *next = new net;
		int nextnet = top.at(i);

		auto it = (find_if(netlist.begin(), netlist.end(), [&nextnet](const net& p) {return p.netnum == nextnet; }));
		if (nextnet == 0) {
			continue;
		}
		else if (it != netlist.end) {
			(*it)->indexes.push_back(i);
			(*it)->directions.push_back(true);
			(*it)->endind = i;
		}
		else {
			next->startind = i;
			next->directions.push_back(true);
			next->endind = i;
			next->indexes.push_back(i);
			next->netnum = nextnet;
		}
	}
	//bottom part
}