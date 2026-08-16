#include "MotionSerialize.h"
#include <sstream>

#include "vector"

namespace rbt {
namespace melfa {

std::string typeToString(int typeCode) {
    std::string ret;
    std::vector<std::string> list = {
        "None",        "PtpAbsCoord", "PtpAbsJoint",
        "LinAbsCoord", "LinAbsJoint", "PtpRelCoord",
        "PtpRelJoint", "LinRelCoord", "LinRelJoint",
    };
    ret = list.at(typeCode + 1);
    return ret;
}

std::string dataToString(double* data) {
    std::stringstream ss;
    ss << '(';
    for (int i = 0; i < 6; ++i) {
        if (i) {
            ss << ", ";
        }
        ss << data[i];
    }
    ss << ')';
    return ss.str();
}

std::string motionToString(int typeCode, double* data) {
    std::string motionStr =
        "Motion: " + typeToString(typeCode) + dataToString(data);
    return motionStr;
}

}  // namespace melfa
}  // namespace rbt
