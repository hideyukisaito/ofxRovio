#pragma once

#include "ofMain.h"
#include "ofxHttpUtils.h"
#include "IPVideoGrabber.h"
#include "Poco/DynamicAny.h"

using Poco::DynamicAny;

namespace ofxRovio
{
    // representing image quality
    enum CameraCompressRatio {
        CAM_COMPRESS_LOW    = 0,
        CAM_COMPRESS_MEDIUM = 1,
        CAM_COMPRESS_HIGH   = 2,
    };
    
    enum CameraBrightness {
        CAM_BRIGHTNESS_DARKEST   = 0,
        CAM_BRIGHTNESS_DARKER    = 1,
        CAM_BRIGHTNESS_DARK      = 2,
        CAM_BRIGHTNESS_MEDIUM    = 3,
        CAM_BRIGHTNESS_BRIGHT    = 4,
        CAM_BRIGHTNESS_BRIGHTER  = 5,
        CAM_BRIGHTNESS_BRIGHTEST = 6,
    };
    
    enum CameraResolution {
        QCIF = 0, // 176x144
        QVGA = 1, // 320x240
        SIF  = 2, // 352x240
        VGA  = 3, // 640x480
    };
    
    enum DriveSpeed {
        SPEED_FASTEST     = 1,
        SPEED_FASTER      = 2,
        SPEED_BIT_FASTER  = 3,
        SPEED_FAST        = 4,
        SPEED_MEDIUM_FAST = 5,
        SPEED_MEDIUM_SLOW = 6,
        SPEED_SLOW        = 7,
        SPEED_BIT_SLOWER  = 8,
        SPEED_SLOWER      = 9,
        SPEED_SLOWEST     = 10,
    };
    
    enum SignalStrength {
        SIGNAL_STRONG,
        SIGNAL_MEDIUM,
        SIGNAL_WEAK
    };
    
    enum RoomID {
        HOME_BASE = 0,
        MUTABLE_ROOM_PROJECTOR,
    };
    
    enum NextRoom {
        NO_ROOM_FOUND = -1,
        MUTABLE_ROOM_1 = 1,
        MUTABLE_ROOM_2 = 2,
        MUTABLE_ROOM_3 = 3,
        MUTABLE_ROOM_4 = 4,
        MUTABLE_ROOM_5 = 5,
        MUTABLE_ROOM_6 = 6,
        MUTABLE_ROOM_7 = 7,
        MUTABLE_ROOM_8 = 8,
        MUTABLE_ROOM_9 = 9,
    };
    
    enum State {
        STATE_IDLE           = 0,
        STATE_DRIVING_HOME   = 1,
        STATE_DOCKING        = 2,
        STATE_EXECUTING_PATH = 3,
        STATE_RECORDING_PATH = 4,
    };
    
    enum Flag {
        FLAG_HOME_POSITION     = 1,
        FLAG_OBSTACLE_DETECTED = 2,
        FLAG_IR_ACTIVATED      = 4,
    };
    
    enum BatteryStatus {
        BATTERY_NORMAL,
        BATTERY_MEDIUM,
        BATTERY_LOW,
    };
    
    enum ChargingStatus {
        CHARGING,
        NOT_CHARGING,
    };
    
    enum HeadPosition {
        HEAD_POSITION_LOW,
        HEAD_POSITION_MID,
        HEAD_POSITION_HIGH,
    };
    
    enum Action {
        GET_REPORT              = 1,
        START_RECORDING         = 2,
        ABORT_RECORDING         = 3,
        STOP_RECORDING          = 4,
        DELETE_PATH             = 5,
        GET_PATH_LIST           = 6,
        PLAY_PATH_FORWARD       = 7,
        PLAY_PATH_BACKWARD      = 8,
        STOP_PLAYING            = 9,
        PAUSE_PLAYING           = 10,
        RENAME_PATH             = 11,
        GO_HOME                 = 12,
        GO_HOME_AND_DOCK        = 13,
        UPDATE_HOME_POSITION    = 14,
        SET_TUNING_PARAMS       = 15,
        GET_TUNING_PARAMS       = 16,
        RESET_NAV_STATE_MACHINE = 17,
        MANUAL_DRIVE            = 18,
        GET_MCU_REPORT          = 20,
        CLEAR_ALL_PATHS         = 21,
        GET_STATUS              = 22,
        SAVE_PARAMS             = 23,
        READ_PARAMS             = 24,
        GET_LIBNS_VERSION       = 25,
        SEND_MAIL               = 26,
        RESET_HOME_LOCATION     = 27,
    };
    
    enum DriveMode {
        STOP_MOVING               = 0,
        MOVE_FORWARD              = 1,
        MOVE_BACKWARD             = 2,
        MOVE_LEFT                 = 3,
        MOVE_RIGHT                = 4,
        ROTATE_LEFT_BY_SPEED      = 5,
        ROTATE_RIGHT_BY_SPEED     = 6,
        DIAGONAL_FORWARD_LEFT     = 7,
        DIAGONAL_FORWARD_RIGHT    = 8,
        DIAGONAL_BACKWARD_LEFT    = 9,
        DIAGONAL_BACKWARD_RIGHT   = 10,
        HEAD_UP                   = 11,
        HEAD_DOWN                 = 12,
        HEAD_MIDDLE               = 13,
        ROTATE_LEFT_BY_20DEGREES  = 17,
        ROTATE_RIGHT_BY_20DEGREES = 18,
    };
    
    class RovioHttpForm;
    class RovioHttpResponse;
    class RovioHttpUtils;
    class Status;
    class Rovio;
    
};


class ofxRovio::RovioHttpForm : public ofxHttpForm
{

public:
    Action actionCommand;
};


class ofxRovio::RovioHttpResponse : public ofxHttpResponse
{

public:

    RovioHttpResponse() {}
    RovioHttpResponse(HTTPResponse& pocoResponse, std::istream &bodyStream, string turl, Action actionCommand, bool binary = false);
    
    Action actionCommand;
};


class ofxRovio::RovioHttpUtils : public ofxHttpUtils
{

public:
    
    void addUrl(string url);
    void addUrl(string url, Action actionCommand);
    RovioHttpResponse getUrl(string url, Action actionCommand);
    
    void threadedFunction();
    
    ofEvent<RovioHttpResponse> rovioResponseEvent;
    
protected:
    
    std::queue<RovioHttpForm> forms;
    
};


class ofxRovio::Status
{
    
public:
    
    Status();
    ~Status();
    
    void setStatusFromResponse(RovioHttpResponse response);
    void setStatus(string response);
    
    ofPoint getPosition();
    ofPoint getNormalizedPosition();
    float getTheta();
    RoomID getRoomID();
    SignalStrength getNavigationSignalStrength();
    SignalStrength getBeaconSignalStrength();
    int getBeaconX();
    NextRoom getNextRoom();
    State getState();
    int getCurrentPathNum();
    CameraBrightness getBrightness();
    BatteryStatus getBatteryStatus();
    bool isCharging();
    HeadPosition getHeadPosition();
    int getACFrequencies();
    
private:
    
    ofPoint position;
    float theta;
    RoomID roomID;
    unsigned int navigationSignalStrength;
    unsigned int beaconSignalStrength;
    int beaconX;
    State state;
    Flag flag;
    int charging;
};


class ofxRovio::Rovio
{
    
public:
    
    Rovio();
    ~Rovio();
    
    
    void setup(const string hostname, const string username = "", const string password = "");
    void start();
    void stop();
    void startStreaming();
    void stopStreaming();
    void update();
    void draw();
    void draw(float x, float y);
    void draw(float x, float y, float width, float height);
    
    void getReport();
    void startRecording();
    void abortRecording();
    void stopRecording(const string name);
    void deletePath(const string name);
    void getPathList();
    void playPathForward(const string name);
    void playPathBackward(const string name);
    void stopPlaying();
    void pausePlaying();
    void renamePath(const string oldName, const string newName);
    void goHome();
    void goHomeAndDock();
    void updateHomePosition();
    void setTuningParams();
    void getTuningParams();
    void resetNavStateMachine();
    void getMCUReport();
    void clearAllPaths();
    void getState();
    void saveParameters(long index, long value);
    void readParameters(long index);
    void getLibNSVersion();
    void sendMailWithImage(const string address);
    void resetHomeLocation();
    
    void stopMoving();
    void moveForward(DriveSpeed speed);
    void moveBackward(DriveSpeed speed);
    void moveLeft(DriveSpeed speed);
    void moveRight(DriveSpeed speed);
    void rotateLeft(DriveSpeed speed);
    void rotateRight(DriveSpeed speed);
    void moveDiagonallyForwardLeft(DriveSpeed speed);
    void moveDiagonallyForwardRight(DriveSpeed speed);
    void moveDiagonallyBackwardLeft(DriveSpeed speed);
    void moveDiagonallyBackwardRight(DriveSpeed speed);
    void headUp();
    void headDown();
    void headMiddle();
    void rotateLeftBy20Degrees(DriveSpeed speed);
    void rotateRightBy20Degrees(DriveSpeed speed);
    
    void setCameraCompressRatio(CameraCompressRatio ratio);
    void setCameraResolution(CameraResolution resolution);
    void setCameraFrameRate(int framerate);
    void setCameraBrightness(CameraBrightness brightness);
    void setSpeakerVolume(int volume);
    void setMicVolume(int volume);
    
    string getHostname();
    string getUsername();
    string getPassword();
    
    ofx::Video::SharedIPVideoGrabber getStream();
    
    Status getStatus();
    
    ofEvent<RovioHttpResponse> responseReceived;
    ofEvent<Status> statusUpdated;
    
private:

    RovioHttpUtils httpUtils;
    
    ofx::Video::SharedIPVideoGrabber ipVidGrabber;
    
    string hostname;
    string username;
    string password;
    bool bStart;
    bool bStartStreaming;
    
    Status status;
    
    void manualDrive(DriveMode mode, DriveSpeed speed);
    void sendRequest(string url, map<string, DynamicAny> params);
    void sendCommandRequest(Action action);
    void sendCommandRequest(Action action, map<string, DynamicAny> params);
    void newResponseEvent(RovioHttpResponse &response);
};