#ifndef MITSUBISHI_H
#define MITSUBISHI_H

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include "MotionTypeDef.h"

namespace rbt {
namespace melfa {

class Motion {
   public:
    Motion(int typeCode, const std::vector<double>& values)
        : typeCode(typeCode), values(values) {
        if (values.size() < 6) {
            throw std::runtime_error("Invalid vector should >= 6");
        }
    }

    Motion(int typeCode, const std::initializer_list<double>& values)
        : Motion(typeCode, {values.begin(), values.end()}) {}

    int typeCode;
    std::vector<double> values;
};

typedef std::vector<Motion> BatchMotion;

class Mitsubishi {
   public:
    Mitsubishi();

    Mitsubishi(const std::string& ip, int port = 10001);

    virtual ~Mitsubishi();

    void connect(const std::string& ip, int port = 10001);

    void setSpeed(int ovrd);

    int getSpeed();

    void setAcceleration(int accel);

    int getAcceleration();

    void setPTPSpeed(int speed);

    int getPTPSpeed();

    void setLinSpeed(double speed);

    double getLinSpeed();

    std::string getState();

    void setServoState(int state);

    int getServoState();

    bool isOperating();

    void errorReset();

    int getErrorCode();

    std::string getErrorMessage();

    void movePtpAbsCoord(double* coords);

    void movePtpAbsJoint(double* joints);

    void movePtpRelCoord(double* coords);

    void movePtpRelJoint(double* joints);

    void moveLinAbsCoord(double* coords);

    void moveLinAbsJoint(double* joints);

    void moveLinRelCoord(double* coords);

    void moveLinRelJoint(double* joints);

    void movePtpAbsCoord(double* coords, int flag1, int flag2 = 0);

    void movePtpRelCoord(double* coords, int flag1, int flag2 = 0);

    void moveLinAbsCoord(double* coords, int flag1, int flag2 = 0);

    void moveLinRelCoord(double* coords, int flag1, int flag2 = 0);

    void move(int typeCode, double* values);

    void move(int typeCode, double* values, int flag1, int flag2 = 0);

    std::vector<double> getCurrentJoint();

    std::vector<double> getCurrentPosition();

    std::pair<int, int> getCurrentPoseFlag();

    void waitForIdle(int timeout = -1);

    void setToolCoord(double* coords);

    void setBaseCoord(double* coords);

    std::vector<double> getToolCoord();

    std::vector<double> getBaseCoord();

    void startContMotion(const BatchMotion& motion);

    void endContMotion();

    std::vector<std::string> listPrograms();

    void overwriteProgram(const std::string& name,
                          const std::vector<std::string>& commands);

    void selectProgram(const std::string& name);

    void runCurrentSelectedProgram();

   private:
    boost::asio::ip::tcp::iostream socketstream;

    std::string ip;
    int port;

    std::string sendBuf;
    std::string recvBuf;

    int seqNum;

    void sendInitCmd();

    void takeOperationRight();

    void releaseOperationRight();

    void sendReqWithOperationRight();

    void sendRequest();

    std::string sendRequest0(const std::string& cmd);

    static std::string encodeHCCommand(int num, char type,
                                       const std::string& cmd);

    static std::string decodeHCCommand(std::string HCCmd);

    static bool checkQoK(const std::string& cmd);

    static bool checkAlrm(const std::string& cmd);

    static std::vector<std::string> splitCmd(const std::string& cmd);

    static std::string vec2Str(double* values);

    static std::vector<double> parseVector(const std::string& str);

    static int parseIntVal(const std::string& str);

    static double parseDoubleVal(const std::string& str);
};

}  // namespace melfa
}  // namespace rbt

#endif  // MITSUBISHI_H
