//資訊三甲_10427101_馬若芸 

//********************************************************************************************/
//SIC & SIC/XE Assembler
//********************************************************************************************/
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdio>
#include <cstring> 
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>

#define TableSize 	100  	//hash table大小 

using namespace std;

typedef struct tableUnit	//token
{
	int num;		//token編號 
	string cont;	//token內容 
} tUnit;

typedef struct instrUnit	//instruction token
{
	int num;		//token編號 
	string instr;
	int contNum;	//label個數 
	int format; 
	int opcode;
} iUnit;

typedef struct statmentUnit
{
	int line;
	int loc;	
	string statment;
	string label;
	string instr;
	vector<string> conts;	//使用的label 
	int format;
	int opcode;
	
	string objCode;
	
	bool isComment;
	bool isIndexReg;
	bool isPesudo;
	bool isImmediate;
	bool isIndirect;
	bool lexicalError;
	bool syntaxError;
	
	bool isLTORG;	
	bool addLTORG;		//要增加的literal 
	string contLTORG;
	
	bool hasOper;		//處理ex: MAXLEN EQU BUFFER-BUFFEND 
	string contOper;
} sUnit;

typedef struct formatFor34	
{
	string n;		//@      n & i = 1 ==> sic/xe
	string i;		//#
	string x;		//index reg
	string b;		//base reg
	string p;		//program counter
	string e;		//0:format 3	1:format 4 
} format34;

bool ReadWriteFile();							//讀輸入檔並開始寫入輸出檔 
void WhichTable(string, vector<tUnit>, vector<iUnit>);			//根據讀入的table檔案放入相對應的vector中 
void InputTable();								//讀入table檔案 
void CheckTable(string);						//分析token 
void ExtraTable(string);						//把多個相連的token，更仔細地切割、分析 
void GetToken(string);							//把讀進的內容切成一個個token 
bool isDeli(char);								//token是否為符號 
bool isNum(string);								//token是否為數字 
int setHashKey(string);							//計算hash值，回傳至insertHashLinearn() 
void insertHashLinear(string, tUnit[], int);	//線性探測建立雜湊表 
void resetList(tUnit[]);						//初始化需要作hash的table 
string HexToStr(string);

vector<iUnit> tSetInstr;	//(1)instruction table
vector<tUnit> tSetPE;		//(2)pesudo and extra table
vector<tUnit> tSetReg;		//(3)register table
vector<tUnit> tSetDeli;		//(4)delimiter table
tUnit 	  	  tSetSym[TableSize];		//(5)symbol table
tUnit         tSetIR[TableSize];		//(6)integer/real table
tUnit         tSetStr[TableSize];		//(7)string table
int readTables = 0;
string fName = "\0";
ofstream fileOut;
int currentLine = 5;
bool isSIC = false, isSICXE = false;
string currentLTORG = "\0";

sUnit sData;
vector<sUnit>	sSet;

void introMsg(void)			// initiate an opening message
{   
	cout << "**************************************************" << endl;
    cout << " SIC & SIC/XE Assembler                          *" << endl;
    cout << " Step 1: Read and show tables                    *" << endl;
    cout << " Step 2: SIC & SIC/XE Assembler                  *" << endl;
    cout << "**************************************************" << endl;
    cout << "##################################################" << endl;
    cout << " Step 1: Read and show tables  "                    << endl;
    cout << "##################################################" << endl;
} //introMsg()
  
void initeData(){		//初始化一個敘述內容 
	sData.line = 0;
	sData.loc = 0x0;
	sData.statment = "\0";
	sData.label = "\0";
	sData.instr = "\0";
	sData.conts.clear();
	sData.format = 0;
	sData.opcode = 0x0;
	sData.objCode = "\0";
	sData.isComment = false;
	sData.isIndexReg = false;
	sData.isPesudo = false;
	sData.isImmediate = false;
	sData.isIndirect = false;
	sData.lexicalError = false;
	sData.syntaxError = false;
	
	sData.isLTORG = false;
	sData.addLTORG = false;
	sData.contLTORG = "\0";
	sData.hasOper = false;
	sData.contOper = "\0";
	
}

void SetOpcode(){		//設定指令的format、使用的label數、opcode 
	//tSetInstr
	for(int i = 0; i < tSetInstr.size(); i++){
		if(stricmp(tSetInstr[i].instr.c_str(), "add") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x18;
		} 
		else if(stricmp(tSetInstr[i].instr.c_str(), "addf") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x58;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "addr") == 0){
			tSetInstr[i].format = 2;
			tSetInstr[i].contNum = 2;
			tSetInstr[i].opcode = 0x90;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "and") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x40;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "clear") == 0){	
			tSetInstr[i].format = 2;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0xB4;
		} 
		else if(stricmp(tSetInstr[i].instr.c_str(), "comp") == 0){
			tSetInstr[i].format = 3;	
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x28;
		} 
		else if(stricmp(tSetInstr[i].instr.c_str(), "compf") == 0){	
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x88;
		} 
		else if(stricmp(tSetInstr[i].instr.c_str(), "compr") == 0){
			tSetInstr[i].format = 2;
			tSetInstr[i].contNum = 2;
			tSetInstr[i].opcode = 0xA0;
		} 
		else if(stricmp(tSetInstr[i].instr.c_str(), "div") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x24;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "divf") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x64;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "divr") == 0){
			tSetInstr[i].format = 2;
			tSetInstr[i].contNum = 2;
			tSetInstr[i].opcode = 0x9C;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "fix") == 0){
			tSetInstr[i].format = 1;
			tSetInstr[i].contNum = 0;
			tSetInstr[i].opcode = 0xC4;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "float") == 0){
			tSetInstr[i].format = 1;
			tSetInstr[i].contNum = 0;
			tSetInstr[i].opcode = 0xC0;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "hio") == 0){
			tSetInstr[i].format = 1;
			tSetInstr[i].contNum = 0;
			tSetInstr[i].opcode = 0xF4;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "j") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x3C;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "jeq") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x30;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "jgt") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x34;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "jlt") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x38;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "jsub") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x48;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "lda") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x00;
		}
		else if(stricmp(tSetInstr[i].instr.c_str(), "ldb") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x68;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "ldch") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x50;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "ldcf") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x70;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "ldl") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x08;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "lds") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x6C;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "ldt") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x74;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "ldx") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x04;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "lps") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0xD0;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "mul") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x20;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "mulf") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x60;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "mulr") == 0){
			tSetInstr[i].format = 2;
			tSetInstr[i].contNum = 2;
			tSetInstr[i].opcode = 0x98;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "norm") == 0){
			tSetInstr[i].format = 1;
			tSetInstr[i].contNum = 0;
			tSetInstr[i].opcode = 0xC8;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "or") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x44;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "rd") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0xD8;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "rmo") == 0){
			tSetInstr[i].format = 2;
			tSetInstr[i].contNum = 2;
			tSetInstr[i].opcode = 0xAC;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "rsub") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 0;
			tSetInstr[i].opcode = 0x4C;
		}
		else if(stricmp(tSetInstr[i].instr.c_str(), "shiftl") == 0){
			tSetInstr[i].format = 2;
			tSetInstr[i].contNum = 2;
			tSetInstr[i].opcode = 0xA4;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "shiftr") == 0){
			tSetInstr[i].format = 2;
			tSetInstr[i].contNum = 2;
			tSetInstr[i].opcode = 0xA8;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "sio") == 0){
			tSetInstr[i].format = 1;
			tSetInstr[i].contNum = 0;
			tSetInstr[i].opcode = 0xF0;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "ssk") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0xEC;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "sta") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x0C;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "stb") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x78;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "stch") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x54;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "stf") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x80;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "sti") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0xD4;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "stl") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x14;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "sts") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x7C;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "stsw") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0xE8;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "stt") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x84;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "stx") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x10;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "sub") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x1C;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "subf") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x5C;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "subr") == 0){
			tSetInstr[i].format = 2;
			tSetInstr[i].contNum = 2;
			tSetInstr[i].opcode = 0x94;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "svc") == 0){
			tSetInstr[i].format = 2;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0xB0;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "td") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0xE0;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "tio") == 0){
			tSetInstr[i].format = 1;
			tSetInstr[i].contNum = 0;
			tSetInstr[i].opcode = 0xF8;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "tix") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0x2C;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "tixr") == 0){
			tSetInstr[i].format = 2;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0xB8;
		}	
		else if(stricmp(tSetInstr[i].instr.c_str(), "wd") == 0){
			tSetInstr[i].format = 3;
			tSetInstr[i].contNum = 1;
			tSetInstr[i].opcode = 0xDC;
		}
	}
		
} 

bool ReadWriteFile()		//讀輸入檔並開始寫入輸出檔 
{	
	string temp = "\0";
	sUnit tempEX;
	vector<sUnit> setLTORG;
	while(fName != "0"){
		cout << "Input file name...[0]Quit: " << endl;
		cin >> fName;
		string infName = fName + ".txt";
		ifstream inFile;
		inFile.open(infName.c_str());		//讀書入檔 
		if (!inFile.is_open())
			cout << endl << "### " << infName << " does not exist! ###" << endl;
  	    else{
  	    	string outFileName =  fName + "_output.txt";		//寫輸出檔 
			fileOut.open(outFileName.c_str(), ios::out | ios::trunc);
			
  			while ( !inFile.eof() ) {    
  				if (getline(inFile, temp) && temp != "\0"){	
					//fileOut << temp <<endl;
					initeData();
					sData.statment = temp;
  					if(temp[0] != '.')
						GetToken(temp);			//把讀進的內容切成一個個token
					else{
						sData.isComment = true;
					}
  					
					sData.line = currentLine;
  					
  					if((sData.isLTORG) && (stricmp(sData.instr.c_str(), "LTORG") != 0) && (stricmp(sData.instr.c_str(), "END") != 0)){
						currentLTORG = sData.conts[0];		//新增literal內容 
						  
						tempEX.statment = sData.contLTORG;
  						tempEX.label = sData.conts[0];
  						tempEX.objCode = sData.objCode;
  						tempEX.isPesudo = false;
  						tempEX.addLTORG = true;
  						tempEX.isLTORG = true;
  						
  						tempEX.line = sData.line + 5;
						tempEX.loc = 0x0;
						tempEX.instr = "\0";
						tempEX.conts.clear();
						tempEX.format = 0;
						tempEX.opcode = 0x0;
						tempEX.isComment = false;
						tempEX.isIndexReg = false;
						tempEX.isImmediate = false;
						tempEX.isIndirect = false;
						tempEX.lexicalError = false;
						tempEX.syntaxError = false;
	
						tempEX.contLTORG = "\0";
						tempEX.hasOper = false;
						tempEX.contOper = "\0";
						
						bool isCreated = false;
						for(int k = 0; !setLTORG.empty() && (k < setLTORG.size()); k++){
							if(strcmp(setLTORG[k].statment.c_str(), tempEX.statment.c_str()) == 0)
								isCreated = true;
						}
						
						if(!isCreated)	setLTORG.push_back(tempEX);					
						sSet.push_back(sData);
					} 					
  					else if((!setLTORG.empty()) && ((stricmp(sData.instr.c_str(), "LTORG") == 0) || (stricmp(sData.instr.c_str(), "END") == 0))){ 																		 					
  						sSet.push_back(sData);					//出現LTORG或END時 
  						for(int k = 0; k < setLTORG.size(); k++)//要接著將之前新增的litera內容也寫入檔案 
						  sSet.push_back(setLTORG[k]);
						  
						setLTORG.clear();
					}
					else{
						sSet.push_back(sData);		
					}
  						
					currentLine = currentLine + 5;
    			}//if		
  			}//while
			return true; 		
		}//else
	}//while

	return false;
} //ReadWriteFile()  
  
void WhichTable(string num, vector<tUnit> aSet, vector<iUnit> iSet)			//根據讀入的table檔案放入相對應的vector中 
{
   	string tName;
	if(num == "1" && tSetInstr.empty()){			//Instruction Table
		tSetInstr = iSet;
		SetOpcode();
		readTables = readTables + 1;
		tName = "Instruction Table 1";
	}	
	else if(num == "2" && tSetPE.empty()){			//Pseudo and Extra Table
		tSetPE = aSet;
		readTables = readTables + 1;
		tName = "Pseudo and Extra Table 2";
	}	
	else if(num == "3" && tSetReg.empty()){			//Register Table
		tSetReg = aSet;
		readTables = readTables + 1;
		tName = "Register Table 3";
	}	
	else if(num == "4" && tSetDeli.empty()){		//Delimiter Table
		tSetDeli = aSet;
		readTables = readTables + 1;
		tName = "Delimiter Table 4";
	}
		
	cout << tName << endl;
	if(num == "1"){
		for(int i = 0; i < iSet.size(); i++){			//螢幕輸出table內容 
			cout << "\t[" << iSet[i].num << "] " << iSet[i].instr << setw(4);
			if(!((i+1)% 5))
				cout << endl;
		}//for
	}
	else{
		for(int i = 0; i < aSet.size(); i++){			//螢幕輸出table內容 
			cout << "\t[" << aSet[i].num << "] " << aSet[i].cont << setw(4);
			if(!((i+1)% 5))
				cout << endl;
		}//for		
	}
} //WhichTable()

void InputTable()			//讀入table檔案 
{
	ifstream inFile;
	string numTable = "-1", fileName = "Table";
	vector<tUnit> tSet;
	vector<iUnit> iSet;
	tUnit tNode ;
	iUnit iNode ;
    tNode.num = 0;
    iNode.num = 0;
	
	while(numTable != "0"){
		cout << endl << "Input the table number: 1 ~ 4...,[0]Quit: ";
		cin >> numTable;
	
		fileName = fileName + numTable + ".table";	
		inFile.open(fileName.c_str());
	
		if(numTable != "0"){
			if (!inFile.is_open())
				cout << endl << "### " << fileName << " does not exist! ###" << endl;
  	    	else{		
				if(strcmp(numTable.c_str(), "1") == 0){
			  		while ( !inFile.eof () ) {    
  	  					inFile >> iNode.instr;
						iNode.num = iNode.num + 1; 
						iSet.push_back(iNode);
   					}//while
				  }	
				  else{
				  	while ( !inFile.eof () ) {    
  	  					inFile >> tNode.cont;
						tNode.num = tNode.num + 1; 
						tSet.push_back(tNode);
   					}//while
				  }
			
				WhichTable(numTable, tSet, iSet);		//根據讀入的table檔案放入相對應的vector中
				inFile.close();	
			}
		}//if
		else{
			if(readTables != 4)
				cout << endl << "### Must read Table1 ~ Table4 done!! ###\n";
			else
				return;
		}
	
		numTable = "-1", fileName = "Table";
		tNode.num = 0;
		tSet.clear();
	}//while

} //InputTable()

void CheckTable(string token)			//分析token 
{
	for(int i = 0; i < tSetInstr.size(); i++){				//此token屬於Instruction Table
		if(stricmp(tSetInstr[i].instr.c_str(), token.c_str()) == 0){
			sData.instr = token;
			sData.opcode = tSetInstr[i].opcode;
			if(sData.format == 0)
				sData.format = tSetInstr[i].format;

			return;
		}	
	}
	
	for(int i = 0; i < tSetPE.size(); i++){					//此token屬於Pseudo and Extra Table
		if(stricmp(tSetPE[i].cont.c_str(), token.c_str()) == 0){
			sData.instr = token;
			sData.isPesudo = true;
			if((stricmp("LTORG", token.c_str()) == 0) || (stricmp("END", token.c_str()) == 0)){
				sData.isLTORG = true;	
			}
			
			return;
		}	
	}
	
	for(int i = 0; i < tSetReg.size(); i++){				//此token屬於Register Table
		if(stricmp(tSetReg[i].cont.c_str(), token.c_str()) == 0){
			sData.conts.push_back(token);
			if(stricmp(token.c_str(), "X") == 0)	//使用index reg 
				sData.isIndexReg = true;
				
			return;
		}		
	}
	
	for(int i = 0; i < tSetDeli.size(); i++){				//此token屬於Delimiter Table
		if(strcmp(tSetDeli[i].cont.c_str(), token.c_str()) == 0){
			
			if((!sData.conts.empty()) && ((strcmp(tSetDeli[i].cont.c_str(), "+") == 0) || (strcmp(tSetDeli[i].cont.c_str(), "-") == 0) || (strcmp(tSetDeli[i].cont.c_str(), "*") == 0) || (strcmp(tSetDeli[i].cont.c_str(), "/") == 0))){
				sData.contOper = token;
				sData.hasOper = true;
			}
			if((sData.conts.empty()) && ((strcmp(tSetDeli[i].cont.c_str(), "*") == 0))){
				sData.conts.push_back(currentLTORG);	//將"*"用真正的literal值替代 
				currentLTORG = "\0";
			}
			else if(strcmp(tSetDeli[i].cont.c_str(), "+") == 0){		//format 4
				sData.format = 4;
			}
			else if(strcmp(tSetDeli[i].cont.c_str(), "#") == 0){	//immiediate mode
				sData.isImmediate = true;
			}
			else if(strcmp(tSetDeli[i].cont.c_str(), "@") == 0){	//indirect mode
				sData.isIndirect = true;
			}
			
			return;
		}		
	}

	for(int i = 0; i < TableSize; i++){						
		if((tSetSym[i].num != -999) && (strcmp(tSetSym[i].cont.c_str(), token.c_str()) == 0)){
			if(sData.instr == "\0")
				sData.label = token;
			else
				sData.conts.push_back(token);
				
			return;
		}	
		else if((tSetIR[i].num != -999) && (strcmp(tSetIR[i].cont.c_str(), token.c_str()) == 0)){
			sData.conts.push_back(token);
			return;
		}
	}
	
	ExtraTable(token);				//把多個相連的token，更仔細地切割、分析，還有String Table的部分 
} //CheckTable()

void ExtraTable(string token)				//把多個相連的token，更仔細地切割、分析
{											//若在table中找不到，則要做hash function新增至table中 
	string temp1, temp2;
	bool created = false;		//檢查token是否以建立在table中 
	
	if(token != "\0"){
		
		if(isNum(token)){								//此token屬於Integer/real
			insertHashLinear(token, tSetIR, 6);
			CheckTable(token);
		}
		else{							
			if((token[0] == 'C' || token[0] == 'c') && token[1] == '\'' && token[token.length()-1] == '\''){
				temp1 = token[1];						//此token屬於String					
				temp2 = token[token.length()-1];
				CheckTable(temp1);
				token = token.substr(2, token.length()-3);		//把string和delimiter分割開，個別分析 
				for(int i = 0; i < TableSize; i++){
					if((tSetStr[i].num != -999) && (strcmp(tSetStr[i].cont.c_str(), token.c_str()) == 0)){
						sData.conts.push_back(token);
						created = true;
					}//if	
				}//for
				if(!created){
					insertHashLinear(token, tSetStr, 7);
					for(int i = 0; i < TableSize; i++){
						if((tSetStr[i].num != -999) && (strcmp(tSetStr[i].cont.c_str(), token.c_str()) == 0)){
							sData.conts.push_back(token);
						}//if	
					}//for
				}//if
					
				for(int i = 0; i < sData.conts[0].length(); i++){   //string to ASCII in objCode
					int nn = sData.conts[0][i];
					std::stringstream ss;
					ss << setiosflags(ios::uppercase) << std::hex << nn;
  		
					sData.objCode = sData.objCode + ss.str();
				}//for				
				CheckTable(temp2);			
			}//if (C' ')字串 
			else if((token[0] == 'C' || token[0] == 'c') && (token[1] == '\'' || token[token.length()-1] == '\'') && (token[token.length()-1] != token[1])){
				sData.lexicalError = true;		//單引號不對稱
			}//else if
			else if((token[0] == 'X' || token[0] == 'x') && token[1] == '\'' && token[token.length()-1] == '\''){
				temp1 = token[1];						//此token屬於Integer/real
				temp2 = token[token.length()-1];
				CheckTable(temp1);
				token = token.substr(2, token.length()-3);		//把integer和delimiter分割開，個別分析 
				for(int i = 0; i < TableSize; i++){
					if((tSetIR[i].num != -999) && (strcmp(tSetIR[i].cont.c_str(), token.c_str()) == 0)){
						sData.conts.push_back(token);
						created = true;
					}//if
				}//for
				if(!created){
					insertHashLinear(token, tSetIR, 6);
					for(int i = 0; i < TableSize; i++){
						if((tSetIR[i].num != -999) && (strcmp(tSetIR[i].cont.c_str(), token.c_str()) == 0)){
							sData.conts.push_back(token);
						}//if
					}//for
				}//if
				
				sData.objCode = token;  //(X' ')16進位數字 的objcode				
				CheckTable(temp2);
			}//else if (X' ')16進位數字 
			else if((token[0] == 'X' || token[0] == 'x') && (token[1] == '\'' || token[token.length()-1] == '\'') && (token[token.length()-1] != token[1])){
				sData.lexicalError = true;		//單引號不對稱
			}//else if
			else if((token[0] != 'X' || token[0] != 'x') && (token[0] != 'C' || token[0] != 'c') && token[1] == '\'' && token[token.length()-1] == '\''){
				sData.lexicalError = true;		//不是C、X符號
			}//else if
			else if(isDeli(token[0]) && !sData.lexicalError){			//處理類似註解、immiediate、indirect addressing表示或type 4指令 
				if(token[0] == '.'){				//處理註解，會標示"."內容，但註解內容不翻譯只輸出 
					temp1 = token[0];
					CheckTable(temp1);
					sData.isComment = true;
				}//if 註解 
				else{								//處理類似immiediate、indirect addressing表示或type 4指令
					temp1 =  token[0];
					if(strcmp(temp1.c_str(), "=") == 0){	//處理literal 
						sData.contLTORG = token;
						sData.contLTORG = "*\t" + sData.contLTORG;
						sData.isLTORG = true;
					}
					
					token = token.substr(1, token.length()-1);
					CheckTable(temp1);
					CheckTable(token);	
				}//else  temp1:符號 
			}//else if
			else if(sData.lexicalError)	return;
			else if(token[token.length()-1] == ','){		//處理有ex:STCH RECORD, X中"RECORD,"的情形 
				temp1 = token[token.length()-1];			//is lexical analysis error!!!
				token = token.substr(0, token.length()-1);
				CheckTable(token);
				CheckTable(temp1);							
				sData.lexicalError = true;
			}//else if  逗號後不接任何label ==> lexical error 
			else{	//label間有符號連接 
				string temp = "\0";
				for(int i = 0; i < token.length(); i++){
					if(! isDeli(token[i])){
						temp = temp + token[i];
					}
					else if(isDeli(token[i]) && !sData.lexicalError){ //isDeli(token[i])
						CheckTable(temp);
						temp1 = token[i];
						CheckTable(temp1);
						temp = "\0";
					}
					else if(sData.lexicalError)	return;		//是符號但不是table中的符號 
				}//for
			
				if(temp.length() == token.length()){
					insertHashLinear(token, tSetSym, 5);		//處理symbol
					for(int i = 0; i < TableSize; i++){
						if((tSetSym[i].num != -999) && (strcmp(tSetSym[i].cont.c_str(), token.c_str()) == 0)){
							if(sData.instr == "\0")
								sData.label = token;
							else
								sData.conts.push_back(token);
							
						}	
					}
				}				 
				else
					CheckTable(temp);					
			}
				
		}//else
	} //token != "\0"
	
} //ExtraTable()

void GetToken(string word)				//把讀進的內容切成一個個token 
{
  char string[] = "" ;
  char * cut;
  cut = strtok (const_cast<char  *>(word.c_str())," \t\n");
  
  while (cut != NULL)
  {
    CheckTable(cut);
    cut = strtok (NULL, " \t\n");
  }
	
} //GetToken()

bool isDeli(char test)			//token是否為符號 
{	
	bool found = false;
	string token = "\0";
	token += test;
	if((test >= '!' && test <= '/') || 
	   (test >= ':' && test <= '@') ||
	   (test >= '[' && test < 'a') ||
	   (test >= '{' && test <= '~')){
	   	for(int i = 0; i < tSetDeli.size(); i++)				//此token屬於Delimiter Table
			if(strcmp(tSetDeli[i].cont.c_str(), token.c_str()) == 0)
				found = true;
		if(! found){
			sData.lexicalError = true;
		}	
	   	return true;
	   }		
	else
		return false;
} //isDeli() 

bool isNum(string test)			//token是否為數字 
{	
	int i = 0;		
	while((test != " ") && (i < test.length())){
		if(test[i] >= '0' && test[i] <= '9')
			i++;
		else
			return false;	
	}
	
	return true;	
} //isNum()

int setHashKey(string test)         //hash函數 =（每個字元對應的ASCII編碼相加） 除以 100(hash table size) 取餘數                                
{   
  int sum = 0, pos = 0;
  for(int i = 0; i < test.length(); i++)			//每個字元對應的ASCII編碼相加   
  		sum = sum + test[i]; 	

  sum = sum % 100;
  pos = sum ;
  return pos;
} //setHashKey()

void insertHashLinear(string test, tUnit list[], int tableType)          // 線性探測建立雜湊表                             
{   														// 依據setHashKey()把資料放到對應的位址  					
	int pos = setHashKey(test);			//算位址 
	int orgPos = 0;
	bool ok = false;
	for(; ! ok; pos++){		// 找位址放入，若遇到位址不是空的，則加一找下一個位址 
		if(pos >= TableSize)	pos = pos - TableSize;		// 超過table大小，再從[0]開始 
		
		if(list[pos].num == -999){				//此位址為空，放入資料 
			list[pos].num = pos;
			list[pos].cont = test;
			ok = true;
			orgPos = pos;
		}				    
	}                                                                  
} //insertHashLinear()

void resetList(tUnit list[])     //初始化雜湊表                                    
{   
	for(int i = 0; i < TableSize; i++){	
		list[i].cont = "\0";
		list[i].num = -999;
	}                                                                                 
} //resetList()
 
int StrToHex(string sNum){			//字串===>16進位數 
	int decNum = atoi(sNum.c_str());
	
	std::stringstream ss;
	ss << uppercase << std::hex << decNum;

	int num;
	ss >> num;
	
	return num;	
}

string HexToStr(int iNum, int nibble){			//16進位數===>字串 
	
	std::stringstream ss;
	
	if(iNum < 100 && nibble == 2)
		ss << setfill('0') << setw(3) << uppercase << std::hex << iNum;
	else if(nibble == 2)
		ss << setfill('0') << setw(2) << uppercase << std::hex << iNum;
	else if(nibble == 1)
		ss << uppercase << std::hex << iNum;
	else if(nibble == 3)
		ss << setfill('0') << setw(2) << uppercase << std::hex << iNum;
	
	return ss.str();
}

string IntToStr(int iNum){			//整數(10進位)===>字串 
	
	std::stringstream ss;
	ss << std::dec << iNum;
	
	return ss.str();
}

string HexCharToBinStr(char cNum){			//16進位字元===>2進位字串 
	//string sNum = HexToStr(iNum);
	string opcode = "\0";
	if(cNum == '0')
		opcode = "0000"; 
	else if(cNum == '1')
		opcode = "0001";
	else if(cNum == '2')
		opcode = "0010";
	else if(cNum == '3')
		opcode = "0011";
	else if(cNum == '4')
		opcode = "0100";
	else if(cNum == '5')
		opcode = "0101";
	else if(cNum == '6')
		opcode = "0110";
	else if(cNum == '7')
		opcode = "0111";
	else if(cNum == '8')
		opcode = "1000";
	else if(cNum == '9')
		opcode = "1001";
	else if(cNum == 'A' || cNum == 'a')
		opcode = "1010";
	else if(cNum == 'B' || cNum == 'b')
		opcode = "1011";
	else if(cNum == 'C' || cNum == 'c')
		opcode = "1100";
	else if(cNum == 'D' || cNum == 'd')
		opcode = "1101";
	else if(cNum == 'E' || cNum == 'e')
		opcode = "1110";
	else if(cNum == 'F' || cNum == 'f')
		opcode = "1111";
		
	return opcode;
		
}

int BinStrToHex(string sNum){			//2進位字串===>16進位數 
	int iNum = 0x0;
	if(stricmp(sNum.c_str(), "0000") == 0)
		iNum = 0x0; 
	else if(stricmp(sNum.c_str(), "0001") == 0)
		iNum = 0x1;
	else if(stricmp(sNum.c_str(), "0010") == 0)
		iNum = 0x2;
	else if(stricmp(sNum.c_str(), "0011") == 0)
		iNum = 0x3;
	else if(stricmp(sNum.c_str(), "0100") == 0)
		iNum = 0x4;
	else if(stricmp(sNum.c_str(), "0101") == 0)
		iNum = 0x5;
	else if(stricmp(sNum.c_str(), "0110") == 0)
		iNum = 0x6;
	else if(stricmp(sNum.c_str(), "0111") == 0)
		iNum = 0x7;
	else if(stricmp(sNum.c_str(), "1000") == 0)
		iNum = 0x8;
	else if(stricmp(sNum.c_str(), "1001") == 0)
		iNum = 0x9;
	else if(stricmp(sNum.c_str(), "1010") == 0)
		iNum = 0xA;
	else if(stricmp(sNum.c_str(), "1011") == 0)
		iNum = 0xB;
	else if(stricmp(sNum.c_str(), "1100") == 0)
		iNum = 0xC;
	else if(stricmp(sNum.c_str(), "1101") == 0)
		iNum = 0xD;
	else if(stricmp(sNum.c_str(), "1110") == 0)
		iNum = 0xE;
	else if(stricmp(sNum.c_str(), "1111") == 0)
		iNum = 0xF;
		
	return iNum;
		
}

void setObjCodeSIC(){			//設定SIC的objcode 
	bool found = false;
	for(int i = 0; i < sSet.size(); i++){
		if(!sSet[i].conts.empty()){
			for(int j = 0; j < sSet.size(); j++){
				if((! sSet[i].isPesudo) && (strcmp(sSet[i].conts[0].c_str(), sSet[j].label.c_str()) == 0)){			
					if(sSet[i].isIndexReg){			//使用index reg							
						sSet[i].objCode = HexToStr(sSet[i].opcode, 3) + HexToStr(sSet[j].loc + 0x8000, 2);
					}
					else	
						sSet[i].objCode = HexToStr(sSet[i].opcode, 3) + HexToStr(sSet[j].loc, 2);
					found = true;
					break;
				}//if
			}//for	
			if((! sSet[i].isPesudo) && (! sSet[i].isLTORG) && (! found)){			//呼叫不存在的label
			     sSet[i].syntaxError = true;
			}  
			else	found = false;
		}//if
		else if((! sSet[i].isPesudo) && (! sSet[i].isLTORG)){		//未使用label的指令 
			sSet[i].objCode = HexToStr(sSet[i].opcode, 3) + "0000";
		}//else
	}//for	
}

void setObjCodeSICXE(){			//設定SIC/XE的objcode 
	for(int i = 0; i < sSet.size(); i++){
		if(sSet[i].format == 1){		// format 1
			sSet[i].objCode = HexToStr(sSet[i].opcode, 3);
		}
		else if(sSet[i].format == 2){			// format 2
			string numR1 = "\0", numR2 = "\0";
			for(int n = 0; n < sSet[i].conts.size(); n++){
				for(int k = 0; k < tSetReg.size(); k++){				//此token屬於Register Table
					if(stricmp(tSetReg[k].cont.c_str(), sSet[i].conts[n].c_str()) == 0){
						if(numR1 == "\0")
							numR1 = IntToStr(tSetReg[k].num-1);
						else
							numR2 = IntToStr(tSetReg[k].num-1);
						
					}//if		
				}//for
			}//for
			if(numR1 == "\0" && numR2 == "\0"){		//呼叫不存在的reg 
				sSet[i].syntaxError = true;
			}  
			else{
				if(numR2 == "\0")	numR2 = "0";
				sSet[i].objCode = HexToStr(sSet[i].opcode, 3) + numR1 + numR2;
			}						
		}
		else if((sSet[i].format == 3) || (sSet[i]).format == 4){ // format == 3/4	
			format34 type;
			string opcodeBin = "0", tempHex = "\0", address = "\0";
			string objTemp1 = "\0", objTemp2 = "\0", objTemp3 = "\0";
				
			if(!sSet[i].conts.empty() || stricmp(sSet[i].instr.c_str(), "RSUB") == 0){
				tempHex = HexToStr(sSet[i].opcode, 3);
				opcodeBin = HexCharToBinStr(tempHex[0]) + HexCharToBinStr(tempHex[1]);
				
				if(sSet[i].isImmediate && !sSet[i].isIndirect){		//immiediat mode
					type.n = "0";
					type.i = "1";
				}
				else if(!sSet[i].isImmediate && sSet[i].isIndirect){	//indirect mode
					type.n = "1";
					type.i = "0";
				}
				else if(sSet[i].isImmediate && sSet[i].isIndirect){		//同時有"#"和"@"符號 
					sSet[i].syntaxError = true;
				}
				else{		//	SIC/XE type
					type.n = "1";
					type.i = "1";
				}
				
				if(sSet[i].isIndexReg)	type.x = "1";
				else	type.x = "0";
					
				if(sSet[i].format == 3){		//format == 3
					type.e = "0";
						
					if(!sSet[i].conts.empty() && isNum(sSet[i].conts[0]) && !sSet[i].isLTORG){			//type 3__one
						type.p = "0";   
						type.b = "0"; 
						string temp = "\0", dispNum = "\0";
						temp = HexToStr(StrToHex(sSet[i].conts[0]), 2);
						for(int w = 0; w < (3 - temp.length()); w++){
							dispNum = dispNum + "0";
						}
						dispNum = dispNum + temp;
						for(int r = 0; r < 4; r++)	objTemp1 += opcodeBin[r];
						for(int r = 4; r < 6; r++)	objTemp2 += opcodeBin[r];
						objTemp2 = objTemp2 + type.n +  type.i;
						objTemp3 = type.x + type.b + type.p + type.e;
						objTemp1 = HexToStr(BinStrToHex(objTemp1), 1);
						objTemp2 = HexToStr(BinStrToHex(objTemp2), 1);
						objTemp3 = HexToStr(BinStrToHex(objTemp3), 1);
						sSet[i].objCode = objTemp1 + objTemp2 + objTemp3 + dispNum;
					}//type 3__one
					else if(stricmp(sSet[i].instr.c_str(), "RSUB") == 0){
						type.p = "0";   
						type.b = "0"; 

						for(int r = 0; r < 4; r++)	objTemp1 += opcodeBin[r];
						for(int r = 4; r < 6; r++)	objTemp2 += opcodeBin[r];
						objTemp2 = objTemp2 + type.n +  type.i;
						objTemp3 = type.x + type.b + type.p + type.e;
						objTemp1 = HexToStr(BinStrToHex(objTemp1), 1);
						objTemp2 = HexToStr(BinStrToHex(objTemp2), 1);
						objTemp3 = HexToStr(BinStrToHex(objTemp3), 1);
						sSet[i].objCode = objTemp1 + objTemp2 + objTemp3 + "000";
					}
					else{					//type 3__two
						type.p = "1";   
						type.b = "0";   
						int disp = 0x0, labelLoc = 0x0;
						bool found = false;
						for(int j = 0; j < sSet.size(); j++){
							if((! sSet[i].isPesudo) && (strcmp(sSet[i].conts[0].c_str(), sSet[j].label.c_str()) == 0)){			
								labelLoc = sSet[j].loc;	
								found = true;		
								break;
							}//if
						}//for
						
						if((! sSet[i].isPesudo) && (! found)){
						 sSet[i].syntaxError = true;			//出現使用未宣告的label 
					   }
						else{
							disp = labelLoc - sSet[i+1].loc;
							int testDisp = disp;
							if(disp < 0) disp = disp + 0x1000;
							
							if((testDisp < -2048) || (testDisp > 2047)){		//program counter超出範圍，改用base reg 
								type.p = "0";    
								type.b = "1";   
								string baseLabel = "\0";
								int baseLoc = 0x0;
								for(int w = 0; w < sSet.size(); w++){
									if(stricmp("BASE", sSet[w].instr.c_str()) == 0){			
										baseLabel = sSet[w].conts[0];				
										break;
									}//if
								}//for
								
								for(int h = 0; h < sSet.size(); h++){
									if(strcmp(baseLabel.c_str(), sSet[h].label.c_str()) == 0){			
										baseLoc = sSet[h].loc;				
										break;
										}//if
									}//for
							
								disp = labelLoc - baseLoc;
							
								if((disp < 0) || (disp > 4095)){
									sSet[i].syntaxError;	//超出pc、base reg的位址範圍 
						        }
						        
								if(disp < 0) disp = disp + 0x1000;
							}// use base reg
							
							for(int r = 0; r < 4; r++)	objTemp1 += opcodeBin[r];
							for(int r = 4; r < 6; r++)	objTemp2 += opcodeBin[r];
							objTemp2 = objTemp2 + type.n +  type.i;
							objTemp3 = type.x + type.b + type.p + type.e;
							objTemp1 = HexToStr(BinStrToHex(objTemp1), 1);
							objTemp2 = HexToStr(BinStrToHex(objTemp2), 1);
							objTemp3 = HexToStr(BinStrToHex(objTemp3), 1);
							sSet[i].objCode = objTemp1 + objTemp2 + objTemp3 + HexToStr(disp, 2);							
						}// is found					
					}//type 3__two
				}// format == 3
				else if(sSet[i].format == 4){		// format 4
					type.e = "1";
					type.b = "0";
					type.p = "0";
					for(int r = 0; r < 4; r++)	objTemp1 += opcodeBin[r];
					for(int r = 4; r < 6; r++)	objTemp2 += opcodeBin[r];
					objTemp2 = objTemp2 + type.n +  type.i;
					objTemp3 = type.x + type.b + type.p + type.e;
					objTemp1 = HexToStr(BinStrToHex(objTemp1), 1);
					objTemp2 = HexToStr(BinStrToHex(objTemp2), 1);
					objTemp3 = HexToStr(BinStrToHex(objTemp3), 1);
					if(!sSet[i].conts.empty() && isNum(sSet[i].conts[0])){
						string temp = "\0", dispNum = "\0";
						temp = HexToStr(StrToHex(sSet[i].conts[0]), 2);
						for(int w = 0; w < (5 - temp.length()); w++){
							dispNum = dispNum + "0";
						}
						dispNum = dispNum + temp;
						
						sSet[i].objCode = objTemp1 + objTemp2 + objTemp3 + dispNum;
					}
					else if(stricmp(sSet[i].instr.c_str(), "RSUB") == 0){
						sSet[i].objCode = objTemp1 + objTemp2 + objTemp3 + "00000";
					}
					else{
						bool found = false;
						for(int j = 0; j < sSet.size(); j++){
							if((! sSet[i].isPesudo) && (strcmp(sSet[i].conts[0].c_str(), sSet[j].label.c_str()) == 0)){			
								address = HexToStr(sSet[j].loc, 2);	
								found = true;		
								break;
							}//if
						}//for
						
						if((! sSet[i].isPesudo) && (! found)){		//呼叫不存在的label 
							sSet[i].syntaxError = true;
						}  
						else{
							string tempAD = "\0";
							for(int k = 0; k < 5 - address.length(); k++)	tempAD = tempAD + "0";
							address = tempAD + address;
							sSet[i].objCode = objTemp1 + objTemp2 + objTemp3 + address;					
						}// is found					
					}//else										
				}//else format == 4 					
			}//if
			else if(! sSet[i].isComment){
				sSet[i].syntaxError = true;
			}//else
		}// esle format == 3/4		
	}//for			
}

void setLoc(){			//設定行數 
	int currentLoc = 0x0;
	for(int i = 0; i < sSet.size(); i++){
		if(!sSet[i].isComment){
			if(stricmp(sSet[i].instr.c_str(), "START") == 0){		//根據START的初始位置來判斷是SIC or SIC/XE 
				currentLoc = strtol(sSet[i].conts[0].c_str(), NULL, 16);
				sSet[i].loc = currentLoc;
				if(currentLoc == 0x1000){
					isSIC = true;
				}	
				else{
					isSICXE = true;
				}
					
			}
			else if(stricmp(sSet[i].instr.c_str(), "EQU") == 0){
					if(sSet[i].hasOper){
						int oper1, oper2;
						for(int q = 0; q < sSet.size(); q++){
							if(strcmp(sSet[q].label.c_str(), sSet[i].conts[0].c_str()) == 0)
								oper1 = sSet[q].loc;
							else if(strcmp(sSet[q].label.c_str(), sSet[i].conts[1].c_str()) == 0)
								oper2 = sSet[q].loc;
						}
						
						if(strcmp(sSet[i].contOper.c_str(), "-") == 0)
							sSet[i].loc = oper1 - oper2;
						else if(strcmp(sSet[i].contOper.c_str(), "+") == 0)
							sSet[i].loc = oper1 + oper2;
						else if(strcmp(sSet[i].contOper.c_str(), "*") == 0)
							sSet[i].loc = oper1 * oper2;
						else if(strcmp(sSet[i].contOper.c_str(), "/") == 0)
							sSet[i].loc = oper1 / oper2;
					}
					else if(!sSet[i].conts.empty() && isNum(sSet[i].conts[0]) && !sSet[i].isLTORG){
						currentLoc = StrToHex(sSet[i].conts[0]);
						sSet[i].loc = currentLoc;
						currentLoc = currentLoc + 0x3;
					}
					else{
						sSet[i].loc = currentLoc;
					}		 
			}
			else{
				sSet[i].loc = currentLoc;
				if(stricmp(sSet[i].instr.c_str(), "BYTE") == 0){
					int num = sSet[i].objCode.length() / 2;
					if(num == 1)
						currentLoc = currentLoc + 0x1;
					else if(num == 2)
						currentLoc = currentLoc + 0x2;
					else if(num == 3)
						currentLoc = currentLoc + 0x3;
					else //if(num == 4)
						currentLoc = currentLoc + 0x4;
				}
				else if(stricmp(sSet[i].instr.c_str(), "WORD") == 0){
					currentLoc = currentLoc + 0x3;
					for(int n = 0; n < (6-HexToStr(StrToHex(sSet[i].conts[0]), 2).length()); n++){
						sSet[i].objCode = sSet[i].objCode + "0";
					}
					sSet[i].objCode = sSet[i].objCode + HexToStr(StrToHex(sSet[i].conts[0]), 2);
				}
				else if(stricmp(sSet[i].instr.c_str(), "RESB") == 0){
					currentLoc = currentLoc + StrToHex(sSet[i].conts[0]);
				}
				else if(stricmp(sSet[i].instr.c_str(), "RESW") == 0){
					currentLoc = currentLoc + 0x3;
				}
				else if(isSICXE){
					if(sSet[i].format == 1) currentLoc = currentLoc + 0x1;
					else if(sSet[i].format == 2) currentLoc = currentLoc + 0x2;
					else if(sSet[i].format == 3) currentLoc = currentLoc + 0x3;
					else if(sSet[i].format == 4) currentLoc = currentLoc + 0x4;
					else if((sSet[i].isLTORG) && (stricmp(sSet[i].instr.c_str(), "LTORG") != 0) && (stricmp(sSet[i].instr.c_str(), "END") != 0)){
						int num = sSet[i].objCode.length() / 2;	//處理C'' & X'' 
						if(num == 1)
							currentLoc = currentLoc + 0x1;
						else if(num == 2)
							currentLoc = currentLoc + 0x2;
						else if(num == 3)
							currentLoc = currentLoc + 0x3;
						else //if(num == 4)
							currentLoc = currentLoc + 0x4;
					}
				}
				else{
					currentLoc = currentLoc + 0x3; 
				}//else
			
			}//else
		}//if 此行不是註解 	
	}//for
}

void CheckError(){			//檢查敘述用的label數是否和該指令用的label數相同 
	for(int i = 0; i < sSet.size(); i++){
		for(int w = 0; w < tSetInstr.size(); w++){				//此token屬於Instruction Table
			if(stricmp(tSetInstr[w].instr.c_str(), sSet[i].instr.c_str()) == 0){
				if((sSet[i].conts.size() != tSetInstr[w].contNum) && (! sSet[i].isIndexReg)){
					sSet[i].syntaxError = true;
				}	
			}	
		}
	}
}

int main(void)
{   introMsg();                         //initiate an opening message
	InputTable();						//讀入table檔案
	do
	{	resetList(tSetSym); 			//初始化雜湊表 
  		resetList(tSetIR); 
  		resetList(tSetStr);  
		cout << "##################################################" << endl;
    	cout << " Step 2: SIC & SIC/XE Assembler  "                  << endl;
    	cout << "##################################################" << endl << endl;
//********************************************************************************************/
        try
        {	if(ReadWriteFile())			//是否成功讀input檔、寫output檔 
				cout << endl << "=== The output file has been created!! ===" << endl;

			setLoc();
			CheckError();
			if(isSIC){
				setObjCodeSIC();			
			}
			else{	//is SIC/XE
				setObjCodeSICXE();
			}
			fileOut << "Line\tLoc\tSource statement" << setw(10) << "\t\t" << "Object code\n\n";
			for(int i = 0; i < sSet.size(); i++){
				if((sSet[i].isComment) || (stricmp(sSet[i].instr.c_str(), "END") == 0) || (stricmp(sSet[i].instr.c_str(), "BASE") == 0) || (stricmp(sSet[i].instr.c_str(), "LTORG") == 0)){
					fileOut << dec << sSet[i].line << "\t\t" << sSet[i].statment;
					if(sSet[i].lexicalError) fileOut << "\t\t" << left << setw(10) << "Lexical Error" << endl; 
					else if(sSet[i].syntaxError) fileOut << "\t\t" << left << setw(10) << "Syntax Error" << endl;  
					else fileOut << endl;
				}
				else{
					if(sSet[i].addLTORG)
						fileOut << "\t";
					else
						fileOut << dec << sSet[i].line << "\t";
						
					if(isSICXE){
						if(sSet[i].loc < 0x10)
							fileOut << "000" << setiosflags(ios::uppercase) << hex << sSet[i].loc << "\t";
						else if(sSet[i].loc > 0x999)
							fileOut << setiosflags(ios::uppercase) << hex << sSet[i].loc << "\t";
						else
							fileOut << "00" << setiosflags(ios::uppercase) << hex << sSet[i].loc << "\t";
					}	
					else		
						fileOut << setiosflags(ios::uppercase) << hex << sSet[i].loc << "\t";
						
					fileOut << left << setw(20) << sSet[i].statment << "\t\t" << left << setw(10);
					if(sSet[i].lexicalError) fileOut << "Lexical Error" << endl; 
					else if(sSet[i].syntaxError) fileOut << "Syntax Error" << endl;  
					else fileOut << sSet[i].objCode << endl;
				}	
										
			}
			fileOut.close();		
			sSet.clear();	
			currentLine = 5;
			isSIC = false, isSICXE = false;	
            cout << endl << endl << "[0]Quit or [Any other]continue?" << endl;
            string quitOrNot;
			cin >> quitOrNot;
			if (!quitOrNot.compare("0")) // press 0 to stop execution
				return 0;
        }	// end try
//********************************************************************************************/
        catch (std::bad_alloc& ba)								            // unable to allocate dynamic space
        {   std::cerr << endl << "bad_alloc caught: " << ba.what() << endl;
        }   // end catch
	}while(1);
}	// end main
//********************************************************************************************/
