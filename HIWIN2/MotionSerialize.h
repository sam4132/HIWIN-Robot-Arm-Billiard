#ifndef _MITSUBISHI_SERIALIZE_H_
#define _MITSUBISHI_SERIALIZE_H_

#include <string>

namespace rbt {
namespace melfa {

std::string typeToString(int typeCode);

std::string dataToString(double* data);

std::string motionToString(int typeCode, double* data);

}  // namespace melfa
}  // namespace rbt

#endif
