
#include "precompiled.pch"
#include <cstdlib>
#include <boost/timer/timer.hpp>

using namespace boost;
using namespace std;

#include "common-defs.h"
#include "asmc.h"
#include "dta.h"
#include "dspn.h"
#include "cslta.h"

#ifdef WIN32
#  include <windows.h>
#  include <io.h>
#  include <fcntl.h>
#endif


//-----------------------------------------------------------------------------

#ifdef WIN32
const char* BinaryLogicOp_Str[] = {
	"AND", "OR", "IMPLY"
};
const char* UnaryLogicOp_Not = "NOT ";
#else
const char* BinaryLogicOp_Str[] = {
	"∧", "∨", "->"
};
const char* UnaryLogicOp_Not = "¬";
#endif


//-----------------------------------------------------------------------------

GlobalStatus  g_status;

GlobalStatus::GlobalStatus() {
	writeMathKernelOutput = false;
	extSolverUsed = EDS_none;
}

GlobalStatus::~GlobalStatus() {
	safe_delete_ptr_mapvals(mcobjects);
	safe_delete(g_status.dtaCCNamOrds);
	safe_delete(g_status.dtaActs);
	safe_delete(g_status.dtaSPExprs);
}

Pipe& GlobalStatus::GetMathKernel() {
	// Open the kernel if it's the first time
	if (!mathKernel) {
		// Open the MathKernel log, if set
		FILE *outRdLog=NULL, *outWrLog=NULL;
		if (writeMathKernelOutput) {
			outRdLog = fopen(mathRdLogFile.c_str(), "w");
			outWrLog = fopen(mathWrLogFile.c_str(), "w");
		}
		
		// Open the MathKernel and issue the library loading commands
		if (!mathKernel.Open((format("%1% -noprompt") 
							  % mathKernelName).str().c_str(),
							 "\n\nExxit[];\n", outRdLog, outWrLog)) // TODO ripristinare Exit
		{
			cout << "Couldn't start the MathKernel "<<mathKernelName<<endl;
			exit(-1);
		}
		
		// Test if the libraries have been already loaded
		double alreadyLoaded = 0;
		mathKernel.Write("Print[If[NameQ[\"RrgToDot\"],1,-1]];\n");
		mathKernel.ReadDouble(&alreadyLoaded);
		if (alreadyLoaded < 0) {
			const char *libraries[] = {
				"<< SPNica`Master` \n",
				"<< SPNica`Expolynomials` \n",
				"<< SPNica`ProveSPN` \n",
				"<< SPNica`MarkovChainAnalysis` \n"
			};
			const size_t NUM_LIBS = sizeof(libraries) / sizeof(libraries[0]);
			
			cout << "Loading MathKernel libraries... " << flush;
			for (size_t l=0; l<NUM_LIBS; l++) {
				mathKernel.Write(libraries[l]);
				mathKernel.Write("Print[LIBLOADED];\n");
				cout << (l+1) << "/" << NUM_LIBS << "  " << flush;
				if (!mathKernel.SkipUntil("LIBLOADED")) {
					cout << "The mathKernel exited unexpectedly." << endl;
					exit(-1);
				}
			}
			cout << "done!" << endl;
		}
	}
	
	return mathKernel;
}

string GlobalStatus::GetOutputFileName(const char* name) const
{
	ostringstream outfn;
	if (outputDir.size() > 0) {
		outfn << outputDir;
		if (*outputDir.rend() != DIRSEP)
			outfn << DIRSEP;
	}
	outfn << name;
	return outfn.str();
}




//-----------------------------------------------------------------------------

bool SymbolTable::HasSymbol(const string& name) const
{
	return (find(symbols.begin(), symbols.end(), name) != symbols.end());
}

SymbolID SymbolTable::GetIDFromName(const string& name) const
{
	vector<string>::const_iterator it;
	it = find(symbols.begin(), symbols.end(), name);
	assert (it != symbols.end());
	return (it - symbols.begin());
}

SymbolID SymbolTable::AddNewSymbol(const string& name) {
	assert(!HasSymbol(name));
	SymbolID symId = symbols.size();
	symbols.push_back(name);
	return symId;
}

const string& SymbolTable::GetSymbolName(SymbolID symId) const
{
	assert(symId < symbols.size());
	return symbols[symId];
}

void SymbolTable::RemapSymbolIDs(const vector<size_t>& RemapTable) 
{
	assert(RemapTable.size() == NumSymbols());
	vector<string> remapped(NumSymbols());
	
	for (size_t i=0; i<RemapTable.size(); i++)
		remapped[i] = symbols[ RemapTable[i] ];
	
	symbols.swap(remapped);
}

void SymbolTable::operator = (const SymbolTable& st)
{
	symbols = st.symbols;
}

ostream& operator << (ostream& os, const SymbolTable& sm)
{
	size_t count = 0;
	os << "{";
	vector<string>::const_iterator it;
	for (it = sm.symbols.begin(); it != sm.symbols.end(); ++it) {
		os << (count++ == 0 ? " " : ", ") << *it;
	}
	os << " }";
	return os;
}







//-----------------------------------------------------------------------------

string htmlize_utf8_str(const string & s)
{
	const char hex[] = "0123456789ABCDEF";
	ostringstream out;
	string::const_iterator it = s.begin();
	
	while (it != s.end()) {
		if ((*it & 0x80) == 0)               // ASCII 7-bit character
			out << *it;
		else {
			int accum;
			if ((*it & 0xE0) == 0xC0) {      // UTF-8 2-byte wide character
				accum = (*it & 0x1F) << 6;
				it++;
				accum |= (*it & 0x3F);
			}
			else if ((*it & 0xF0) == 0xE0) { // UTF-8 3-byte wide character
				accum = (*it & 0x0F) << 12;
				it++;
				accum |= (*it & 0x3F) << 6;
				it++;
				accum |= (*it & 0x3F);
			}
			else continue;
			
			out << "&#x";
			out << hex[(accum & 0xF000) >> 12];
			out << hex[(accum & 0x0F00) >> 8];
			out << hex[(accum & 0x00F0) >> 4];
			out << hex[(accum & 0x000F)];
			out << ";";
		}
		++it;
	}
	return out.str();
}





//-----------------------------------------------------------------------------

#ifdef WIN32

struct WINPOPEN_T {
	HANDLE	 pipein[2], pipeout[2], pipeerr[2];
	FILE    *fin, *fout;
};
typedef struct WINPOPEN_T  WINPOPEN_T;


static bool create_winpipe(HANDLE *readwrite)
{
	SECURITY_ATTRIBUTES sa;

	sa.nLength = sizeof(sa);
	sa.bInheritHandle = 1;
	sa.lpSecurityDescriptor = NULL;
	if (!CreatePipe(&readwrite[0], &readwrite[1], &sa, 1 << 13))
		return false;

	return true;
}


WINPOPEN_T* winpopen(const char *cmd)
{
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFOA siStartInfo;
	int success;

	WINPOPEN_T* wpt = new WINPOPEN_T;
	memset(wpt, 0, sizeof(*wpt));
	wpt->pipein[0]   = INVALID_HANDLE_VALUE;
	wpt->pipein[1]   = INVALID_HANDLE_VALUE;
	wpt->pipeout[0]  = INVALID_HANDLE_VALUE;
	wpt->pipeout[1]  = INVALID_HANDLE_VALUE;
	wpt->pipeerr[0]  = INVALID_HANDLE_VALUE;
	wpt->pipeerr[1]  = INVALID_HANDLE_VALUE;

	if (!create_winpipe(wpt->pipein) ||
		!create_winpipe(wpt->pipeout) ||
		!create_winpipe(wpt->pipeerr))
		goto finito;


	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb           = sizeof(STARTUPINFO);
	siStartInfo.hStdInput    = wpt->pipein[0];
	siStartInfo.hStdOutput   = wpt->pipeout[1];
	siStartInfo.hStdError    = wpt->pipeerr[1];
	siStartInfo.dwFlags	     = STARTF_USESTDHANDLES;

	success = CreateProcessA(NULL,
							(LPSTR)cmd,		   // command line 
							NULL,              // process security attributes 
							NULL,              // primary thread security attributes 
							TRUE,              // handles are inherited 
							DETACHED_PROCESS,  // creation flags: Ohne Fenster (?)
							NULL,              // use parent's environment 
							NULL,              // use parent's current directory 
							&siStartInfo,      // STARTUPINFO pointer 
							&piProcInfo);      // receives PROCESS_INFORMATION 
	if (!success) {
		cout << "Couldn't open the Pipe\n cmdline="<<cmd<<endl;
		if (GetLastError()==2)
			cout << " File Not Found." << endl;
	}
	if (!success)
		goto finito;

	CloseHandle(piProcInfo.hThread);
	CloseHandle(piProcInfo.hProcess);

	CloseHandle(wpt->pipein[0]);  wpt->pipein[0]  = INVALID_HANDLE_VALUE;
	CloseHandle(wpt->pipeout[1]); wpt->pipeout[1] = INVALID_HANDLE_VALUE;
	CloseHandle(wpt->pipeerr[1]); wpt->pipeerr[1] = INVALID_HANDLE_VALUE;

	wpt->fin  = _fdopen(_open_osfhandle((long)wpt->pipeout[0], _O_TEXT), "r");
	wpt->fout = _fdopen(_open_osfhandle((long)wpt->pipein[1],  _O_TEXT), "w");

finito:
	if (!wpt->fin || ! wpt->fout)
	{
		if (wpt->pipein[0]  != INVALID_HANDLE_VALUE)
			CloseHandle(wpt->pipein[0]);
		if (wpt->pipein[1]  != INVALID_HANDLE_VALUE)
			CloseHandle(wpt->pipein[1]);
		if (wpt->pipeout[0] != INVALID_HANDLE_VALUE)
			CloseHandle(wpt->pipeout[0]);
		if (wpt->pipeout[1] != INVALID_HANDLE_VALUE)
			CloseHandle(wpt->pipeout[1]);
		if (wpt->pipeerr[0] != INVALID_HANDLE_VALUE)
			CloseHandle(wpt->pipeerr[0]);
		if (wpt->pipeerr[1] != INVALID_HANDLE_VALUE)
			CloseHandle(wpt->pipeerr[1]);

		delete wpt;
		wpt = NULL;
	}
	return wpt;
}


bool winpclose(WINPOPEN_T *wpt) 
{
	if (wpt == NULL)
		return false;

	fclose(wpt->fin);
	fclose(wpt->fout);

	//CloseHandle(wpt->pipein[1]);  wpt->pipein[1]  = INVALID_HANDLE_VALUE;
	//CloseHandle(wpt->pipeout[0]); wpt->pipeout[0] = INVALID_HANDLE_VALUE;
	CloseHandle(wpt->pipeerr[0]); wpt->pipeerr[0] = INVALID_HANDLE_VALUE;

	delete wpt;
	return true;
}


#endif  // WIN32


//-----------------------------------------------------------------------------
Pipe::Pipe() {
	exitStmt = NULL;
#ifdef WIN32
	wpt = NULL;
#else
	pfile = NULL;
#endif
}
Pipe::~Pipe() {  Close();  }

bool Pipe::Open(const char *cmd, const char *_exitStmt,
				FILE *_outRd, FILE *_outWr) 
{
	Close();
	exitStmt = _exitStmt;
	outRd = _outRd;
	outWr = _outWr;
#ifdef WIN32
	wpt = winpopen(cmd);
	return (wpt != NULL);
#else
	pfile = popen(cmd, "r+");
	return (pfile != NULL);
#endif
}

void Pipe::Close() 
{
#ifdef WIN32
	if (wpt != NULL) {
		Write(exitStmt);
		winpclose(wpt);
		wpt = NULL;
	}
#else
	if (pfile != NULL) {
		Write(exitStmt);
		pclose(pfile);
		pfile = NULL;
	}
#endif
	if (outRd != NULL) {
		fclose(outRd);
		outRd = NULL;
	}
	if (outWr != NULL) {
		fclose(outWr);
		outWr = NULL;
	}
}

Pipe::operator bool () const {
#ifdef WIN32
	return (wpt != NULL);
#else
	return (pfile != NULL);
#endif
}

bool Pipe::Write(const char* str) {
#ifdef WIN32
	if (wpt==NULL || fprintf(wpt->fout, "%s", str) < 0)
#else
	if (pfile==NULL || fprintf(pfile, "%s", str) < 0)
#endif
	{
		Close();
		return false;
	}
#ifdef WIN32
	fflush(wpt->fout);
#else
	fflush(pfile);
#endif
	if (outWr != NULL) {
		fprintf(outWr, "%s", str);
		fflush(outWr);
	}
	return true;
}

bool Pipe::ReadLine(char* line, size_t max, bool skipEmpty) {
	do {
#ifdef WIN32
		if (NULL == fgets(line, max, wpt->fin)) 
#else
		if (NULL == fgets(line, max, pfile)) 
#endif
		{
			Close();
			return false;
		}
		if (outRd != NULL) {
			fprintf(outRd, "%s", line);
			fflush(outRd);
		}
		size_t len = strlen(line);
		if (skipEmpty && len<=1)
			continue; // read another line
		if (len > 0 && line[len-1]=='\n')
			line[len-1] = '\0';
		return true;
	} while(1);
}

bool Pipe::ReadDouble(double *pVal) {
	const int MAXBUF = 256;
	char buffer[MAXBUF+1];
	if (!ReadLine(buffer, MAXBUF, true))
		return false;
	int expN;
	int numRd = sscanf(buffer, "%lf*^%d", pVal, &expN);
	if (numRd == 2)
		*pVal *= pow(10.0, (double)expN);
	
	return (numRd>0);
}

bool Pipe::SkipUntil(const char* endLine) {
	const int MAXBUF = 256;
	char buffer[MAXBUF+1];
	do {
		if (!ReadLine(buffer, MAXBUF, false))
			return false;
	} while(0 != strcmp(buffer, endLine));
	return true;
}












//-----------------------------------------------------------------------------
PUBLIC_API Constant* Constant_New(const char *v)
{
	return new Constant(v);
}

PUBLIC_API int Constant_IsDefined(const char *name)
{
	if (GS_RetrieveMCObjectByName(name, MCOT_CONSTANT) != NULL)
		return 1;
	return 0;
}

PUBLIC_API char* Constant_GetOrErr(const char *name)
{
	// Find the constant named *name
	Constant *c = (Constant*)GS_RetrieveMCObjectByName(name, MCOT_CONSTANT);
	if (c == NULL) {
		g_status.lastError = (format("Constant named \"%1%\" doesn't exist.") % name).str();
		return NULL;
	}
	
	return strdup(c->value.c_str());
}








//-----------------------------------------------------------------------------

PUBLIC_API void IDList_Delete(IDList *idl) {
	delete idl;
}
PUBLIC_API IDList* IDList_New() {
	return new IDList;
}
PUBLIC_API IDList* IDList_AddID(IDList *idl, const char *newID) {
	string nID(newID);
	idl->idList.push_back(nID);
	return idl;
}
PUBLIC_API IDList* IDList_AddUniqueID(IDList *idl, const char *newID) {
	string nID(newID);
	if (idl->idList.end() != find(idl->idList.begin(), idl->idList.end(), nID)) {
		cout << "Warning: identifier "<<nID<<" has been repeated." << endl;
		return idl;
	}
	idl->idList.push_back(nID);
	return idl;
}


PUBLIC_API void IDValList_Delete(IDValList *idvl) {
	delete idvl;
}
PUBLIC_API IDValList* IDValList_New() {
	return new IDValList;
}
PUBLIC_API IDValList* IDValList_AddIDVal(IDValList *idvl, const char *newID, 
										 const char *val) 
{
	idvl->idValList.push_back(IdValuePair(newID, atoi(val)));
	return idvl;
}
PUBLIC_API IDValList* IDValList_AddUniqueIDVal(IDValList *idvl, const char *newID, 
											   const char *val) 
{
	list<IdValuePair>::const_iterator iter;
	for (iter = idvl->idValList.begin(); iter != idvl->idValList.end(); ++iter)
		if (iter->ID == newID) {
			cout << "Warning: identifier "<<newID<<" has been repeated." << endl;
			return idvl;
		}

	idvl->idValList.push_back(IdValuePair(newID, atoi(val)));
	return idvl;
}
	
	





//-----------------------------------------------------------------------------

const char* MCOT_Names[] = {
	"ASMC", "DTA", "RESULT", "CONSTANT",
	"All Types"
};

PUBLIC_API void Exec_StoreMCObject(MCObject *obj, const char *name)
{
	// Object name must be unique
	if (GS_RetrieveMCObjectByName(name, MCOT_ALL_TYPES) != NULL) {
		g_status.lastError = (format("Object name \"%1%\" already used.") % name).str();
		delete obj;
		return;
	}
	// Object should be valid
	string reason;	
	if (!obj->isValidObject(reason)) {
		g_status.lastError = (format("Object \"%1%\" is not valid.\n  Reason: %2%\n") 
							  % name % reason).str();
		delete obj;
		return;
	}
	
	// Save the object in the global namespace
	g_status.mcobjects[string(name)] = obj;
	cout << MCOT_Names[obj->GetObjectType()];
	cout << " instance \""<<name<<"\" loaded." << endl;
}


PUBLIC_API void Exec_ClearMCObject(const char *name)
{
	MCObject *mcobj = GS_RetrieveMCObjectByName(name, MCOT_ALL_TYPES);
	if (mcobj != NULL) {
		g_status.mcobjects.erase(string(name));
		cout << MCOT_Names[mcobj->GetObjectType()];
		cout << " instance \""<<name<<"\" cleared." << endl;	
		delete mcobj;
	}
	else {
		cout << "There is no object named \""<<name<<"\"." << endl;
	}
}

PUBLIC_API void Exec_RenameMCObject(const char *oldname, const char *newname)
{
	// Object name must be unique
	if (GS_RetrieveMCObjectByName(newname, MCOT_ALL_TYPES) != NULL) {
		g_status.lastError = (format("Object name \"%1%\" already used.") % newname).str();
		return;
	}
	MCObject *mcobj = GS_RetrieveMCObjectByName(oldname, MCOT_ALL_TYPES);
	if (mcobj != NULL) {
		g_status.mcobjects.erase(string(oldname));
		cout << MCOT_Names[mcobj->GetObjectType()];
		cout << " instance \""<<oldname<<"\" renamed to \"" << newname << "\"." << endl;	
		g_status.mcobjects[string(newname)] = mcobj;
	}
	else {
		cout << "There is no object named \""<<oldname<<"\"." << endl;
	}
}



PUBLIC_API void Exec_SaveMCObjectAsGml(const char *name, const char *filename,
									   const char *args)
{
	MCObject *mcobj = GS_RetrieveMCObjectByName(name, MCOT_ALL_TYPES);
	if (mcobj == NULL) {
		g_status.lastError = (format("There is no object named \"%1%\".") 
							  % name).str();
		return;
	}
	
	string fn = g_status.GetOutputFileName(filename);
	ofstream out(fn.c_str());
	bool success = mcobj->WriteAsGmlFile(out, name, args);
	out.close();
	
	if (out && success)
		cout << "GML file \""<<filename<<"\" saved." << endl;
	else {
		cout << "Couldn't save file \""<<filename<<"\"." << endl;
		remove(filename);
	}
}


PUBLIC_API void Exec_LoadDspnCoords(const char *result, const char *filename,
									const char *args)
{
	CslTaResult *res = (CslTaResult*) GS_RetrieveMCObjectByName(result, MCOT_RESULT);
	if (NULL == res) {
		cout << "Couldn't find a named result called \""<<result<<"\"." << endl;
		return;
	}
	
	string fn = g_status.GetOutputFileName(filename);
	ifstream gmlFile(fn.c_str());
	if (!gmlFile) {
		cout << "Couldn't open the file \""<<filename<<"\"." << endl;
		return;
	}
	
	bool success = res->LoadDspnCoords(gmlFile, args);
	if (success) 
		cout << "Coordinate file \""<<filename<<"\" loaded." << endl;
	else 
		cout << "Couldn't load the coordinates." << endl;
}


PUBLIC_API void Exec_PlanarizeDspn(const char *result, const char *args)
{
	CslTaResult *res = (CslTaResult*) GS_RetrieveMCObjectByName(result, MCOT_RESULT);
	if (NULL == res) {
		cout << "Couldn't find a named result called \""<<result<<"\"." << endl;
		return;
	}
	
	bool success = res->PlanarizeDspn(args);
	if (success) 
		cout << "Planarized." << endl;
	else 
		cout << "Couldn't planarize." << endl;
}


PUBLIC_API void Exec_SaveDspn(const char *result, const char *filenamePrefix,
							  const char *args, const char *formatName)
{
	CslTaResult *res = (CslTaResult*) GS_RetrieveMCObjectByName(result, MCOT_RESULT);
	if (NULL == res) {
		cout << "Couldn't find a named result called \""<<result<<"\"." << endl;
		return;
	}
	
	DSPN dspn;
	if (!res->GetDspnToSave(args, dspn)) {
		cout << "There's no available DSPN with arguments: "<<args << endl;
		return;
	}
	
	if (0==strcmp(formatName, "DSPNexpress")) {
		string fnPref = g_status.GetOutputFileName(filenamePrefix);
		string fnNet = fnPref + ".NET";   // FIXME - rimettere a posto
		string fnDef = fnPref + ".DEF";
		ofstream netFile(fnNet.c_str());
		ofstream defFile(fnDef.c_str());
		
		dspn.WriteAsDSPNexpressFile(netFile, defFile, result);		
		bool success = ( netFile && defFile);
		netFile.close();
		defFile.close();
		
		if (success) 
			cout << "DSPNexpress files \""<<fnPref<<".(NET/DEF)\" saved." << endl;
		else {
			cout << "Couldn't save DSPNexpress files." << endl;
			remove(fnNet.c_str());
			remove(fnDef.c_str());
		}
	}
	else if (0==strcmp(formatName, "TimeNET")) {
		string fn = g_status.GetOutputFileName(filenamePrefix); 
		fn += ".TN";
		ofstream netFile(fn.c_str());
		
		dspn.WriteAsTimeNetFile(netFile, result);		
		bool success = netFile.good();
		netFile.close();
		
		if (success) 
			cout << "TimeNET DSPN file \""<<fn<<"\" saved." << endl;
		else {
			cout << "Couldn't save TimeNET files." << endl;
			remove(fn.c_str());
		}
	}
	else if (0==strcmp(formatName, "SPNica")) {
		string fn = g_status.GetOutputFileName(filenamePrefix); 
		fn += ".m";
		ofstream netFile(fn.c_str());
		
		dspn.AddInitialTangibleMark();
		dspn.AddAbsorbingLoops((const char**)s_absorbPlaces, 
							   NUM_ABSORBING_PLACES);
		dspn.WriteAsSPNicaFile(netFile, result);
		bool success = (bool)netFile;
		netFile.close();
		
		if (success) 
			cout << "SPNica DSPN file \""<<fn<<"\" saved." << endl;
		else {
			cout << "Couldn't save SPNica files." << endl;
			remove(fn.c_str());
		}
	}	
	else if (0==strcmp(formatName, "GreatSPN")) {
		string fnPref = g_status.GetOutputFileName(filenamePrefix);
		string fnNet = fnPref + ".net";
		string fnDef = fnPref + ".def";
		ofstream netFile(fnNet.c_str());
		ofstream defFile(fnDef.c_str());
		
		dspn.AddInitialTangibleMark();
		bool isDtaSemiDet = (dspn.FindPlaceByName("NonDet") != size_t(-1));
		const char *measures[2] = {
			"|Success 2.0 0.2 : E{#PxTop};\n"
			"|Failure 3.5 0.2 : E{#PxBot};\n",
			"|Success 2.0 0.2 : E{#PxTop};\n"
			"|Failure 3.5 0.2 : E{#PxBot};\n"
			"|NonDet  5.0 0.2 : E{#PxNonDet};\n",
		};
		dspn.WriteAsGreatSPNFile(netFile, defFile, result,
								 measures[isDtaSemiDet ? 1 : 0]);		
		bool success = (netFile && defFile);
		netFile.close();
		defFile.close();
		
		if (success) 
			cout << "GreatSPN files \""<<fnPref<<".(net/def)\" saved." << endl;
		else {
			cout << "Couldn't save GreatSPN files." << endl;
			remove(fnNet.c_str());
			remove(fnDef.c_str());
		}
	}
	else {
		cout << "Unknown format "<<formatName << endl;
	}	
}


PUBLIC_API void Exec_SaveTRG(const char *result, const char *filenamePrefix)
{
	CslTaResult *res = (CslTaResult*) GS_RetrieveMCObjectByName(result, MCOT_RESULT);
	if (NULL == res) {
		cout << "Couldn't find a named result called \""<<result<<"\"." << endl;
		return;
	}
	
	DSPN dspn;
	if (!res->GetDspnToSave("DSPN", dspn)) {
		cout << "There are no stochastic informations with this result." << endl;
		return;
	}

	cout << "Generating the TRG... " << endl;
	string fn = g_status.GetOutputFileName(filenamePrefix);
	
	char *tmpName = tempnam(TEMPNAM_DIR, "trg-net-");
	if (tmpName == NULL) {
		cout << "Couln't get a temporary file name." << endl;
		return;
	}
	string finalPdfName(fn);
	finalPdfName += ".pdf";

	if (g_status.extSolverUsed == EDS_SPNica) {
		// Open the stream with SPNica
		Pipe& mathKernel = g_status.GetMathKernel();
		dspn.AddInitialTangibleMark();
		dspn.AddAbsorbingLoops((const char**)s_absorbPlaces, 
							   NUM_ABSORBING_PLACES);
		ostringstream os;
		dspn.WriteAsSPNicaFile(os, "trg_dspn");
		os << "RRG = First[{ReducedReachabilityGraph[SPN], SPN}];\n";
		os << "RrgToDot[RRG, SPN, \""<<tmpName<<"\", False, PN, TN];\n";
		os << "Print[ALLDONE];\n";
		
		if (!mathKernel.Write(os.str())) {
			cout << "Coudn't send commands to the MathKernel." << endl;
			return;
		}
		mathKernel.SkipUntil("ALLDONE");
		
		ostringstream dotcmd;
		dotcmd << "dot -Tpdf "<<tmpName<<" -o \""<<finalPdfName<<"\" " << NO_CMD_OUTPUT;
		std::system(dotcmd.str().c_str());
		remove(tmpName);
	}
	else {
		string netName(tmpName), defName(tmpName), dotName(tmpName), pdfName(tmpName);
		netName += ".net";
		defName += ".def";
		dotName += "-TRG.dot";
		pdfName += "-TRG.pdf";
		ofstream net(netName.c_str()), def(defName.c_str());
		dspn.WriteAsGreatSPNFile(net, def, "", "");
		net.close();
		def.close();
		
		ostringstream cmd;
		cmd << g_status.mrpPath << " -load "<<tmpName<<" -trg -dot " << NO_CMD_OUTPUT;
		std::system(cmd.str().c_str());
		remove(netName.c_str());
		remove(defName.c_str());
		remove(dotName.c_str());
		rename(pdfName.c_str(), finalPdfName.c_str());
	}
	
	cout << "File \""<<finalPdfName<<"\" saved." << endl;
	free(tmpName);
}


PUBLIC_API void Exec_EvaluateCslTaExpr(const char *result,
									   const char *asmcName,
									   const char *evalState,
									   CslTaExpr *expr)
{
	ASMC *asmc = (ASMC*) GS_RetrieveMCObjectByName(asmcName, MCOT_ASMC);
	if (NULL == asmc) {
		cout << "Couldn't find an ASMC named \""<<asmcName<<"\"." << endl;
	}
	else if (NULL != GS_RetrieveMCObjectByName(result, MCOT_RESULT)) {
		cout << "A result object named \""<<result<<"\" already exists." << endl;
	}
	else {	
		size_t state = size_t(-1);
		if (evalState != NULL) { // Find the evaluated state index
			ASMC::vertex_iter_t it, it_end;
			for (tie(it, it_end) = vertices(asmc->graph); it != it_end; ++it) {
				if (get(asmc->vertPMap, *it).vertexName == evalState) {
					state = *it;
					break;
				}
			}
			if (state == size_t(-1)) {
				cout << "There is no state named \""<<evalState<<"\" in the ASMC." << endl;
				delete expr;
				return;
			}
		}
		
		// Evaluate the CslTa expression against this ASMC
		timer::cpu_timer evalTimer;
		cout << "Evaluating CSL-TA expression";
		if (evalState != NULL)
			cout << " in state \""<<evalState<<"\"";
		cout << ":\n   "<<(*expr) << endl;
		CslTaResult *res = new CslTaResult;
		
		// Prepare the evaluation mask
		vector<bool> mask(num_vertices(asmc->graph), (evalState == NULL));
		if (evalState != NULL)
			mask[state] = true;
		
		// Evaluate the selected states of the ASMC
		if (!(expr->PrepareEvaluation(*asmc) && 
			  expr->Evaluate(*asmc, mask, *res)))
		{
			cout << "Couldn't evaluate the expression." << endl;
			delete res;
		}
		else {
			bool noprob = res->probs.empty();
			cout << endl;
			cout << "Results:   state      value"<<(noprob ? "" : "    probability")<<endl;
			cout << "--------------------------------------------" <<endl;
			ASMC::vertex_iter_t it, it_end;
			for (tie(it, it_end) = vertices(asmc->graph); it != it_end; ++it) {
				if (!mask[*it])
					continue;
				cout << right<<setw(15) << get(asmc->vertPMap, *it).vertexName;
				cout << "       " << (res->eval[*it] ? "true  " : "false ");
				if (!noprob) {
					if (res->probs[*it] < -1)
						cout << "    -";
					else
						cout << (format("   %.7lf") % res->probs[*it]);
				}
				cout << endl;
			}
			cout << endl;
			
			Exec_StoreMCObject(res, result);
			cout << "Evaluation time: " << evalTimer.format() << endl;
		}
	}
	
	delete expr;
}


PUBLIC_API void Exec_SetOutputDir(const char *dirName)
{
	g_status.outputDir = dirName;
	cout << "Output Directory changed to: \""<<dirName<<"\"." << endl;
}


PUBLIC_API MCObject* GS_RetrieveMCObjectByName(const char *name,
											   MCObjectType mcot)
{
	map<string, MCObject*>::const_iterator it;
	it = g_status.mcobjects.find(name);
	if (it == g_status.mcobjects.end())
		return NULL;
	if (mcot != MCOT_ALL_TYPES && it->second->GetObjectType() != mcot)
		return NULL; // object of another type
	return it->second;
}



PUBLIC_API void GS_StoreTempDtaParams(DtaCCNamOrd* CCNamOrds, IDList* Acts, IDList* SPs) 
{
	safe_delete(g_status.dtaCCNamOrds);
	safe_delete(g_status.dtaActs);
	safe_delete(g_status.dtaSPExprs);
	
	g_status.dtaCCNamOrds = CCNamOrds;
	g_status.dtaActs = Acts;
	g_status.dtaSPExprs = SPs;
}


PUBLIC_API const char* GS_GetLastError()
{
	if (g_status.lastError.empty())
		return NULL;
	return g_status.lastError.c_str();
}


PUBLIC_API void GS_SetError(const char *e) {
	g_status.lastError = e;
}
void GS_SetError(const string& e) {
	g_status.lastError = e;
}









//-----------------------------------------------------------------------------

PUBLIC_API char* Number_Op(const char *n1, char op, const char *n2)
{
	// Decode the numbers
	char *endptr;
	double a = strtod(n1, &endptr);
	if (endptr == n1) {
		GS_SetError((format("Error: couldn't understand number %1%.") % n1).str());
		return strdup("0");
	}
	double b = strtod(n2, &endptr);
	if (endptr == n1) {
		GS_SetError((format("Error: couldn't understand number %1%.") % n2).str());
		return strdup("0");
	}
	
	// Execute the operation
	double result;
	switch (op) {
		case '+':	result = a + b;   break;
		case '-':	result = a - b;   break;
		case '*':	result = a * b;   break;
		case '/':	
			if (b == 0) {
				GS_SetError("Error: Division by zero!");
				return strdup("0");
			}
			result = a / b;   
			break;
		default:    
			assert(false);  
			return NULL;
	}
	
	char buffer[128];
	sprintf(buffer, "%lf", result);
	return strdup(buffer);
}








/*
#inc*lude "asmc.cpp"
#inc*lude "dta.cpp"
#inc*lude "cslta.cpp"
// */



int g_interactive_mode = 0;

PUBLIC_API int yyparse();
PUBLIC_API void lexer_open_file(FILE *file);

PUBLIC_API void prompt() {
	if (g_interactive_mode)
		cout << "CSLTA> " << flush;
};


bool ReadConfigFile(const char* filename="CslTA-Solver.conf")
{
	// Read the configuration file
	ifstream conf(filename);
	if (!conf) {
		cout << "Couldn't find the configuration file \""<<filename<<"\"." << endl;
		return false;
	}
	const int CONFBUF = 256;
	char buffer[CONFBUF+1], name[CONFBUF+1];
	int value;
	bool readDspnSolver=false;
	bool readMathKernel=false;
	bool readMathWrLog=false, readMathRdLog=false, readMathLogActive=false;
	bool readDspnExpressPath=false, readReadUrgPath=false;
	bool readMrpSolverPath=false, readMrpSolverLog=false, readMrpSolverExtraArg=false;
	bool readTestSolverDspnDir=false;
	
	while (conf.getline(buffer, CONFBUF)) {
		if (1==sscanf(buffer, "EXTERNALDSPNSOLVER %[^\n]", name)) {
			if (0==strcmp(name, "SPNica")) 
			{ g_status.extSolverUsed = EDS_SPNica; }
			else if (0==strcmp(name, "DSPNexpress")) 
			{ g_status.extSolverUsed = EDS_DSPNexpress; }
			else if (0==strcmp(name, "MRPSolver")) 
			{ g_status.extSolverUsed = EDS_MRPSolver; }
			else if (0==strcmp(name, "TestSolver")) 
			{ g_status.extSolverUsed = EDS_TestSolver; }
			else {
				cout << "Unknown solver "<<name<<"." << endl;
				return false;
			}
			readDspnSolver = true;
		}
		else if (1==sscanf(buffer, "MATHKERNEL %[^\n]", name)) {
			g_status.mathKernelName = name;
			readMathKernel = true;
		}
		else if (1==sscanf(buffer, "MATHWRITELOG %[^\n]", name)) {
			g_status.mathWrLogFile = name;
			readMathWrLog=true;
		}
		else if (1==sscanf(buffer, "MATHREADLOG %[^\n]", name)) {
			g_status.mathRdLogFile = name;
			readMathRdLog=true;
		}
		else if (1==sscanf(buffer, "MATHLOG %i", &value)) {
			g_status.writeMathKernelOutput = (value!=0);
			readMathLogActive=true;
		}
		else if (1==sscanf(buffer, "DSPNexpressPATH %[^\n]", name)) {
			g_status.dspnExpressPath = name;
			readDspnExpressPath = true;
		}
		else if (1==sscanf(buffer, "MRPSolverPath %[^\n]", name)) {
			g_status.mrpPath = name;
			readMrpSolverPath = true;
		}
		else if (1==sscanf(buffer, "MRPSolverLogFile %[^\n]", name)) {
			g_status.mrpLogFile = name;
			readMrpSolverLog = true;
		}
		else if (1==sscanf(buffer, "MRPSolverExtraArgs %[^\n]", name)) {
			g_status.extraArgs = name;
			readMrpSolverExtraArg = true;
		}
		else if (1==sscanf(buffer, "TestSolverOUTPUTBASENAME %[^\n]", name)) {
			g_status.testSolverDspnName = name;
			readTestSolverDspnDir = true;
		}
		else if (1==sscanf(buffer, "DSPNexpressREADURGPATH %[^\n]", name)) {
			g_status.readUrgPath = name;
			readReadUrgPath = true;
		}
	}	
	conf.close();
	
#define TestConfDefined(bConf, str) \
	do { if (!(bConf)) { cout << ("Couldn't find a " str " line in the configuration.") << endl; \
		return false; \
	} } while(0)
	
	// Test completeness of the configuration file
	TestConfDefined(readDspnSolver, "EXTERNALDSPNSOLVER <>");
	TestConfDefined(readMathKernel, "MATHKERNEL <>");
	TestConfDefined(readMathWrLog, "MATHWRITELOG <>");
	TestConfDefined(readMathRdLog, "MATHREADLOG <>");
	TestConfDefined(readMathLogActive, "MATHLOG <>");
	TestConfDefined(readDspnExpressPath, "DSPNexpressPATH <>");
	TestConfDefined(readReadUrgPath, "DSPNexpressREADURGPATH <>");
	TestConfDefined(readMrpSolverPath, "MRPSolverPath <>");
	//TestConfDefined(readMrpSolverLog, "MRPSolverLogFile <>");
	TestConfDefined(readMrpSolverExtraArg, "MRPSolverExtraArgs <>");
	TestConfDefined(readTestSolverDspnDir, "TestSolverOUTPUTBASENAME <>");
	
	return true;
}


int main(int argc, char* argv[])
{
	FILE *file;
	bool configRead = false;
	

	// TODO: print a better banner
	cout << "----------------------------------------------------------------------" << endl;
	cout << "--                  CSL^TA Solver - (version 0.1)                   --" << endl;
	cout << "--                      Universita' di Torino                       --" << endl;
	cout << "-- If you find bugs or unexpected behaviours, please contact us at: --" << endl;
	cout << "--             amparore@di.unito.it     susi@di.unito.it            --" << endl;
	cout << "----------------------------------------------------------------------" << endl;
	
	for (int i=1; i<argc; i++) {
		if (0 == strcmp(argv[i], "-c") && i+1<argc && !configRead) {
			if (!ReadConfigFile(argv[i+1]))
				return -1;
			configRead = true;
			i++;
		}
		else if (0 == strcmp(argv[i], "-i")) {
			if (!configRead) {
				if (!ReadConfigFile())
					return -1;
				configRead = true;
			}
			g_interactive_mode = 1;
			cout << "Interactive mode enabled." << endl;
			prompt();
			return yyparse();
		}
		else if (NULL != (file = fopen(argv[i], "r"))) {
			if (!configRead) {
				if (!ReadConfigFile())
					return -1;
				configRead = true;
			}
			lexer_open_file(file);
			int retval = yyparse();
#ifdef WIN32
			system("PAUSE");
#endif
			return retval;
		}
		else {
			cout << "Unrecognized option or invalid filename \""<<argv[i]<<"\"." << endl;
			break;
		}		
	}
	
	cout << ("Usage: CslTa-Solver [-c configFile] [-i] [filename] \n"
			 "   -c configFile  Config file location.\n"
			 "   -i             Interactive mode.\n"
			 "   filename       CslTa file to load in batch mode.") << endl;
    return -1;
}








