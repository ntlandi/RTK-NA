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

bool sortNet(const net *a, const net *b);

net * first;
vector<net*> netlist;

void parse(string);
void arraytonet();
int findExistingNet(int);

int main(int argc, char* argv[])
{
	parse(argv[0]);
	arraytonet();
    return 0;
}

void parse(string fileloc) {
	ifstream file;
	file.open("C:\\Users\\Nick Landi\\Desktop\\test.txt");

	string line;
	getline(file, line);
	size_t index = 0;
	size_t previndex = 0;

	while (index < line.length()) {
		index = line.find(" ", (size_t)previndex);
		top.push_back(stoi(line.substr(previndex, index - previndex)));
		previndex = index + 1;
	}

	index = 0;
	previndex = 0;
	getline(file, line);
	while (index < line.length()) {
		index = line.find(" ", (size_t)previndex);
		bot.push_back(stoi(line.substr(previndex, index - previndex )));
		previndex = index + 1;
	}
}

void arraytonet() {
	int previndex;
	//top part
	for (int i = 0; i < top.size(); i++) {
		net *next = new net;
		int nextnet = top.at(i);
		int net = findExistingNet(nextnet);

		//auto it = (find_if(netlist.begin(), netlist.end(), [&nextnet](const net& p) {return p.netnum == nextnet; }));
		if (nextnet == 0) {
			continue;
		}
		else if (net != -1) {
			netlist[net]->indexes.push_back(i);
			netlist[net]->directions.push_back(true);
			netlist[net]->endind = i;
		}
		else {
			next->startind = i;
			next->directions.push_back(true);
			next->endind = i;
			next->indexes.push_back(i);
			next->netnum = nextnet;
			netlist.push_back(next);
		}
	}
	//bottom part
	for (int i = 0; i < bot.size(); i++) {
		net *next = new net;
		int nextnet = bot.at(i);
		int net = findExistingNet(nextnet);

		//auto it = (find_if(netlist.begin(), netlist.end(), [&nextnet](const net& p) {return p.netnum == nextnet; }));
		if (nextnet == 0) {
			continue;
		}
		else if (net != -1) {
			netlist[net]->indexes.push_back(i);
			netlist[net]->directions.push_back(false);
			netlist[net]->endind = i;
		}
		else {
			next->startind = i;
			next->directions.push_back(false);
			next->endind = i;
			next->indexes.push_back(i);
			next->netnum = nextnet;
			netlist.push_back(next);
		}
	}

	sort(netlist.begin(), netlist.end(), sortNet);
}

bool sortNet(const net *a, const net *b) {
	return a->netnum < b->netnum;
}

int findExistingNet(int net) {

	for (int i = 0; i < netlist.size(); i++) {
		if (netlist[i]->netnum == net) {
			return i;
		}
	}

	return -1;
}