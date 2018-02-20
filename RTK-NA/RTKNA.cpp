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
#include <set>
#include <map>

using namespace std;
vector<int> top;
vector<int> bot;

map<int, int> netcol_max; //net number, max col
map<int, int> netcol_min; //net no , min col

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

	//Zoning Code
	int maxnum = 0;
	for (int i = 0; i < static_cast<int>(top.size()); i++) {
		maxnum = (top[i] > maxnum) ? top[i] : maxnum;
		maxnum = (bot[i] > maxnum) ? bot[i] : maxnum;
	}
	for (int k = 1; k <= static_cast<int>(top.size()); k++) {
		if (netcol_max.find(top[k - 1]) == netcol_max.end()) {
			//cout << "Not found";
			netcol_max[top[k - 1]] = k;
			//cout << netcol_max[top[k - 1]];
		}
		else if (k>netcol_max[top[k - 1]]) {
			netcol_max[top[k - 1]] = k;
			//cout << "Found";
		}
		if (netcol_max.find(bot[k - 1]) == netcol_max.end()) {
			//cout << "Not found";
			netcol_max[bot[k - 1]] = k;
			//cout << netcol_max[top[k - 1]];
		}
		else if (k>netcol_max[bot[k - 1]]) {
			netcol_max[bot[k - 1]] = k;
			//cout << "Found";
		}

		if (netcol_min.find(top[k - 1]) == netcol_min.end()) {
			//cout << "Not found";
			netcol_min[top[k - 1]] = k;
			//cout << netcol_min[top[k - 1]];
		}
		else if (k<netcol_min[top[k - 1]]) {
			netcol_min[top[k - 1]] = k;
			//cout << "Found";
		}
		if (netcol_min.find(bot[k - 1]) == netcol_min.end()) {
			//cout << "Not found";
			netcol_min[bot[k - 1]] = k;
			//cout << netcol_min[top[k - 1]];
		}
		else if (k<netcol_min[bot[k - 1]]) {
			netcol_min[bot[k - 1]] = k;
			//cout << "Found";
		}
	}

	vector< vector<int> > col;
	for (int i1 = 0; i1 < static_cast<int>(top.size()); i1++) {
		vector<int> row; // Create an empty row
		for (int j1 = 1; j1 < maxnum + 1; j1++) {
			if (i1 <= netcol_max[j1] && i1 >= netcol_min[j1]) {
				row.push_back(j1); // Add an element (column) to the row
			}
		}
		col.push_back(row); // Add the row to the main vector
	}

	int p = 0;

	vector<vector<int>> zone(static_cast<int>(top.size()), vector<int>(5, 0));
	for (int k1 = 1; k1 < static_cast<int>(top.size()) - 1; k1++) {
		//if (k1== static_cast<int>(top.size()-1) {

		if ((includes(col[k1].begin(), col[k1].end(), col[k1 + 1].begin(), col[k1 + 1].end())) || (includes(col[k1 + 1].begin(), col[k1 + 1].end(), col[k1].begin(), col[k1].end()))) {
			//cout << "Subset";
			std::vector<int> temp_colunion(static_cast<int>(col[k1].size()) * 10);
			std::vector<int>::iterator it;
			it = set_union(col[k1].begin(), col[k1].end(), col[k1 + 1].begin(), col[k1 + 1].end(), temp_colunion.begin());
			temp_colunion.resize(it - temp_colunion.begin());

			zone[p] = (includes(zone[p].begin(), zone[p].end(), temp_colunion.begin(), temp_colunion.end())) ? zone[p] : temp_colunion;

		}
		else {
			//cout << "\n not a subset";
			//p = (sub_count > 0) ? (p + 1) : p;
			p += 1;
			zone[p] = col[k1 + 1];
		}

	}
	zone.erase(zone.begin() + p + 1, zone.end());
	//cout << "P is " << p << endl;
	for (int k1 = 0; k1 < static_cast<int>(zone.size()); k1++) {
		cout << endl;
		cout << "Zone no" << (k1 + 1) << endl;
		for (int k2 = 0; k2 < static_cast<int>(zone[k1].size()); k2++) {
			cout << " " << zone[k1][k2];
		}
	}
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