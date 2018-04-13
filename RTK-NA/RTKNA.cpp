// RTKNA.cpp : Defines the entry point for the console application.
//


#include "RTKheader.hpp"
vector<int> top, bot, zoneEnd;
vector<vector<string>> zone, final_zone, ini_zone, ininet, inidog, finalnet, finaldog;
vector<string> union_zone_diff, *predpath, union_zone, tops, bots, L, Ldog, ignore, netsatvertex;
net * first;
vector<net*> netlist;
vector<VCG*> allVCG, source, sink, mergedVCG;

bool dog, merging, doglegAlldone, outputFlag, suppressFlag;
int dogcounter;
vector<float> netvertex;
double highesthold, C, lowesthold;

int main(int argc, char *argv[])
{
	predpath = new vector<string>();
	dogcounter = 0;
	merging = false, outputFlag = false, dog = false, suppressFlag = false;
	string filepath, dog1;

	//getline(cin, filepath);

	if (argc > 7)
	{
		for (int i = 1; i < argc; i++) {
			string arg = argv[i];
			if (arg == "-d") {
				dog = true;
				cout << "doglegging enabled\n";
			}
			else if (arg == "-c" || arg == "-C") {
				C = atoi(argv[i + 1]);
				cout << "C: " + to_string(C) + " ";
			}
			else if (arg == "-f" || arg == "-F") {
				highesthold = atoi(argv[i + 1]);
				cout << "f: " + to_string(highesthold) + " ";
			}
			else if (arg == "-g" || arg == "-G") {
				lowesthold = atoi(argv[i + 1]);
				cout << "g: " + to_string(lowesthold) + " ";
			}
			else if (arg == "-i") {
				filepath = argv[i + 1];
			}
			else if (arg == "--suppress") {
				suppressFlag = true;
			}
		}
	}
	else {
		//printHelp();
		cout << "incorrect amount of variables";
		return 1;
	}

	if (!suppressFlag)
	{
		cout << "\n--------------------------BEGIN PROGRAM--------------------------\n";
	}
	clock_t start = clock();
	parse(filepath);
	arraytonet();

	//VCG 
	makeVCG();
	if (dog) {
		doglegAll();
		sourceAndSink();
		findDistance();
	}

	//Zoning Code
	if (!suppressFlag)
	{
		cout << "\n--------------------------BEGIN ZONING--------------------------\n";
	}
		Zoning();
		final_zone.resize(static_cast<int>(zone.size()));
		Zone_Sort();
		Zone_union();
		Zone_diff_union();
		convertToNetDog();
	
	if (!suppressFlag)
	{
		cout << "\n--------------------------BEGIN MERGING--------------------------\n";
	}
	while (Merge() > 0);

	cout << "\n\nMerging finished\nMerge stats:\nMerged nets: " + to_string(mergedVCG.size()) + "\nTotal height: " + to_string(allVCG.size()) +"\n";


	printf("\n\nProgram runtime(ms): %d\n\n", clock() - start);
	cout << "drawing results...\n\n";
	makedrawvertex();
	draw();

	return 0;
}

int origindex;

bool pathToSink(string netid, string dogid, int index) {
	for (size_t i = 0; i < allVCG[index]->decendents.size(); i++) {
		if (findDesc(allVCG[VCGexists(allVCG[index]->decendents[i], allVCG[index]->dogdesc[i])]->decendents.begin(), netid, dogid, VCGexists(allVCG[index]->decendents[i], allVCG[index]->dogdesc[i])) != -1) {
			return false;
		}
		else {
			if (!pathToSink(netid, dogid, VCGexists(allVCG[index]->decendents[i], allVCG[index]->dogdesc[i]))) {
				return false;
			}
		}
	}
	if (origindex == index)
	{
		return true;
	}
}

bool pathToSource(string netid, string dogid, int index) {
	for (size_t i = 0; i < allVCG[index]->predecessors.size(); i++) {
		if (findPred(allVCG[VCGexists(allVCG[index]->predecessors[i], allVCG[index]->dogpred[i])]->predecessors.begin(), netid, dogid, VCGexists(allVCG[index]->predecessors[i], allVCG[index]->dogpred[i])) != -1) {
			return false;
		}
		else {
			if (!pathToSource(netid, dogid, VCGexists(allVCG[index]->predecessors[i], allVCG[index]->dogpred[i]))) {
				return false;
			}
		}
	}
	if (origindex == index)
	{
		return true;
	}
}


///////////////////////////////////////////////////////////////////////
//                             Merging 								 //
///////////////////////////////////////////////////////////////////////

#pragma region Merging

int findininet(vector<string>::iterator it, string net, string dog, int index) {
	vector<string>::iterator it2;
	it2 = find(ininet[index].begin() + (it - ininet[index].begin()), ininet[index].end(), net);
	if (it2 == ininet[index].end()) {
		return -1;
	}
	int ind = it2 - ininet[index].begin();
	if (inidog[index][ind] == dog) {
		return ind;
	}
	else {
		return findininet(it2 + 1, net, dog, index);
	}
}

int findfinalnet(vector<string>::iterator it, string net, string dog, int index) {
	vector<string>::iterator it2;
	it2 = find(finalnet[index].begin() + (it - finalnet[index].begin()), finalnet[index].end(), net);
	if (it2 == finalnet[index].end()) {
		return -1;
	}
	int ind = it2 - finalnet[index].begin();
	if (finaldog[index][ind] == dog) {
		return ind;
	}
	else {
		return findfinalnet(it2 + 1, net, dog, index);
	}
}

int findL(vector<string>::iterator it, string net, string dog) {
	//string::iterator it2;
	int i = (it - L.begin());
	auto it2 = find(L.begin() + i, L.end(), net);
	if (it2 == L.end()) {
		return -1;
	}
	int ind = it2 - L.begin();
	if (Ldog[ind] == dog) {
		return ind;
	}
	else {
		return findL(it2 + 1, net, dog);
	}
}

int Merge() {
	merging = true;
	vector<string> R, zoneHold, Rdog, zoneHoldDog;
	vector<string> m, n;
	L.clear();
	Ldog.clear();
	int counter = 0, ind;
	for (size_t i = 0; i < zone.size() - 1; i++) {
		zoneHold = finalnet[i];
		zoneHoldDog = finaldog[i];
		R = ininet[i + 1];
		Rdog = inidog[i + 1];
		L.insert(L.end(), zoneHold.begin(), zoneHold.end());
		Ldog.insert(Ldog.end(), zoneHoldDog.begin(), zoneHoldDog.end());
		if (!R.empty())
		{
			m = f(R, Rdog);
			if (!m.empty())
			{
				n = g(L, Ldog, m);
			}
			else {
				continue;
			}

			if (!n.empty())
			{
				if (!suppressFlag)
				{
					cout << "Merging nets " + n[0] + n[1] + " and " + m[0] + m[1] + "...\n";
				}
				updateVCG(n, m);
				updateZones(n, m);

				while ((ind = findL(L.begin(), n[0], n[1])) != -1)
				{
					L.erase(L.begin() + ind);
					Ldog.erase(Ldog.begin() + ind);
				}

				counter++;

				sourceAndSink();
				findDistance();
			}
		}

	}
	return counter;
}

vector<string> f(vector<string> Q, vector<string> N) {

	double highest = highesthold;
	vector<string> high;

	for (size_t i = 0; i < Q.size(); i++) {
		double hold = C * (allVCG[VCGexists((Q[i]), N[i])]->distanceToSource + allVCG[VCGexists((Q[i]), N[i])]->distanceToSink) + max(allVCG[VCGexists((Q[i]), N[i])]->distanceToSink, allVCG[VCGexists((Q[i]), N[i])]->distanceToSource);
		if (hold > highest) {
			highest = hold;
			if (high.empty())
			{
				high.push_back(Q[i]);
				high.push_back(N[i]);
			}
			else {
				high[0] = Q[i];
				high[1] = N[i];
			}
		}
	}
	return high;
}

vector<string> g(vector<string> P, vector<string> E, vector<string> m) {

	double lowest = lowesthold;
	vector<string> low;
	for (size_t i = 0; i < P.size(); i++) {
		double hold = C * (max(allVCG[VCGexists(P[i], E[i])]->distanceToSource, allVCG[VCGexists((m[0]), m[1])]->distanceToSource) + max(allVCG[VCGexists((P[i]), E[i])]->distanceToSink, allVCG[VCGexists((m[0]), m[1])]->distanceToSink)
			- max(allVCG[VCGexists((P[i]), E[i])]->distanceToSource + allVCG[VCGexists((P[i]), E[i])]->distanceToSink, allVCG[VCGexists((m[0]), m[1])]->distanceToSink + allVCG[VCGexists(m[0], m[1])]->distanceToSource))
			- sqrt(allVCG[VCGexists((m[0]), m[1])]->distanceToSource * allVCG[VCGexists((P[i]), E[i])]->distanceToSource) - sqrt(allVCG[VCGexists((m[0]), m[1])]->distanceToSink * allVCG[VCGexists((P[i]), E[i])]->distanceToSink);

		origindex = VCGexists(m[0], m[1]);
		if (hold < lowest && pathToSink(P[i], E[i], origindex) && pathToSource(P[i], E[i], origindex)) {
			lowest = hold;
			if (low.empty())
			{
				low.push_back(P[i]);
				low.push_back(E[i]);
			}
			else {
				low[0] = P[i];
				low[1] = E[i];
			}
		}
	}

	return low;
}

vector<vector<string>> unique(vector<string> a1, vector<string> a2, vector<string> b1, vector<string> b2) {
	vector<string> reta = a1, retb = a2;
	bool flag = false;

	for (size_t i = 0; i < b1.size(); i++) {
		for (size_t j = 0; j < reta.size(); j++) {
			if (reta[j] == b1[i] && retb[j] == b2[i]) {
				flag = true;
				break;
			}
		}
		if (!flag) {
			reta.push_back(b1[i]);
			retb.push_back(b2[i]);
		}
		flag = false;
	}

	vector<vector<string>> ret;
	ret.push_back(reta);
	ret.push_back(retb);
	return ret;
}

void updateVCG(vector<string> a, vector<string> b) {
	vector<string> newdesc, newpred, newdogdesc, newdogpred;


#pragma region desc
	vector<vector<string>> s = unique(allVCG[VCGexists(a[0], a[1])]->decendents, allVCG[VCGexists(a[0], a[1])]->dogdesc, allVCG[VCGexists(b[0], b[1])]->decendents, allVCG[VCGexists(b[0], b[1])]->dogdesc);
	newdesc = s[0];
	newdogdesc = s[1];

	s = unique(allVCG[VCGexists(a[0], a[1])]->predecessors, allVCG[VCGexists(a[0], a[1])]->dogpred, allVCG[VCGexists(b[0], b[1])]->predecessors, allVCG[VCGexists(b[0], b[1])]->dogpred);
	newpred = s[0];
	newdogpred = s[1];
#pragma endregion

	for (size_t i = 0; i < allVCG.size(); i++) {
		//if any occurance of a in desc
		int descind, predind;

#pragma region Replacement Block

		if (findDesc(allVCG[i]->decendents.begin(), a[0], a[1], i) != -1)
		{
			while ((descind = findDesc(allVCG[i]->decendents.begin(), a[0], a[1], i)) != -1) {
				allVCG[i]->decendents.erase(allVCG[i]->decendents.begin() + descind);
				allVCG[i]->dogdesc.erase(allVCG[i]->dogdesc.begin() + descind);
			}

			//prevent double edge
			if (!(find(allVCG[i]->decendents.begin(), allVCG[i]->decendents.end(), a[0] + "," + b[0]) != allVCG[i]->decendents.end()) && !(find(allVCG[i]->dogdesc.begin(), allVCG[i]->dogdesc.end(), (a[1].empty() ? " " : a[1]) + "," + (b[1].empty() ? " " : b[1])) != allVCG[i]->dogdesc.end())) {
				allVCG[i]->decendents.push_back(a[0] + "," + b[0]);
				allVCG[i]->dogdesc.push_back((a[1].empty() ? " " : a[1]) + "," + (b[1].empty() ? " " : b[1]));
			}
		}

		if (findDesc(allVCG[i]->decendents.begin(), b[0], b[1], i) != -1)
		{
			while ((descind = findDesc(allVCG[i]->decendents.begin(), b[0], b[1], i)) != -1) {

				allVCG[i]->decendents.erase(allVCG[i]->decendents.begin() + descind);
				allVCG[i]->dogdesc.erase(allVCG[i]->dogdesc.begin() + descind);
			}

			if (!(find(allVCG[i]->decendents.begin(), allVCG[i]->decendents.end(), a[0] + "," + b[0]) != allVCG[i]->decendents.end()) && !(find(allVCG[i]->dogdesc.begin(), allVCG[i]->dogdesc.end(), (a[1].empty() ? " " : a[1]) + "," + (b[1].empty() ? " " : b[1])) != allVCG[i]->dogdesc.end())) {
				allVCG[i]->decendents.push_back(a[0] + "," + b[0]);
				allVCG[i]->dogdesc.push_back((a[1].empty() ? " " : a[1]) + "," + (b[1].empty() ? " " : b[1]));
			}
		}

		if (findPred(allVCG[i]->predecessors.begin(), a[0], a[1], i) != -1)
		{
			while ((predind = findPred(allVCG[i]->predecessors.begin(), a[0], a[1], i)) != -1) {

				allVCG[i]->predecessors.erase(allVCG[i]->predecessors.begin() + predind);
				allVCG[i]->dogpred.erase(allVCG[i]->dogpred.begin() + predind);
			}

			if (!(find(allVCG[i]->predecessors.begin(), allVCG[i]->predecessors.end(), a[0] + "," + b[0]) != allVCG[i]->predecessors.end()) && !(find(allVCG[i]->dogpred.begin(), allVCG[i]->dogpred.end(), (a[1].empty() ? " " : a[1]) + "," + (b[1].empty() ? " " : b[1])) != allVCG[i]->dogpred.end())) {
				allVCG[i]->predecessors.push_back(a[0] + "," + b[0]);
				allVCG[i]->dogpred.push_back((a[1].empty() ? " " : a[1]) + "," + (b[1].empty() ? " " : b[1]));
			}
		}

		if (findPred(allVCG[i]->predecessors.begin(), b[0], b[1], i) != -1)
		{
			while ((predind = findPred(allVCG[i]->predecessors.begin(), b[0], b[1], i)) != -1) {

				allVCG[i]->predecessors.erase(allVCG[i]->predecessors.begin() + predind);
				allVCG[i]->dogpred.erase(allVCG[i]->dogpred.begin() + predind);
			}

			if (!(find(allVCG[i]->predecessors.begin(), allVCG[i]->predecessors.end(), a[0] + "," + b[0]) != allVCG[i]->predecessors.end()) && !(find(allVCG[i]->dogpred.begin(), allVCG[i]->dogpred.end(), (a[1].empty() ? " " : a[1]) + "," + (b[1].empty() ? " " : b[1])) != allVCG[i]->dogpred.end())) {
				allVCG[i]->predecessors.push_back(a[0] + "," + b[0]);
				allVCG[i]->dogpred.push_back((a[1].empty() ? " " : a[1]) + "," + (b[1].empty() ? " " : b[1]));
			}
		}

	}
#pragma endregion


#pragma region new VCG object creation
	VCG *combine = new VCG();
	combine->decendents = newdesc;
	combine->predecessors = newpred;
	combine->dogdesc = newdogdesc;
	combine->dogpred = newdogpred;
	combine->netid = a[0] + "," + b[0];
	combine->dogid = (a[1].empty() ? " " : a[1]) + "," + (b[1].empty() ? " " : b[1]);
	combine->distanceToSink = max(allVCG[VCGexists(a[0], a[1])]->distanceToSink, allVCG[VCGexists(b[0], b[1])]->distanceToSink);
	combine->distanceToSource = max(allVCG[VCGexists(a[0], a[1])]->distanceToSource, allVCG[VCGexists(b[0], b[1])]->distanceToSource);
	combine->startind = min(allVCG[VCGexists(a[0], a[1])]->startind, allVCG[VCGexists(b[0], b[1])]->startind);
	combine->endind = max(allVCG[VCGexists(a[0], a[1])]->endind, allVCG[VCGexists(b[0], b[1])]->endind);

	if (a[1].find(',') == a[1].npos)
	{
		mergedVCG.push_back(allVCG[VCGexists(a[0], a[1])]);
	}
	if (b[1].find(',') == b[1].npos)
	{
		mergedVCG.push_back(allVCG[VCGexists(b[0], b[1])]);
	}
#pragma endregion

#pragma region Removal of desc and pred that contain a or b in new VCG
	allVCG.erase(remove(allVCG.begin(), allVCG.end(), allVCG[VCGexists(a[0], a[1])]), allVCG.end());
	allVCG.erase(remove(allVCG.begin(), allVCG.end(), allVCG[VCGexists(b[0], b[1])]), allVCG.end());
	allVCG.push_back(combine);

	int d;
	while ((d = findDesc(combine->decendents.begin(), a[0], a[1], allVCG.size() - 1)) != -1) {
		combine->decendents.erase(combine->decendents.begin() + d);
		combine->dogdesc.erase(combine->dogdesc.begin() + d);
	}
	while ((d = findDesc(combine->decendents.begin(), b[0], b[1], allVCG.size() - 1)) != -1) {
		combine->decendents.erase(combine->decendents.begin() + d);
		combine->dogdesc.erase(combine->dogdesc.begin() + d);
	}
	while ((d = findPred(combine->predecessors.begin(), a[0], a[1], allVCG.size() - 1)) != -1) {
		combine->predecessors.erase(combine->predecessors.begin() + d);
		combine->dogpred.erase(combine->dogpred.begin() + d);
	}
	while ((d = findPred(combine->predecessors.begin(), b[0], b[1], allVCG.size() - 1)) != -1) {
		combine->predecessors.erase(combine->predecessors.begin() + d);
		combine->dogpred.erase(combine->dogpred.begin() + d);
	}
#pragma endregion

}

void updateZones(vector<string> a, vector<string> b) {
	int ind, ind1, ind2, i1, i2;
	for (size_t i = 0; i < ininet.size(); i++) {
		if ((ind = findininet(ininet[i].begin(), a[0], a[1], i)) != -1)
		{
			ind1 = ind;
			ininet[i].erase(ininet[i].begin() + ind1);
			inidog[i].erase(inidog[i].begin() + ind1);
			i1 = i;
		}
		if ((ind = findininet(ininet[i].begin(), b[0], b[1], i)) != -1)
		{
			ind2 = ind;
			ininet[i].erase(ininet[i].begin() + ind2);
			inidog[i].erase(inidog[i].begin() + ind2);
			i2 = i;
		}
	}

	if (i1 > i2) {
		ininet[i2].insert(ininet[i2].begin() + ind2, a[0] + "," + b[0]);
		inidog[i2].insert(inidog[i2].begin() + ind2, (a[1].empty() ? " " : a[1]) + "," + (b[1].empty() ? " " : b[1]));
	}
	else {
		ininet[i1].insert(ininet[i1].begin() + ind1, a[0] + "," + b[0]);
		inidog[i1].insert(inidog[i1].begin() + ind1, (a[1].empty() ? " " : a[1]) + "," + (b[1].empty() ? " " : b[1]));
	}

	for (size_t i = 0; i < finalnet.size(); i++) {
		if ((ind = findfinalnet(finalnet[i].begin(), a[0], a[1], i)) != -1)
		{
			ind1 = ind;
			finalnet[i].erase(finalnet[i].begin() + ind1);
			finaldog[i].erase(finaldog[i].begin() + ind1);
			i1 = i;

		}
		if ((ind = findfinalnet(finalnet[i].begin(), b[0], b[1], i)) != -1)
		{
			ind2 = ind;
			finalnet[i].erase(finalnet[i].begin() + ind2);
			finaldog[i].erase(finaldog[i].begin() + ind2);
			i2 = i;
		}
	}

	if (i1 < i2) {
		finalnet[i2].insert(finalnet[i2].begin() + ind2, a[0] + "," + b[0]);
		finaldog[i2].insert(finaldog[i2].begin() + ind2, (a[1].empty() ? " " : a[1]) + "," + (b[1].empty() ? " " : b[1]));
	}
	else {
		finalnet[i1].insert(finalnet[i1].begin() + ind1, a[0] + "," + b[0]);
		finaldog[i1].insert(finaldog[i1].begin() + ind1, (a[1].empty() ? " " : a[1]) + "," + (b[1].empty() ? " " : b[1]));
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

	vector< vector<string> > col;
	for (size_t i1 = 0; i1 < tops.size(); i1++) {
		vector<string> row; // Create an empty row
		for (size_t j1 = 0; j1 < allVCG.size(); j1++) {
			if (i1 <= allVCG[j1]->endind && i1 >= allVCG[j1]->startind) {
				row.push_back(allVCG[j1]->netid + allVCG[j1]->dogid); // Add an element (column) to the row
			}
		}
		sort(row.begin(), row.end()); // Sort strings 
		col.push_back(row); // Add the row to the main vector
	}

	int p = 0;
	zone.push_back(col[0]);

	for (int k1 = 0; k1 < static_cast<int>(tops.size()) - 1; k1++) {

		if ((includes(col[k1].begin(), col[k1].end(), col[k1 + 1].begin(), col[k1 + 1].end())) || (includes(col[k1 + 1].begin(), col[k1 + 1].end(), col[k1].begin(), col[k1].end()))) {
			//"Subset";
			std::vector<string> temp_colunion(col[k1].size() + col[k1 + 1].size());
			std::vector<string>::iterator it;
			it = set_union(col[k1].begin(), col[k1].end(), col[k1 + 1].begin(), col[k1 + 1].end(), temp_colunion.begin());
			temp_colunion.resize(it - temp_colunion.begin());

			if (includes(temp_colunion.begin(), temp_colunion.end(), zone[p].begin(), zone[p].end())) {
				zone[p] = temp_colunion;

			}

		}
		else {
			//"not a subset";

			p += 1;
			zone.push_back(col[k1 + 1]);
		}

	}
	zone.erase(zone.begin() + p + 1, zone.end());

	if (!suppressFlag)
	{
		for (int k1 = 0; k1 < static_cast<int>(zone.size()); k1++) {

			cout << endl;
			cout << "Zone no" << (k1 + 1) << endl;
			for (int k2 = 0; k2 < static_cast<int>(zone[k1].size()); k2++) {
				cout << " " << zone[k1][k2];
			}
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
		vector<string> temp_zone_diff(zone[l1].size() + zone[l1 + 1].size());

		it1 = set_difference(zone[l1].begin(), zone[l1].end(), zone[l1 + 1].begin(), zone[l1 + 1].end(), temp_zone_diff.begin());
		temp_zone_diff.resize(it1 - temp_zone_diff.begin());

		final_zone[l1] = temp_zone_diff;

		vector<string> temp_zone_diff1(zone[l1].size() + zone[l1 + 1].size());

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

void convertToNetDog() {

	for (size_t i = 0; i < ini_zone.size(); i++) {
		vector<string> done1, done2;
		for (size_t j = 0; j < ini_zone[i].size(); j++) {
			vector<string> id;
			id = separateTrack(ini_zone[i][j]);
			done1.push_back(id[0]);
			done2.push_back(id[1]);
		}
		ininet.push_back(done1);
		inidog.push_back(done2);
	}

	for (size_t i = 0; i < final_zone.size(); i++) {
		vector<string> done1, done2;
		for (size_t j = 0; j < final_zone[i].size(); j++) {
			vector<string> id;
			id = separateTrack(final_zone[i][j]);
			done1.push_back(id[0]);
			done2.push_back(id[1]);
		}
		finalnet.push_back(done1);
		finaldog.push_back(done2);
	}
}
#pragma endregion


///////////////////////////////////////////////////////////////////////
//                         VCG construction							 //
///////////////////////////////////////////////////////////////////////

#pragma region VCG
				//Constructs the original VCG graph
void makeVCG() {
	if (!suppressFlag)
	{
		cout << "\n--------------------------BEGIN VCG CONSTRUCTION--------------------------";
	}
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
		if (VCGexists(to_string(netlist[i]->netnum), "") == -1) {
			VCG *next = new VCG();

			next->netid = to_string(netlist[i]->netnum);
			next->dogid = "";
			next->indexes = netlist[i]->indexes;
			next->directions = netlist[i]->directions;

			allVCG.push_back(next);
		}
	}

	for (size_t i = 0; i < netlist.size(); i++) {
		allVCG[VCGexists(to_string(netlist[i]->netnum), "")]->indexes = netlist[i]->indexes;
		allVCG[VCGexists(to_string(netlist[i]->netnum), "")]->directions = netlist[i]->directions;
		allVCG[VCGexists(to_string(netlist[i]->netnum), "")]->startind = netlist[i]->startind;
		allVCG[VCGexists(to_string(netlist[i]->netnum), "")]->endind = netlist[i]->endind;
	}

	sourceAndSink();
	if (!suppressFlag)
	{
		cout << "\nInitial VCG Construction done...";
	}
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

int mergedVCGexists(string netid, string dogid) {
	for (size_t i = 0; i < mergedVCG.size(); i++) {
		if (mergedVCG[i]->netid == netid && mergedVCG[i]->dogid == dogid) {
			return i;
		}
	}

	return -1;
}

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

bool findDistance() {
	if (!suppressFlag)
	{
		cout << "\nBegin pathfinding algoritm...\n";
	}
	for (int i = 0; i < (int)source.size(); i++) {
		vector<string> p = vector<string>();
		vector<string> f = vector<string>();
		p.push_back(source[i]->netid);
		f.push_back(source[i]->dogid);
		if (!distFromSource(source[i]->netid, source[i]->dogid, 0, p, f)) {
			if (!merging)
			{
				sourceAndSink();
				i = -1;
			}
			else {
				return false;
			}
		}
	}

	for (size_t i = 0; i < sink.size(); i++) {
		distFromSink(sink[i]->netid, sink[i]->dogid, 0);
	}
	if (!suppressFlag)
	{
		cout << "\nPathfinding finished...\n";
	}
}

int findPath(vector<string> path, vector<string> dogpath, string desc, string dogdesc) {
	for (size_t i = 0; i < path.size(); i++) {
		if (path[i] == desc && dogpath[i] == dogdesc) {
			return i;
		}
	}
	return -1;
}

bool distFromSource(string netid, string dogid, int counter, vector<string> path, vector<string> dogpath) {
	bool flag = true;
	if (counter > allVCG[VCGexists(netid, dogid)]->distanceToSource) {
		allVCG[VCGexists(netid, dogid)]->distanceToSource = counter;
	}

	int index;
	for (size_t i = 0; i < allVCG[(index = VCGexists(netid, dogid))]->decendents.size(); i++) {
		int index;
		if ((index = findPath(path, dogpath, allVCG[(index = VCGexists(netid, dogid))]->decendents[i], allVCG[(index = VCGexists(netid, dogid))]->dogdesc[i])) != -1) {
			if (merging) {
				cout << "ERROR: Cycle created by merging";
				exit(-2);
			}
			if (!suppressFlag)
			{
				cout << "\nCycle detected: begin +1 doglegging...\n";
			}
			path.erase(path.begin(), path.begin() + index);
			dogpath.erase(dogpath.begin(), dogpath.begin() + index);
			dogleg(path, dogpath);
			flag = false;
			break;
		}
		path.push_back(allVCG[VCGexists(netid, dogid)]->decendents[i]);
		dogpath.push_back(allVCG[VCGexists(netid, dogid)]->dogdesc[i]);
		if (allVCG[VCGexists(netid, dogid)]->decendents.size() > 0) {
			if (!distFromSource(allVCG[VCGexists(netid, dogid)]->decendents[i], allVCG[VCGexists(netid, dogid)]->dogdesc[i], counter + 1, path, dogpath))
			{
				return false;
			}
			path.pop_back();
			dogpath.pop_back();
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
		predpath->push_back(netid + dogid);
		distFromSink(allVCG[VCGexists(netid, dogid)]->predecessors[i], allVCG[VCGexists(netid, dogid)]->dogpred[i], counter + 1);
		if (allVCG[VCGexists(netid, dogid)]->predecessors.size() > 0)
		{
			predpath->pop_back();
		}
	}
}

int getNetlistInd(int index) {
	for (size_t i = 0; i < netlist.size(); i++) {
		if (index == netlist[i]->netnum) {
			return i;
		}
	}
}

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
		if (previndex < line.length())
		{
			top.push_back(stoi(line.substr(previndex, index - previndex)));
		}

		previndex = index + 1;
	}

	index = 0;
	previndex = 0;
	getline(file, line);
	while (index < line.length()) {
		index = line.find(" ", (size_t)previndex);
		if (previndex < line.length())
		{
			bot.push_back(stoi(line.substr(previndex, index - previndex)));
		}

		previndex = index + 1;
	}
}

//converts the input into a useable netlist using net structs
void arraytonet() {
	int previndex;
	//top part of input
	for (int i = 0; i < top.size(); i++) {
		net *next = new net, *next1 = new net;
		int nextnet = top.at(i);
		int net = findExistingNet(nextnet);

		if (nextnet == 0) {
			goto label;
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

	label:
		nextnet = bot.at(i);
		net = findExistingNet(nextnet);

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
			next1->startind = i;
			next1->directions.push_back(false);
			next1->endind = i;
			next1->indexes.push_back(i);
			next1->netnum = nextnet;
			netlist.push_back(next1);
		}
	}

	sort(netlist.begin(), netlist.end(), sortNet);
	cout << "Parsing done.\nNetlist stats:\nNetlist size: " + to_string(netlist.size()) + "\nTrack length: " + to_string(top.size()) + "\n";
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
//							DOGLEGGING								 //
///////////////////////////////////////////////////////////////////////
#pragma region Doglegging

void dogleg(vector<string> path, vector<string> dogpath) {
	int index = -1;
	bool direction;
	bool direcflag = false;
	int count = 0;

	//find index at which cycle begins
trynewdog:


	if (count > path.size() - 2) {
		cout << "WOOF WOOF, Cyclic path cannot be resolved:\n " + path[0] + dogpath[0] + " and " + path[1] + dogpath[1] + " cannot be doglegged because they have no space";
		exit(-1);
	}
	VCG *hold = allVCG[VCGexists(path.at(count), dogpath.at(count))];
	VCG *under = allVCG[VCGexists(path.at(count + 1), dogpath.at(count + 1))];
	VCG *above = allVCG[VCGexists(path.at(count >= 1 ? count - 1 : path.size() - 1), dogpath.at(count >= 1 ? count - 1 : path.size() - 1))];


	for (size_t j = 0; j < hold->directions.size(); j++) {
		if (hold->directions[j]) {
			//check  for dogid

			if (bots[hold->indexes[j]] == (under->netid + under->dogid) && tops[j].compare(bots[j])) {
				index = hold->indexes[j];
				direction = true;
				direcflag = true;
				break;
			}

		}
		if (!hold->directions[j]) {
			//check for dogid
			if (tops[hold->indexes[j]] == (above->netid + above->dogid) && tops[j].compare(bots[j])) {
				index = hold->indexes[j];
				direction = false;
				direcflag = true;
				break;
			}
		}
	}

	if (!direcflag) {
		count++;
		goto trynewdog;
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
			if (offender == top[i] || !tops[i].compare(bots[i])) {
				offender = bot[i];
				continue;
			}
			dogindex = i;
			break;
		}
		else {
			if (offender == bot[i] || !tops[i].compare(bots[i])) {
				offender = top[i];
				continue;
			}
			dogindex = i;
			break;
		}
	}
	// Dont dogleg if end index of net to be doglegged is the dogindex
	if (dogindex == allVCG[VCGexists(path.at(count), dogpath.at(count))]->endind) {
		count++;
		goto trynewdog;
	}
	if (!suppressFlag)
	{
		cout << "+1 doglegging at index: " + to_string(dogindex) + " netid: " + hold->netid + hold->dogid + "...\n";
	}
	updateVCGDog(index, dogindex, hold);
	if (!suppressFlag)
	{
		cout << "Dogleg successful. Restarting pathfinding algorithm\n";
	}
}

#pragma region dogleg all code
void doglegAllHelper(int netindex) {

	vector<string> topVec, botVec;

#pragma region determine if double or single dog
	int holdind = 0, ind = 0;
	string line1 = tops[netindex], line2 = bots[netindex];

	bool flag1 = false, flag2 = false;
	for (char c : line1) {
		if (c == '0' && holdind == 0) {
			return;
		}
		if (c >= 65) {
			if (flag1)
			{
				flag2 = true;
				break;
			}
			flag1 = true;
		}
		if (!flag1)
		{
			holdind++;
		}
	}

	if (flag2) {
		topVec.push_back(line1.substr(0, holdind));
		if (topVec[0].compare("0"))
		{
			topVec.push_back(line1.substr(holdind, 1));
			topVec.push_back(line1.substr(holdind + 1, 1));
		}
	}
	else {
		topVec.push_back(line1.substr(0, holdind));
		if (topVec[0].compare("0"))
		{
			topVec.push_back(line1.substr(holdind, 1));
		}
	}

	holdind = 0;
	flag1 = false;
	flag2 = false;

	for (char c : line2) {
		if (c == '0' && holdind == 0) {
			return;
		}
		if (c >= 65) {
			if (flag1)
			{
				flag2 = true;
				break;
			}
			flag1 = true;
		}
		if (!flag1)
		{
			holdind++;
		}
	}

	if (flag2) {
		botVec.push_back(line2.substr(0, holdind));
		botVec.push_back(line2.substr(holdind, 1));
		botVec.push_back(line2.substr(holdind + 1, 1));
	}
	else {
		botVec.push_back(line2.substr(0, holdind));
		botVec.push_back(line2.substr(holdind, 1));
	}
#pragma endregion

	//get assoc VCG objects
	VCG *top1 = nullptr, *top2 = nullptr, *bot1 = nullptr, *bot2 = nullptr;

	if (topVec.size() == 2) {
		top1 = allVCG[VCGexists(topVec[0], topVec[1])];
	}
	else if (topVec.size() == 3) {
		top1 = allVCG[VCGexists(topVec[0], topVec[1])];
		top2 = allVCG[VCGexists(topVec[0], topVec[2])];
	}

	if (botVec.size() == 2) {
		bot1 = allVCG[VCGexists(botVec[0], botVec[1])];
	}
	else if (botVec.size() == 3) {
		bot1 = allVCG[VCGexists(botVec[0], botVec[1])];
		bot2 = allVCG[VCGexists(botVec[0], botVec[2])];
	}

#pragma region Desc and Pred

	if (bot1 != nullptr && top1 != nullptr && ((bot1->netid != top1->netid)? true:(top1->dogid != bot1->dogid)))
	{
		top1->decendents.push_back(bot1->netid);
		top1->dogdesc.push_back(bot1->dogid);
		bot1->predecessors.push_back(top1->netid);
		bot1->dogpred.push_back(top1->dogid);
	}

	if (top2 != nullptr && bot2 != nullptr) {
		if ((top2->netid != bot1->netid)?true:(top2->dogid != bot1->dogid))
		{
			bot1->predecessors.push_back(top2->netid);
			bot1->dogpred.push_back(top2->dogid);
			top2->decendents.push_back(bot1->netid);
			top2->dogdesc.push_back(bot1->dogid);
		}
		if ((bot2->netid != top1->netid)? true:(bot2->dogid != top1->dogid))
		{
			bot2->predecessors.push_back(top1->netid);
			bot2->dogpred.push_back(top1->dogid);
			top1->decendents.push_back(bot2->netid);
			top1->dogdesc.push_back(bot2->dogid);
		}
		if ((top2->netid != bot2->netid)? true:(top2->dogid != bot2->dogid))
		{
			top2->decendents.push_back(bot2->netid);
			top2->dogdesc.push_back(bot2->dogid);
			bot2->predecessors.push_back(top2->netid);
			bot2->dogpred.push_back(top2->dogid);
		}
	}
	else if (top2 != nullptr && ((top2->netid != bot1->netid)? true:(top2->dogid != bot1->dogid))) {
		bot1->predecessors.push_back(top2->netid);
		bot1->dogpred.push_back(top2->dogid);
		top2->decendents.push_back(bot1->netid);
		top2->dogdesc.push_back(bot1->dogid);
	}
	else if (bot2 != nullptr && ((bot2->netid != top1->netid)? true:(bot2->dogid != top1->dogid))) {
		top1->decendents.push_back(bot2->netid);
		top1->dogdesc.push_back(bot2->dogid);
		bot2->predecessors.push_back(top1->netid);
		bot2->dogpred.push_back(top1->dogid);
	}
#pragma endregion
}

void doglegAll() {
	size_t end = allVCG.size();
	int counter = 0;
	for (size_t i = 0; i < end; i++) {
		VCG *hold = allVCG[counter];
		if (hold->dogid == "")
		{
			for (size_t j = 1; j < hold->indexes.size(); j++) {
				VCG *next = new VCG();
				next->netid = hold->netid;
				stringstream ss;
				ss << (char)('A' + j - 1);
				ss >> next->dogid;

				next->indexes.push_back(hold->indexes[j - 1]);
				next->indexes.push_back(hold->indexes[j]);

				next->directions.push_back(hold->directions[j - 1]);
				next->directions.push_back(hold->directions[j]);

				next->startind = min(next->indexes[0], next->indexes[1]);
				next->endind = max(next->indexes[0], next->indexes[1]);

				if (hold->directions[j]) {
					tops[hold->indexes[j]] += next->dogid;
				}
				else {
					bots[hold->indexes[j]] += (next->dogid);
				}

				if (hold->directions[j - 1]) {
					tops[hold->indexes[j - 1]] += next->dogid;
				}
				else {
					bots[hold->indexes[j - 1]] += (next->dogid);
				}
				allVCG.push_back(next);
			}

			allVCG.erase(remove(allVCG.begin(), allVCG.end(), hold), allVCG.end());
		}
		else {
			dogcounter++;
			counter++;
		}
	}

	createDoglegVCG();
	doglegAlldone = true;
}

vector<string> getVec(string line) {
	vector<string> vec;
	int holdind = 0;
	bool flag1 = false, flag2 = false;
	for (char c : line) {
		if (c == '0' && holdind == 0) {
			holdind++;
			break;
		}
		if (c >= 65) {
			if (flag1)
			{
				flag2 = true;
				break;
			}
			flag1 = true;
		}
		if (!flag1)
		{
			holdind++;
		}
	}

	if (flag2) {
		vec.push_back(line.substr(0, holdind));
		if (vec[0].compare("0"))
		{
			vec.push_back(line.substr(holdind, 1));
			vec.push_back(line.substr(holdind + 1, 1));
		}
	}
	else {
		vec.push_back(line.substr(0, holdind));
		if (vec[0].compare("0"))
		{
			vec.push_back(line.substr(holdind, 1));
		}
	}

	return vec;
}

void createDoglegVCG() {
	for (size_t i = 0; i < tops.size(); i++) {
		doglegAllHelper(i);
	}

	for (int i = 0; i < dogcounter; i++)
	{
		VCG *hold = allVCG[i];
		hold->decendents.clear();
		hold->dogdesc.clear();
		hold->predecessors.clear();
		hold->dogpred.clear();

		for (size_t j = 0; j < hold->indexes.size(); j++)
		{

			int holdind = 0, ind = 0;
			int netindex = hold->indexes[j];
			vector<string> topVec = getVec(tops[netindex]), botVec = getVec(bots[netindex]);

			if (j >= hold->directions.size()) {


				if (botVec.size() > 1)
				{
					hold->decendents.push_back(botVec[0]);
					hold->dogdesc.push_back(botVec[1]);
					if (botVec.size() == 3) {
						hold->decendents.push_back(botVec[0]);
						hold->dogdesc.push_back(botVec[2]);
					}
				}
				if (topVec.size() > 1)
				{
					hold->predecessors.push_back(topVec[0]);
					hold->dogpred.push_back(topVec[1]);
					if (topVec.size() == 3) {
						hold->predecessors.push_back(topVec[0]);
						hold->dogpred.push_back(topVec[2]);
					}
				}

				vector<string> vec = getVec(tops[hold->indexes[j]]);

				if (vec.size() > 0) {
					if (vec.size() == 2) {
						int allind = VCGexists(vec[0], vec[1]);

						allVCG[allind]->decendents.push_back(hold->netid);
						allVCG[allind]->dogdesc.push_back(hold->dogid);
					}
					else if (vec.size() == 3) {
						int allind = VCGexists(vec[0], vec[1]);

						allVCG[allind]->decendents.push_back(hold->netid);
						allVCG[allind]->dogdesc.push_back(hold->dogid);

						allind = VCGexists(vec[0], vec[2]);

						allVCG[allind]->decendents.push_back(hold->netid);
						allVCG[allind]->dogdesc.push_back(hold->dogid);
					}
				}

				vec = getVec(bots[hold->indexes[j]]);

				if (vec.size() > 0) {
					if (vec.size() == 2) {
						int allind = VCGexists(vec[0], vec[1]);

						allVCG[allind]->predecessors.push_back(hold->netid);
						allVCG[allind]->dogpred.push_back(hold->dogid);
					}
					else if (vec.size() == 3) {
						int allind = VCGexists(vec[0], vec[1]);

						allVCG[allind]->predecessors.push_back(hold->netid);
						allVCG[allind]->dogpred.push_back(hold->dogid);

						allind = VCGexists(vec[0], vec[2]);

						allVCG[allind]->predecessors.push_back(hold->netid);
						allVCG[allind]->dogpred.push_back(hold->dogid);
					}
				}
				continue;
			}

			if (hold->directions[j]) {
				if (botVec.size() > 1)
				{
					VCG *other = allVCG[VCGexists(botVec[0], botVec[1])];
					other->predecessors.push_back(hold->netid);
					other->dogpred.push_back(hold->dogid);
					hold->decendents.push_back(botVec[0]);
					hold->dogdesc.push_back(botVec[1]);
					if (botVec.size() == 3) {
						other = allVCG[VCGexists(botVec[0], botVec[2])];
						other->predecessors.push_back(hold->netid);
						other->dogpred.push_back(hold->dogid);
						hold->decendents.push_back(botVec[0]);
						hold->dogdesc.push_back(botVec[2]);
					}



				}
			}
			else {
				if (topVec.size() > 1)
				{
					VCG *other = allVCG[VCGexists(topVec[0], topVec[1])];
					other->decendents.push_back(hold->netid);
					other->dogdesc.push_back(hold->dogid);
					hold->predecessors.push_back(topVec[0]);
					hold->dogpred.push_back(topVec[1]);
					if (topVec.size() == 3) {
						other = allVCG[VCGexists(topVec[0], topVec[2])];
						other->decendents.push_back(hold->netid);
						other->dogdesc.push_back(hold->dogid);
						hold->predecessors.push_back(topVec[0]);
						hold->dogpred.push_back(topVec[2]);
					}
				}
			}


		}
	}
}
#pragma endregion


#pragma region +1 Helper Methods
int VCGexistsDog(string id) {
	int i = 0;
	bool flag = false;
	for (char c : id) {
		if (c >= 65) {
			flag = true;
			break;
		}
		i++;
	}

	string id1 = id.substr(0, i);
	if (flag)
	{
		string id2 = id.substr(i);
	}

	return VCGexists(id.substr(0, i), flag ? id.substr(i) : "");
}

int findPred(vector<string>::iterator it, string net, string dog, int index) {
	vector<string>::iterator it2;
	it2 = find(allVCG[index]->predecessors.begin() + (it - allVCG[index]->predecessors.begin()), allVCG[index]->predecessors.end(), net);
	if (it2 == allVCG[index]->predecessors.end()) {
		return -1;
	}
	int ind = it2 - allVCG[index]->predecessors.begin();
	if (allVCG[index]->dogpred[ind] == dog) {
		return ind;
	}
	else {
		return findPred(it2 + 1, net, dog, index);
	}
}

int findDesc(vector<string>::iterator it, string net, string dog, int index) {
	vector<string>::iterator it2;
	it2 = find(allVCG[index]->decendents.begin() + (it - allVCG[index]->decendents.begin()), allVCG[index]->decendents.end(), net);
	if (it2 == allVCG[index]->decendents.end()) {
		return -1;
	}
	int ind = it2 - allVCG[index]->decendents.begin();
	if (allVCG[index]->dogdesc[ind] == dog) {
		return ind;
	}
	else {
		return findDesc(it2 + 1, net, dog, index);
	}
}

vector<string> separateTrack(int index, bool track) {
	vector<string> ret;
	int i = 0;
	bool flag = false;
	for (char c : track ? tops[index] : bots[index]) {
		if (c >= 65) {
			flag = true;
			break;
		}
		i++;
	}

	if (flag) {
		ret.push_back(track ? tops[index].substr(0, i) : bots[index].substr(0, i));
		ret.push_back(track ? tops[index].substr(i) : bots[index].substr(i));
	}
	else {
		ret.push_back(track ? tops[index].substr(0, i) : bots[index].substr(0, i));
		ret.push_back("");
	}
	return ret;
}

vector<string> separateTrack(string id) {
	vector<string> ret;
	int i = 0;
	bool flag = false;
	for (char c : id) {
		if (c >= 65) {
			flag = true;
			break;
		}
		i++;
	}

	ret.push_back(id.substr(0, i));

	if (flag)
	{
		ret.push_back(id.substr(i));
	}
	else {
		ret.push_back("");
	}


	return ret;
}
#pragma endregion

void lexiDog(string netid, string dogid) {
	vector<VCG*> ret;

	//Nets to be updated after dogleg
	for (size_t i = 0; i < allVCG.size(); i++) {
		if (allVCG[i]->netid == netid && allVCG[i]->dogid > dogid) {
			ret.push_back(allVCG[i]);
		}
	}

	for (size_t i = 0; i < tops.size(); i++) {

		if (VCGexistsDog(tops[i]) != -1)
		{ // Change all dogleg names after the net which is being doglegged 
			if ((allVCG[VCGexistsDog(tops[i])]->netid == netid) && (allVCG[VCGexistsDog(tops[i])]->dogid > dogid)) {
				vector<string> id = separateTrack(i, true);
				stringstream ss;
				ss << (char)(id[1][0] + 1);
				string s;
				ss >> s;
				tops[i] = netid + s;
			}
		}
		if (VCGexistsDog(bots[i]) != -1) {
			if (allVCG[VCGexistsDog(bots[i])]->netid == netid && allVCG[VCGexistsDog(bots[i])]->dogid > dogid) {
				vector<string> id = separateTrack(i, false);
				stringstream ss;
				ss << (char)(id[1][0] + 1);
				string s;
				ss >> s;
				bots[i] = netid + s;
			}
		}
	}
	// Change predescessors and descendents dogids after net to be doglegged
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
				predind++;
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
				descind++;
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
	bool skip = false;

	if (rem->dogid != "") {
		lexiDog(rem->netid, rem->dogid);
	}

	//remove every reference of doglegged thing
	for (size_t i = 0; i < allVCG.size(); i++) {
		int predind;
		int descind;

		while ((descind = findDesc(allVCG[i]->decendents.begin(), rem->netid, rem->dogid, i)) != -1)
		{
			allVCG[i]->decendents.erase(allVCG[i]->decendents.begin() + descind);
			allVCG[i]->dogdesc.erase(allVCG[i]->dogdesc.begin() + descind);
		}

		while ((predind = findPred(allVCG[i]->predecessors.begin(), rem->netid, rem->dogid, i)) != -1)
		{
			allVCG[i]->predecessors.erase(allVCG[i]->predecessors.begin() + predind);
			allVCG[i]->dogpred.erase(allVCG[i]->dogpred.begin() + predind);
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
			id = separateTrack(i, false);

			if (id[0] != "0" && id[0].compare(rem->netid))
			{
				a->decendents.push_back(id[0]);//separate bots
				a->dogdesc.push_back(id[1]);
			}

			a->indexes.push_back(i);
			a->directions.push_back(true);
			if (bots[i] != tops[i])
			{
				allVCG[VCGexistsDog(bots[i])]->predecessors.push_back(a->netid);
				allVCG[VCGexistsDog(bots[i])]->dogpred.push_back(a->dogid);
			}


			tops[i] = a->netid + s;

			// update tops
		}
		if (!bots[i].compare(net)) {
			id = separateTrack(i, true);
			if (id[0] != "0" && id[0].compare(rem->netid))
			{
				a->predecessors.push_back(id[0]);
				a->dogpred.push_back(id[1]);
			}
			a->indexes.push_back(i);
			a->directions.push_back(false);

			allVCG[VCGexistsDog(tops[i])]->decendents.push_back(a->netid);
			allVCG[VCGexistsDog(tops[i])]->dogdesc.push_back(a->dogid);

			bots[i] = a->netid + s; //update bots
		}
		if (skip) {
			tops[i] = a->netid + s;
			skip = false;
		}
	}

	skip = false;

	stringstream ss1;
	ss1 << (char)('A' + rem->dogcount + 1);
	s = "";
	ss1 >> s;
	b->netid = rem->netid;
	b->dogid = s;
	b->dogcount = rem->dogcount + 1; // change for double dogleg when necessary

	for (size_t i = dogind; i < tops.size(); i++) {
		if (!tops[i].compare(net)) {
			id = separateTrack(i, false);

			if (id[0] != "0" && id[0].compare(rem->netid))
			{
				b->decendents.push_back(id[0]);
				b->dogdesc.push_back(id[1]);
			}
			b->indexes.push_back(i);
			b->directions.push_back(true);
			if (bots[i] != "0")
			{
				allVCG[VCGexistsDog(bots[i])]->predecessors.push_back(b->netid);
				allVCG[VCGexistsDog(bots[i])]->dogpred.push_back(b->dogid);
			}

			if (tops[i].compare(bots[i]))
			{
				tops[i] = b->netid + s;
			}
			else {
				skip = true;
			}//update tops
		}
		if (!bots[i].compare(net)) {
			id = separateTrack(i, true);
			if (id[0] != "0" && id[0].compare(rem->netid))
			{
				b->predecessors.push_back(id[0]);
				b->dogpred.push_back(id[1]);
			}
			b->indexes.push_back(i);
			b->directions.push_back(false);
			if (tops[i] != "0") {
				allVCG[VCGexistsDog(tops[i])]->decendents.push_back(b->netid);
				allVCG[VCGexistsDog(tops[i])]->dogdesc.push_back(b->dogid);
			}
			bots[i] = b->netid + s; //update bots
		}

		if (skip) {
			tops[i] = b->netid + s;
			skip = false;
		}
	}
	id = separateTrack(dogind, false);
	if (id[0].compare("0") && tops[dogind] != bots[dogind])
	{
		allVCG[VCGexists(id[0], id[1])]->predecessors.push_back(a->netid);
		allVCG[VCGexists(id[0], id[1])]->predecessors.push_back(b->netid);
		allVCG[VCGexists(id[0], id[1])]->dogpred.push_back(a->dogid);
		allVCG[VCGexists(id[0], id[1])]->dogpred.push_back(b->dogid);
		a->decendents.push_back(id[0]);
		a->dogdesc.push_back(id[1]);
		b->decendents.push_back(id[0]);
		b->dogdesc.push_back(id[1]);
	}

	id = separateTrack(dogind, true);
	if (id[0].compare("0") && tops[dogind] != bots[dogind])
	{
		allVCG[VCGexists(id[0], id[1])]->decendents.push_back(a->netid);
		allVCG[VCGexists(id[0], id[1])]->decendents.push_back(b->netid);
		allVCG[VCGexists(id[0], id[1])]->dogdesc.push_back(a->dogid);
		allVCG[VCGexists(id[0], id[1])]->dogdesc.push_back(b->dogid);
		a->predecessors.push_back(id[0]);
		a->dogpred.push_back(id[1]);
		b->predecessors.push_back(id[0]);
		b->dogpred.push_back(id[1]);
	}

	a->startind = rem->startind;
	a->endind = dogind;
	a->indexes.push_back(dogind);
	b->startind = dogind;
	b->endind = rem->endind;
	b->indexes.push_back(dogind);

	allVCG.push_back(a);
	allVCG.push_back(b);
	allVCG.erase(remove(allVCG.begin(), allVCG.end(), rem), allVCG.end());
}

int findIn(vector<string> v1, vector<string> v2, string a, string b) {
	for (size_t i = 0; i < v1.size(); i++) {
		if (v1[i] == a && v2[i] == b) {
			return i;
		}
	}
	return -1;
}
#pragma endregion

///////////////////////////////////////////////////////////////////////
//                             OpenGL								 //
///////////////////////////////////////////////////////////////////////


#pragma region OpenGL
bool sortheighthelper(VCG *dat1, VCG *dat2) {
	return (dat1->distanceToSink) < (dat2->distanceToSink);
}

int netexistsonTrack(string netid, string dogid) {
	vector<string> holdnet, holddog;
	for (size_t i = 0; i < allVCG.size(); i++) {
		stringstream ss(allVCG[i]->netid);
		while (ss.good())
		{
			string substr;
			getline(ss, substr, ',');
			holdnet.push_back(substr);
		}
		stringstream ss1(allVCG[i]->dogid);
		while (ss1.good())
		{
			string substr;
			getline(ss1, substr, ',');
			holddog.push_back(((!substr.compare("")) ? " " : substr));
		}
		for (size_t j = 0; j < holdnet.size(); j++)
		{
			if (holdnet[j] == netid && holddog[j] == ((!dogid.compare("")) ? " " : dogid)) {
				return i;
			}
		}
		holdnet.clear();
		holddog.clear();
	}
	return -1;
}

void updatenetvertex(VCG* net, vector<string> netdog, bool flag)
{
	float x, y, z = 0;
	float maxX = tops.size()*1.2; // to scale between 0 - 1
	float maxY = allVCG.size()*1.2;

	if (flag)
	{

		x = net->startind / maxX;
		y = (netexistsonTrack(netdog[0], netdog[1])) / maxY;
		netvertex.push_back(x);
		netvertex.push_back(y);
		netvertex.push_back(z);
		netsatvertex.push_back(net->netid + net->dogid);

		x = net->endind / maxX;
		y = (netexistsonTrack(netdog[0], netdog[1])) / maxY;
		netvertex.push_back(x);
		netvertex.push_back(y);
		netvertex.push_back(z);
		netsatvertex.push_back(net->netid + net->dogid);

	}
	else {

		x = net->startind / maxX;
		y = (netexistsonTrack(netdog[0], netdog[2])) / maxY;
		netvertex.push_back(x);
		netvertex.push_back(y);
		netvertex.push_back(z);
		netsatvertex.push_back(net->netid + net->dogid);

		x = net->endind / maxX;
		y = (netexistsonTrack(netdog[0], netdog[2])) / maxY;
		netvertex.push_back(x);
		netvertex.push_back(y);
		netvertex.push_back(z);
		netsatvertex.push_back(net->netid + net->dogid);
	}
}

void updateVertnetvertex(VCG* net, vector<string> netdog, bool flag)
{
	float x, y, z = 0;
	float maxX = tops.size()*1.2; // to scale between 0 - 1
	float maxY = allVCG.size()*1.2;

	for (size_t i = 0; i < net->indexes.size(); i++) {

		if (i<net->directions.size())
		{
			if (net->directions[i]) {
				if (flag)
				{
					x = net->indexes[i] / maxX;
					y = (netexistsonTrack(netdog[0], netdog[1])) / maxY;
					netvertex.push_back(x);
					netvertex.push_back(y);
					netvertex.push_back(z);
					netsatvertex.push_back(net->netid + net->dogid);

					y = 0.9;
					netvertex.push_back(x);
					netvertex.push_back(y);
					netvertex.push_back(z);
					netsatvertex.push_back(net->netid + net->dogid);
				}
				else {
					x = net->indexes[i] / maxX;
					y = (netexistsonTrack(netdog[0], netdog[2])) / maxY;
					netvertex.push_back(x);
					netvertex.push_back(y);
					netvertex.push_back(z);
					netsatvertex.push_back(net->netid + net->dogid);

					y = 0.9;
					netvertex.push_back(x);
					netvertex.push_back(y);
					netvertex.push_back(z);
					netsatvertex.push_back(net->netid + net->dogid);

				}

			}

			else {
				if (flag)
				{

					x = net->indexes[i] / maxX;
					y = (netexistsonTrack(netdog[0], netdog[1])) / maxY;
					netvertex.push_back(x);
					netvertex.push_back(y);
					netvertex.push_back(z);
					netsatvertex.push_back(net->netid + net->dogid);

					y = -0.1;
					netvertex.push_back(x);
					netvertex.push_back(y);
					netvertex.push_back(z);
					netsatvertex.push_back(net->netid + net->dogid);
				}
				else {

					x = net->indexes[i] / maxX;
					y = (netexistsonTrack(netdog[0], netdog[2])) / maxY;
					netvertex.push_back(x);
					netvertex.push_back(y);
					netvertex.push_back(z);
					netsatvertex.push_back(net->netid + net->dogid);

					y = -0.1;
					netvertex.push_back(x);
					netvertex.push_back(y);
					netvertex.push_back(z);
					netsatvertex.push_back(net->netid + net->dogid);
				}

			}
		}
		else {
			//find its +1 dogleg partner
			stringstream ss;
			ss << (char)(net->dogid[0] + 1);
			string s;
			ss >> s;
			
			x = net->endind / maxX;
			y = (netexistsonTrack(net->netid, net->dogid)) / maxY;
			netvertex.push_back(x);
			netvertex.push_back(y);
			netvertex.push_back(z);

			y = (netexistsonTrack(net->netid, s)) / maxY;
			netvertex.push_back(x);
			netvertex.push_back(y);
			netvertex.push_back(z);
			netsatvertex.push_back(net->netid + net->dogid + s);
			netsatvertex.push_back(net->netid + net->dogid + s);
			
		}
	}
}


void makedrawvertex() {
	sort(allVCG.begin(), allVCG.end(), sortheighthelper);
	int validtop1, validtop2, validbot1, validbot2;
	vector<string> topVec, botVec;
	for (size_t i = 0; i < tops.size(); i++)
	{
		if (dog)
		{
			topVec = getVec(tops[i]);
			botVec = getVec(bots[i]);
		}
		else
		{
			topVec = separateTrack(i, true);
			botVec = separateTrack(i, false);
		}
		VCG *top1 = nullptr, *top2 = nullptr, *bot1 = nullptr, *bot2 = nullptr;

		if (topVec[0].compare("0")) {
			validtop1 = VCGexists(topVec[0], topVec[1]);

			if (topVec.size() == 2) {
				if (validtop1 != -1)
				{
					top1 = allVCG[VCGexists(topVec[0], topVec[1])];
				}
				else {
					top1 = mergedVCG[mergedVCGexists(topVec[0], topVec[1])];
					validtop1 = mergedVCGexists(topVec[0], topVec[1]);
				}
			}
			else if (topVec.size() == 3) {
				validtop2 = VCGexists(topVec[0], topVec[2]);
				if (validtop1 != -1)
				{
					top1 = allVCG[VCGexists(topVec[0], topVec[1])];
				}
				else {
					top1 = mergedVCG[mergedVCGexists(topVec[0], topVec[1])];
					validtop1 = mergedVCGexists(topVec[0], topVec[1]);
				}

				if (validtop2 != -1)
				{
					top2 = allVCG[VCGexists(topVec[0], topVec[2])];
				}
				else {
					top2 = mergedVCG[mergedVCGexists(topVec[0], topVec[2])];
					validtop2 = mergedVCGexists(topVec[0], topVec[2]);
				}

			}
		}

		if (botVec[0].compare("0")) {
			validbot1 = VCGexists(botVec[0], botVec[1]);

			if (botVec.size() == 2) {
				if (validbot1 != -1)
				{
					bot1 = allVCG[VCGexists(botVec[0], botVec[1])];
				}
				else {
					bot1 = mergedVCG[mergedVCGexists(botVec[0], botVec[1])];
					validbot1 = mergedVCGexists(botVec[0], botVec[1]);
				}
			}
			else if (botVec.size() == 3) {
				validbot2 = VCGexists(botVec[0], botVec[2]);
				if (validbot1 != -1)
				{
					bot1 = allVCG[VCGexists(botVec[0], botVec[1])];
				}
				else {
					bot1 = mergedVCG[mergedVCGexists(botVec[0], botVec[1])];
					validbot1 = mergedVCGexists(botVec[0], botVec[1]);
				}
				if (validbot2 != -1)
				{
					bot2 = allVCG[VCGexists(botVec[0], botVec[2])];
				}
				else {
					bot2 = mergedVCG[mergedVCGexists(botVec[0], botVec[2])];
					validbot2 = mergedVCGexists(botVec[0], botVec[2]);
				}
			}
		}
		if (top1 != nullptr) {
			vector<string>::iterator i = find(netsatvertex.begin(), netsatvertex.end(), top1->netid + top1->dogid);
			//eliminates duplicate vertices
			if (i == netsatvertex.end())
			{
				updatenetvertex(top1, topVec, true);
				updateVertnetvertex(top1, topVec, true);
			}
		}

		if (top2 != nullptr) {
			vector<string>::iterator i = find(netsatvertex.begin(), netsatvertex.end(), top2->netid + top2->dogid);
			//eliminates duplicate vertices
			if (i == netsatvertex.end())
			{
				updatenetvertex(top2, topVec, false);
				updateVertnetvertex(top2, topVec, false);
			}
		}

		if (bot1 != nullptr) {
			vector<string>::iterator i = find(netsatvertex.begin(), netsatvertex.end(), bot1->netid + bot1->dogid);
			//eliminates duplicate vertices
			if (i == netsatvertex.end())
			{
				updatenetvertex(bot1, botVec, true);
				updateVertnetvertex(bot1, botVec, true);
			}

		}
		if (bot2 != nullptr) {
			vector<string>::iterator i = find(netsatvertex.begin(), netsatvertex.end(), bot2->netid + bot2->dogid);
			//eliminates duplicate vertices
			if (i == netsatvertex.end())
			{
				updatenetvertex(bot2, botVec, false);
				updateVertnetvertex(bot2, botVec, false);
			}

		}
	}
}


//settings
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

	int i = netvertex.size() / 3;
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, netvertex.size() * sizeof(float), &netvertex.front(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);


	
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

		// 
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, i);
		

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
	glViewport(0, 0, width, height);
}
#pragma endregion
