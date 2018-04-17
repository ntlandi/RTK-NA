#pragma once
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
#include <glad/glad.h>
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
	vector<string> decendents, predecessors, dogdesc, dogpred;
	int distanceToSource = 0;
	int distanceToSink = 0;
	vector<int> indexes;
	vector<bool> directions;
	int dogcount = 0;
};

#pragma region methods
//Parsing Methods
void parse(string);
void arraytonet();
int findExistingNet(int);
void trackToString();
void printToFile();


//VCG methods
void makeVCG();
int VCGexists(string, string);
void sourceAndSink();
bool findDistance();
bool distFromSource(string, string, int, vector<string>, vector<string>);
void distFromSink(string, string, int);

//Zoning Methods
void Zoning();
void Zone_Sort();
void Zone_union();
void Zone_diff_union();
vector<string> zoneConvertToStringAll(string);

//Merging methods
int Merge();
vector<string> f(vector<string>, vector<string>);
vector<string> g(vector<string>, vector<string>, vector<string>);
void updateVCG(vector<string> a, vector<string> b);
void updateZones(vector<string>, vector<string>);


bool sortNet(const net *a, const net *b);
//Doglegging and dogleg helper functions
void dogleg(vector<string>, vector<string>);
void updateVCGDog(int, int, VCG*);
int VCGexistsDog(string id);
vector<string> separateTrack(int, bool);
vector<string> separateTrack(string);
int findPred(vector<string>::iterator it, string net, string dog, int);
int findDesc(vector<string>::iterator it, string net, string dog, int);
void convertToNetDog();
void doglegAll();
void createDoglegVCG();
vector<string> getVec(string line);
int findIn(vector<string> v1, vector<string> v2, string a, string b);

//Graphing methods
int draw(void);
void processInput(GLFWwindow *);
void framebuffer_size_callback(GLFWwindow* , int , int);
bool sortheighthelper(VCG *, VCG *);
void makedrawvertex();
int mergedVCGexists(string, string);
int netexistsonTrack(string, string);
void translatearray();
#pragma endregion