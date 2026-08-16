#ifndef _MOTIONTYPEDEF_H_
#define _MOTIONTYPEDEF_H_

namespace rbt {
namespace melfa {

enum FL1 {
    FLIP = 0,
    NON_FLIP = 1,

    BELOW = 0,
    ABOVE = 2,

    LEFT = 0,
    RIGHT = 4,
};

enum TypeBit { CoordBit = 0, CtrlBit = 1, MoveBit = 2 };

// Enumeration for Coordinate or Joint
enum CoordType { Coord = 0, Joint = 1 };

// Enumeration for PTP or Linear
enum CtrlType { PTP = 0, Linear = 2 };

// Enumeration for Absolute or Relative
enum MoveType { Absolute = 0, Relative = 4 };

enum TypeCode {
    None = -1,
    PtpAbsCoord = 0,
    PtpAbsJoint = 1,
    LinAbsCoord = 2,
    LinAbsJoint = 3,
    PtpRelCoord = 4,
    PtpRelJoint = 5,
    LinRelCoord = 6,
    LinRelJoint = 7,
};

}  // namespace melfa
}  // namespace rbt

#endif
