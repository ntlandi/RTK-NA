// RTKNA.cpp : Defines the entry point for the console application.
//


#include "RTKheader.hpp"
vector<int> top;
vector<int> bot;
vector<vector<string>> zone;
vector<vector<string>> final_zone;
vector<string> union_zone;
vector<string> union_zone_diff;
vector<vector<string>> ini_zone;
vector<vector<string>> ininet, finalnet, inidog, finaldog;
net * first;
vector<net*> netlist;
vector<VCG*> allVCG, source, sink, mergedVCG;
bool dog, merging;
vector<string> tops, bots, L, Ldog;
vector<int> zoneEnd;


int main(int argc, char* argv[])
{
	merging = false;
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

	if (dog) {
		doglegAll();
	}

	//Zoning Code

	//zone = vector<vector<string>>(static_cast<int>(top.size()), vector<string>(20, "0"));
	Zoning();
	final_zone.resize(static_cast<int>(zone.size()));
	Zone_Sort();
	Zone_union();
	Zone_diff_union();
	convertToNetDog();
	//zoneEnd = zoneEnds();      //Pending fix if needed
 	

	while (Merge() > 0);

	printf("\n\n%d", clock() - start);
	printToFile();
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
			n = g(L, Ldog, (m = f(R, Rdog)));

			if (!n.empty())
			{
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
	double C = 100;
	double highest = 0;
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
	double C = 100;
	double lowest = 100000;
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

void updateVCG(vector<string> a,  vector<string> b) {
	vector<string> newdesc, newpred, newdogdesc, newdogpred;
	

#pragma region desc
	newdesc = allVCG[VCGexists(a[0],a[1])]->decendents;
	newdesc.insert(newdesc.begin(), allVCG[VCGexists(b[0],b[1])]->decendents.begin(), allVCG[VCGexists(b[0],b[1])]->decendents.end());

	newpred = allVCG[VCGexists(a[0],a[1])]->predecessors;
	newpred.insert(newpred.begin(), allVCG[VCGexists(b[0],b[1])]->predecessors.begin(), allVCG[VCGexists(b[0],b[1])]->predecessors.end());

	newdogdesc = allVCG[VCGexists(a[0],a[1])]->dogdesc;
	newdogdesc.insert(newdogdesc.begin(), allVCG[VCGexists(b[0],b[1])]->dogdesc.begin(), allVCG[VCGexists(b[0],b[1])]->dogdesc.end());

	newdogpred = allVCG[VCGexists(a[0],a[1])]->dogpred;
	newdogpred.insert(newdogpred.begin(), allVCG[VCGexists(b[0],b[1])]->dogpred.begin(), allVCG[VCGexists(b[0],b[1])]->dogpred.end());
#pragma endregion

	for (size_t i = 0; i < allVCG.size(); i++) {
		//if any occurance of a in desc
		int descind, predind;
#pragma region Replacement Block
		if (find(allVCG[i]->decendents.begin(), allVCG[i]->decendents.end(), a[0]) != allVCG[i]->decendents.end() && find(allVCG[i]->dogdesc.begin(), allVCG[i]->dogdesc.end(), a[1]) != allVCG[i]->dogdesc.end() && allVCG[i]->decendents.size() > 0)
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

		if (find(allVCG[i]->decendents.begin(), allVCG[i]->decendents.end(), b[0]) != allVCG[i]->decendents.end() && find(allVCG[i]->dogdesc.begin(), allVCG[i]->dogdesc.end(), b[1]) != allVCG[i]->dogdesc.end() && allVCG[i]->decendents.size() > 0)
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

		if (find(allVCG[i]->predecessors.begin(), allVCG[i]->predecessors.end(), a[0]) != allVCG[i]->predecessors.end() && allVCG[i]->predecessors.size() > 0 && find(allVCG[i]->dogpred.begin(), allVCG[i]->dogpred.end(), a[1]) != allVCG[i]->dogpred.end())
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

		if (find(allVCG[i]->predecessors.begin(), allVCG[i]->predecessors.end(), b[0]) != allVCG[i]->predecessors.end() && allVCG[i]->predecessors.size() > 0 && find(allVCG[i]->dogpred.begin(), allVCG[i]->dogpred.end(), b[1]) != allVCG[i]->dogpred.end())
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
		if ((ind = findininet(ininet[i].begin(),  a[0],a[1], i)) != -1)
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
		sort(row.begin(),row.end()); // Sort strings 
		col.push_back(row); // Add the row to the main vector
	}

	int p = 0;
	zone.push_back(col[0]);

	for (int k1 = 0; k1 < static_cast<int>(tops.size()) - 1; k1++) {
		
		if ((includes(col[k1].begin(), col[k1].end(), col[k1 + 1].begin(), col[k1 + 1].end())) || (includes(col[k1 + 1].begin(), col[k1 + 1].end(), col[k1].begin(), col[k1].end()))) {
			//"Subset";
			std::vector<string> temp_colunion(col[k1].size() + col[k1+1].size());
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
	zone.erase(zone.begin() + p+1, zone.end());
	
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
		vector<string> temp_zone_diff(zone[l1].size() + zone[l1+1].size());

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
}

bool distFromSource(string netid, string dogid, int counter, vector<string> path, vector<string> dogpath) {
	bool flag = true;
	if (counter > allVCG[VCGexists(netid, dogid)]->distanceToSource) {
		allVCG[VCGexists(netid, dogid)]->distanceToSource = counter;
	}
	
	for (size_t i = 0; i < allVCG[VCGexists(netid, dogid)]->decendents.size(); i++) {
		if (find(path.begin(), path.end(), allVCG[VCGexists(netid, dogid)]->decendents[i]) != path.end() && find(dogpath.begin(), dogpath.end(), allVCG[VCGexists(netid, dogid)]->dogdesc[i]) != dogpath.end()) {
			if (1)
			{	
				vector<string>::iterator it = find(path.begin(), path.end(), allVCG[VCGexists(netid, dogid)]->decendents[i]);
				int ind = it - path.begin();
				path.erase(path.begin(), find(path.begin(), path.end(), allVCG[VCGexists(netid, dogid)]->decendents[i]));
				dogpath.erase(dogpath.begin(), dogpath.begin()+ind);
				dogleg(path, dogpath);
				flag = false;
				break;
			}
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
		distFromSink(allVCG[VCGexists(netid, dogid)]->predecessors[i], allVCG[VCGexists(netid, dogid)]->dogpred[i], counter + 1);
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


	for (size_t j = 0; j < hold->indexes.size(); j++) {
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
	if (dogindex== allVCG[VCGexists(path.at(count), dogpath.at(count))]->endind) {
		count++;
		goto trynewdog;
	}
	updateVCGDog(index, dogindex, hold);
}

void doglegAll() {
	for (size_t i = 0; i < allVCG.size(); i++) {
		VCG *hold = allVCG[VCGexists(allVCG[i]->netid, allVCG[i]->dogid)];
		if (hold->dogid == "")
		{
			for (size_t j = 1; j < hold->indexes.size(); j++) {
				VCG *next = new VCG();
				next->netid = hold->netid;
				stringstream ss;
				ss << (char)('A' + j - 1);
				ss >> next->dogid;

				next->indexes.push_back(hold->indexes[j]);
				next->directions.push_back(hold->directions[j]);
				
				if (hold->directions[j]) {
					tops[hold->indexes[j]] = (next->netid + next->dogid);
				}
				else {
					bots[hold->indexes[j]] = (next->netid + next->dogid);
				}
				allVCG.push_back(next);
			}

			allVCG.erase(remove(allVCG.begin(), allVCG.end(), hold), allVCG.end());
		}
	}

	createDoglegVCG();
}

void createDoglegVCG() {
	for (size_t i = 0; i < tops.size(); i++) {
		VCG* holdtop = allVCG[VCGexistsDog(tops[i])];
		VCG* holdbot = allVCG[VCGexistsDog(bots[i])];

		if (findDesc(holdtop->decendents.begin(), holdbot->netid, holdbot->dogid, VCGexistsDog(tops[i])) == -1) {
			holdtop->decendents.push_back(holdbot->netid);
			holdtop->dogdesc.push_back(holdbot->dogid);
		}

		if (findPred(holdbot->predecessors.begin(), holdtop->netid, holdtop->dogid, VCGexistsDog(bots[i])) == -1) {
			holdbot->predecessors.push_back(holdtop->netid);
			holdbot->dogpred.push_back(holdtop->dogid);
		}
	}
}

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
		if (VCGexistsDog(bots[i]) != -1){
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
	b->startind = dogind;
	b->endind = rem->endind;

	allVCG.push_back(a);
	allVCG.push_back(b);
	allVCG.erase(remove(allVCG.begin(), allVCG.end(), rem), allVCG.end());
}
#pragma endregion


///////////////////////////////////////////////////////////////////////
//                             Printing								 //
///////////////////////////////////////////////////////////////////////

#pragma region Printing
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

	for (size_t i = 0; i < allVCG.size(); i++) {
		string s = allVCG[i]->netid;

		nets = VCGParse(s);

		for (size_t j = 0; j < nets.size(); j++) {
			netind = getNetlistInd(stoi(nets[j]));

			f << (to_string(netlist[netind]->netnum) + " ");
			f << ("s " + to_string(netlist[netind]->startind) + " ");
			f << ("e " + to_string(netlist[netind]->endind) + " ");
			f << ("i " + vectorToString(netlist[netind]->indexes));
			f << ("d " + vectorToString(netlist[netind]->directions) + " ");
			f << "\n";
		}
		f << "|\n";
	}
	f.close();
}
#pragma endregion


///////////////////////////////////////////////////////////////////////
//                             OpenGL								 //
///////////////////////////////////////////////////////////////////////

#pragma region OpenGL

// settings
//const unsigned int SCR_WIDTH = 800;
//const unsigned int SCR_HEIGHT = 600;
//
//const char *vertexShaderSource = "#version 330 core\n"
//"layout (location = 0) in vec3 aPos;\n"
//"void main()\n"
//"{\n"
//"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
//"}\0";
//const char *fragmentShaderSource = "#version 330 core\n"
//"out vec4 FragColor;\n"
//"void main()\n"
//"{\n"
//"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
//"}\n\0";
//
//int draw(void)
//{
//	// glfw: initialize and configure
//	// ------------------------------
//	glfwInit();
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//	// glfw window creation
//	// --------------------
//	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "YK Routing", NULL, NULL);
//	if (window == NULL)
//	{
//		std::cout << "Failed to create GLFW window" << std::endl;
//		glfwTerminate();
//		return -1;
//	}
//	glfwMakeContextCurrent(window);
//	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//
//	// glad: load all OpenGL function pointers
//	// ---------------------------------------
//	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//	{
//		std::cout << "Failed to initialize GLAD" << std::endl;
//		return -1;
//	}
//
//	// build and compile our shader program
//	// ------------------------------------
//	// vertex shader
//	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
//	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
//	glCompileShader(vertexShader);
//	// check for shader compile errors
//	int success;
//	char infoLog[512];
//	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
//	if (!success)
//	{
//		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
//		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
//	}
//	// fragment shader
//	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
//	glCompileShader(fragmentShader);
//	// check for shader compile errors
//	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
//	if (!success)
//	{
//		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
//		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
//	}
//	// link shaders
//	int shaderProgram = glCreateProgram();
//	glAttachShader(shaderProgram, vertexShader);
//	glAttachShader(shaderProgram, fragmentShader);
//	glLinkProgram(shaderProgram);
//	// check for linking errors
//	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
//	if (!success) {
//		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
//		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
//	}
//	glDeleteShader(vertexShader);
//	glDeleteShader(fragmentShader);
//
//	// set up vertex data (and buffer(s)) and configure vertex attributes
//	// ------------------------------------------------------------------
//	float vertices[] = {
//		-1.0f, -0.521f, 0.0f, // left  
//		0.22f, -0.521f, 0.0f, // right 
//		0.5f, -0.5f, 0.0f,  // top
//		0.5f, 0.5f, 0.0f
//	};
//
//	unsigned int VBO, VAO;
//	glGenVertexArrays(1, &VAO);
//	glGenBuffers(1, &VBO);
//	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
//	glBindVertexArray(VAO);
//
//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//	glEnableVertexAttribArray(0);
//
//	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
//	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
//	glBindVertexArray(0);
//
//
//	// uncomment this call to draw in wireframe polygons.
//	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//
//	// render loop
//	// -----------
//	while (!glfwWindowShouldClose(window))
//	{
//		// input
//		// -----
//		processInput(window);
//
//		// render
//		// ------
//		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//		glClear(GL_COLOR_BUFFER_BIT);
//
//		// draw our first triangle
//		glUseProgram(shaderProgram);
//		glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
//
//		glDrawArrays(GL_LINES, 0, 4);
//		// glBindVertexArray(0); // no need to unbind it every time 
//
//		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
//		// -------------------------------------------------------------------------------
//		glfwSwapBuffers(window);
//		glfwPollEvents();
//	}
//
//	// optional: de-allocate all resources once they've outlived their purpose:
//	// ------------------------------------------------------------------------
//	glDeleteVertexArrays(1, &VAO);
//	glDeleteBuffers(1, &VBO);
//
//	// glfw: terminate, clearing all previously allocated GLFW resources.
//	// ------------------------------------------------------------------
//	glfwTerminate();
//	return 0;
//}
//
//
//// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
//// ---------------------------------------------------------------------------------------------------------
//void processInput(GLFWwindow *window)
//{
//	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//		glfwSetWindowShouldClose(window, true);
//}
//
//// glfw: whenever the window size changed (by OS or user resize) this callback function executes
//// ---------------------------------------------------------------------------------------------
//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//	// make sure the viewport matches the new window dimensions; note that width and 
//	// height will be significantly larger than specified on retina displays.
//	glViewport(0, 0, width, height);
//}
#pragma endregion
