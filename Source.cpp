#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <stdlib.h>
using namespace std;

class unit {
public:
    string parameter; //int8,16,etc
    string uname;   //a,b,c,etc
    string utype; //i,e, input,output, wire

    string getDetails() {
        return this->uname + " " + this->parameter + "  " + this->utype;
    }
    int getParam() {
        map <string, int> temp;
        temp["Int1"] = 1;
        temp["Int2"] = 2;
        temp["Int8"] = 8;
        temp["Int16"] = 16;
        temp["Int32"] = 32;
        temp["Int64"] = 64;
        temp["UInt1"] = 1;
        temp["UInt2"] = 2;
        temp["UInt8"] = 8;
        temp["UInt16"] = 16;
        temp["UInt32"] = 32; 
        temp["UInt64"] = 64;
        return temp[this->parameter];
    }
};

map <string, string> sizes;
//functions for converting to verilog
string toVerilog(vector<string> infile,string filename);  
int getConnections(vector<string> line, vector <unit> &inputs);
string getParam(vector<unit> inputs);
string getIO(vector<unit>inputs);
string operations(vector<string> line, int i);

//utility functions
vector<string> tokenize(string s, string tokenizer);
int myStoi(string s);
void outFileFun(string outfilename, string output);
string removeSpaces(string s);

int main(int argc,char ** argv) {
    
    string s, temp, filestr = "", outfilename = "out.v";
    
    if (argc > 2) { //if filenames are given in argument
        for (int i = 0; i < strlen(argv[0]); i++) {
            s = s + argv[0][i];
        }
        outfilename = "";
        for (int i = 0; i < strlen(argv[1]); i++) {
            outfilename += argv[1][i];
        }
    }
    else { //input file name from user
        cout << "Enter file path\n";
        cin >> s;
    }
    string filename = s;
    filename = "\\" +s;
    int t1 = 0, t2 = 0;

    for (int i = filename.length() - 1; i >= 0; i--) {
        if (filename[i] == '.') {
            t2 = i;
        }
        if (filename[i] == '\\') {
            t1 = i;
            filename=filename.substr(t1+1, t2 - t1-1);
            break;
        }
    }
    ifstream infile;
    infile.open(s);
    vector <string> ifile;
    if (infile.is_open()) {
        while (getline(infile, temp)) {  //storing input file in file
            if (temp.length() > 2) {
                if (temp.substr(0, 2) != "//") {
                    temp = removeSpaces(temp);
                    filestr += temp + "\n";
                    ifile.push_back(temp);
                }
            }
        }
        infile.close();
        cout << "--InFile--\n"+ filestr +"\n";
        string output = toVerilog(ifile,filename);
        cout << "--OutFile--\n\n\n"+ output + "\n\n";
        outFileFun(outfilename, output);
    }
    else {
        cout << "File missing/Incorrect path\n";
    }
}

//writes string to file
void outFileFun(string outfilename,string output) {
    ofstream outfile;
    outfile.open(outfilename);
    outfile << output;
    outfile.close();
}

//Removes spaces and inline coments
string removeSpaces(string s) {
    int count = 0;
    for (int i = 0; i < s.length(); i++) {
        if (s[i] == ' '||s[i]=='\t') {
            count++;
        }
        else {
            count = 0;
        }
        if (count > 1) {
            i--;
            s.erase(s.begin() + i);
        }
        if (s[i] == '/') {
            s= s.substr(0, i);
        }
    }
    if (s[0] == ' '|| s[0] == '\t') {
        s.erase(s.begin());
    }
    if (s[s.length()-1] == ' '|| s[s.length() - 1] == '\t') {
        s.erase(s.begin()+s.length()-1);
    }
    return s;
}


string toVerilog(vector<string> infile,string filename) {

    string ofile = "`timescale 1ns / 1ps\n\nmodule "+filename+"(";
    string wires = "";
    vector <unit> inputs;
    int i=getConnections(infile, inputs);
 
    int j = 0;
    for (int i = 0; i < inputs.size(); i++) {
    
    }
    while (inputs[j].utype != "wire") {
        ofile = ofile + inputs[j].uname + ", ";
        j++;
    }
    ofile += "Clk, Rst);\n";
    ofile += getParam(inputs)+"\n";
    ofile += getIO(inputs) + "\n";
    ofile += operations(infile, i) + "\n";

    return ofile;
}
int getConnections(vector <string> lines, vector <unit> &inputs) {
    int i = 0;
    while (i < lines.size()) {
        string line = lines[i];
        vector <string> tokens = tokenize(line," ");
        if (tokens[0] != "input" && tokens[0] != "wire" && tokens[0] != "output"&& tokens[0]!="register") {
            return i;
            break;
        }

        string ltype = tokens[0];
        string param = tokens[1];
        unit temp;
        temp.utype = ltype;
        temp.parameter = param;
        int j = 2;
        while (j<tokens.size()) {
            temp.uname = tokens[j].substr(0,tokens[j].length()-1);
            j++;
            inputs.push_back(temp);
            sizes[temp.uname] = temp.parameter;
        }
        inputs.pop_back();
        temp.uname = tokens[j-1];
        sizes[temp.uname] = temp.parameter;
        inputs.push_back(temp);
        i++;
    }
   
}

//returns parameter part of output file
string getParam(vector<unit> inputs) {
    string parameters = "";
    map <string, string> param;
    for (int i = 0; i < inputs.size(); i++) {
        if (param.find(inputs[i].parameter) == param.end()) {
            param[inputs[i].parameter] = 'a';
            parameters += "\tparameter "+inputs[i].parameter + " = " + to_string(inputs[i].getParam()) + ";\n";
        }
    }
    return parameters;
}

//returns IO declaration  of outputfile
string getIO(vector<unit>inputs) {
    string IO = "\tinput Rst, Clk;\n";
    IO+="\t" + inputs[0].utype + " " + "[" + to_string(inputs[0].getParam() - 1) + ":0] " + inputs[0].uname;
    for (int i = 1; i < inputs.size(); i++) {
        if (inputs[i].parameter == inputs[i-1].parameter &&inputs[i].utype == inputs[i-1].utype) {
            IO += ", " + inputs[i].uname;
        }
        else {
            if (inputs[i].getParam() == 1) {
                IO += ";\n\t" + inputs[i].utype + " " + inputs[i].uname;
            }
            else {
                IO += ";\n\t" + inputs[i].utype + " " + "[" + to_string(inputs[i].getParam() - 1) + ":0] " + inputs[i].uname;
            }
         }
        
    }
    IO += ";";
    return IO;
}


//Converts component statements
string operations(vector<string> infile,int i) {  
    map <string, string> oprtrs;
    oprtrs["+"] = "ADD";
    oprtrs["-"] = "SUB";
    oprtrs["*"] = "MUL";
    oprtrs["<<"] = "SHL";
    oprtrs[">>"] = "SHR";
    oprtrs[">"] = "COMP"; 
    oprtrs["<"] = "COMP";
    oprtrs["=="] = "COMP";
    oprtrs[":"] = "MUX2x1";
    oprtrs["="] = "Reg";

    map <string, int> counter;
    counter["+"] = 0; counter["-"] = 0; counter["*"] = 0; counter["<<"] = 0; counter[">>"] = 0;
    counter["<"] = 0; counter[">"] = 0; counter[":"] = 0; counter["=="] = 0; counter["=" ] = 0;
    string result="\t";
    while (i < infile.size()  && infile[i].length() > 2) {
        vector <string> tline = tokenize(infile[i], " ");
        string oprtr = tline[tline.size() - 2];
        
        // Checks for errors in input
        for (int k = 0; k < tline.size(); k += 2) {
            if (sizes.find(tline[k]) == sizes.end()) {
                outFileFun("out.v", "Error in input file\n\n");
                cout << "Error in input file\n\n";
                exit(-1);
            }
        }
        if (oprtrs.find(oprtr) == oprtrs.end()) {
            cout << "Error in input file\n\n";
            outFileFun("out.v","Error in input file\n\n");
            exit(-1);
        }
        ///////////////////////////////
        string comon = "";
        string linetype = sizes[tline[0]];

        //linetype in case of comparator
        if (oprtr == ">" || oprtr == "<"||oprtr=="==") {
            int l1 = myStoi(sizes[tline[2]]);
            int l2 = myStoi(sizes[tline[4]]);
            int grtr = l1 > l2 ? l1 : l2;
            linetype = sizes[tline[0]];
            for (int i = 0; i < linetype.length(); i++) {
                if (linetype[i] >= '0'&&linetype[i] <= '9') {
                    linetype = linetype.substr(0,i);
                    break;
                }
            }
            linetype += to_string(grtr);
            counter["<"] += 1;
            counter[">"] += 1;
            counter["=="] += 1;
        }
        else {
            counter[oprtr] += 1;
        }
        /////////////////////////////////

        comon += oprtrs[oprtr] + " #(.DATAWIDTH(" + linetype + "))";
        comon += oprtrs[oprtr] + "_" + to_string(counter[oprtr]);
         
        if (oprtr != ":" && oprtr != ">" && oprtr != "=" && oprtr != "<" && oprtr!="==") {
            result +=comon+ "(" + tline[2] + ", " + tline[4] + ", " + tline[0] + ");\n\t";
        }
        else if (oprtr == "=") {
            result += comon + "(" + tline[2] + ", Clk, Rst, " + tline[0] + ");\n\t";
        }
        else if (oprtr == ":") {
            result += comon + "(" + tline[6] + ", " + tline[4] + ", " + tline[2] + ", " + tline[0] + ");\n\t";
        }
        else if (oprtr == ">"|| oprtr == "<"|| oprtr=="==") {
            result += comon + "(" + tline[2] + ", " + tline[4] + ", " + tline[0] + ", gt, lt, eq);\n\t";
        }
        i++;
    }
    result += "\nendmodule\n";
    return result;

}


//tokenizes a string w.r.t. a tokenizer
vector<string> tokenize(string s, string tokenizer) {
    int pos;
    vector <string> result;
    while ((pos = s.find(tokenizer)) != std::string::npos) {
        result.push_back(s.substr(0, pos));
        s.erase(0, pos + tokenizer.length());
    }
    if (s != " ") {
        result.push_back(s);
    }
    return result;
}
//returns integer from a string
int myStoi(string s){
    int r = 0;
    for (int i = 0; i < s.length(); i++) {
        if (s[i] >= '0'&&s[i] <= '9') {
            r = stoi(s.substr(i));
            break;
        }
    }
    return r;
}