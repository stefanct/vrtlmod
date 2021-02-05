////////////////////////////////////////////////////////////////////////////////
/// @file vrtlmodapi_header.cpp
/// @date Created on Wed Dec 09 13:32:12 2020 (johannes.geier@tum.de)
/// @author Johannes Geier (johannes.geier@tum.de)
////////////////////////////////////////////////////////////////////////////////

#include "vrtlmod/vapi/generator.hpp"

namespace vapi {

void VapiGenerator::VapiHeader::generate_body(void){
	std::stringstream x, entries;
	VapiGenerator& gen = VapiGenerator::_i();

	x <<
"#ifndef __" << gen.mTopTypeName << "VRTLMODAPI_VRTLMODAPI_HPP__ \n\
#define __" << gen.mTopTypeName << "VRTLMODAPI_VRTLMODAPI_HPP__ \n"
		<< std::endl
		<<
"#include <vector> \n\
#include <memory> \n\
#include \"verilated.h\" \n\
#include \"" << gen.get_targetdictionary_relpath() << "\" \n"
		<<
"class " << gen.mTopTypeName << ";" << std::endl
		<< std::endl
    <<
"#define FI_LIKELY(x)   __builtin_expect(!!(x), 1) \n\
#define FI_UNLIKELY(x) __builtin_expect(!!(x), 0) \n\n\
#define SEQ_TARGET_INJECT(TDentry) { \\\n\
	if(FI_UNLIKELY((TDentry).enable)) { \\\n\
		if(((TDentry).cntr <= 0) and (TDentry).mask) { \\\n\
			if(FI_LIKELY(((TDentry).inj_type == INJ_TYPE::BITFLIP))){ \\\n\
				*((TDentry).data) = *((TDentry).data) ^ (TDentry).mask; \\\n\
			} \\\n\
			(TDentry).cntr++; \\\n\
		} \\\n\
	} \\\n\
}"
		<<  std::endl
		<<
"#define SEQ_TARGET_INJECT_W(TDentry, word) { \\\n\
	if(FI_UNLIKELY((TDentry).enable)) { \\\n\
		if(((TDentry).cntr <= 0) and (TDentry).mask[(word)]) { \\\n\
			if(FI_LIKELY(((TDentry).inj_type == INJ_TYPE::BITFLIP))){ \\\n\
				((TDentry).data[(word)]) = ((TDentry).data[(word)]) ^ (TDentry).mask[(word)]; \\\n\
			} \\\n\
			(TDentry).cntr++; \\\n\
		} \\\n\
	} \\\n\
}"
		<<  std::endl
		<<
"#define INT_TARGET_INJECT(TDentry) { \\\n\
	SEQ_TARGET_INJECT(TDentry) \\\n\
}"
		<<	std::endl
		<<
"#define INT_TARGET_INJECT_W(TDentry, words) { \\\n\
	for(unsigned i = 0; i < words; ++i) { \\\n\
		SEQ_TARGET_INJECT_W(TDentry, i) } \\\n\
}"
		<<  std::endl
		<<
"class " << gen.mTopTypeName << "VRTLmodAPI : public TD_API { \n\
public: \n\
	static " << gen.mTopTypeName << "VRTLmodAPI& i(void) { \n\
		static " << gen.mTopTypeName << "VRTLmodAPI _instance; \n\
		return (_instance); \n\
	} \n\
	private: \n\
	" << gen.mTopTypeName << "VRTLmodAPI(void); \n\
	" << gen.mTopTypeName << "VRTLmodAPI(" << gen.mTopTypeName << "VRTLmodAPI const&); \n\
	void operator=(" << gen.mTopTypeName << "VRTLmodAPI const&); \n\
public: \n\
	virtual ~" << gen.mTopTypeName << "VRTLmodAPI(void) { } \n\
}; \n"
		<< std::endl
		<<
"#endif /* __" << gen.mTopTypeName << "VRTLMODAPI_VRTLMODAPI_HPP__ */";

	body_ = x.str();
}

} // namespace vapi
