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
//#include <glad/glad.h>
//#include <GLFW/glfw3.h>

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
bool findDistance();
bool distFromSource(string, string, int, vector<string>, vector<string>);
void distFromSink(string, string, int);
int Merge();
vector<string> f(vector<string>, vector<string>);
vector<string> g(vector<string>, vector<string>, vector<string>);
void updateVCG(vector<string> a, vector<string> b);
void updateZones(vector<string>, vector<string>);
bool sortNet(const net *a, const net *b);
vector<string> zoneConvertToStringAll(string);
void trackToString();
void printToFile();
void dogleg(vector<string>, vector<string>);
void updateVCGDog(int, int, VCG*);
//void processInput(GLFWwindow *);
//void framebuffer_size_callback(GLFWwindow* , int , int);
int VCGexistsDog(string id);
vector<string> separateTrack(int, bool);
vector<string> separateTrack(string);
int findPred(vector<string>::iterator it, string net, string dog, int);
int findDesc(vector<string>::iterator it, string net, string dog, int);
void convertToNetDog();
void doglegAll();
void createDoglegVCG();
#pragma endregion