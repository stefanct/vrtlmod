////////////////////////////////////////////////////////////////////////////////
/// @file generator.cpp
/// @date Created on Mon Jan 07 14:12:11 2020
/// @modified on Wed Dec 09 13:32:12 2020 (johannes.geier@tum.de)
/// @author Johannes Geier (johannes.geier@tum.de)
////////////////////////////////////////////////////////////////////////////////

#include "vrtlmod/vapi/generator.hpp"
#include "vrtlmod/vapi/target.hpp"
#include "vrtlmod/util/system.hpp"
#include "vrtlmod/util/logging.hpp"

#include <string>
#include <sstream>
#include <fstream>
#include <chrono>
#include <ctime>

#include "boost/filesystem.hpp"
namespace fs = boost::filesystem;

namespace vapi {

VapiGenerator::VapiGenerator(void) :
		outdir() {
}

int VapiGenerator::init(const char *pTargetXmlFile, const char *pOutdir) {
	outdir = pOutdir;
	return (XmlHelper::init(pTargetXmlFile));
}

std::string VapiGenerator::getTDExternalDecl(void) {
	static bool once = false;
	if (!once) {
		once = true;
		return ("sTD_t& gTD = VRTLmodAPI::i().get_struct(); \t// global target dictionary\n");
	} else {
		return ("extern sTD_t& gTD;\n");
	}
}

std::string VapiGenerator::getInludeStrings(void) {
	std::stringstream ret;
	ret <<
"/* Includes for Target Injection API */ \n\
#include \"" << API_DIRPREFIX << "/" << API_TD_DIRPREFIX << "/" << API_TD_HEADER_NAME << "\" \n\
#include \"" << API_DIRPREFIX << "/" << API_HEADER_NAME << "\"\n";
	return (ret.str());
}

std::string VapiGenerator::get_intermittenInjectionStmtString(Target &t) {
	std::stringstream ret;
	if (t.mElData.vrtlCxxType.find("WData") == std::string::npos) {
		ret << "INT_TARGET_INJECT(gTD." << get_targetdictionaryTargetClassDeclName(t);
	} else { // insert word accessed write
		int words = -1;
		for (int i = t.mElData.nmbBits; i >= 0; i -= 32) {
			words++;
		}
		ret << "INT_TARGET_INJECT_W(gTD." << get_targetdictionaryTargetClassDeclName(t) << ", " << words; //word;
	}
	ret << ")";

	return (ret.str());
}

std::string VapiGenerator::get_sequentInjectionStmtString(Target &t, int word //expression
		) {
	std::stringstream ret;
	if (word < 0) { // insert simple variable access
		ret << "SEQ_TARGET_INJECT(gTD." << get_targetdictionaryTargetClassDeclName(t);

	} else { // insert word accessed write
		ret << "SEQ_TARGET_INJECT_W(gTD." << get_targetdictionaryTargetClassDeclName(t) << ", " << word;
	}
	ret << ")";
	t.mSeqInjCnt++;
	return (ret.str());
}

int VapiGenerator::isExprTarget(const char *pExpr) {
	int ret = -1;
	ExprT x = ExprT(pExpr);
	std::string expre;
	if (x.prefix == "vlSymsp") {
		expre = x.object + std::string(".") + x.name;
	} else if (x.prefix == "vlTOPp") {
		expre = std::string("TOP.") + x.name;
	}
	for (auto const &it : mTargets) {
		ret++;
		std::string target = it->mElData.hierarchy;
		if (target == expre) {
			return ret;
		}
	}
	return (-1);
}

Target& VapiGenerator::getExprTarget(int idx) {
	return (*mTargets.at(idx));
}

std::string VapiGenerator::get_targetdictionaryTargetClassDeclName(Target &t) {
	std::string ret = "e_";
	ret += t.get_hierarchyDedotted();
	return (ret);
}

std::string VapiGenerator::get_targetdictionaryTargetClassDefName(Target &t) {
	std::string ret = "TDentry_";
	ret += t.get_hierarchyDedotted();
	return (ret);
}

std::string VapiGenerator::get_targetdictionaryEntryTypeDefString(Target &t) {
	std::stringstream ss;
	ss << "/* (TDentry-Id " << t.get_index() << "):" << t << " */" << std::endl;
	ss << "class " << get_targetdictionaryTargetClassDefName(t) << ": public TDentry {" << std::endl;
	ss << "public:" << std::endl;
//	ss << "\t\t" << "unsigned bits;" << std::endl;

	ss << "\t" << t.mElData.vrtlCxxType.substr(0, t.mElData.vrtlCxxType.find("[")) << "* data;" << "\t// " << t.mElData.vrtlCxxType << std::endl;
	if (t.mElData.words <= 1) {
		ss << "\t" << t.mElData.vrtlCxxType << " mask;" << std::endl;
		ss << "\t" << "void reset_mask(void){mask = 0;}" << std::endl;
		if (t.mElData.vrtlCxxType == "QData") {
			ss << "\t" << "void set_maskBit(unsigned bit){VL_ASSIGNBIT_QO(1, bit, mask, 0);}" << std::endl;
		} else {
			ss << "\t" << "void set_maskBit(unsigned bit){VL_ASSIGNBIT_IO(1, bit, mask, 0);}" << std::endl;
		}
	} else {
		ss << "\t" << t.mElData.vrtlCxxType.substr(0, t.mElData.vrtlCxxType.find("[")) << " mask[" << t.mElData.words << "];" << std::endl;
		ss << "\t" << "void reset_mask(void){" << std::endl;
		for (unsigned i = 0; i < t.mElData.words; i++) {
			ss << "\t\t" << "mask[" << i << "] = 0;" << std::endl;
		}
		ss << "\t}" << std::endl;
		ss << "\t" << "void set_maskBit(unsigned bit){VL_ASSIGNBIT_WO(1, bit, mask, 1);}" << std::endl;
	}
	ss <<
"	void read_data(uint8_t* pData) { \n\
		unsigned byte = 0; \n\
		uint8_t* xData = reinterpret_cast<uint8_t*>(data); \n\
		for(unsigned bit = 0; bit < bits; bit++){ \n\
			if((bit % 8)==0){ \n\
				pData[byte] = xData[byte]; \n\
				byte++; \n\
			} \n\
		} \n\
	}" << std::endl;

	ss <<
"	" << get_targetdictionaryTargetClassDefName(t) << "(const char* name, " << t.mElData.vrtlCxxType.substr(0, t.mElData.vrtlCxxType.find("[")) << "* data) \n\
		: TDentry(name, " << t.index << ", " << t.mElData.nmbBits << ") \n\
		, data(data) \n\
		, mask() {} \n\
};" << std::endl;

	t.mTD_typedef = ss.str();

	return (t.mTD_typedef);
}

int VapiGenerator::build_API(void) {
	std::string api_dir = std::string(outdir) + std::string("/") + std::string( API_DIRPREFIX) + std::string("/");

	if( !fs::exists(fs::path(api_dir)) ) {
		fs::create_directory( fs::path(api_dir) );
	}
	vapisrc_.write( (api_dir + std::string(API_SOURCE_NAME)).c_str());
	vapiheader_.write((api_dir + std::string(API_HEADER_NAME)).c_str());

	std::string targetdictionary_dir = api_dir + std::string(API_TD_DIRPREFIX) + std::string("/");
	if( !fs::exists(fs::path(targetdictionary_dir)) ) {
		fs::create_directory( fs::path(targetdictionary_dir) );
	}
	td_.write( (targetdictionary_dir + std::string(API_TD_HEADER_NAME)).c_str() );

	for (const auto &it : mTargets) {
		if ((it->mSeqInjCnt / it->mElData.words) > 2) {
			util::logging::log(util::logging::WARNING, std::string("More than 2 sequential injections for: ") + it->_self());
		}
	}
	return (1);
}

int VapiGenerator::build_targetdictionary_HPP(const char *outputdir) {
	std::string filepath = outputdir;
	filepath += "/";
	filepath += API_TD_HEADER_NAME;
	std::ifstream ifile(filepath);
	std::stringstream filetemplate;
	std::ofstream file;

	if (ifile.is_open()) {
		std::string tmp;
		while (std::getline(ifile, tmp)) {
			if (tmp.find("//<INSERT_HEADER_COMMMENT>") != std::string::npos) {
				filetemplate << get_fileheader(API_TD_HEADER_NAME);
			} else if (tmp.find("//<INSERT_TD_CLASSES>") != std::string::npos) {
				for (auto const &it : mTargets) {
					filetemplate << std::endl << get_targetdictionaryEntryTypeDefString(*it);
				}
				filetemplate << std::endl << "typedef struct sTD {" << std::endl;
				for (auto const &it : mTargets) {
					filetemplate << "\t" << get_targetdictionaryTargetClassDefName(*it) << "& " << get_targetdictionaryTargetClassDeclName(*it) << ";"
							<< std::endl;
				}
				filetemplate << "\tsTD(" << std::endl;
				bool first = true;
				for (auto const &it : mTargets) {
					if (first) {
						first = false;
					} else {
						filetemplate << ", " << std::endl;
					}
					filetemplate << "\t\t" << get_targetdictionaryTargetClassDefName(*it) << "& a" << it->index;
				}
				filetemplate << ") : " << std::endl;
				first = true;
				for (auto const &it : mTargets) {
					if (first) {
						first = false;
					} else {
						filetemplate << "," << std::endl;
					}
					filetemplate << "\t\t\t " << get_targetdictionaryTargetClassDeclName(*it) << "(a" << it->index << ")";
				}
				filetemplate << "{}" << std::endl;
				filetemplate << "} sTD_t;" << std::endl;

			} else {
				util::strhelp::replace(tmp, "//<INSERT_TOP_INCLUDE>", std::string("#include \"") + mTopTypeName + ".h\"");
				util::strhelp::replace(tmp, "<INSERT_VTOPTYPE>", mTopTypeName);
				filetemplate << tmp << std::endl;
			}
		}
		ifile.close();
	}

	file.open(filepath);
	if (file.fail()) {
		return (-1);
	}
	file << filetemplate.str();

	file.close();

	return (1);
}

int VapiGenerator::build_API_HPP(const char *outputdir) {
	std::ofstream file;
	std::string filepath = outputdir;
	filepath += "/";
	filepath += API_HEADER_NAME;
	std::ifstream ifile(filepath);
	std::stringstream filetemplate;


	if (ifile.is_open()) {
		std::string tmp;
		while (std::getline(ifile, tmp)) {
			if (tmp.find("//<INSERT_HEADER_COMMMENT>") != std::string::npos) {
				filetemplate << get_fileheader(API_HEADER_NAME);
			} else {
				util::strhelp::replace(tmp, "//<INSERT_TOP_INCLUDE>", std::string("#include \"") + mTopTypeName + ".h\"");
				util::strhelp::replace(tmp, "<INSERT_VTOPTYPE>", mTopTypeName);
				filetemplate << tmp << std::endl;
			}
		}
		ifile.close();
	}

	file.open(filepath);
	if (file.fail()) {
		return (-1);
	}
	file << filetemplate.str();
	file.close();

	return (1);
}

int VapiGenerator::build_API_CPP(const char *outputdir) {
	std::string filepath = outputdir;
	filepath += "/";
	filepath += API_SOURCE_NAME;
	std::ifstream ifile(filepath);
	std::stringstream filetemplate;
	std::ofstream file;

	if (ifile.is_open()) {
		std::string tmp;
		while (std::getline(ifile, tmp)) {
			filetemplate << tmp << std::endl;
		}
		ifile.close();
	}

	file.open(filepath);
	if (file.fail()) {
		return (-1);
	}
	file << get_fileheader(API_SOURCE_NAME);

	file << std::endl;
	file << "// Vrtl-specific includes:" << std::endl;
	file << "#include \"" << "../" << mTopTypeName << ".h\"" << std::endl;
	file << "#include \"" << "../" << mTopTypeName << "__Syms.h\"" << std::endl;
	file << "// General API includes:" << std::endl;
	file << filetemplate.str();
	file << std::endl;

	file << std::endl;
	file << "VRTLmodAPI::VRTLmodAPI(void) :" << std::endl << "\tmVRTL(* new " << mTopTypeName << ")," << std::endl << "TD_API()" << std::endl
			<< "{TD_API::init(mVRTL);}";
	file << std::endl;

	file << "void TD_API::init(" << mTopTypeName << "& pVRTL){" << std::endl;
	file << "mTD = new sTD(" << std::endl;
	std::string top = "pVRTL";

	bool first = true;
	for (auto const &it : mTargets) {
		if (first) {
			first = false;
		} else {
			file << "," << std::endl;
		}
		file << "\t\t* new " << get_targetdictionaryTargetClassDefName(*it) << "(\"" << it->mElData.hierarchy << "\", ";
		std::string hier = it->get_hierarchy();
		auto fdot = hier.find(".");
		if (fdot != std::string::npos) {
			if (it->mElData.words > 1) {
				file << top << ".__VlSymsp->TOPp->" << hier.substr(0, fdot) << "->" << hier.substr(fdot + 1);
			} else {
				file << "&(" << top << ".__VlSymsp->TOPp->" << hier.substr(0, hier.find(".")) << "->" << hier.substr(hier.find(".") + 1) << ")";
			}
		} else {
			if (it->mElData.words > 1) {
				file << top << ".__VlSymsp->TOPp->" << hier;
			} else {
				file << "&(" << top << ".__VlSymsp->TOPp->" << hier << ")";
			}
		}
		file << ")";
	}
	file << std::endl << "\t);" << std::endl;
	file << std::endl;
	for (auto const &it : mTargets) {
		file << "\tmEntryList.push_back(&(mTD->" << get_targetdictionaryTargetClassDeclName(*it) << "));" << std::endl;
	}
	file << "}" << std::endl;

	file.close();

	return (1);
}

std::string VapiGenerator::get_fileheader(const char *filename) {
	std::stringstream x;
	std::time_t timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	x << "////////////////////////////////////////////////////////////////////////////////" << std::endl;
	x << "/// @file " << API_TD_HEADER_NAME << std::endl;
	x << "/// @brief vrtlmod_api main header" << std::endl;
	x << "/// @details Automatically generated from: " << mFilepath << std::endl;
	x << "/// @date Created on " << std::ctime(&timestamp);
	x << "/// @author vapi_generator version " << APIBUILDER_VERSION << std::endl;
	x << "////////////////////////////////////////////////////////////////////////////////" << std::endl;
	return (x.str());
}
} // namespace vapi
