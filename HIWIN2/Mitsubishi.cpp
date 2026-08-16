#include "Mitsubishi.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <boost/system/error_code.hpp>

using namespace std;
using namespace boost::asio;

namespace rbt {
namespace melfa {

Mitsubishi::Mitsubishi() : socketstream(), ip(), port(0), seqNum(0) {}

Mitsubishi::Mitsubishi(const string& ip, int port)
    : socketstream(), ip(ip), port(port), seqNum(0) {
    this->connect(ip, port);
}

Mitsubishi::~Mitsubishi() {}

void Mitsubishi::connect(const string& ip, int port) {
    this->ip = ip;
    this->port = port;
    ip::address_v4 addr = ip::address_v4::from_string(ip);
    ip::tcp::endpoint endpoint(addr, port);
    socketstream.connect(endpoint);
    sendBuf.reserve(200);
    recvBuf.reserve(200);
    sendInitCmd();
}

void Mitsubishi::setSpeed(int ovrd) {
    sendBuf = "1;1;OVRD=" + to_string(ovrd);
    this->sendReqWithOperationRight();
}

int Mitsubishi::getSpeed() {
    sendBuf = "1;1;OVRD";
    this->sendRequest();
    return std::stoi(this->recvBuf.substr(3));
}

void Mitsubishi::setAcceleration(int accel) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("Accel ");
    sendBuf.append(to_string(accel));
    sendBuf.append(", ");
    sendBuf.append(to_string(accel));
    this->sendReqWithOperationRight();
}

int Mitsubishi::getAcceleration() {
    sendBuf = "1;1;VALM_Acl;M";
    this->sendRequest();
    return this->parseIntVal(this->recvBuf);
}

void Mitsubishi::setPTPSpeed(int speed) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("JOvrd ");
    sendBuf.append(to_string(speed));
    this->sendReqWithOperationRight();
}

int Mitsubishi::getPTPSpeed() {
    sendBuf = "1;1;VALM_JOvrd;M";
    this->sendRequest();
    return this->parseIntVal(this->recvBuf);
}

void Mitsubishi::setLinSpeed(double speed) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("Spd ");
    sendBuf.append(to_string(speed));
    this->sendReqWithOperationRight();
}

double Mitsubishi::getLinSpeed() {
    sendBuf = "1;1;VALM_Spd;M";
    this->sendRequest();
    return this->parseDoubleVal(this->recvBuf);
}

string Mitsubishi::getState() {
    sendBuf = "1;1;STATE";
    this->sendRequest();
    vector<string> strs = splitCmd(this->recvBuf);
    return strs[4];
}

void Mitsubishi::setServoState(int state) {
    if (state) {
        sendBuf = "1;1;SRVON";
        this->sendReqWithOperationRight();
        int i = 100;
        while (i-- > 0) {
            string state = this->getState();
            if ((std::stoi(state.substr(0, 1), 0, 16) & 2) > 0) {
                break;
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }

    } else {
        sendBuf = "1;1;SRVOFF";
        this->sendRequest();
    }
}

int Mitsubishi::getServoState() {
    string state = this->getState();
    return (std::stoi(state.substr(0, 1), 0, 16) & 2) > 0;
}

bool Mitsubishi::isOperating() {
    string state = this->getState();
    if (!checkAlrm(this->recvBuf)) {
        throw runtime_error("Controller Alarm: " + state.substr(4));
    }
    return (std::stoi(state.substr(0, 1), 0, 16) & 4) > 0;
}

void Mitsubishi::errorReset() {
    sendBuf = "1;1;RSTALRM";
    this->sendRequest();
}

int Mitsubishi::getErrorCode() {
    string state = this->getState();
    return std::stoi(state.substr(4));
}

string Mitsubishi::getErrorMessage() {
    string ret;
    sendBuf = "1;1;ERRORRD";
    this->sendRequest();
    vector<string> strs = this->splitCmd(this->recvBuf);
    if (strs.size() > 0) {
        ret = strs[2] + ": " + strs[3];
    }
    return ret;
}

void Mitsubishi::movePtpAbsCoord(double* coords) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("Ptmp = ");
    sendBuf.append(vec2Str(coords));
    this->sendReqWithOperationRight();
    sendBuf = "1;1;EXECMov Ptmp";
    this->sendReqWithOperationRight();
    if (!checkAlrm(this->recvBuf)) {
        throw runtime_error("Controller Alarm: " +
                            to_string(this->getErrorCode()));
    }
}

void Mitsubishi::movePtpAbsJoint(double* joints) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("Jtmp = ");
    sendBuf.append(vec2Str(joints));
    this->sendReqWithOperationRight();
    sendBuf = "1;1;EXECMov Jtmp";
    this->sendReqWithOperationRight();
    if (!checkAlrm(this->recvBuf)) {
        throw runtime_error("Controller Alarm: " +
                            to_string(this->getErrorCode()));
    }
}

void Mitsubishi::movePtpRelCoord(double* coords) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("Ptmp = ");
    sendBuf.append(vec2Str(coords));
    this->sendReqWithOperationRight();
    sendBuf = "1;1;EXECMov Ptmp+P_Curr";
    this->sendReqWithOperationRight();
    if (!checkAlrm(this->recvBuf)) {
        throw runtime_error("Controller Alarm: " +
                            to_string(this->getErrorCode()));
    }
}

void Mitsubishi::movePtpRelJoint(double* joints) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("Jtmp = ");
    sendBuf.append(vec2Str(joints));
    this->sendReqWithOperationRight();
    sendBuf = "1;1;EXECMov Jtmp+J_Curr";
    this->sendReqWithOperationRight();
    if (!checkAlrm(this->recvBuf)) {
        throw runtime_error("Controller Alarm: " +
                            to_string(this->getErrorCode()));
    }
}

void Mitsubishi::moveLinAbsCoord(double* coords) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("Ptmp = ");
    sendBuf.append(vec2Str(coords));
    this->sendReqWithOperationRight();
    sendBuf = "1;1;EXECMvs Ptmp";
    this->sendReqWithOperationRight();
    if (!checkAlrm(this->recvBuf)) {
        throw runtime_error("Controller Alarm: " +
                            to_string(this->getErrorCode()));
    }
}

void Mitsubishi::moveLinAbsJoint(double* joints) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("Jtmp = ");
    sendBuf.append(vec2Str(joints));
    this->sendReqWithOperationRight();
    sendBuf = "1;1;EXECMvs Jtmp";
    this->sendReqWithOperationRight();
    if (!checkAlrm(this->recvBuf)) {
        throw runtime_error("Controller Alarm: " +
                            to_string(this->getErrorCode()));
    }
}

void Mitsubishi::moveLinRelCoord(double* coords) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("Ptmp = ");
    sendBuf.append(vec2Str(coords));
    this->sendReqWithOperationRight();
    sendBuf = "1;1;EXECMvs Ptmp+P_Curr";
    this->sendReqWithOperationRight();
    if (!checkAlrm(this->recvBuf)) {
        throw runtime_error("Controller Alarm: " +
                            to_string(this->getErrorCode()));
    }
}

void Mitsubishi::moveLinRelJoint(double* joints) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("Jtmp = ");
    sendBuf.append(vec2Str(joints));
    this->sendReqWithOperationRight();
    sendBuf = "1;1;EXECMvs Jtmp+J_Curr";
    this->sendReqWithOperationRight();
    if (!checkAlrm(this->recvBuf)) {
        throw runtime_error("Controller Alarm: " +
                            to_string(this->getErrorCode()));
    }
}

void Mitsubishi::movePtpAbsCoord(double* coords, int flag1, int flag2) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("Ptmp = ");
    sendBuf.append(vec2Str(coords));
    sendBuf.append("(" + to_string(flag1) + "," + to_string(flag2) + ")");
    this->sendReqWithOperationRight();
    sendBuf = "1;1;EXECMov Ptmp";
    this->sendReqWithOperationRight();
    if (!checkAlrm(this->recvBuf)) {
        throw runtime_error("Controller Alarm: " +
                            to_string(this->getErrorCode()));
    }
}

void Mitsubishi::movePtpRelCoord(double* coords, int flag1, int flag2) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("Ptmp = ");
    sendBuf.append(vec2Str(coords));
    sendBuf.append("(" + to_string(flag1) + "," + to_string(flag2) + ")");
    this->sendReqWithOperationRight();
    sendBuf = "1;1;EXECMov Ptmp+P_Curr";
    this->sendReqWithOperationRight();
    if (!checkAlrm(this->recvBuf)) {
        throw runtime_error("Controller Alarm: " +
                            to_string(this->getErrorCode()));
    }
}

void Mitsubishi::moveLinAbsCoord(double* coords, int flag1, int flag2) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("Ptmp = ");
    sendBuf.append(vec2Str(coords));
    sendBuf.append("(" + to_string(flag1) + "," + to_string(flag2) + ")");
    this->sendReqWithOperationRight();
    sendBuf = "1;1;EXECMvs Ptmp Type 0,2";
    this->sendReqWithOperationRight();
    if (!checkAlrm(this->recvBuf)) {
        throw runtime_error("Controller Alarm: " +
                            to_string(this->getErrorCode()));
    }
}

void Mitsubishi::moveLinRelCoord(double* coords, int flag1, int flag2) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("Ptmp = ");
    sendBuf.append(vec2Str(coords));
    sendBuf.append("(" + to_string(flag1) + "," + to_string(flag2) + ")");
    this->sendReqWithOperationRight();
    sendBuf = "1;1;EXECMvs Ptmp+P_Curr Type 0,2";
    this->sendReqWithOperationRight();
    if (!checkAlrm(this->recvBuf)) {
        throw runtime_error("Controller Alarm: " +
                            to_string(this->getErrorCode()));
    }
}

void Mitsubishi::move(int typeCode, double* values) {
    switch (typeCode) {
        case TypeCode::None:
            // Nothing
            break;
        case TypeCode::PtpAbsCoord:
            movePtpAbsCoord(values);
            break;
        case TypeCode::PtpAbsJoint:
            movePtpAbsJoint(values);
            break;
        case TypeCode::LinAbsCoord:
            moveLinAbsCoord(values);
            break;
        case TypeCode::LinAbsJoint:
            moveLinAbsJoint(values);
            break;
        case TypeCode::PtpRelCoord:
            movePtpRelCoord(values);
            break;
        case TypeCode::PtpRelJoint:
            movePtpRelJoint(values);
            break;
        case TypeCode::LinRelCoord:
            moveLinRelCoord(values);
            break;
        case TypeCode::LinRelJoint:
            moveLinRelJoint(values);
            break;
        default:
            throw runtime_error("Invalid typecode");
    }
}

void Mitsubishi::move(int typeCode, double* values, int flag1, int flag2) {
    if ((typeCode & CoordBit) == CoordType::Coord) {
        switch (typeCode) {
            case TypeCode::PtpAbsCoord:
                movePtpAbsCoord(values, flag1, flag2);
                break;
            case TypeCode::LinAbsCoord:
                moveLinAbsCoord(values, flag1, flag2);
                break;
            case TypeCode::PtpRelCoord:
                movePtpRelCoord(values, flag1, flag2);
                break;
            case TypeCode::LinRelCoord:
                moveLinRelCoord(values, flag1, flag2);
                break;
            default:
                throw runtime_error("Invalid TypeCode");
        }
    } else {
        this->move(typeCode, values);
    }
}

vector<double> Mitsubishi::getCurrentJoint() {
    vector<double> ret(6, 0);
    sendBuf = "1;1;JPOSF";
    this->sendRequest();
    vector<string> strs = splitCmd(this->recvBuf);
    for (size_t i = 0; i < 6; i++) {
        ret[i] = std::stod(strs[2 * i + 1]);
    }
    return ret;
}

vector<double> Mitsubishi::getCurrentPosition() {
    vector<double> ret(6, 0);
    sendBuf = "1;1;PPOSF";
    this->sendRequest();
    vector<string> strs = splitCmd(this->recvBuf);
    for (size_t i = 0; i < 6; i++) {
        ret[i] = std::stod(strs[i * 2 + 1]);
    }
    return ret;
}

pair<int, int> Mitsubishi::getCurrentPoseFlag() {
    sendBuf = "1;1;PPOSF";
    this->sendRequest();
    vector<string> strs = splitCmd(this->recvBuf);
    string flag = strs[13];
    size_t sp = flag.find(",");
    int fl1 = stoi(flag);
    int fl2 = stoi(flag.substr(sp + 1));
    return make_pair(fl1, fl2);
}

void Mitsubishi::waitForIdle(int timeout) {
    if (timeout < 0) {
        while (this->isOperating()) {
            this_thread::sleep_for(chrono::milliseconds(5));
        }
    } else {
        chrono::system_clock::time_point timestamp =
            chrono::system_clock::now();
        while (true) {
            chrono::system_clock::time_point now = chrono::system_clock::now();
            if (!this->isOperating() || (now - timestamp).count() > timeout) {
                break;
            }
            this_thread::sleep_for(chrono::milliseconds(5));
        }
    }
}

void Mitsubishi::setToolCoord(double* coords) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("Tool ");
    sendBuf.append(vec2Str(coords));
    this->sendReqWithOperationRight();
}

void Mitsubishi::setBaseCoord(double* coords) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("Base ");
    sendBuf.append(vec2Str(coords));
    this->sendReqWithOperationRight();
}

vector<double> Mitsubishi::getToolCoord() {
    vector<double> ret(8, 0);
    sendBuf = "1;1;VALP_Tool;P";
    this->sendRequest();
    ret = this->parseVector(this->recvBuf);
    return ret;
}

vector<double> Mitsubishi::getBaseCoord() {
    vector<double> ret(8, 0);
    sendBuf = "1;1;VALP_Base;P";
    this->sendRequest();
    ret = this->parseVector(this->recvBuf);
    return ret;
}

void Mitsubishi::startContMotion(const BatchMotion& motion) {
    std::vector<string> commandVec;
    commandVec.push_back("Cnt 1");
    for (size_t i = 0; i < motion.size(); ++i) {
        Motion m = motion[i];
        if ((m.typeCode & MoveBit) == Relative) {
            throw runtime_error("Not support relative motion");
        }
        string varName = "P1";
        if ((m.typeCode & CoordBit) == Coord) {
            varName = "P" + to_string(i + 1);
        } else {
            varName = "J" + to_string(i + 1);
        }
        string methodName = "Mov";
        if ((m.typeCode & CtrlBit) == PTP) {
            methodName = "Mov";
        } else {
            methodName = "Mvs";
        }

        commandVec.push_back(varName + " = " + vec2Str(m.values.data()));
        commandVec.push_back(methodName + " " + varName);
    }
    commandVec.push_back("Hlt");

    overwriteProgram("TEST", commandVec);
    selectProgram("TEST");
    runCurrentSelectedProgram();
}

void Mitsubishi::endContMotion() { selectProgram("TEST"); }

std::vector<std::string> Mitsubishi::listPrograms() {
    std::vector<std::string> programNames;
    sendBuf = "1;1;PDIR<";
    this->sendRequest();
    std::vector<std::string> ret = splitCmd(this->recvBuf);
    int count = stoi(ret[3]);
    for (int i = 1; i <= count; ++i) {
        char buf[100] = {};
        sprintf(buf, "1;1;PDIR%02d", i);
        sendBuf = buf;
        this->sendRequest();
        std::vector<std::string> ret2 = splitCmd(this->recvBuf);
        string name = ret2[0].substr(0, ret2[0].size() - 4);
        programNames.push_back(name);
    }
    return programNames;
}

void Mitsubishi::overwriteProgram(const std::string& name,
                                  const std::vector<std::string>& commands) {
    sendBuf = "1;1;NEW";
    this->sendRequest();
    sendBuf = "1;1;ECLR";
    this->sendRequest();
    sendBuf = "1;1;LOAD=" + name;
    this->sendRequest();
    sendBuf = "1;1;ECLR";
    this->sendRequest();
    sendBuf = "1;1;PRTVERVALS";
    this->sendRequest();
    for (size_t i = 0; i < commands.size(); ++i) {
        sendBuf = "1;1;EMDAT" + to_string(i + 1) + " " + commands[i];
        this->sendRequest();
    }
    sendBuf = "1;1;SAVE";
    this->sendRequest();
}

void Mitsubishi::selectProgram(const std::string& name) {
    sendBuf = "1;1;XSTP";
    this->sendRequest();
    sendBuf = "1;1;XRST";
    this->sendRequest();
    sendBuf = "1;1;XCLR";
    this->sendRequest();
    sendBuf = "1;1;PRGLOAD=" + name;
    this->sendReqWithOperationRight();
}

void Mitsubishi::runCurrentSelectedProgram() {
    sendBuf = "1;0;RUN;-1";
    sendReqWithOperationRight();
}

/*
void Mitsubishi::setContMotion(bool enable) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("Cnt ");
    if (enable) {
        sendBuf.append("1");
    } else {
        sendBuf.append("0");
    }
    this->sendReqWithOperationRight();
}*/

/*
void Mitsubishi::setContMotion(int startDis, int endDis) {
    sendBuf = "1;1;EXEC";
    sendBuf.append("Cnt 1");
    sendBuf.append(to_string(startDis));
    sendBuf.append(" ");
    sendBuf.append(to_string(endDis));
    this->sendReqWithOperationRight();
}*/

/*
void Mitsubishi::delay(double sec) {
    sendBuf = "1;1;EXECDly ";
    sendBuf.append(to_string(sec));
    this->sendReqWithOperationRight();
}*/

void Mitsubishi::sendInitCmd() {
    // write(socket, buffer("1;1;OPEN=TOOLBOX"));
    socketstream << "1;1;OPEN=TOOLBOX" << flush;

    string recv;
    char recvBuf[512] = {};
    socketstream.peek();
    size_t inBytes = socketstream.readsome(recvBuf, 511);
    recvBuf[inBytes] = '\0';
    recv = recvBuf;

    // cout << recv.size() << ":" << recv << endl;
    if (!checkQoK(recv)) throw std::runtime_error("Error on OPEN");
    // write(socket, buffer("1;1;CHGPRT=HC"));
    socketstream << "1;1;CHGPRT=HC" << flush;
    socketstream.peek();
    inBytes = socketstream.readsome(recvBuf, 511);
    recvBuf[inBytes] = '\0';
    recv = recvBuf;

    // cout << recv.size() << ":"<< recv << endl;
    if (!checkQoK(recv)) throw std::runtime_error("Error on CHGPRT");
}

void Mitsubishi::takeOperationRight() {
    int i = 5;
    bool ok = false;
    while (i-- > 0 && !ok) {
        string recv = this->sendRequest0("1;1;CNTLON");
        ok = this->checkQoK(recv);
        if (!ok) {
            this_thread::sleep_for(chrono::milliseconds(50));
        }
    }
    if (!ok) {
        throw runtime_error("Fail to take operation right.");
    }
}

void Mitsubishi::releaseOperationRight() {
    string recv = this->sendRequest0("1;1;CNTLOFF");
    if (!this->checkQoK(recv)) {
        throw runtime_error("Command: 1;1;CNTLOFF Failed.");
    }
}

void Mitsubishi::sendReqWithOperationRight() {
    this->takeOperationRight();
    this->recvBuf = this->sendRequest0(this->sendBuf);
    this->releaseOperationRight();
    if (!this->checkQoK(this->recvBuf)) {
        throw runtime_error("Command: \"" + this->sendBuf +
                            "\" Failed: " + this->recvBuf);
    }
}

void Mitsubishi::sendRequest() {
    this->recvBuf = sendRequest0(this->sendBuf);
    if (!this->checkQoK(this->recvBuf)) {
        throw runtime_error("Command: \"" + this->sendBuf +
                            "\" Failed: " + this->recvBuf);
    }
}

string Mitsubishi::sendRequest0(const string& cmd) {
    string ret;
    socketstream << encodeHCCommand(this->seqNum, 'R', cmd) << flush;

    string recv;
    std::getline(socketstream, recv, (char)0x03);

    // cout << recv.size() << ":" << recv << endl;
    recv = decodeHCCommand(recv);
    // cout << recv.size() << ":" << recv << endl;
    if (recv != "00") throw runtime_error("Error on recieve A cmd: " + recv);

    std::getline(socketstream, recv, (char)0x03);
    ret = decodeHCCommand(recv);
    // cout << recvBuf << endl;

    socketstream << encodeHCCommand(this->seqNum, 'J', "00") << flush;
    seqNum = (seqNum + 1) % 999;
    return ret;
}

string Mitsubishi::encodeHCCommand(int num, char type, const string& cmd) {
    string ret;
    ret.reserve(100);
    char buf[10];
    ret.push_back(2);
    ret.append("HC");
    std::sprintf(buf, "%03d", num);
    ret.append(buf);
    ret.append("000000");
    ret.push_back(type);
    std::sprintf(buf, "%04X", (unsigned int)cmd.size());
    ret.append(buf);
    ret.append(cmd);
    int checksum = 0;
    for (size_t i = 1; i < ret.size(); i++) {
        checksum ^= ret[i];
    }
    std::sprintf(buf, "%02X", checksum);
    ret.append(buf);
    ret.push_back(3);

    return ret;
}

string Mitsubishi::decodeHCCommand(string HCCmd) {
    int start = HCCmd.find((char)2) + 17;
    int end = HCCmd.size() - 2;
    return HCCmd.substr(start, end - start);
}

bool Mitsubishi::checkQoK(const std::string& cmd) {
    if (cmd.size() < 3) throw std::runtime_error("Invalid cmd length!");
    if (cmd.substr(0, 3) == "QoK") {
        return true;
    } else if (cmd.substr(0, 3) == "Qok") {
        return true;
    } else {
        return false;
    }
}

bool Mitsubishi::checkAlrm(const std::string& cmd) {
    if (cmd.size() < 3) throw std::runtime_error("Invalid cmd length!");
    if (cmd.substr(0, 3) == "QoK") {
        return true;
    } else {
        return false;
    }
}

vector<string> Mitsubishi::splitCmd(const std::string& cmd) {
    vector<string> ret;
    string buf;
    stringstream ss(cmd.substr(3));
    while (getline(ss, buf, ';')) {
        ret.push_back(buf);
    }
    return ret;
}

string Mitsubishi::vec2Str(double* values) {
    string ret = "(";
    for (int i = 0; i < 6; i++) {
        if (i) {
            ret.push_back(',');
        }
        ret.append(to_string(values[i]));
    }
    ret.push_back(')');
    return ret;
}

vector<double> Mitsubishi::parseVector(const std::string& str) {
    vector<double> ret;
    size_t startIdx = str.find("(", 0);
    size_t endIdx = str.find(")", startIdx);
    if (startIdx != string::npos && endIdx != string::npos) {
        stringstream ss(str.substr(startIdx + 1, endIdx - startIdx - 1));
        string s;
        while (getline(ss, s, ',')) {
            ret.push_back(stod(s));
        }
    }
    return ret;
}

int Mitsubishi::parseIntVal(const std::string& str) {
    size_t idx = str.find("=");
    string s = str.substr(idx + 1);
    return stoi(s);
}

double Mitsubishi::parseDoubleVal(const std::string& str) {
    size_t idx = str.find("=");
    string s = str.substr(idx + 1);
    return stod(s);
}

}  // namespace melfa
}  // namespace rbt
