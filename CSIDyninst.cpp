/*
TODO:
1. BBinstrument(): instrument functions
*/

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstddef>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include "config.h"
#include "types.h"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include <map>
#include <sstream>
#include <climits>
#include <set>
using namespace std;

#include "instConfig.h"
#include "instUnmap.h"
// DyninstAPI includes
#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_flowGraph.h"
#include "BPatch_function.h"
#include "BPatch_point.h"


using namespace Dyninst;




//cmd line options
char *originalBinary;
char *instrumentedBinary;
bool verbose = false;
char* csifuzz_dir = NULL;  //Output dir of csifuzz results

bool isPrep = false, // preprocessing
    isOracle = false, // instrument oracle
    isTrimmer = false, // trimmer
    isCrasher = false,
    isTracer = false; // instrument tracer

u32 flag_id = 0;

std::map<unsigned long, u16> ramdom_map;
std::map<unsigned long, u32> flag_map;
// [bb_id, bb_addr, inst_begin, inst_end]
std::map <u32, std::vector<u64> > addr_inst_map;


// call back functions
BPatch_function *OracleBB;
BPatch_function *initAflForkServer;
BPatch_function *TracerBB;
BPatch_function *TrimmerBB;
BPatch_function *CrasherBB;

const char *instLibrary = "./libCSIDyninst.so";

static const char *OPT_STR = "i:o:vb:PFTMC";
static const char *USAGE = " -i <binary> -o <binary> -b <csifuzz-dir> -F(PTMC)\n \
            -i: Input binary \n \
            -o: Output binary\n \
            -b: Output dir of csifuzz results\n \
            -v: Verbose output\n \
            -P: The initial preprocessing (counting edges and blocks; writing address files.)\n \
            -F: The full-speed oracle\n \
            -T: The tracer\n \
            -M: The trimmer\n \
            -C: The crasher\n";

bool parseOptions(int argc, char **argv)
{

    int c;
    while ((c = getopt (argc, argv, OPT_STR)) != -1) {
        switch ((char) c) {
        case 'i':
            originalBinary = optarg;
            break;
        case 'o':
            instrumentedBinary = optarg;
            break;
        case 'b':
            csifuzz_dir = optarg;
            break;
        case 'v':
            verbose = true;
            break;
        case 'P':
            isPrep = true;
            break;
        case 'F':
            isOracle = true;
            break;
        case 'T':
            isTracer = true;
            break;
        case 'M':
            isTrimmer = true;
            break;
        case 'C':
            isCrasher = true;
            break;
        default:
            cerr << "Usage: " << argv[0] << USAGE;
            return false;
        }
    }

    if(originalBinary == NULL) {
        cerr << "Input binary is required!"<< endl;
        cerr << "Usage: " << argv[0] << USAGE;
        return false;
    }

    if((instrumentedBinary == NULL) && (isPrep == false)) {
        cerr << "Output binary or -P is required!" << endl;
        cerr << "Usage: " << argv[0] << USAGE;
        return false;
    }

    if(csifuzz_dir == NULL){
        cerr << "Output directory for csifuzz is required!" << endl;
        cerr << "Usage: " << argv[0] << USAGE;
        return false;
    }

    if ((isPrep == false) && (isOracle == false) && (isTracer == false)
             && (isTrimmer == false) &&(isCrasher == false)){
        cerr << "Specify -P, -T, -F, C, or -M" << endl;
        cerr << "Usage: " << argv[0] << USAGE;
        return false;
    }

    return true;
}

BPatch_function *findFuncByName (BPatch_image * appImage, char *funcName)
{
    BPatch_Vector < BPatch_function * >funcs;

    if (NULL == appImage->findFunction (funcName, funcs) || !funcs.size ()
        || NULL == funcs[0]) {
        cerr << "Failed to find " << funcName << " function." << endl;
        return NULL;
    }

    return funcs[0];
}

//skip some functions
bool isSkipFuncs(char* funcName){
    if (string(funcName) == string("first_init") ||
        string(funcName) == string("__mach_init") ||
        string(funcName) == string("_hurd_init") ||
        string(funcName) == string("_hurd_preinit_hook") ||
        string(funcName) == string("doinit") ||
        string(funcName) == string("doinit1") ||
        string(funcName) == string("init") ||
        string(funcName) == string("init1") ||
        string(funcName) == string("_hurd_subinit") ||
        string(funcName) == string("init_dtable") ||
        string(funcName) == string("_start1") ||
        string(funcName) == string("preinit_array_start") ||
        string(funcName) == string("_init") ||
        string(funcName) == string("init") ||
        string(funcName) == string("fini") ||
        string(funcName) == string("_fini") ||
        string(funcName) == string("_hurd_stack_setup") ||
        string(funcName) == string("_hurd_startup") ||
        string(funcName) == string("register_tm_clones") ||
        string(funcName) == string("deregister_tm_clones") ||
        string(funcName) == string("frame_dummy") ||
        string(funcName) == string("__do_global_ctors_aux") ||
        string(funcName) == string("__do_global_dtors_aux") ||
        string(funcName) == string("__libc_csu_init") ||
        string(funcName) == string("__libc_csu_fini") ||
        string(funcName) == string("start") ||
        string(funcName) == string("_start") || 
        string(funcName) == string("__libc_start_main") ||
        string(funcName) == string("__gmon_start__") ||
        string(funcName) == string("__cxa_atexit") ||
        string(funcName) == string("__cxa_finalize") ||
        string(funcName) == string("__assert_fail") ||
        string(funcName) == string("_dl_start") || 
        string(funcName) == string("_dl_start_final") ||
        string(funcName) == string("_dl_sysdep_start") ||
        string(funcName) == string("dl_main") ||
        string(funcName) == string("_dl_allocate_tls_init") ||
        string(funcName) == string("_dl_start_user") ||
        string(funcName) == string("_dl_init_first") ||
        string(funcName) == string("_dl_init")) {
        return true; //skip these functions
        }
    return false;    
}


//count the number of bbs
bool count_bbs(BPatch_binaryEdit * appBin, BPatch_image *appImage, 
                    vector < BPatch_function * >::iterator funcIter, 
                    char* funcName, fs::path output_dir){

    fs::path addr_id_file = output_dir / BLOCK_ADDR_ID;

    BPatch_function *curFunc = *funcIter;
    BPatch_flowGraph *appCFG = curFunc->getCFG();

    BPatch_Set < BPatch_basicBlock * > allBlocks;
    if (!appCFG->getAllBasicBlocks (allBlocks)) {
        cerr << "Failed to find basic blocks for function " << funcName << endl;
        return false;
    } else if (allBlocks.size () == 0) {
        cerr << "No basic blocks for function " << funcName << endl;
        return false;
    }

    ofstream BBid_file;
    BBid_file.open(addr_id_file.c_str(), ios::out | ios::app | ios::binary);

    set < BPatch_basicBlock *>::iterator bb_iter;

    unsigned long bb_addr = 0;
    u16 random_id;
    

    for (bb_iter = allBlocks.begin (); bb_iter != allBlocks.end (); bb_iter++){
        BPatch_basicBlock * block = *bb_iter;
        vector<pair<Dyninst::InstructionAPI::Instruction, Dyninst::Address> > insns;
        block->getInstructions(insns);

        //Dyninst::Address addr = insns.back().second;  //addr: equal to offset when it's binary rewrite
        Dyninst::InstructionAPI::Instruction insn = insns.back().first; 
        Dyninst::InstructionAPI::Operation op = insn.getOperation();
        //Dyninst::InstructionAPI::InsnCategory category = insn.getCategory();
        Dyninst::InstructionAPI::Expression::Ptr expt = insn.getControlFlowTarget();

        //BPatch_point *bbEntry = (*bb_iter)->findEntryPoint();
        bb_addr = block->getStartAddress();
        if(BBid_file.is_open()){
            random_id = rand() % USHRT_MAX;  // USHRT_MAX = (1<<16)
            BBid_file << bb_addr << " " << random_id << " " << flag_id << endl;   
            flag_id ++; 
            if (flag_id >= BYTES_FLAGS){
                flag_id = BYTES_FLAGS - 1;
            } 
        }
        else{
            cout << "cannot open the file: " << addr_id_file.c_str() << endl;
            return false;
        }

    }

    BBid_file.close();
 

    return true;
    
}

// read addresses and ids from files; ensure that Oracle and Tracer has the same ids
bool readAddrs(fs::path output_dir){
    
    fs::path bb_file = output_dir / BLOCK_ADDR_ID;
    ifstream BB_ID_IO;
    
    /*recover addresses, ids*/
    struct stat inbuff;
    unsigned long bb_addr;
    u16 random_id;
    u32 flag_id;

    if (stat(bb_file.c_str(), &inbuff) == 0){ // file  exists
        BB_ID_IO.open (bb_file.c_str()); //read file
        if (BB_ID_IO.is_open()){
            while (BB_ID_IO >> bb_addr >> random_id >> flag_id){
                ramdom_map.insert(make_pair(bb_addr, random_id));
                flag_map.insert(make_pair(bb_addr, flag_id));

                // for writing addr_inst files
                if (isOracle || isCrasher){
                    addr_inst_map[flag_id].push_back(bb_addr);
                }
            }
            BB_ID_IO.close();
        }

    }
    else{
        cout << "Please create address-ids first." <<endl;
        return false;
    }


    return true;

}


/*
// to write files that saves addr, inst_addr; 
    sort them in terms of edge_id
//inst_addr_file: the mapping file outputed by dyninst
            [inst_begin, inst_end, edge, src_addr, des_addr] or
            [inst_begin, inst_end, block, bb_addr]
output_dir: dir to be written files
bint: 1, oracle; 2, crasher
*/

bool writeInstAddr(fs::path output_dir, fs::path inst_addr_file){
    //addr_inst_map
    char buff[256];
    char *tmp, *tmp_left;
    //bool isedge = false;
    unsigned long bb_addr, inst_begin, inst_end;
    std::map<unsigned long, u32>::iterator itid;

    // read mapping addrs
    
    ifstream mapping_io (inst_addr_file.c_str());
    if (mapping_io.is_open()){
        while (mapping_io){
            bb_addr =0;
            inst_begin=0;
            inst_end=0;
            tmp = NULL;
            tmp_left = NULL;
            
            
            mapping_io.getline(buff, sizeof(buff));

            tmp = strtok_r (buff, ",", &tmp_left);
            if (tmp != NULL) {
                inst_begin = strtoul(tmp, NULL, 16);
            }
            else continue;
   
            tmp = strtok_r (NULL, ",", &tmp_left);
            if (tmp != NULL) {
                inst_end = strtoul(tmp, NULL, 16);
            }
            else continue;
        
            tmp = strtok_r (NULL, ",", &tmp_left);
            if (tmp == NULL) continue;
            if (strcmp(tmp,"block") == 0){ // for blocks

                if (tmp_left != NULL) {
                    bb_addr = strtoul(tmp_left, NULL, 16);
                }
                else continue;

                itid = flag_map.find(bb_addr);
                if (itid != flag_map.end()){
                    addr_inst_map[(*itid).second].push_back(inst_begin);
                    addr_inst_map[(*itid).second].push_back(inst_end);
                }
            } 
        }
        mapping_io.close();
    }
    else{
        cout << "generate mapping files first." << endl;
        return false;
    }

    // writing to a file
    fs::path sort_map_path;
    if (isOracle){
        sort_map_path = output_dir / ORACLE_BLOCK_MAP;
    }
    else {
        sort_map_path = output_dir / CRASHER_BLOCK_MAP;
    }
     
    ofstream write_map (sort_map_path.c_str(), ios::out | ios::app | ios::binary);
    if (write_map.is_open()){
        for (auto itwrite = addr_inst_map.begin(); itwrite != addr_inst_map.end(); itwrite++){
            if ((*itwrite).second.size() != 3) continue;
            // [id, inst_begin, inst_end, bb_addr]
            write_map << (*itwrite).first << ","<< (*itwrite).second[1] << "," << (*itwrite).second[2]<< "," << (*itwrite).second[0] << endl;
            
        }
        write_map.close();
    }
    else{
        cout << "wrong writing edge mapping."<<endl;
        return false;
    }
    
    return true;
}

// instrument at  
bool instOracleBB(BPatch_binaryEdit * appBin, BPatch_function * instFunc, BPatch_point * instrumentPoint, 
                        u16 random_id, u32 flag_id){
    vector<BPatch_snippet *> cond_args;
    BPatch_constExpr CondID(random_id);
    cond_args.push_back(&CondID);
    BPatch_constExpr FlagID(flag_id);
    cond_args.push_back(&FlagID);
    

    BPatch_funcCallExpr instCondExpr(*instFunc, cond_args);

    BPatchSnippetHandle *handle =
            appBin->insertSnippet(instCondExpr, *instrumentPoint, BPatch_callBefore, BPatch_firstSnippet);
    if (!handle) {
            cerr << "Failed to insert instrumention in basic block at id: " << flag_id << endl;
            return false;
        }
    return true;         

}


// instrument at  
bool instTracerBB(BPatch_binaryEdit * appBin, BPatch_function * instFunc, BPatch_point * instrumentPoint, 
                        u16 random_id, u32 flag_id){
   
    vector<BPatch_snippet *> cond_args;
    BPatch_constExpr CondID(random_id);
    cond_args.push_back(&CondID);
    BPatch_constExpr FlagID(flag_id);
    cond_args.push_back(&FlagID);

    BPatch_funcCallExpr instCondExpr(*instFunc, cond_args);

    BPatchSnippetHandle *handle =
            appBin->insertSnippet(instCondExpr, *instrumentPoint, BPatch_callBefore, BPatch_firstSnippet);
    if (!handle) {
            cerr << "Failed to insert instrumention in basic block at id: " << flag_id << endl;
            return false;
        }
    return true;         

}


// instrument at  
bool instTrimmerBB(BPatch_binaryEdit * appBin, BPatch_function * instFunc, BPatch_point * instrumentPoint, 
                        u16 random_id){
    vector<BPatch_snippet *> cond_args;
    BPatch_constExpr CondID(random_id);
    cond_args.push_back(&CondID);

    BPatch_funcCallExpr instCondExpr(*instFunc, cond_args);

    BPatchSnippetHandle *handle =
            appBin->insertSnippet(instCondExpr, *instrumentPoint, BPatch_callBefore, BPatch_firstSnippet);
    if (!handle) {
            cerr << "Failed to insert instrumention in basic block at random id: " << random_id << endl;
            return false;
        }
    return true;         

}


/*instrument at bbs for one function
*/
bool bbInstrument(BPatch_binaryEdit * appBin, BPatch_image *appImage, 
                    vector < BPatch_function * >::iterator funcIter, char* funcName,
                    fs::path output_dir){
    
    u16 random_id = 0;
    u32 flag_id;
    unsigned long bb_addr;

    BPatch_function *curFunc = *funcIter;
    BPatch_flowGraph *appCFG = curFunc->getCFG ();

    BPatch_Set < BPatch_basicBlock * > allBlocks;
    if (!appCFG->getAllBasicBlocks (allBlocks)) {
        cerr << "Failed to find basic blocks for function " << funcName << endl;
        return false;
    } else if (allBlocks.size () == 0) {
        cerr << "No basic blocks for function " << funcName << endl;
        return false;
    }

    set < BPatch_basicBlock *>::iterator bb_iter;
    for (bb_iter = allBlocks.begin (); bb_iter != allBlocks.end (); bb_iter++){
        BPatch_basicBlock * block = *bb_iter;
        vector<pair<Dyninst::InstructionAPI::Instruction, Dyninst::Address> > insns;
        block->getInstructions(insns);

        //Dyninst::Address addr = insns.back().second;  //addr: equal to offset when it's binary rewrite
        Dyninst::InstructionAPI::Instruction insn = insns.back().first; 
        Dyninst::InstructionAPI::Operation op = insn.getOperation();
        //Dyninst::InstructionAPI::InsnCategory category = insn.getCategory();
        Dyninst::InstructionAPI::Expression::Ptr expt = insn.getControlFlowTarget();


        BPatch_point *bbEntry = (*bb_iter)->findEntryPoint();
        bb_addr = block->getStartAddress();
        // random id
        std::map<unsigned long, u16>::iterator random_iter = ramdom_map.find(bb_addr);
        if (random_iter != ramdom_map.end()){ // found it
            random_id = random_iter->second;
        }
        else{
            cout << "Check BB IDs fail, at block addr: " << bb_addr << endl;
            return false;
        }
        // flag id
        std::map<unsigned long, u32>::iterator flag_iter = flag_map.find(bb_addr);
        if (flag_iter != flag_map.end()){ // found it
            flag_id = flag_iter->second;
        }
        else{
            cout << "Check BB IDs fail, at block addr: " << bb_addr << endl;
            return false;
        }

        //rosen
        if (isOracle){
            if (!instOracleBB(appBin, OracleBB, bbEntry, random_id, flag_id))
                cout << "BB instrument error." << endl;
        }
        else if (isTracer){
            if (!instTracerBB(appBin, TracerBB, bbEntry, random_id, flag_id))
                cout << "BB instrument error." << endl;
        }
        else if (isTrimmer){
            if (!instTrimmerBB(appBin, TrimmerBB, bbEntry, random_id))
                cout << "BB instrument error." << endl;
        }
        else if (isCrasher){
            if (!instOracleBB(appBin, CrasherBB, bbEntry, random_id, flag_id))
                cout << "BB instrument error." << endl;
        }

    }
    
    return true;
}


/* insert forkserver at the beginning of main
    funcInit: function to be instrumented, i.e., main

*/

bool insertForkServer(BPatch_binaryEdit * appBin, BPatch_function * instIncFunc,
                         BPatch_function *funcInit){

    /* Find the instrumentation points */
    vector < BPatch_point * >*funcEntry = funcInit->findPoint (BPatch_entry);

    if (NULL == funcEntry) {
        cerr << "Failed to find entry for function. " <<  endl;
        return false;
    }

    //cout << "Inserting init callback." << endl;
    BPatch_Vector < BPatch_snippet * >instArgs; 

    BPatch_funcCallExpr instIncExpr(*instIncFunc, instArgs);

    /* Insert the snippet at function entry */
    BPatchSnippetHandle *handle =
        appBin->insertSnippet (instIncExpr, *funcEntry, BPatch_callBefore, BPatch_firstSnippet);
    if (!handle) {
        cerr << "Failed to insert forkserver callback." << endl;
        return false;
    }
    return true;
}

int main (int argc, char **argv){

     if(!parseOptions(argc,argv)) {
        return EXIT_FAILURE;
    }

    fs::path out_dir (reinterpret_cast<const char*>(csifuzz_dir)); // files for csifuzz results
    fs::path oracle_map = out_dir / ORACLE_INST_ADDR;
    fs::path crasher_map = out_dir / CRASHER_INST_ADDR;

    /* start instrumentation*/
    BPatch bpatch;

     // mapping address
    if (isOracle){
        BPatch::bpatch->setMappingFilePath(oracle_map.c_str());
    }
    else if (isCrasher){
        BPatch::bpatch->setMappingFilePath(crasher_map.c_str());
    }
    else{
        BPatch::bpatch->setMappingFilePath("/dev/null");
    }

    // skip all libraries unless -l is set
    BPatch_binaryEdit *appBin = bpatch.openBinary (originalBinary, false);
    if (appBin == NULL) {
        cerr << "Failed to open binary" << endl;
        return EXIT_FAILURE;
    }


    BPatch_image *appImage = appBin->getImage ();

    
    vector < BPatch_function * > allFunctions;
    appImage->getProcedures(allFunctions);

    if (!appBin->loadLibrary (instLibrary)) {
        cerr << "Failed to open instrumentation library " << instLibrary << endl;
        cerr << "It needs to be located in the current working directory." << endl;
        return EXIT_FAILURE;
    }

    initAflForkServer = findFuncByName (appImage, (char *) "initAflForkServer");
 
  
    OracleBB = findFuncByName (appImage, (char *) "OracleBB");
    TracerBB = findFuncByName (appImage, (char *) "TracerBB");
    TrimmerBB = findFuncByName (appImage, (char *) "TrimmerBB");
    CrasherBB = findFuncByName (appImage, (char *) "CrasherBB");

    if (!initAflForkServer || !OracleBB || !TracerBB || !TrimmerBB || !CrasherBB) {
        cerr << "Instrumentation library lacks callbacks!" << endl;
        return EXIT_FAILURE;
    }


    /* 
    count the number of blocks
    */
   if (isPrep){
       // iterate over all functions to count blocks
        for (auto countIter = allFunctions.begin (); countIter != allFunctions.end (); ++countIter) {
            BPatch_function *countFunc = *countIter;
            char funcName[1024];
            countFunc->getName (funcName, 1024);
            
            if(isSkipFuncs(funcName)) continue;
            //count edges
            if(!count_bbs(appBin, appImage, countIter, funcName, out_dir)) 
                                cout << "Empty function" << funcName << endl;      
        }
        return EXIT_SUCCESS; 
   }
   
   // read address-ids from files
    if(!readAddrs(out_dir)) {
        cout << "Fail to read addresses." << endl;
        return EXIT_FAILURE;
    }

   
    vector < BPatch_function * >::iterator funcIter;
    for (funcIter = allFunctions.begin (); funcIter != allFunctions.end (); ++funcIter) {
        BPatch_function *curFunc = *funcIter;
        char funcName[1024];
        curFunc->getName (funcName, 1024);
        if(isSkipFuncs(funcName)) continue;
        //instrument at edges
        if (!bbInstrument(appBin, appImage, funcIter, funcName, out_dir)) {
            cout << "fail to instrument function: " << funcName << endl;
            // return EXIT_FAILURE;
        }
    }

    BPatch_function *funcToPatch = NULL;
    BPatch_Vector<BPatch_function*> funcs;
    
    appImage->findFunction("main",funcs);  //"main"
    if(!funcs.size()) {
        cerr << "Couldn't locate main, check your binary. "<< endl;
        return EXIT_FAILURE;
    }
    // there should really be only one
    funcToPatch = funcs[0];

    if(!insertForkServer (appBin, initAflForkServer, funcToPatch)){
        cerr << "Could not insert init callback at main." << endl;
        return EXIT_FAILURE;
    }

    if(verbose){
        cout << "Saving the instrumented binary to " << instrumentedBinary << "..." << endl;
    }
    // save the instrumented binary
    if (!appBin->writeFile (instrumentedBinary)) {
        cerr << "Failed to write output file: " << instrumentedBinary << endl;
        return EXIT_FAILURE;
    }

    sleep(1);
    if (isOracle || isCrasher){
        
        fs::path tmp_map;
        if (isOracle) tmp_map = oracle_map;
        else if (isCrasher) tmp_map = crasher_map;

        if (!writeInstAddr(out_dir, tmp_map)) {
            cerr << "Failed to write mapping file: " << endl;
            return EXIT_FAILURE;
        }

    }   

    if(verbose){
        cout << "All done! Happy fuzzing!" << endl;
    }

    return EXIT_SUCCESS;


}