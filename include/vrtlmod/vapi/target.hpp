////////////////////////////////////////////////////////////////////////////////
/// @file target.hpp
/// @date Created on Mon Jan 10 18:21:20 2020
/// @modified on Wed Dec 09 13:32:12 2020 (johannes.geier@tum.de)
/// @author Johannes Geier (johannes.geier@tum.de)
////////////////////////////////////////////////////////////////////////////////

#ifndef __VRTLMOD_VAPI_TARGET_HPP__
#define __VRTLMOD_VAPI_TARGET_HPP__

#include <string>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
/// @brief namespace for all API building functionalities
namespace vapi {
class VapiGenerator;
////////////////////////////////////////////////////////////////////////////////
/// @struct sXmlEl
/// @brief C++ struct equivalent for RegPicker's Xml elements
/// @author Johannes Geier (johannes.geier@tum.de)
////////////////////////////////////////////////////////////////////////////////
typedef struct sXmlEl {
	typedef enum {
		REG = 1, CONST = 2, WIRE = 3, UNDEF = 0
	} signalClass_t;
	///////////////////////////////////////////////////////////////////////
	/// \brief Element's name
	std::string name;
	///////////////////////////////////////////////////////////////////////
	/// \brief Element's VRTL hierarchy
	std::string hierarchy;
	///////////////////////////////////////////////////////////////////////
	/// \brief Element's signal class
	signalClass_t signalClass;
	///////////////////////////////////////////////////////////////////////
	/// \brief Element's number of bits
	unsigned int nmbBits;
	///////////////////////////////////////////////////////////////////////
	/// \brief Element's VRTL type
	std::string type;
	///////////////////////////////////////////////////////////////////////
	/// \brief Number of words
	unsigned int words;
	///////////////////////////////////////////////////////////////////////
	/// \brief Element's VRTL c++ type (CData, IData, WData[..], ...)
	std::string vrtlCxxType;
	///////////////////////////////////////////////////////////////////////
	/// \brief Copy Constructor
	sXmlEl(const sXmlEl &x) :
			name(x.name), hierarchy(x.hierarchy), signalClass(x.signalClass), nmbBits(x.nmbBits), type(x.type), words(x.words), vrtlCxxType(x.vrtlCxxType) {
	}
	///////////////////////////////////////////////////////////////////////
	/// \brief Constructor
	sXmlEl(const char *name = "", const char *hierarchy = "", const char *signalClass_s = "", unsigned int nmbBits = 0, const char *type = "",
			const char *vrtlCxxType = "") :
				name(name), hierarchy(hierarchy), signalClass(UNDEF), nmbBits(nmbBits), type(type), words(1), vrtlCxxType(vrtlCxxType) {
			if (signalClass_s != NULL) {
				std::string x = signalClass_s;
				if (x == "register") {
					signalClass = REG;
				} else if (x == "wire") {
					signalClass = WIRE;
				} else if (x == "constant") {
					signalClass = CONST;
				} else {
					signalClass = UNDEF;
				}
			}
			if (sXmlEl::vrtlCxxType.find("[") != std::string::npos) {
				auto brOpen = sXmlEl::vrtlCxxType.find('[');
				auto brClose = sXmlEl::vrtlCxxType.find(']');
				words = std::stoi(sXmlEl::vrtlCxxType.substr(brOpen + 1, brClose - brOpen - 1));

				if (sXmlEl::nmbBits <= 32) {
					sXmlEl::nmbBits = words * sXmlEl::nmbBits;
				}
			}
		}
} sXmlEl_t;

class ExprT {
public:
	std::string expr;
	std::string prefix;
	std::string object;
	std::string name;
	ExprT(const char *Expr);
};

////////////////////////////////////////////////////////////////////////////////
/// @class Target
/// @brief Corresponds to RegPicker-Xml element information
/// @author Johannes Geier (johannes.geier@tum.de)
////////////////////////////////////////////////////////////////////////////////
class Target {
	friend VapiGenerator;
protected:
	///////////////////////////////////////////////////////////////////////
	/// \brief Number of times the target received a SEQ_INJ
	unsigned mSeqInjCnt;
	///////////////////////////////////////////////////////////////////////
	/// \brief Unique index of target
	unsigned int index;
public:
	///////////////////////////////////////////////////////////////////////
	/// \brief Xml data
	sXmlEl_t mElData;
	///////////////////////////////////////////////////////////////////////
	/// \brief String containing the target's API target dictionary definition string
	std::string mTD_typedef;
	///////////////////////////////////////////////////////////////////////
	/// \brief String containing the target's API target dictionary declaration string
	std::string mTD_decl;
	///////////////////////////////////////////////////////////////////////
	/// \brief Returns the targets hierarchy
	std::string get_hierarchy(void);
	///////////////////////////////////////////////////////////////////////
	/// \brief Returns the targets undotted hierarchy ("__DOT__"s instead of "."s)
	std::string get_hierarchyDedotted(void);
	///////////////////////////////////////////////////////////////////////
	/// \brief Returns the targets index
	unsigned int get_index(void) const {
		return (index);
	}
	///////////////////////////////////////////////////////////////////////
	/// \brief Self representation of this target
	std::string _self(void) const;
	///////////////////////////////////////////////////////////////////////
	/// \brief Constructor
	/// \param index
	/// \param data
	Target(unsigned int index, sXmlEl_t &data);
	///////////////////////////////////////////////////////////////////////
	/// \brief Destructor
	virtual ~Target(void) {
	}
	///////////////////////////////////////////////////////////////////////
	/// \brief Overloaded operator for stream access of class (reference)
	friend std::ostream& operator<<(std::ostream &os, const Target &obj) {
		os << obj._self();
		return os;
	}
	///////////////////////////////////////////////////////////////////////
	/// \brief Overloaded operator for stream access of class (pointer)
	friend std::ostream& operator<<(std::ostream &os, const Target *obj) {
		os << *obj;	//->_self();
		return os;
	}
};

} // namespace vapi

#endif /* __VRTLMOD_VAPI_TARGET_HPP__ */