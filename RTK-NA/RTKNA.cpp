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
//#include <glad/glad.h>
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
	string netid;
	int startind, endind;
	string dogid = "";
	vector<string> decendents, predecessors,dogdesc,dogpred;
	int distanceToSource = 0;
	int distanceToSink = 0;
	vector<int> indexes;
	vector<bool> directions;
	int dogcount = 0;
};


vector<int> top;
vector<int> bot;
vector<vector<string>> zone;
vector<vector<string>> final_zone;
vector<string> union_zone;
vector<string> union_zone_diff;
vector<vector<string>> ini_zone;
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
int VCGexists(string, string);
//void transientRemoval();
void Zone_Sort();
void Zone_union();
void Zone_diff_union();
void sourceAndSink();
void findDistance();
bool distFromSource(string, string, int, vector<string>*, vector<string> *);
void distFromSink(string, string, int);
//int Merge();
string f(vector<string>);
string g(vector<string>, string);
//vector<string> zoneConvertToString(string index);
//vector<string> zoneConvertToStringFinal(string index); //Because we're lazy
//void zonesToString();
//void updateVCG(string a, string adog, string b, string bdog);
void updateZones(string, string);
bool sortNet(const net *a, const net *b);
vector<string> zoneConvertToStringAll(string);
vector<int> zoneEnds();
void trackToString();
void printToFile();
void dogleg(vector<string> *, vector<string>*);
void updateVCGDog(int, int, VCG*);
void processInput(GLFWwindow *);
void framebuffer_size_callback(GLFWwindow* , int , int);
string findExistingNetDog(string );
void arraytonetDog();
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

	//VCG 
	makeVCG();
	arraytonetDog();

	//Zoning Code

	zone = vector<vector<string>>(static_cast<int>(top.size()), vector<string>(20, "0"));
	Zoning();
	final_zone.resize(static_cast<int>(zone.size()));
	Zone_Sort();
	Zone_union();
	Zone_diff_union();
	//zonesToString();
	//zoneEnd = zoneEnds();      //Pending fix if needed
 	

	//while (Merge() > 0);

	printf("\n\n%d", clock() - start);
	printToFile();
	return 0;
}


///////////////////////////////////////////////////////////////////////
//                             Merging 								 //
///////////////////////////////////////////////////////////////////////

#pragma region Merging
//int Merge() {
//	vector<string> L, R, zoneHold;
//	string m, n;
//	int counter = 0;
//	for (size_t i = 0; i < zone.size() - 1; i++) {
//		zoneHold = final_zones[i];
//		R = ini_zones[i + 1];
//		L.insert(L.end(), zoneHold.begin(), zoneHold.end());
//		if (!R.empty())
//		{
//			n = g(L, (m = f(R)));
//
//			updateVCG(n, m);
//			updateZones(n, m);
//
//			L.erase(remove(L.begin(), L.end(), n), L.end());
//			L.push_back((n + "," + m));
//			counter++;
//
//			sourceAndSink();
//			findDistance();
//		}
//
//	}
//	return counter;
//}
//
//string f(vector<string> Q) {
//	double C = 100;
//	double highest = 0;
//	string high = "0";
//	for (size_t i = 0; i < Q.size(); i++) {
//		double hold = C * (allVCG[VCGexists((Q[i]))]->distanceToSource + allVCG[VCGexists((Q[i]))]->distanceToSink) + max(allVCG[VCGexists((Q[i]))]->distanceToSink, allVCG[VCGexists((Q[i]))]->distanceToSource);
//		if (hold > highest) {
//			highest = hold;
//			high = Q[i];
//		}
//	}
//	return high;
//}
//
//
//string g(vector<string> P, string m) {
//	double C = 100;
//	double lowest = 100000;
//	string low = "0";
//	for (size_t i = 0; i < P.size(); i++) {
//		double hold = C * (max(allVCG[VCGexists((P[i]))]->distanceToSource, allVCG[VCGexists((m))]->distanceToSource) + max(allVCG[VCGexists((P[i]))]->distanceToSink, allVCG[VCGexists((m))]->distanceToSink)
//			- max(allVCG[VCGexists((P[i]))]->distanceToSource + allVCG[VCGexists((P[i]))]->distanceToSink, allVCG[VCGexists((m))]->distanceToSink + allVCG[VCGexists((m))]->distanceToSource))
//			- sqrt(allVCG[VCGexists((m))]->distanceToSource * allVCG[VCGexists((P[i]))]->distanceToSource) - sqrt(allVCG[VCGexists((m))]->distanceToSink * allVCG[VCGexists((P[i]))]->distanceToSink);
//
//		if (hold < lowest) {
//			lowest = hold;
//			low = P[i];
//		}
//	}
//
//	return low;
//}
//
//vector<string> zoneConvertToString(string index) {
//	vector<string> ret;
//	for (size_t i = 0; i < ini_zone[stoi(index)].size(); i++) {
//		ret.push_back(to_string(ini_zone[stoi(index)][i]));
//	}
//	return ret;
//}
//
//vector<string> zoneConvertToStringFinal(string index) {
//	vector<string> ret;
//	for (size_t i = 0; i < final_zone[stoi(index)].size(); i++) {
//		ret.push_back(to_string(final_zone[stoi(index)][i]));
//	}
//	return ret;
//}
//
//vector<string> zoneConvertToStringAll(string index) {
//	vector<string> ret;
//	for (size_t i = 0; i < zone[stoi(index)].size(); i++) {
//		ret.push_back(to_string(zone[stoi(index)][i]));
//	}
//	return ret;
//}
//
//void zonesToString() {
//	for (size_t i = 0; i < ini_zone.size(); i++) {
//		ini_zones.push_back(zoneConvertToString(to_string(i)));
//	}
//
//	for (size_t i = 0; i < final_zone.size(); i++) {
//		final_zones.push_back(zoneConvertToStringFinal(to_string(i)));
//	}
//
//	for (size_t i = 0; i < zone.size(); i++) {
//		zones.push_back(zoneConvertToStringAll(to_string(i)));
//	}
//}
//
//void updateVCG(string a, string adog, string b, string bdog) {
//	vector<string> newdesc, newpred;
//
//	for (size_t i = 0; i < allVCG.size(); i++) {
//		if (find(allVCG[i]->decendents.begin(), allVCG[i]->decendents.end(), a) != allVCG[i]->decendents.end() && allVCG[i]->decendents.size() > 0)
//		{
//			newdesc.insert(newdesc.end(), allVCG[VCGexists(a, adog)]->decendents.begin(), allVCG[VCGexists(a, adog)]->decendents.end());
//			allVCG[i]->decendents.erase(remove(allVCG[i]->decendents.begin(), allVCG[i]->decendents.end(), a), allVCG[i]->decendents.end());
//			if (!(find(allVCG[i]->decendents.begin(), allVCG[i]->decendents.end(), a + "," + b) != allVCG[i]->decendents.end())) {
//				allVCG[i]->decendents.push_back(a + "," + b);
//			}
//		}
//		if (find(allVCG[i]->decendents.begin(), allVCG[i]->decendents.end(), b) != allVCG[i]->decendents.end() && allVCG[i]->decendents.size() > 0)
//		{
//			newdesc.insert(newdesc.end(), allVCG[VCGexists(b, bdog)]->decendents.begin(), allVCG[VCGexists(b, bdog)]->decendents.end());
//			allVCG[i]->decendents.erase(remove(allVCG[i]->decendents.begin(), allVCG[i]->decendents.end(), b), allVCG[i]->decendents.end());
//			if (!(find(allVCG[i]->decendents.begin(), allVCG[i]->decendents.end(), a + "," + b) != allVCG[i]->decendents.end())) {
//				allVCG[i]->decendents.push_back(a + "," + b);
//			}
//		}
//		if (find(allVCG[i]->predecessors.begin(), allVCG[i]->predecessors.end(), a) != allVCG[i]->predecessors.end() && allVCG[i]->predecessors.size() > 0)
//		{
//			newpred.insert(newpred.end(), allVCG[VCGexists(a, adog)]->predecessors.begin(), allVCG[VCGexists(a, adog)]->predecessors.end());
//			allVCG[i]->predecessors.erase(remove(allVCG[i]->predecessors.begin(), allVCG[i]->predecessors.end(), a), allVCG[i]->predecessors.end());
//			if (!(find(allVCG[i]->predecessors.begin(), allVCG[i]->predecessors.end(), a + "," + b) != allVCG[i]->predecessors.end())) {
//				allVCG[i]->predecessors.push_back(a + "," + b);
//			}
//		}
//		if (find(allVCG[i]->predecessors.begin(), allVCG[i]->predecessors.end(), b) != allVCG[i]->predecessors.end() && allVCG[i]->predecessors.size() > 0)
//		{
//			newpred.insert(newpred.end(), allVCG[VCGexists(b, bdog)]->predecessors.begin(), allVCG[VCGexists(b, bdog)]->predecessors.end());
//			allVCG[i]->predecessors.erase(remove(allVCG[i]->predecessors.begin(), allVCG[i]->predecessors.end(), b), allVCG[i]->predecessors.end());
//			if (!(find(allVCG[i]->predecessors.begin(), allVCG[i]->predecessors.end(), a + "," + b) != allVCG[i]->predecessors.end())) {
//				allVCG[i]->predecessors.push_back(a + "," + b);
//			}
//		}
//
//	}
//
//	sort(newdesc.begin(), newdesc.end());
//	sort(newpred.begin(), newpred.end());
//	newdesc.erase(unique(newdesc.begin(), newdesc.end()), newdesc.end());
//	newpred.erase(unique(newpred.begin(), newpred.end()), newpred.end());
//
//	VCG *combine = new VCG();
//	combine->decendents = newdesc;
//	combine->predecessors = newpred;
//	combine->netid = a + "," + b;
//	combine->distanceToSink = max(allVCG[VCGexists(a, adog)]->distanceToSink, allVCG[VCGexists(b, bdog)]->distanceToSink);
//	combine->distanceToSource = max(allVCG[VCGexists(a, adog)]->distanceToSource, allVCG[VCGexists(b, bdog)]->distanceToSource);
//
//	allVCG.erase(remove(allVCG.begin(), allVCG.end(), allVCG[VCGexists(a, adog)]), allVCG.end());
//	allVCG.erase(remove(allVCG.begin(), allVCG.end(), allVCG[VCGexists(b, bdog)]), allVCG.end());
//	allVCG.push_back(combine);
//}
//
//void updateZones(string a, string b) {
//	vector<string>::iterator it1, it2;
//	int ind1, ind2, i1, i2;
//	for (size_t i = 0; i < ini_zones.size(); i++) {
//		if ((it1 = find(ini_zones[i].begin(), ini_zones[i].end(), a)) != ini_zones[i].end())
//		{
//			ind1 = it1 - ini_zones[i].begin();
//			ini_zones[i].erase(ini_zones[i].begin() + ind1);
//			i1 = i;
//
//		}
//		if ((it2 = find(ini_zones[i].begin(), ini_zones[i].end(), b)) != ini_zones[i].end())
//		{
//			ind2 = it2 - ini_zones[i].begin();
//			ini_zones[i].erase(ini_zones[i].begin() + ind2);
//			i2 = i;
//		}
//	}
//
//	if (i1 > i2) {
//		ini_zones[i2].insert(ini_zones[i2].begin() + ind2, a + "," + b);
//	}
//	else {
//		ini_zones[i1].insert(ini_zones[i1].begin() + ind1, a + "," + b);
//	}
//
//	for (size_t i = 0; i < final_zones.size(); i++) {
//		if ((it1 = find(final_zones[i].begin(), final_zones[i].end(), a)) != final_zones[i].end())
//		{
//			ind1 = it1 - final_zones[i].begin();
//			final_zones[i].erase(final_zones[i].begin() + ind1);
//			i1 = i;
//		}
//		if ((it2 = find(final_zones[i].begin(), final_zones[i].end(), b)) != final_zones[i].end())
//		{
//			ind2 = it2 - final_zones[i].begin();
//			final_zones[i].erase(final_zones[i].begin() + ind2);
//			i2 = i;
//		}
//	}
//
//	if (i1 < i2) {
//		final_zones[i2].insert(final_zones[i2].begin() + ind2, a + "," + b);
//	}
//	else {
//		final_zones[i1].insert(final_zones[i1].begin() + ind1, a + "," + b);
//	}
//}
#pragma endregion


///////////////////////////////////////////////////////////////////////
//                             Zoning 								 //
///////////////////////////////////////////////////////////////////////

//creates zones for future use
#pragma region Zoning
void Zoning() {
	//Zoning Code
		
	vector< vector<string> > col;
	for (size_t i1 = 0; i1 < tops.size(); i1++) {
		vector<string> row; // Create an empty row
		for (size_t j1 = 0; j1 < allVCG.size(); j1++) {
			if (i1 <= allVCG[j1]->endind && i1 >= allVCG[j1]->startind) {
				row.push_back(allVCG[j1]->netid); // Add an element (column) to the row
			}
		}
		sort(row.begin(),row.end()); // Sort strings 
		col.push_back(row); // Add the row to the main vector
	}

	int p = 0;


	for (int k1 = 1; k1 < static_cast<int>(tops.size()) - 1; k1++) {
		//if (k1== static_cast<int>(top.size()-1) {

		if ((includes(col[k1].begin(), col[k1].end(), col[k1 + 1].begin(), col[k1 + 1].end())) || (includes(col[k1 + 1].begin(), col[k1 + 1].end(), col[k1].begin(), col[k1].end()))) {
			//cout << "Subset";
			std::vector<string> temp_colunion(static_cast<int>(col[k1].size()) * 10);
			std::vector<string>::iterator it;
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
	set<string> all;
	for (int i = 0; i < zone.size(); i++) {
		all.insert(zone[i].begin(), zone[i].end());
	}
	union_zone = vector<string>(all.begin(), all.end());
}


void Zone_Sort() {
	final_zone.resize(static_cast<int>(zone.size()));
	ini_zone.resize(static_cast<int>(zone.size()));
	vector<string>::iterator it1;
	vector<string>::iterator it2;

	vector<string> temp_diff_union;

	vector<string> last_zone(10);

	for (int l1 = 0; l1 < static_cast<int>(zone.size()) - 1; l1++) {
		vector<string> temp_zone_diff(10);

		it1 = set_difference(zone[l1].begin(), zone[l1].end(), zone[l1 + 1].begin(), zone[l1 + 1].end(), temp_zone_diff.begin());
		temp_zone_diff.resize(it1 - temp_zone_diff.begin());

		final_zone[l1] = temp_zone_diff;

		vector<string> temp_zone_diff1(10);

		if (l1 == 0) {
			ini_zone[l1] = zone[l1];
		}
		it2 = set_difference(zone[l1 + 1].begin(), zone[l1 + 1].end(), zone[l1].begin(), zone[l1].end(), temp_zone_diff1.begin());
		temp_zone_diff1.resize(it2 - temp_zone_diff1.begin());

		ini_zone[l1 + 1] = temp_zone_diff1;

	}


}

void Zone_diff_union() {
	set<string> all1;
	vector<string>::iterator it2;
	vector<string> temp_zone_union(2 * netlist.size());

	for (int i = 0; i < final_zone.size(); i++) {
		all1.insert(final_zone[i].begin(), final_zone[i].end());
	}
	union_zone_diff = vector<string>(all1.begin(), all1.end());

	it2 = set_difference(union_zone.begin(), union_zone.end(), union_zone_diff.begin(), union_zone_diff.end(), temp_zone_union.begin());
	temp_zone_union.resize(it2 - temp_zone_union.begin());

	final_zone[static_cast<int>(zone.size()) - 1] = temp_zone_union;
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
		int at = VCGexists((tops[i]), "");
		//VCG exists
		if (at != -1 && bot[i] != 0 && bots[i] != allVCG[at]->netid) {
			allVCG[at]->decendents.push_back((bots[i]));
			allVCG[at]->dogdesc.push_back("");
		}
		//VCG does not exist
		else if (at == -1 && top[i] != 0 && bot[i] != 0 && bots[i] != tops[i]) {
			VCG *n = new VCG;
			n->netid = (tops[i]);
			n->decendents.push_back((bots[i]));
			n->dogdesc.push_back("");
			allVCG.push_back(n);
		}
	}
	//bottom
	for (size_t i = 0; i < bots.size(); i++) {
		int at = VCGexists((bots[i]), "");
		//VCG exists
		if (at != -1 && top[i] != 0 && tops[i] != allVCG[at]->netid) {
			allVCG[at]->predecessors.push_back((tops[i]));
			allVCG[at]->dogpred.push_back((""));
		}
		//VCG does not exist
		else if (at == -1 && top[i] != 0 && bot[i] != 0 && bots[i] != tops[i]) {
			VCG *n = new VCG;
			n->netid = (bots[i]);
			n->predecessors.push_back((tops[i]));
			n->dogpred.push_back("");
			allVCG.push_back(n);
		}
	}

	for (size_t i = 0; i < netlist.size(); i++) {
		allVCG[VCGexists(to_string(netlist[i]->netnum), "")]->indexes = netlist[i]->indexes;
		allVCG[VCGexists(to_string(netlist[i]->netnum), "")]->directions = netlist[i]->directions;
	}

	sourceAndSink();
	findDistance();
	//transientRemoval();
}

//returns index of netid requested
//for use with makeVCG and transient removal
int VCGexists(string netid, string dogid) {
	for (size_t i = 0; i < allVCG.size(); i++) {
		if (allVCG[i]->netid == netid && allVCG[i]->dogid == dogid) {
			return i;
		}
	}

	return -1;
}

//Removes transient edges with low effort O(n3)
//void transientRemoval() {
//	vector<string> possibleRemove;
//	vector<string> dogRemove;
//
//	//remove transient edges
//	//iterate through all VCG
//	for (size_t i = 0; i < allVCG.size(); i++) {
//		possibleRemove = allVCG[i]->decendents;
//		//iterate through decendents
//		for (size_t j = 0; j < possibleRemove.size(); j++) {
//			//get list of decendents of decendents
//			VCG *check = allVCG[VCGexists(possibleRemove[j])];
//			//remove common decendents from tallest ancestor
//			for (size_t k = 0; k < check->decendents.size(); k++) {
//				possibleRemove.erase(std::remove(possibleRemove.begin(), possibleRemove.end(), check->decendents[k]), possibleRemove.end());
//			}
//		}
//		allVCG[i]->decendents = possibleRemove;
//	}
//
//	for (size_t i = 0; i < allVCG.size(); i++) {
//		possibleRemove = allVCG[i]->predecessors;
//		//iterate through predecessors
//		for (size_t j = 0; j < possibleRemove.size(); j++) {
//			//get list of preds of preds
//			VCG *check = allVCG[VCGexists(possibleRemove[j])];
//			//remove common predecessor from tallest offspring
//			for (size_t k = 0; k < check->predecessors.size(); k++) {
//				possibleRemove.erase(std::remove(possibleRemove.begin(), possibleRemove.end(), check->predecessors[k]), possibleRemove.end());
//			}
//		}
//		allVCG[i]->predecessors = possibleRemove;
//	}
//}

//fills the source and sink vectors
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
	for (int i = 0; i < (int)source.size(); i++) {
		vector<string> *p = new vector<string>();
		vector<string> *f = new vector<string>();
		if (!distFromSource(source[i]->netid, source[i]->dogid, 0, p, f)) {
			sourceAndSink();
			i = -1;
		}
	}

	for (size_t i = 0; i < sink.size(); i++) {
		distFromSink(sink[i]->netid, sink[i]->dogid, 0);
	}
}

bool distFromSource(string netid, string dogid, int counter, vector<string> *path, vector<string> *dogpath) {
	bool flag = true;
	if (counter > allVCG[VCGexists(netid, dogid)]->distanceToSource) {
		allVCG[VCGexists(netid, dogid)]->distanceToSource = counter;
	}
	
	for (size_t i = 0; i < allVCG[VCGexists(netid, dogid)]->decendents.size(); i++) {
		if (find(path->begin(), path->end(), allVCG[VCGexists(netid, dogid)]->decendents[i]) != path->end()) {
			path->erase(path->begin(), find(path->begin(), path->end(), allVCG[VCGexists(netid, dogid)]->decendents[i]));
			dogleg(path, dogpath);
			flag = false;
			break;
		}
		path->push_back(allVCG[VCGexists(netid, dogid)]->decendents[i]);
		dogpath->push_back(allVCG[VCGexists(netid, dogid)]->dogdesc[i]);
		if (allVCG[VCGexists(netid, dogid)]->decendents.size() > 0) {
			distFromSource(allVCG[VCGexists(netid, dogid)]->decendents[i], allVCG[VCGexists(netid, dogid)]->dogdesc[i], counter + 1, path, dogpath);
		}
	}

	if (flag) {
		return true;
	}
	else {
		return false;
	}
}

void distFromSink(string netid, string dogid, int counter) {
	if (counter > allVCG[VCGexists(netid, dogid)]->distanceToSink) {
		allVCG[VCGexists(netid, dogid)]->distanceToSink = counter;
	}
	for (size_t i = 0; i < allVCG[VCGexists(netid, dogid)]->predecessors.size(); i++) {
		distFromSink(allVCG[VCGexists(netid, dogid)]->predecessors[i], allVCG[VCGexists(netid, dogid)]->predecessors[i], counter + 1);
	}
}

int getNetlistInd(int index) {
	for (size_t i = 0; i < netlist.size(); i++) {
		if (index == netlist[i]->netnum) {
			return i;
		}
	}
}

//vector<int> zoneEnds() {
//	vector<int> ret;
//	int maxEnd = 0;
//	for (size_t i = 0; i < final_zone.size(); i++) {
//		for (size_t j = 0; j < final_zone[i].size(); j++)
//		{
//			maxEnd = (maxEnd < netlist[getNetlistInd(final_zone[i][j])]->endind) ? netlist[getNetlistInd(final_zone[i][j])]->endind : maxEnd;
//		}
//		ret.push_back(maxEnd);
//		maxEnd = 0;
//	}
//
//	return ret;
//}

void trackToString() {
	for (size_t i = 0; i < top.size(); i++) {
		tops.push_back(to_string(top[i]));
	}
	for (size_t i = 0; i < bot.size(); i++) {
		bots.push_back(to_string(bot[i]));
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

void arraytonetDog() {
	string previndex;
	//top part of input
	for (int i = 0; i < tops.size(); i++) {
		VCG *next = new VCG;
		string nextnet = tops.at(i);
		string net = findExistingNetDog(nextnet);

		if (nextnet.compare("0")) {
			continue;
		}
		else if (!(net.compare("-1"))) {
			allVCG[VCGexistsDog(net)]->indexes.push_back(i);
			allVCG[VCGexistsDog(net)]->directions.push_back(true);
			allVCG[VCGexistsDog(net)]->endind = i;
		}
		else {
			next->startind = i;
			next->directions.push_back(true);
			next->endind = i;
			next->indexes.push_back(i);
			next->netid = nextnet;
			allVCG.push_back(next);
		}
	}
	//bottom part of input
	for (int i = 0; i < bot.size(); i++) {
		VCG *next = new VCG;
		string nextnet = tops.at(i);
		string net = findExistingNetDog(nextnet);

		if (nextnet.compare("0")) {
			continue;
		}
		else if (!(net.compare("-1"))) {
			allVCG[VCGexistsDog(net)]->indexes.push_back(i);
			allVCG[VCGexistsDog(net)]->directions.push_back(true);
			if (allVCG[VCGexistsDog(net)]->startind > i) {
				allVCG[VCGexistsDog(net)]->startind = i;
			}
			if (allVCG[VCGexistsDog(net)]->endind < i) {
				allVCG[VCGexistsDog(net)]->endind = i;
			}
		}
		else {
			next->startind = i;
			next->directions.push_back(false);
			next->endind = i;
			next->indexes.push_back(i);
			next->netid = nextnet;
			allVCG.push_back(next);
		}
	}
	
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
string findExistingNetDog(string net) {

	for (int i = 0; i < allVCG.size(); i++) {
		if (allVCG[i]->netid.compare(net)) {
			return allVCG[i]->netid;
		}
	}

	return "-1";
}
#pragma endregion


///////////////////////////////////////////////////////////////////////
//							DOGLEGGING								 //
///////////////////////////////////////////////////////////////////////

void dogleg(vector<string> *path, vector<string> *dogpath) {
	int index = -1;
	bool direction;

	//find index at which cycle begins
	VCG *hold = allVCG[VCGexists(path->at(0), dogpath->at(0))];
	VCG *under = allVCG[VCGexists(path->at(1), dogpath->at(0))];
	VCG *above = allVCG[VCGexists(path->at(path->size() - 1), dogpath->at(path->size() - 1))];
	
	for (size_t j = 0; j < hold->indexes.size(); j++) {
		if (hold->directions[j]) {
			//check  for dogid

			if (bots[hold->indexes[j]] == (under->netid + under->dogid)) {
				index = hold->indexes[j];
				direction = true;
				break;
			}
			
		}
		if (!hold->directions[j]) {
			//check for dogid
			if (tops[hold->indexes[j]] == (above->netid + above->dogid)) {
				index = hold->indexes[j];
				direction = false;
				break;
			}
		}
	}


	// find possible dogleg index
	int offender, dogindex;
	if (direction) {
		offender = bot[index];
	}
	else {
		offender = top[index];
	}

	for (int i = index + 1; i < top.size(); i++) {
		if (direction) {
			if (offender == top[i]) {
				offender = bot[i];
				continue;
			}
			dogindex = i;
			break;
		}
		else {
			if (offender == bot[i]) {
				offender = top[i];
				continue;
			}
			dogindex = i;
		}
	}

	updateVCGDog(index, dogindex, hold);
}

int VCGexistsDog(string id) {
	int i = 0;
	bool flag = false;
	for (char c : id) {
		if (c > 65) {
			flag = true;
			break;
		}
		i++;
	}

	return VCGexists(id.substr(0, i), flag ? id.substr(i + 1) : "");
}

int findPred(vector<string>::iterator it, string net, string dog, int index) {
	vector<string>::iterator it2;
	it2 = find(allVCG[index]->predecessors.begin() + (it - allVCG[index]->predecessors.begin()), allVCG[index]->predecessors.end(), net);
	if (it2 == allVCG[index]->predecessors.end()) {
		return -1;
	}
	int ind = it - allVCG[index]->predecessors.begin();
	if (allVCG[index]->dogpred[ind] == dog) {
		return ind;
	}
	else {
		findPred(it2 + 1, net, dog, index);
	}
}

int findDesc(vector<string>::iterator it, string net, string dog, int index) {
	vector<string>::iterator it2;
	it2 = find(allVCG[index]->decendents.begin() + (it - allVCG[index]->decendents.begin()), allVCG[index]->decendents.end(), net);
	if (it2 == allVCG[index]->decendents.end()) {
		return -1;
	}
	int ind = it - allVCG[index]->decendents.begin();
	if (allVCG[index]->dogdesc[ind] == dog) {
		return ind;
	}
	else {
		findDesc(it2 + 1, net, dog, index);
	}
}

vector<string> separateTrack(int index, bool track) {
	vector<string> ret;
	int i = 0;
	bool flag = false;
	for (char c : track ? tops[index] : bots[index]) {
		if (c > 65) {
			flag = true;
			break;
		}
		i++;
	}

	if (flag) {
		ret.push_back(track ? tops[index].substr(0, i) : bots[index].substr(0, i));
		ret.push_back(track ? tops[index].substr(i + 1) : bots[index].substr(i + 1));
	}
	else {
		ret.push_back(track ? tops[index].substr(0, i) : bots[index].substr(0, i));
		ret.push_back("");
	}
	return ret;
}

void lexiDog(string netid, string dogid) {
	vector<VCG*> ret;

	for (size_t i = 0; i < allVCG.size(); i++) {
		if (allVCG[i]->netid == netid && allVCG[i]->dogid < dogid) {
			ret.push_back(allVCG[i]);
		}
	}

	for (size_t i = 0; i < tops.size(); i++) {
		
		if (tops[i] < (netid + dogid)) {
			stringstream ss;
			ss << (dogid[0] + 1);
			string s;
			ss >> s;
			tops[i] = netid + s;
		}
		if (bots[i] < (netid + dogid)) {
			stringstream ss;
			ss << (dogid[0] + 1);
			string s;
			ss >> s;
			bots[i] = netid + s;
		}
	}

	for (VCG* v : ret) {
		for (size_t i = 0; i < allVCG.size(); i++) {
			int predind = 0, descind = 0;
			while ((predind = findPred(allVCG[i]->predecessors.begin() + predind, v->netid, v->dogid, i)) != -1)
			{
				if (predind > 0) {
					char c = allVCG[i]->dogpred[predind][0];
					c++;
					stringstream ss;
					ss << c;
					string s;
					ss >> s;
					allVCG[i]->dogpred[predind] = s;
				}
			}
			while ((descind = findDesc(allVCG[i]->decendents.begin() + descind, v->netid, v->dogid, i)) != -1) {
				if (descind > 0) {
					char c = allVCG[i]->dogdesc[descind][0];
					c++;
					stringstream ss;
					ss << c;
					string s;
					ss >> s;
					allVCG[i]->dogdesc[descind] = s;
				}
			}
		}

		char c = v->dogid[0];
		c++;
		stringstream ss;
		ss << c;
		string s;
		ss >> s;
		v->dogid = s;
		v->dogcount++;
	}
}

void updateVCGDog(int ind, int dogind, VCG* rem) {
	VCG *a = new VCG();
	VCG *b = new VCG();

	if (rem->dogid != "") {
		lexiDog(rem->netid, rem->dogid);
	}

	//remove every reference of doglegged thing
	for (size_t i = 0; i < allVCG.size(); i++) {
		int predind = findPred(allVCG[i]->predecessors.begin(), rem->netid, rem->dogid, i);
		int descind = findDesc(allVCG[i]->decendents.begin(), rem->netid, rem->dogid, i);
		
		if (descind != -1)
		{
			allVCG[i]->decendents.erase(remove(allVCG[i]->decendents.begin() + descind , allVCG[i]->decendents.end(), rem->netid), allVCG[i]->decendents.end());
			allVCG[i]->dogdesc.erase(remove(allVCG[i]->dogdesc.begin() + descind, allVCG[i]->dogdesc.end(), rem->dogid), allVCG[i]->dogdesc.end());			
		}
		
		if (predind != -1)
		{
			allVCG[i]->predecessors.erase(remove(allVCG[i]->predecessors.begin() + predind, allVCG[i]->predecessors.end(), rem->netid), allVCG[i]->predecessors.end());
			allVCG[i]->dogpred.erase(remove(allVCG[i]->dogpred.begin() + predind, allVCG[i]->dogpred.end(), rem->dogid), allVCG[i]->dogpred.end());
		}
	}

	stringstream ss;
	ss << (char)('A' + rem->dogcount);
	string s;
	ss >> s;
	a->netid = rem->netid;
	a->dogid = s;
	a->dogcount = rem->dogcount;
	string net = rem->netid + rem->dogid; // change for double dogleg when necessary
	
	vector<string> id;
	for (size_t i = 0; i < dogind; i++) {
		if (!tops[i].compare(net)) {
			id = separateTrack(i, true);
			a->decendents.push_back(id[0]);//separate bots
			a->dogdesc.push_back(id[1]);
			a->indexes.push_back(i);
			a->directions.push_back(true);
			allVCG[VCGexistsDog(bots[i])]->predecessors.push_back(a->netid);
			allVCG[VCGexistsDog(bots[i])]->dogpred.push_back(a->dogid);
			tops[i] = tops[i] + s;
		}
		else if (!bots[i].compare(net)) {
			id = separateTrack(i, false);
			a->predecessors.push_back(id[0]);
			a->dogpred.push_back(id[1]);
			a->indexes.push_back(i);
			a->directions.push_back(false);
			allVCG[VCGexistsDog(tops[i])]->decendents.push_back(a->netid);
			allVCG[VCGexistsDog(tops[i])]->dogdesc.push_back(a->dogid);
			bots[i] = bots[i] + s;
		}
	}
	

	stringstream ss1;
	ss1 << (char)('A' + rem->dogcount + 1);
	s = "";
	ss1 >> s;
	b->netid = rem->netid;
	b->dogid = s;
	b->dogcount = rem->dogcount + 1; // change for double dogleg when necessary

	for (size_t i = dogind; i < tops.size(); i++) {
		if (!tops[i].compare(net)) {
			id = separateTrack(i, true);
			b->decendents.push_back(id[0]);
			b->dogdesc.push_back(id[1]);
			b->indexes.push_back(i);
			b->directions.push_back(true);
			allVCG[VCGexistsDog(bots[i])]->predecessors.push_back(b->netid);
			allVCG[VCGexistsDog(bots[i])]->dogpred.push_back(b->dogid);
			tops[i] = tops[i] + s;
		}
		else if (!bots[i].compare(net)) {
			id = separateTrack(i, false);
			b->predecessors.push_back(id[0]);
			b->dogpred.push_back(id[1]);
			b->indexes.push_back(i);
			b->directions.push_back(false);
			allVCG[VCGexistsDog(tops[i])]->decendents.push_back(b->netid);
			allVCG[VCGexistsDog(tops[i])]->dogdesc.push_back(b->dogid);
			bots[i] = bots[i] + s;
		}
	}

	allVCG.push_back(a);
	allVCG.push_back(b);
	allVCG.erase(remove(allVCG.begin(), allVCG.end(), rem), allVCG.end());
}

///////////////////////////////////////////////////////////////////////
//                             Printing								 //
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

///////////////////////////////////////////////////////////////////////
//                             OpenGL								 //
///////////////////////////////////////////////////////////////////////

#pragma region OpenGL

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";
const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";

int draw(void)
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "YK Routing", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// build and compile our shader program
	// ------------------------------------
	// vertex shader
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// fragment shader
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// link shaders
	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		-1.0f, -0.521f, 0.0f, // left  
		0.22f, -0.521f, 0.0f, // right 
		0.5f, -0.5f, 0.0f,  // top
		0.5f, 0.5f, 0.0f
	};

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);


	// uncomment this call to draw in wireframe polygons.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// draw our first triangle
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

		glDrawArrays(GL_LINES, 0, 4);
		// glBindVertexArray(0); // no need to unbind it every time 

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}
#pragma endregion
