#include "ofxRovio.h"
#include "Poco/StringTokenizer.h"

using namespace ofxRovio;
using Poco::StringTokenizer;

#pragma mark - class RovioHttpResponse

RovioHttpResponse::RovioHttpResponse(HTTPResponse& pocoResponse, std::istream &bodyStream, string turl, Action actionCommand, bool binary) : ofxHttpResponse(pocoResponse, bodyStream, turl, binary)
{
    this->actionCommand = actionCommand;
}


#pragma mark - class RovioHttpUtils

void RovioHttpUtils::addUrl(string url)
{
    RovioHttpForm form;
	form.action=url;
	form.method=OFX_HTTP_GET;
    form.name=form.action;
    
	RovioHttpUtils::addForm(form);
}

void RovioHttpUtils::addUrl(string url, Action actionCommand)
{
    RovioHttpForm form;
	form.action=url;
	form.method = OFX_HTTP_GET;
    form.name=form.action;
    form.actionCommand = actionCommand;
    
	RovioHttpUtils::addForm(form);
}

void RovioHttpUtils::addForm(RovioHttpForm form)
{
	lock();
    forms.push(form);
    unlock();
}

RovioHttpResponse RovioHttpUtils::getUrl(string url, Action actionCommand)
{
    RovioHttpResponse response;
    try
    {
		URI uri(url.c_str());
		std::string path(uri.getPathAndQuery());
		
        if (path.empty()) path = "/";
        
		HTTPClientSession session(uri.getHost(), uri.getPort());
		HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
		
        if (auth.getUsername() != "") auth.authenticate(req);
		
        session.setTimeout(Poco::Timespan(timeoutSeconds,0));
		session.sendRequest(req);
        
		HTTPResponse res;
		istream& rs = session.receiveResponse(res);
        
		response = RovioHttpResponse(res, rs, path, actionCommand);
        
		if (response.status >= 300 && response.status < 400)
        {
			Poco::URI uri(req.getURI());
			uri.resolve(res.get("Location"));
			response.location = uri.toString();
		}
        
		ofNotifyEvent( rovioResponseEvent, response, this );
	}
    catch (Exception& exc)
    {
		std::cerr << exc.displayText() << "\n";
	}
	return response;
}

void RovioHttpUtils::threadedFunction()
{
    while( isThreadRunning() == true )
    {
		lock();
    	if (forms.size() > 0)
        {
			RovioHttpForm form = forms.front();
	    	unlock();
			if (form.method == OFX_HTTP_POST)
            {
				doPostForm(form);
				printf("RovioHttpForm: (thread running) form submitted (post): %s\n", form.name.c_str());
			}
            else
            {
				string url = ofxHttpUtils::generateUrl(form);
                Action actionCommand = form.actionCommand;
				printf("RovioHttpForm: (thread running) form submitted (get): %s\n", form.name.c_str());
				RovioHttpUtils::getUrl(url, actionCommand);
			}
    		lock();
			forms.pop();
	    	unlock();
    	}
        else
        {
    		unlock();
    	}
    	ofSleepMillis(10);
    }
    
}


#pragma mark - class Status

Status::Status()
{
    
}

Status::~Status()
{
    
}

void Status::setStatusFromResponse(RovioHttpResponse response)
{
    if (200 != response.status) return;
    
    string body = response.responseBody.getText();
    
    StringTokenizer t1(body, "\n");
    
    for (StringTokenizer::Iterator it = t1.begin(); t1.end() != it; ++it)
    {
        StringTokenizer t2(*it, "|", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
        
        for (StringTokenizer::Iterator it2 = t2.begin(); t2.end() != it2; ++it2)
        {
            StringTokenizer t3(*it2, "=", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
            
            string key = t3[0];
            DynamicAny value(t3[1]);
            
            if ("x" == key) {
                position.x = value.convert<float>();
            } else if ("y" == key) {
                position.y = value.convert<float>();
            } else if ("theta" == key) {
                theta = value.convert<float>();
            } else if ("room" == key) {
                roomID = (RoomID)value.convert<int>();
            } else if ("ss" == key) {
                navigationSignalStrength = value.convert<unsigned int>();
            } else if ("beacon" == key) {
                beaconSignalStrength = value.convert<unsigned int>();
            } else if ("beacon_x" == key) {
                beaconX = value.convert<int>();
            } else if ("state" == key) {
                state = (State)value.convert<int>();
            } else if ("flags" == key) {
                flag = (Flag)value.convert<int>();
            } else if ("charging" == key) {
                charging = value.convert<int>();
            }
        }
    }
}

void Status::setStatus(string response)
{
    StringTokenizer t1(response, "\n");
    
    for (StringTokenizer::Iterator it = t1.begin(); t1.end() != it; ++it)
    {
        StringTokenizer t2(*it, "|", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
        
        for (StringTokenizer::Iterator it2 = t2.begin(); t2.end() != it2; ++it2)
        {
            StringTokenizer t3(*it2, "=", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
            
            string key = t3[0];
            DynamicAny value(t3[1]);
            
            if ("x" == key) {
                position.x = value.convert<float>();
            } else if ("y" == key) {
                position.y = value.convert<float>();
            } else if ("theta" == key) {
                theta = value.convert<float>();
            } else if ("room" == key) {
                roomID = (RoomID)value.convert<int>();
            } else if ("ss" == key) {
                navigationSignalStrength = value.convert<unsigned int>();
            } else if ("beacon" == key) {
                beaconSignalStrength = value.convert<unsigned int>();
            } else if ("beacon_x" == key) {
                beaconX = value.convert<int>();
            } else if ("state" == key) {
                state = (State)value.convert<int>();
            } else if ("flags" == key) {
                flag = (Flag)value.convert<int>();
            } else if ("charging" == key) {
                charging = value.convert<int>();
            }
        }
    }
}

ofPoint Status::getPosition()
{
    return position;
}

ofPoint Status::getNormalizedPosition()
{
    return position / ofPoint(32767, 32767, 1);
}

float Status::getTheta()
{
    return theta;
}

RoomID Status::getRoomID()
{
    return roomID;
}

SignalStrength Status::getNavigationSignalStrength()
{
    if (47000 < navigationSignalStrength)
    {
        return SIGNAL_STRONG;
    }
    else if (5000 > navigationSignalStrength)
    {
        return SIGNAL_WEAK;
    }
    return SIGNAL_MEDIUM;
}

SignalStrength Status::getBeaconSignalStrength()
{
    if (47000 < beaconSignalStrength)
    {
        return SIGNAL_STRONG;
    }
    else if (5000 > beaconSignalStrength)
    {
        return SIGNAL_WEAK;
    }
    return SIGNAL_MEDIUM;
}

State Status::getState()
{
    return state;
}


#pragma mark - class Rovio

Rovio::Rovio()
{
    ipVidGrabber = ofx::Video::IPVideoGrabber::makeShared();
}

Rovio::~Rovio()
{
    stop();
}

void Rovio::setup(const string hostname, const string username, const string password)
{
    this->hostname = hostname;
    this->username = username;
    this->password = password;
    
    httpUtils.setBasicAuthentication(this->username, this->password);
}

void Rovio::start()
{
    ofAddListener(httpUtils.rovioResponseEvent, this, &Rovio::newResponseEvent);
    httpUtils.start();
    bStart = true;
}

void Rovio::stop()
{
    stopStreaming();
    ofRemoveListener(httpUtils.rovioResponseEvent, this, &Rovio::newResponseEvent);
    httpUtils.stop();
    bStart = false;
}

void Rovio::startStreaming()
{
    ipVidGrabber->setUsername(username);
    ipVidGrabber->setPassword(password);
    ipVidGrabber->setURI("http://" + hostname + "/GetData.cgi");
    ipVidGrabber->connect();
    bStartStreaming = true;
}

void Rovio::stopStreaming()
{
    ipVidGrabber->disconnect();
    ipVidGrabber->close();
    bStartStreaming = false;
}

void Rovio::update()
{
    if (bStartStreaming) ipVidGrabber->update();
}

void Rovio::draw()
{
    ofSetColor(255, 255, 255);
    ipVidGrabber->draw(0, 0);
}

void Rovio::draw(float x, float y)
{
    ofSetColor(255, 255, 255);
    ipVidGrabber->draw(x, y);
}

void Rovio::draw(float x, float y, float width, float height)
{
    ofSetColor(255, 255, 255);
    ipVidGrabber->draw(x, y, width, height);
}


#pragma mark - Movement Control

void Rovio::getReport()
{
    sendCommandRequest(GET_REPORT);
}

void Rovio::startRecording()
{
    sendCommandRequest(START_RECORDING);
}

void Rovio::abortRecording()
{
    sendCommandRequest(ABORT_RECORDING);
}

void Rovio::stopRecording(const string name)
{
    map<string, DynamicAny> params;
    params.insert(map<string, DynamicAny>::value_type("name", name));
    sendCommandRequest(STOP_RECORDING, params);
}

void Rovio::deletePath(const string name)
{
    map<string, DynamicAny> params;
    params.insert(map<string, DynamicAny>::value_type("name", name));
    sendCommandRequest(DELETE_PATH, params);
}

void Rovio::getPathList()
{
    sendCommandRequest(GET_PATH_LIST);
}

void Rovio::playPathForward(const string name)
{
    map<string, DynamicAny> params;
    params.insert(map<string, DynamicAny>::value_type("name", name));
    sendCommandRequest(PLAY_PATH_FORWARD, params);
}

void Rovio::playPathBackward(const string name)
{
    map<string, DynamicAny> params;
    params.insert(map<string, DynamicAny>::value_type("name", name));
    sendCommandRequest(PLAY_PATH_BACKWARD, params);
}

void Rovio::stopPlaying()
{
    sendCommandRequest(STOP_PLAYING);
}

void Rovio::pausePlaying()
{
    sendCommandRequest(PAUSE_PLAYING);
}

void Rovio::renamePath(const string oldName, const string newName)
{
    map<string, DynamicAny> params;
    params.insert(map<string, DynamicAny>::value_type("name", oldName));
    params.insert(map<string, DynamicAny>::value_type("newname", newName));
    sendCommandRequest(RENAME_PATH, params);
}

void Rovio::goHome()
{
    sendCommandRequest(GO_HOME);
}

void Rovio::goHomeAndDock()
{
    sendCommandRequest(GO_HOME_AND_DOCK);
}

void Rovio::updateHomePosition()
{
    sendCommandRequest(UPDATE_HOME_POSITION);
}

void Rovio::setTuningParams()
{
    sendCommandRequest(SET_TUNING_PARAMS);
}

void Rovio::getTuningParams()
{
    sendCommandRequest(GET_TUNING_PARAMS);
}

void Rovio::resetNavStateMachine()
{
    sendCommandRequest(RESET_NAV_STATE_MACHINE);
}

void Rovio::getMCUReport()
{
    sendCommandRequest(GET_MCU_REPORT);
}

void Rovio::clearAllPaths()
{
    sendCommandRequest(CLEAR_ALL_PATHS);
}

void Rovio::getState()
{
    sendCommandRequest(GET_STATUS);
}

void Rovio::saveParameters(long index, long value)
{
    map<string, DynamicAny> params;
    params.insert(map<string, DynamicAny>::value_type("index", index));
    params.insert(map<string, DynamicAny>::value_type("value", value));
    sendCommandRequest(SAVE_PARAMS, params);
}

void Rovio::readParameters(long index)
{
    map<string, DynamicAny> params;
    params.insert(map<string, DynamicAny>::value_type("index", index));
    sendCommandRequest(READ_PARAMS, params);
}

void Rovio::getLibNSVersion()
{
    sendCommandRequest(GET_LIBNS_VERSION);
}

void Rovio::sendMailWithImage(const string address)
{
    map<string, DynamicAny> params;
    params.insert(map<string, DynamicAny>::value_type("email", address));
    sendCommandRequest(SEND_MAIL, params);
}

void Rovio::resetHomeLocation()
{
    sendCommandRequest(RESET_HOME_LOCATION);
}


#pragma mark - Drive Modes

void Rovio::stopMoving()
{
    manualDrive(STOP_MOVING, SPEED_FASTEST);
}

void Rovio::moveForward(DriveSpeed speed)
{
    manualDrive(MOVE_FORWARD, speed);
}

void Rovio::moveBackward(DriveSpeed speed)
{
    manualDrive(MOVE_BACKWARD, speed);
}

void Rovio::moveLeft(DriveSpeed speed)
{
    manualDrive(MOVE_LEFT, speed);
}

void Rovio::moveRight(DriveSpeed speed)
{
    manualDrive(MOVE_RIGHT, speed);
}

void Rovio::rotateLeft(ofxRovio::DriveSpeed speed)
{
    manualDrive(ROTATE_LEFT_BY_SPEED, speed);
}

void Rovio::rotateRight(ofxRovio::DriveSpeed speed)
{
    manualDrive(ROTATE_RIGHT_BY_SPEED, speed);
}

void Rovio::moveDiagonallyForwardLeft(ofxRovio::DriveSpeed speed)
{
    manualDrive(DIAGONAL_FORWARD_LEFT, speed);
}

void Rovio::moveDiagonallyForwardRight(ofxRovio::DriveSpeed speed)
{
    manualDrive(DIAGONAL_FORWARD_RIGHT, speed);
}

void Rovio::moveDiagonallyBackwardLeft(ofxRovio::DriveSpeed speed)
{
    manualDrive(DIAGONAL_BACKWARD_LEFT, speed);
}

void Rovio::moveDiagonallyBackwardRight(ofxRovio::DriveSpeed speed)
{
    manualDrive(DIAGONAL_BACKWARD_RIGHT, speed);
}

void Rovio::headUp()
{
    manualDrive(HEAD_UP, SPEED_FASTEST);
}

void Rovio::headDown()
{
    manualDrive(HEAD_DOWN, SPEED_FASTEST);
}

void Rovio::headMiddle()
{
    manualDrive(HEAD_MIDDLE, SPEED_FASTEST);
}

void Rovio::rotateLeftBy20Degrees(ofxRovio::DriveSpeed speed)
{
    manualDrive(ROTATE_LEFT_BY_20DEGREES, speed);
}

void Rovio::rotateRightBy20Degrees(ofxRovio::DriveSpeed speed)
{
    manualDrive(ROTATE_RIGHT_BY_20DEGREES, speed);
}

void Rovio::manualDrive(DriveMode mode, DriveSpeed speed)
{
    map<string, DynamicAny> params;
    params.insert(map<string, DynamicAny>::value_type("drive", DynamicAny((int)mode)));
    params.insert(map<string, DynamicAny>::value_type("speed", DynamicAny((int)speed)));
    sendCommandRequest(MANUAL_DRIVE, params);
}


#pragma mark - Camera Controls

void Rovio::setCameraCompressRatio(CameraCompressRatio ratio)
{
    map<string, DynamicAny> params;
    params.insert(map<string, DynamicAny>::value_type("Ratio", DynamicAny((int)ratio)));
    sendRequest("/ChangeCompressRatio.cgi", params);
}

void Rovio::setCameraResolution(CameraResolution resolution)
{
    map<string, DynamicAny> params;
    params.insert( map<string, DynamicAny>::value_type("ResType", DynamicAny((int)resolution)) );
    sendRequest("/ChangeResolution.cgi", params);
}

void Rovio::setCameraFrameRate(int framerate)
{
    map<string, DynamicAny> params;
    params.insert( map<string, DynamicAny>::value_type("Framerate", DynamicAny(framerate)) );
    sendRequest("/ChangeFramerate.cgi", params);
}

void Rovio::setCameraBrightness(CameraBrightness brightness)
{
    map<string, DynamicAny> params;
    params.insert( map<string, DynamicAny>::value_type("Brightness", DynamicAny((int)brightness)) );
    sendRequest("/ChangeBrightness.cgi", params);
}

void Rovio::setSpeakerVolume(int volume)
{
    map<string, DynamicAny> params;
    params.insert( map<string, DynamicAny>::value_type("SpeakerVolume", DynamicAny(volume)) );
    sendRequest("/ChangeSpeakerVolume.cgi", params);
}

void Rovio::setMicVolume(int volume)
{
    map<string, DynamicAny> params;
    params.insert( map<string, DynamicAny>::value_type("MicVolume", DynamicAny(volume)) );
    sendRequest("/ChangeMicVolume.cgi", params);
}


#pragma mark - Request

void Rovio::sendRequest(string url, map<string, DynamicAny> params)
{
    string request = "http://" + hostname + url + "?";
    string paramStr = "";
    
    for (map<string, DynamicAny>::iterator it = params.begin(); it != params.end(); ++it)
    {
        if (it != params.begin()) paramStr += "&";
        paramStr += (*it).first + "=" + (*it).second.convert<string>();
    }
    httpUtils.addUrl(request + paramStr);
}

void Rovio::sendCommandRequest(Action action)
{
    string request = "http://" + hostname + "/rev.cgi?Cmd=nav&action=" + ofToString(action);
    httpUtils.addUrl(request, action);
}

void Rovio::sendCommandRequest(Action action, map<string, Poco::DynamicAny> params)
{
    string request = "http://" + hostname + "/rev.cgi?Cmd=nav&action=" + ofToString(action);
    
    for (map<string, DynamicAny>::iterator it = params.begin(); params.end() != it; ++it)
    {
        request += "&" + (*it).first + "=" + (*it).second.convert<string>();
    }
    httpUtils.addUrl(request, action);
}


#pragma mark -

void Rovio::newResponseEvent(RovioHttpResponse &response)
{
    if (GET_REPORT == response.actionCommand)
    {
        status.setStatusFromResponse(response);
        ofNotifyEvent(Rovio::statusUpdated, status, this);
    }
    ofNotifyEvent(Rovio::responseReceived, response, this);
}


#pragma mark -

string Rovio::getHostname()
{
    return "http://" + hostname;
}

string Rovio::getUsername()
{
    return username;
}

string Rovio::getPassword()
{
    return password;
}

ofx::Video::SharedIPVideoGrabber Rovio::getStream()
{
    return ipVidGrabber;
}

Status Rovio::getStatus()
{
    return status;
}
