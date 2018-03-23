// RTKNA.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cstdio>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <ctime>
#include <sstream>
#include <GLFW/glfw3.h>



using namespace std;

struct net {
	vector<int> indexes;
	vector<bool> directions;
	int netnum;
	int doglegs = 0;
	int counter = 0;
	int startind, endind;
};

struct VCG {
	vector<string> decendents, predecessors;
	string netid;
	int distanceToSource = 0;
	int distanceToSink = 0;
};


vector<int> top;
vector<int> bot;
vector<vector<int>> zone;
vector<vector<int>> final_zone;
vector<int> union_zone;
vector<int> union_zone_diff;
vector<vector<int>> ini_zone;
vector<vector<string>> zones;
vector<vector<string>> ini_zones;
vector<vector<string>> final_zones;
net * first;
vector<net*> netlist;
vector<VCG*> allVCG, source, sink;
bool dog;
vector<string> tops, bots;
vector<int> zoneEnd;

#pragma region methods
void parse(string);
void arraytonet();
int findExistingNet(int);
void makeVCG();
void Zoning();
int VCGexists(string);
void transientRemoval();
void Zone_Sort();
void Zone_union();
void Zone_diff_union();
void sourceAndSink();
void findDistance();
void distFromSource(string, int);
void distFromSink(string, int);
int Merge();
string f(vector<string>);
string g(vector<string>, string);
vector<string> zoneConvertToString(string index);
vector<string> zoneConvertToStringFinal(string index); //Because we're lazy
void zonesToString();
void updateVCG(string a, string b);
void updateZones(string, string);
bool sortNet(const net *a, const net *b);
void zoneDogleg();
vector<string> zoneConvertToStringAll(string);
vector<int> zoneEnds();
void trackToString();
void printToFile();
#pragma endregion



int main(int argc, char* argv[])
{
	string filepath, dog1;
	getline(cin, filepath);

	cout << "\nDoglegging? : \n";
	getline(cin, dog1);
	if (dog1 == "y" || dog1 == "Y" || dog1 == "1") {
		dog = true;
	}
	else {
		dog = false;
	}

	clock_t start = clock();
	parse(filepath);
	arraytonet();

	//Zoning Code

	zone = vector<vector<int>>(static_cast<int>(top.size()), vector<int>(20, 0));
	Zoning();
	final_zone.resize(static_cast<int>(zone.size()));
	Zone_Sort();
	Zone_union();
	Zone_diff_union();
	zonesToString();
	if (dog) {
		zoneDogleg();
	}
	zoneEnd = zoneEnds();

	//VCG 
	makeVCG();
	findDistance();


	

	while (Merge() > 0);

	printf("\n\n%d", clock() - start);
	printToFile();
	return 0;
}


///////////////////////////////////////////////////////////////////////
//                             Merging 								 //
///////////////////////////////////////////////////////////////////////

#pragma region Merging
int Merge() {
	vector<string> L, R, zoneHold;
	string m, n;
	int counter = 0;
	for (size_t i = 0; i < zone.size() - 1; i++) {
		zoneHold = final_zones[i];
		R = ini_zones[i + 1];
		L.insert(L.end(), zoneHold.begin(), zoneHold.end());
		if (!R.empty())
		{
			n = g(L, (m = f(R)));

			updateVCG(n, m);
			updateZones(n, m);

			L.erase(remove(L.begin(), L.end(), n), L.end());
			L.push_back((n + "," + m));
			counter++;

			sourceAndSink();
			findDistance();
		}

	}
	return counter;
}

string f(vector<string> Q) {
	double C = 100;
	double highest = 0;
	string high = "0";
	for (size_t i = 0; i < Q.size(); i++) {
		double hold = C * (allVCG[VCGexists((Q[i]))]->distanceToSource + allVCG[VCGexists((Q[i]))]->distanceToSink) + max(allVCG[VCGexists((Q[i]))]->distanceToSink, allVCG[VCGexists((Q[i]))]->distanceToSource);
		if (hold > highest) {
			highest = hold;
			high = Q[i];
		}
	}
	return high;
}


string g(vector<string> P, string m) {
	double C = 100;
	double lowest = 100000;
	string low = "0";
	for (size_t i = 0; i < P.size(); i++) {
		double hold = C * (max(allVCG[VCGexists((P[i]))]->distanceToSource, allVCG[VCGexists((m))]->distanceToSource) + max(allVCG[VCGexists((P[i]))]->distanceToSink, allVCG[VCGexists((m))]->distanceToSink)
			- max(allVCG[VCGexists((P[i]))]->distanceToSource + allVCG[VCGexists((P[i]))]->distanceToSink, allVCG[VCGexists((m))]->distanceToSink + allVCG[VCGexists((m))]->distanceToSource))
			- sqrt(allVCG[VCGexists((m))]->distanceToSource * allVCG[VCGexists((P[i]))]->distanceToSource) - sqrt(allVCG[VCGexists((m))]->distanceToSink * allVCG[VCGexists((P[i]))]->distanceToSink);

		if (hold < lowest) {
			lowest = hold;
			low = P[i];
		}
	}

	return low;
}

vector<string> zoneConvertToString(string index) {
	vector<string> ret;
	for (size_t i = 0; i < ini_zone[stoi(index)].size(); i++) {
		ret.push_back(to_string(ini_zone[stoi(index)][i]));
	}
	return ret;
}

vector<string> zoneConvertToStringFinal(string index) {
	vector<string> ret;
	for (size_t i = 0; i < final_zone[stoi(index)].size(); i++) {
		ret.push_back(to_string(final_zone[stoi(index)][i]));
	}
	return ret;
}

vector<string> zoneConvertToStringAll(string index) {
	vector<string> ret;
	for (size_t i = 0; i < zone[stoi(index)].size(); i++) {
		ret.push_back(to_string(zone[stoi(index)][i]));
	}
	return ret;
}

void zonesToString() {
	for (size_t i = 0; i < ini_zone.size(); i++) {
		ini_zones.push_back(zoneConvertToString(to_string(i)));
	}

	for (size_t i = 0; i < final_zone.size(); i++) {
		final_zones.push_back(zoneConvertToStringFinal(to_string(i)));
	}

	for (size_t i = 0; i < zone.size(); i++) {
		zones.push_back(zoneConvertToStringAll(to_string(i)));
	}
}

void updateVCG(string a, string b) {
	vector<string> newdesc, newpred;

	for (size_t i = 0; i < allVCG.size(); i++) {
		if (find(allVCG[i]->decendents.begin(), allVCG[i]->decendents.end(), a) != allVCG[i]->decendents.end() && allVCG[i]->decendents.size() > 0)
		{
			newdesc.insert(newdesc.end(), allVCG[VCGexists(a)]->decendents.begin(), allVCG[VCGexists(a)]->decendents.end());
			allVCG[i]->decendents.erase(remove(allVCG[i]->decendents.begin(), allVCG[i]->decendents.end(), a), allVCG[i]->decendents.end());
			if (!(find(allVCG[i]->decendents.begin(), allVCG[i]->decendents.end(), a + "," + b) != allVCG[i]->decendents.end())) {
				allVCG[i]->decendents.push_back(a + "," + b);
			}
		}
		if (find(allVCG[i]->decendents.begin(), allVCG[i]->decendents.end(), b) != allVCG[i]->decendents.end() && allVCG[i]->decendents.size() > 0)
		{
			newdesc.insert(newdesc.end(), allVCG[VCGexists(b)]->decendents.begin(), allVCG[VCGexists(b)]->decendents.end());
			allVCG[i]->decendents.erase(remove(allVCG[i]->decendents.begin(), allVCG[i]->decendents.end(), b), allVCG[i]->decendents.end());
			if (!(find(allVCG[i]->decendents.begin(), allVCG[i]->decendents.end(), a + "," + b) != allVCG[i]->decendents.end())) {
				allVCG[i]->decendents.push_back(a + "," + b);
			}
		}
		if (find(allVCG[i]->predecessors.begin(), allVCG[i]->predecessors.end(), a) != allVCG[i]->predecessors.end() && allVCG[i]->predecessors.size() > 0)
		{
			newpred.insert(newpred.end(), allVCG[VCGexists(a)]->predecessors.begin(), allVCG[VCGexists(a)]->predecessors.end());
			allVCG[i]->predecessors.erase(remove(allVCG[i]->predecessors.begin(), allVCG[i]->predecessors.end(), a), allVCG[i]->predecessors.end());
			if (!(find(allVCG[i]->predecessors.begin(), allVCG[i]->predecessors.end(), a + "," + b) != allVCG[i]->predecessors.end())) {
				allVCG[i]->predecessors.push_back(a + "," + b);
			}
		}
		if (find(allVCG[i]->predecessors.begin(), allVCG[i]->predecessors.end(), b) != allVCG[i]->predecessors.end() && allVCG[i]->predecessors.size() > 0)
		{
			newpred.insert(newpred.end(), allVCG[VCGexists(b)]->predecessors.begin(), allVCG[VCGexists(b)]->predecessors.end());
			allVCG[i]->predecessors.erase(remove(allVCG[i]->predecessors.begin(), allVCG[i]->predecessors.end(), b), allVCG[i]->predecessors.end());
			if (!(find(allVCG[i]->predecessors.begin(), allVCG[i]->predecessors.end(), a + "," + b) != allVCG[i]->predecessors.end())) {
				allVCG[i]->predecessors.push_back(a + "," + b);
			}
		}

	}

	sort(newdesc.begin(), newdesc.end());
	sort(newpred.begin(), newpred.end());
	newdesc.erase(unique(newdesc.begin(), newdesc.end()), newdesc.end());
	newpred.erase(unique(newpred.begin(), newpred.end()), newpred.end());

	VCG *combine = new VCG();
	combine->decendents = newdesc;
	combine->predecessors = newpred;
	combine->netid = a + "," + b;
	combine->distanceToSink = max(allVCG[VCGexists(a)]->distanceToSink, allVCG[VCGexists(b)]->distanceToSink);
	combine->distanceToSource = max(allVCG[VCGexists(a)]->distanceToSource, allVCG[VCGexists(b)]->distanceToSource);

	allVCG.erase(remove(allVCG.begin(), allVCG.end(), allVCG[VCGexists(a)]), allVCG.end());
	allVCG.erase(remove(allVCG.begin(), allVCG.end(), allVCG[VCGexists(b)]), allVCG.end());
	allVCG.push_back(combine);
}

void updateZones(string a, string b) {
	vector<string>::iterator it1, it2;
	int ind1, ind2, i1, i2;
	for (size_t i = 0; i < ini_zones.size(); i++) {
		if ((it1 = find(ini_zones[i].begin(), ini_zones[i].end(), a)) != ini_zones[i].end())
		{
			ind1 = it1 - ini_zones[i].begin();
			ini_zones[i].erase(ini_zones[i].begin() + ind1);
			i1 = i;

		}
		if ((it2 = find(ini_zones[i].begin(), ini_zones[i].end(), b)) != ini_zones[i].end())
		{
			ind2 = it2 - ini_zones[i].begin();
			ini_zones[i].erase(ini_zones[i].begin() + ind2);
			i2 = i;
		}
	}

	if (i1 > i2) {
		ini_zones[i2].insert(ini_zones[i2].begin() + ind2, a + "," + b);
	}
	else {
		ini_zones[i1].insert(ini_zones[i1].begin() + ind1, a + "," + b);
	}

	for (size_t i = 0; i < final_zones.size(); i++) {
		if ((it1 = find(final_zones[i].begin(), final_zones[i].end(), a)) != final_zones[i].end())
		{
			ind1 = it1 - final_zones[i].begin();
			final_zones[i].erase(final_zones[i].begin() + ind1);
			i1 = i;
		}
		if ((it2 = find(final_zones[i].begin(), final_zones[i].end(), b)) != final_zones[i].end())
		{
			ind2 = it2 - final_zones[i].begin();
			final_zones[i].erase(final_zones[i].begin() + ind2);
			i2 = i;
		}
	}

	if (i1 < i2) {
		final_zones[i2].insert(final_zones[i2].begin() + ind2, a + "," + b);
	}
	else {
		final_zones[i1].insert(final_zones[i1].begin() + ind1, a + "," + b);
	}
}
#pragma endregion


///////////////////////////////////////////////////////////////////////
//                             Zoning 								 //
///////////////////////////////////////////////////////////////////////

//creates zones for future use
#pragma region Zoning
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
	vector<int> temp_zone_union(2 * netlist.size());

	for (int i = 0; i < final_zone.size(); i++) {
		all1.insert(final_zone[i].begin(), final_zone[i].end());
	}
	union_zone_diff = vector<int>(all1.begin(), all1.end());

	it2 = set_difference(union_zone.begin(), union_zone.end(), union_zone_diff.begin(), union_zone_diff.end(), temp_zone_union.begin());
	temp_zone_union.resize(it2 - temp_zone_union.begin());

	final_zone[static_cast<int>(zone.size()) - 1] = temp_zone_union;
}

void zoneDogleg() {
	char letter;
	string s;
	for (size_t i = 0; i < netlist.size(); i++) {
		letter = 'A';
		for (size_t j = 0; j < zones.size(); j++) {
			if (find(zones[j].begin(), zones[j].end(), to_string(netlist[i]->netnum)) != zones[j].end()) {
				stringstream ss;
				ss << letter;
				ss >> s;
				replace(zones[j].begin(), zones[j].end(), to_string(netlist[i]->netnum), to_string(netlist[i]->netnum) + s);
				letter++;
				netlist[i]->doglegs++;
			}
		}

	}

	ini_zones = zones;
	final_zones = zones;
}
#pragma endregion


///////////////////////////////////////////////////////////////////////
//                         VCG construction							 //
///////////////////////////////////////////////////////////////////////

#pragma region VCG
				//Constructs the original VCG graph
void makeVCG() {
	trackToString();
	//top
	for (size_t i = 0; i < tops.size(); i++) {
		int at = VCGexists((tops[i]));
		//VCG exists
		if (at != -1 && bot[i] != 0) {
			allVCG[at]->decendents.push_back((bots[i]));
		}
		//VCG does not exist
		else if (at == -1 && top[i] != 0 && bot[i] != 0) {
			VCG *n = new VCG;
			n->netid = (tops[i]);
			n->decendents.push_back((bots[i]));
			allVCG.push_back(n);
		}
	}
	//bottom
	for (size_t i = 0; i < bots.size(); i++) {
		int at = VCGexists((bots[i]));
		//VCG exists
		if (at != -1 && top[i] != 0) {
			allVCG[at]->predecessors.push_back((tops[i]));
		}
		//VCG does not exist
		else if (at == -1 && top[i] != 0 && bot[i] != 0) {
			VCG *n = new VCG;
			n->netid = (bots[i]);
			n->predecessors.push_back((tops[i]));
			allVCG.push_back(n);
		}
	}

	transientRemoval();
	sourceAndSink();
}

//returns index of netid requested
//for use with makeVCG and transient removal
int VCGexists(string netid) {
	for (size_t i = 0; i < allVCG.size(); i++) {
		if (allVCG[i]->netid == netid) {
			return i;
		}
	}

	return -1;
}

//Removes transient edges with low effort O(n3)
void transientRemoval() {
	vector<string> possibleRemove;

	//remove transient edges
	//iterate through all VCG
	for (size_t i = 0; i < allVCG.size(); i++) {
		possibleRemove = allVCG[i]->decendents;
		//iterate through decendents
		for (size_t j = 0; j < possibleRemove.size(); j++) {
			//get list of decendents of decendents
			VCG *check = allVCG[VCGexists(possibleRemove[j])];
			//remove common decendents from tallest ancestor
			for (size_t k = 0; k < check->decendents.size(); k++) {
				possibleRemove.erase(std::remove(possibleRemove.begin(), possibleRemove.end(), check->decendents[k]), possibleRemove.end());
			}
		}
		allVCG[i]->decendents = possibleRemove;
	}

	for (size_t i = 0; i < allVCG.size(); i++) {
		possibleRemove = allVCG[i]->predecessors;
		//iterate through predecessors
		for (size_t j = 0; j < possibleRemove.size(); j++) {
			//get list of preds of preds
			VCG *check = allVCG[VCGexists(possibleRemove[j])];
			//remove common predecessor from tallest offspring
			for (size_t k = 0; k < check->predecessors.size(); k++) {
				possibleRemove.erase(std::remove(possibleRemove.begin(), possibleRemove.end(), check->predecessors[k]), possibleRemove.end());
			}
		}
		allVCG[i]->predecessors = possibleRemove;
	}
}

void sourceAndSink() {
	source.clear();
	sink.clear();
	for (size_t i = 0; i < allVCG.size(); i++) {
		if (allVCG[i]->decendents.size() == 0) {
			sink.push_back(allVCG[i]);
		}
		if (allVCG[i]->predecessors.size() == 0) {
			source.push_back(allVCG[i]);
		}
	}
}

void findDistance() {
	for (size_t i = 0; i < source.size(); i++) {
		distFromSource(source[i]->netid, 0);
	}

	for (size_t i = 0; i < sink.size(); i++) {
		distFromSink(sink[i]->netid, 0);
	}
}

void distFromSource(string netid, int counter) {
	if (counter > allVCG[VCGexists(netid)]->distanceToSource) {
		allVCG[VCGexists(netid)]->distanceToSource = counter;
	}
	for (size_t i = 0; i < allVCG[VCGexists(netid)]->decendents.size(); i++) {
		distFromSource(allVCG[VCGexists(netid)]->decendents[i], counter + 1);
	}
}

void distFromSink(string netid, int counter) {
	if (counter > allVCG[VCGexists(netid)]->distanceToSink) {
		allVCG[VCGexists(netid)]->distanceToSink = counter;
	}
	for (size_t i = 0; i < allVCG[VCGexists(netid)]->predecessors.size(); i++) {
		distFromSink(allVCG[VCGexists(netid)]->predecessors[i], counter + 1);
	}
}

int getNetlistInd(int index) {
	for (size_t i = 0; i < netlist.size(); i++) {
		if (index == netlist[i]->netnum) {
			return i;
		}
	}
}

vector<int> zoneEnds() {
	vector<int> ret;
	int maxEnd = 0;
	for (size_t i = 0; i < final_zone.size(); i++) {
		for (size_t j = 0; j < final_zone[i].size(); j++)
		{
			maxEnd = (maxEnd < netlist[getNetlistInd(final_zone[i][j])]->endind) ? netlist[getNetlistInd(final_zone[i][j])]->endind : maxEnd;
		}
		ret.push_back(maxEnd);
		maxEnd = 0;
	}

	return ret;
}

void trackToString() {
	if (!dog)
	{
		for (size_t i = 0; i < top.size(); i++) {
			tops.push_back(to_string(top[i]));
		}
		for (size_t i = 0; i < bot.size(); i++) {
			bots.push_back(to_string(bot[i]));
		}
	}
	else {
		/*vector<int> seen(netlist.size(), 0);
		vector<int> sameZone;
		int counter = 0;
		for (size_t i = 0; i < top.size(); i++) {
			if (i > zoneEnd[counter]) {
				counter++;
				for (size_t j = 0; j < sameZone.size(); j++) {
					seen[sameZone[j] - 1]++;
				}
				sameZone.clear();
			}
			if (top[i] != 0)
			{
				stringstream ss;
				ss << (char)('A' + (char)(seen[top[i] - 1]));
				string s;
				ss >> s;
				tops.push_back(to_string(top[i]) + s);
				if (find(sameZone.begin(), sameZone.end(), top[i]) == sameZone.end())
				{
					sameZone.push_back(top[i]);
				}
			}
			else {
				tops.push_back("0");
			}
		}

		counter = 0;
		sameZone.clear();
		seen = vector<int>(netlist.size(), 0);
		for (size_t i = 0; i < bot.size(); i++) {
			if (i > zoneEnd[counter]) {
				counter++;
				for (size_t j = 0; j < sameZone.size(); j++) {
					seen[sameZone[j] - 1]++;
				}
				sameZone.clear();
			}
			if (bot[i] != 0)
			{
				stringstream ss;
				ss << (char)('A' + (char)(seen[bot[i] - 1]));
				string s;
				ss >> s;
				bots.push_back(to_string(bot[i]) + s);
				if (find(sameZone.begin(), sameZone.end(), bot[i]) == sameZone.end())
				{
					sameZone.push_back(bot[i]);
				}
			}
			else {
				bots.push_back("0");
			}
		}
		counter = 0;
	*/

	}
}

#pragma endregion

///////////////////////////////////////////////////////////////////////
//                             Parsing								 //
///////////////////////////////////////////////////////////////////////
//parses file into vector of ints to be used later for VCG creation
//and zoning
#pragma region Parsing
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
#pragma endregion


///////////////////////////////////////////////////////////////////////
//                             Parsing								 //
///////////////////////////////////////////////////////////////////////

vector<string> VCGParse(string s) {
	vector<string> nets;
	int index = 0, previndex = 0;
	while (index < s.length()) {
		index = s.find(",", (size_t)previndex);
		nets.push_back((s.substr(previndex, index - previndex)));
		previndex = index + 1;
	}


	return nets;
}

string vectorToString(vector<int> s) {
	string p;
	for (size_t i = 0; i < s.size(); i++) {
		p += to_string(s[i]) + " ";
	}

	return p;
}

string vectorToString(vector<bool> s) {
	string p;
	for (size_t i = 0; i < s.size(); i++) {
		p += s[i] ? "u" : "d";
	}
	return p;
}

bool sortVCG(const VCG *a, const VCG *b) {
	return a->distanceToSource < b->distanceToSource;
}

void printToFile() {
	vector<string> nets;
	string file = "RTK-NA.out";
	ofstream f;
	f.open(file);
	int netind;

	sort(allVCG.begin(), allVCG.end(), sortVCG);

	for(size_t i = 0; i < allVCG.size(); i++) {
		string s = allVCG[i]->netid;

		nets = VCGParse(s);

		for (size_t j = 0; j < nets.size(); j++) {
			netind = getNetlistInd(stoi(nets[j]));

			f << (to_string(netlist[netind]->netnum) + " " );
			f << ("s " + to_string(netlist[netind]->startind) + " " );
			f << ("e " + to_string(netlist[netind]->endind)+ " ");
			f << ("i " + vectorToString(netlist[netind]->indexes));
			f << ("d " + vectorToString(netlist[netind]->directions) + " ");
			f << "\n";
		}
		f << "|\n";
	}
	f.close();
}

int draw(void)
{

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}