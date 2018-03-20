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
vector<vector<int>> zone;
vector<vector<int>> final_zone;
vector<int> union_zone;
vector<int> union_zone_diff;
vector<vector<int>> ini_zone;

map<int, int> netcol_max; //net number, max col
map<int, int> netcol_min; //net no , min col

struct net {
	vector<int> indexes;
	vector<bool> directions;
	int netnum;

	int startind, endind;
};

struct VCG {
	vector<int> top;
	int netid;
};

bool sortNet(const net *a, const net *b);

net * first;
vector<net*> netlist;
vector<VCG*> allVCG, source, sink;


void parse(string);
void arraytonet();
int findExistingNet(int);
void makeVCG();
void Zoning();
int VCGexists(int);
void transientRemoval(); 
void Zone_Sort();
void Zone_union();
void Zone_diff_union();



int main(int argc, char* argv[])
{
	//VCG 
	string filepath;
	getline(cin, filepath);
	parse(filepath);
	arraytonet();
	makeVCG();

	//Zoning Code
	
	zone = vector<vector<int>>(static_cast<int>(top.size()), vector<int>(5, 0));
	Zoning();
	final_zone.resize(static_cast<int>(zone.size()));
	Zone_Sort();
	Zone_union();
	Zone_diff_union();

	return 0;
}


///////////////////////////////////////////////////////////////////////
//                             Zoning 								 //
///////////////////////////////////////////////////////////////////////

//creates zones for future use
void Zoning() {
	//Zoning Code
	int maxnum = 0;
	for (int i = 0; i < static_cast<int>(top.size()); i++) {
		maxnum = (top[i] > maxnum) ? top[i] : maxnum;
		maxnum = (bot[i] > maxnum) ? bot[i] : maxnum;
	}

	vector< vector<int> > col;
	for (int i1 = 0; i1 < static_cast<int>(top.size()); i1++) {
		vector<int> row; // Create an empty row
		for (int j1 = 0; j1 < maxnum; j1++) {
			if (i1 <= netlist[j1]->endind && i1 >= netlist[j1]->startind) {
				row.push_back(j1 + 1); // Add an element (column) to the row
			}
		}
		col.push_back(row); // Add the row to the main vector
	}

	int p = 0;


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

}

void Zone_union() {
	set<int> all;
	for (int i = 0; i < zone.size(); i++) {
		all.insert(zone[i].begin(), zone[i].end());
	}
	union_zone = vector<int>(all.begin(), all.end());
}


void Zone_Sort() {
	final_zone.resize(static_cast<int>(zone.size()));
	ini_zone.resize(static_cast<int>(zone.size()));
	vector<int>::iterator it1;
	vector<int>::iterator it2;

	vector<int> temp_diff_union;

	vector<int> last_zone(10);

	for (int l1 = 0; l1 < static_cast<int>(zone.size()) - 1; l1++) {
		vector<int> temp_zone_diff(10);

		it1 = set_difference(zone[l1].begin(), zone[l1].end(), zone[l1 + 1].begin(), zone[l1 + 1].end(), temp_zone_diff.begin());
		temp_zone_diff.resize(it1 - temp_zone_diff.begin());

		final_zone[l1] = temp_zone_diff;

		vector<int> temp_zone_diff1(10);

		if (l1 == 0) {
			ini_zone[l1] = zone[l1];
		}
		it2 = set_difference(zone[l1 + 1].begin(), zone[l1 + 1].end(), zone[l1].begin(), zone[l1].end(), temp_zone_diff1.begin());
		temp_zone_diff1.resize(it2 - temp_zone_diff1.begin());

		ini_zone[l1 + 1] = temp_zone_diff1;

	}


}

void Zone_diff_union() {
	set<int> all1;
	vector<int>::iterator it2;
	vector<int> temp_zone_union(10);

	for (int i = 0; i < final_zone.size(); i++) {
		all1.insert(final_zone[i].begin(), final_zone[i].end());
	}
	union_zone_diff = vector<int>(all1.begin(), all1.end());

	it2 = set_difference(union_zone.begin(), union_zone.end(), union_zone_diff.begin(), union_zone_diff.end(), temp_zone_union.begin());
	temp_zone_union.resize(it2 - temp_zone_union.begin());

	final_zone[static_cast<int>(zone.size()) - 1] = temp_zone_union;
}

///////////////////////////////////////////////////////////////////////
//                         VCG construction							 //
///////////////////////////////////////////////////////////////////////

//Constructs the original VCG graph
void makeVCG() {
	//determine 
	for (size_t i = 0; i < top.size(); i++) {
		int at = VCGexists(top[i]);
		//VCG exists
		if (at != -1) {
			allVCG[at]->top.push_back(bot[i]);
		}
		//VCG does not exist
		else if (at == -1 && top[i] != 0 && bot[i] != 0) {
			VCG *n = new VCG;
			n->netid = top[i];
			n->top.push_back(bot[i]);
			allVCG.push_back(n);
		}
	}

	for (size_t i = 1; i < netlist.size(); i++) {
		if (VCGexists(i) == -1) {
			VCG *n = new VCG();
			n->netid = i;
			allVCG.push_back(n);
			sink.push_back(n);
		}
	}

	transientRemoval();
}

//returns index of netid requested
//for use with makeVCG and transient removal
int VCGexists(int netid) {
	for (size_t i = 0; i < allVCG.size(); i++) {
		if (allVCG[i]->netid == netid) {
			return i;
		}
	}

	return -1;
}

//Removes transient edges with low effort O(n3)
void transientRemoval() {
	vector<int> possibleRemove;


	//remove transient edges
	//iterate through all VCG
	for (size_t i = 0; i < allVCG.size(); i++) {
		possibleRemove = allVCG[i]->top;
		//iterate through decendents
		for (size_t j = 0; j < possibleRemove.size(); j++) {
			//get list of decendents of decendents
			VCG *check = allVCG[VCGexists(possibleRemove[j])];
			//remove common decendents from tallest ancestor
			for (size_t k = 0; k < check->top.size(); k++) {
				possibleRemove.erase(std::remove(possibleRemove.begin(), possibleRemove.end(), check->top[k]), possibleRemove.end());
			}
		}
		allVCG[i]->top = possibleRemove;
	}
}



///////////////////////////////////////////////////////////////////////
//                             Parsing								 //
///////////////////////////////////////////////////////////////////////
//parses file into vector of ints to be used later for VCG creation
//and zoning
void parse(string fileloc) {
	ifstream file;
	file.open(fileloc);

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
		bot.push_back(stoi(line.substr(previndex, index - previndex)));
		previndex = index + 1;
	}
}

//converts the input into a useable netlist using net structs
void arraytonet() {
	int previndex;
	//top part of input
	for (int i = 0; i < top.size(); i++) {
		net *next = new net;
		int nextnet = top.at(i);
		int net = findExistingNet(nextnet);

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
	//bottom part of input
	for (int i = 0; i < bot.size(); i++) {
		net *next = new net;
		int nextnet = bot.at(i);
		int net = findExistingNet(nextnet);

		if (nextnet == 0) {
			continue;
		}
		else if (net != -1) {
			netlist[net]->indexes.push_back(i);
			netlist[net]->directions.push_back(false);
			if (netlist[net]->startind > i) {
				netlist[net]->startind = i;
			}
			if (netlist[net]->endind < i) {
				netlist[net]->endind = i;
			}
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

//accesory function to sort the netlist by netnum
bool sortNet(const net *a, const net *b) {
	return a->netnum < b->netnum;
}

//checks to see if there is an existing net with netnum net
int findExistingNet(int net) {

	for (int i = 0; i < netlist.size(); i++) {
		if (netlist[i]->netnum == net) {
			return i;
		}
	}

	return -1;
}