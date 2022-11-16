#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <map>
#include <set>
#include <vector>
using namespace std;

template<class TT>
void change(TT& a, TT& b)
{
	TT temp = a;
	a = b;
	b = temp;
}

void binaryConvert(int bits, string& output, int slength);
int intConvert(const string& bits);


int main(int argc, char* argv[])
{
	if (argc >= 4)
	{
		//INPUT KISS
		ifstream iFile;
		iFile.open(argv[1]);


		int numLevelInput;
		int numLevelOutput;
		int LSN = 0;
		int s;
		string type = "";
		string r;
		map<string, int> sign;
		vector<vector<string>> output;
		vector<string> states;
		vector<vector<string>> nextStates;
		vector<int> grouping;

		iFile >> type; //.start_kiss
		iFile >> type >> numLevelInput; //.i
		iFile >> type >> numLevelOutput; //.o
		iFile >> type >> LSN; //.p
		iFile >> type >> s; //.s
		iFile >> type >> r; //.r
		string* toString = new string[s];

		string* LSNin = new string[LSN];
		string* LS1 = new string[LSN];
		string* LS2 = new string[LSN];
		string* LSNout = new string[LSN];

		int index = 0;
		for (int i = 0; i < LSN; i++)
		{
			iFile >> LSNin[i] >> LS1[i] >> LS2[i] >> LSNout[i];
			if (sign.find(LS1[i]) == sign.end())
			{
				toString[index] = LS1[i];
				sign.insert(pair<string, int>(LS1[i], index++));
				states.push_back(LS1[i]);
				output.push_back(vector<string>(pow(2, numLevelInput)));
				output.back()[intConvert(LSNin[i])] = LSNout[i];
				nextStates.push_back(vector<string>(pow(2, numLevelInput)));
				nextStates.back()[intConvert(LSNin[i])] = LS2[i];
				grouping.push_back(-1);
			}
			else
			{
				for (int j = 0; j < output.size(); j++)
				{
					if (states[j] == LS1[i])
					{
						output[j][intConvert(LSNin[i])] = LSNout[i];
						nextStates[j][intConvert(LSNin[i])] = LS2[i];
						break;
					}
				}
			}
		}
		iFile.close(); //.end_kiss


		//OUTPUT DOT FILE
		ofstream oFileOriDot;
		oFileOriDot.open("OriginSTG.dot");
		oFileOriDot << "digraph STG {" << endl;
		oFileOriDot << "\trankdir=LR;" << endl;
		oFileOriDot << endl;
		oFileOriDot << "\tINIT [shape=point];" << endl;
		for (auto it = states.begin(); it != states.end(); it++)
		{
			oFileOriDot << "\t" << *it << " [label=\"" << *it << "\"];" << endl;
		}
		oFileOriDot << endl;
		oFileOriDot << "\tINIT -> " << r << ";" << endl;
		bool* oriOutput = new bool[LSN];
		memset(oriOutput, false, sizeof(bool) * LSN);
		for (int i = 0; i < LSN; i++)
		{
			if (oriOutput[i])
			{
				continue;
			}
			oriOutput[i] = true;
			oFileOriDot << "\t" << LS1[i] << " -> " << LS2[i] << " [label=\"" << LSNin[i] << "/" << LSNout[i];
			for (int j = i + 1; j < LSN; j++)
			{
				if (oriOutput[j] || LS1[i] != LS1[j] || LS2[i] != LS2[j])
				{
					continue;
				}
				oriOutput[j] = true;
				oFileOriDot << ", " << LSNin[j] << "/" << LSNout[j];
			}
			oFileOriDot << "\"];" << endl;
		}
		oFileOriDot << "}" << endl;
		oFileOriDot.close();


		//GROUPING SAME OUTPUT
		int numOfGroup = 0;
		for (int i = 0; i < output.size(); i++)
		{
			if (grouping[i] == -1)
			{
				grouping[i] = numOfGroup;
				for (int j = i + 1; j < output.size(); j++)
				{
					if (grouping[j] == -1)
					{
						bool same = true;
						for (int k = 0; k < pow(2, numLevelInput) && same; k++)
						{
							if (output[i][k] != output[j][k]) same = false;
						}
						if (same)
						{
							grouping[j] = grouping[i];
						}
					}
				}
				numOfGroup++;
			}
		}


		//STATE PAIRS IMPLICATION TABLE
		string*** impli = new string * *[s - 1];
		int** markT = new int* [s - 1];
		for (int i = 0; i < s - 1; i++)
		{
			impli[i] = new string * [s - 1];
			markT[i] = new int[s - 1];
			for (int j = 0; j < s - 1; j++)
			{
				impli[i][j] = new string[(int)pow(2, numLevelInput) * 2];
				markT[i][j] = -1;
			}
		}


		//REMOVING THE incomp PAIRS
		bool rmvImpli = false;
		string sameOutput;
		for (int i = 0; i < s - 1; i++)
		{
			for (int j = 0; j < i + 1; j++)
			{
				if (grouping[i + 1] != grouping[j])
				{
					markT[i][j] = 0;
				}
			}
		}


		//NEXT STATES PAIRS
		for (int i = 0; i < s - 1; i++)
		{
			for (int j = 0; j < i + 1; j++)
			{
				if (markT[i][j] == 0) continue;
				for (int k = 0; k < pow(2, numLevelInput); k++)
				{
					impli[i][j][k * 2] = nextStates[i + 1][k];
					impli[i][j][k * 2 + 1] = nextStates[j][k];
				}
			}
		}


		//TRANSITION PAIRS CHECK AND REMOVAL
		int markCount = 1;
		bool incomp;
		bool diff = true;
		while (diff)
		{
			diff = false;
			for (int i = 0; i < s - 1; i++)
			{
				for (int j = 0; j < i + 1; j++)
				{
					if (markT[i][j] > -1 && markT[i][j] < markCount)
					{
						continue;
					}
					incomp = false;
					for (int k = 0; k < pow(2, numLevelInput) && !incomp; k++)
					{
						int a_sign = sign[impli[i][j][k * 2]];
						int b_sign = sign[impli[i][j][k * 2 + 1]];
						if (a_sign == b_sign)
						{
							continue;
						}
						else if (a_sign < b_sign) change(a_sign, b_sign);
						if (markT[a_sign - 1][b_sign] > -1 && markT[a_sign - 1][b_sign] < markCount)
						{
							markT[i][j] = markCount;
							incomp = true;
							diff = true;
						}
					}
				}
			}
			markCount++;
		}



		//MERGING THE COMPATIBLE STATES
		bool* merge = new bool[s];
		int mergeCount = 0;
		string mergeString;
		set<string> rmvSign;
		memset(merge, false, sizeof(bool) * s);
		for (int i = 0; i < s - 1; i++)
		{
			for (int j = 0; j < i + 1; j++)
			{
				if (markT[i][j] == -1 && !merge[i + 1] && !merge[j]) //compatible
				{
					merge[i + 1] = true;
					if (r == toString[i + 1]) r = toString[j];
					rmvSign.insert(toString[i + 1]);
					mergeCount++;
					for (int a = 0; a < nextStates.size(); a++)
					{
						if (a == i + 1)
						{
							continue;
						}
						for (int b = 0; b < nextStates[a].size(); b++)
						{
							if (nextStates[a][b] == toString[i + 1])
							{
								nextStates[a][b] = toString[j];
							}
						}
					}
					toString[i + 1] = toString[j];
				}
			}
		}


		//IMPLICATION TABLE UPDATED
		int newIndex = 0;
		vector<int> newGrouping;
		vector<string> newState;
		vector<vector<string>> newOutput;
		vector<vector<string>> newNextState;
		sign.clear();
		delete[] toString;
		toString = new string[s - mergeCount];

		for (int i = 0; i < output.size(); i++)
		{
			if (merge[i]) continue;
			toString[newIndex] = states[i];
			sign.insert(std::pair<string, int>(states[i], newIndex++));
			newState.push_back(states[i]);
			newOutput.push_back(output[i]);
			newNextState.push_back(nextStates[i]);
			newGrouping.push_back(grouping[i]);
		}
		grouping = newGrouping;
		output = newOutput;
		states = newState;
		nextStates = newNextState;
		s = newState.size();


		//KISS FILE OUTPUT
		vector<string> newLSNin; //in
		vector<string> newLSNout; //out
		vector<string> newLS1;
		vector<string> newLS2;
		set<string> newSign;

		for (int i = 0; i < output.size(); i++)
		{
			for (int j = 0; j < pow(2, numLevelInput); j++)
			{
				//if (removedindex[i]) continue;
				binaryConvert(j, type, numLevelInput);
				newLSNin.push_back(type);
				newLS1.push_back(states[i]);
				newLS2.push_back(nextStates[i][j]);
				newLSNout.push_back(output[i][j]);
				newSign.insert(states[i]);
			}
		}
		ofstream ofsKISS;
		ofsKISS.open(argv[2]);
		ofsKISS << ".start_kiss\n";
		ofsKISS << ".i " << numLevelInput << "\n";
		ofsKISS << ".o 1\n";
		ofsKISS << ".p " << newLSNin.size() << "\n";
		ofsKISS << ".s " << newSign.size() << "\n";
		ofsKISS << ".r " << r << "\n";
		for (int i = 0; i < newLSNin.size(); i++)
		{
			ofsKISS << newLSNin[i] << " " << newLS1[i] << " " << newLS2[i] << " " << newLSNout[i] << "\n";
		}
		ofsKISS << ".end_kiss\n";
		ofsKISS.close();


		//DOT FILE OUTPUT
		ofstream oFileDot;
		oFileDot.open(argv[3]);
		oFileDot << "digraph STG {" << endl;
		oFileDot << "\trankdir=LR;" << endl;
		oFileDot << endl;
		oFileDot << "\tINIT [shape=point];" << endl;
		for (auto it = newSign.begin(); it != newSign.end(); it++)
		{
			oFileDot << "\t" << *it << " [label=\"" << *it << "\"];" << endl;
		}
		oFileDot << endl;
		oFileDot << "\tINIT -> " << r << ";" << endl;
		bool* print = new bool[newLSNin.size()];
		memset(print, false, sizeof(bool) * newLSNin.size());
		for (int i = 0; i < newLSNin.size(); i++)
		{
			if (print[i])
			{
				continue;
			}
			print[i] = true;
			oFileDot << "\t" << newLS1[i] << " -> " << newLS2[i] << " [label=\"" << newLSNin[i] << "/" << newLSNout[i];
			for (int j = i + 1; j < newLSNin.size(); j++)
			{
				if (print[j] || newLS1[i] != newLS1[j] || newLS2[i] != newLS2[j]) continue;
				print[j] = true;
				oFileDot << ", " << newLSNin[j] << "/" << newLSNout[j];
			}
			oFileDot << "\"];" << endl;
		}
		oFileDot << "}" << endl;
		oFileDot.close();
	}
	else
	{
		cout << "failed to open..." << endl;
	}
	return 0;
}

void binaryConvert(int bits, string& output, int sLength)
{
	output.clear();
	while (bits > 0)
	{
		output = (bits % 2 == 0 ? "0" : "1") + output;
		bits /= 2;
	}
	int lengthOutput = (int)output.length();
	output = string(sLength - lengthOutput, '0') + output;
}

int intConvert(const string& bin)
{
	int w = 0;
	int res = 0;
	int binLength = bin.length() - 1;
	for (int i = binLength; i >= 0; i--)
	{
		res += (bin[i] - '0') * pow(2, w);
		w++;
	}
	return res;
}

