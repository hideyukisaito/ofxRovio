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
    ofxHttpUtils::addUrl(url);
}

void RovioHttpUtils::addUrl(string url, Action actionCommand)
{
    RovioHttpForm form;
	form.action=url;
	form.method = OFX_HTTP_GET;
    form.name=form.action;
    form.actionCommand = actionCommand;
    
	ofxHttpUtils::addForm(form);
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
				printf("ofxHttpUtils: (thread running) form submitted (post): %s\n", form.name.c_str());
			}
            else
            {
				string url = ofxHttpUtils::generateUrl(form);
                Action actionCommand = form.actionCommand;
				printf("ofxHttpUtils: (thread running) form submitted (get): %s\n", form.name.c_str());
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

void Status::setStatus(ofxHttpResponse response)
{
    if (200 != response.status) return;
    
    string body = response.responseBody.getText();
    ofLog() << body;
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


#pragma mark - class Rovio

Rovio::Rovio()
{
    string res = "content: Cmd = nav\n"
    "responses = 0|x=2940|y=1308|theta=2.995|room=0|ss=10444\n"
    "|beacon=0|beacon_x=0|next_room=2|next_room_ss=694\n"
    "|state=0|ui_status=0|resistance=0|sm=15|pp=0|flags=0005\n"
    "|brightness=6|resolution=3|video_compression=2|frame_rate=30\n"
    "|privilege=0|user_check=1|speaker_volume=31|mic_volume=16\n"
    "|wifi_ss=199|show_time=0|ddns_state=0|email_state=0\n"
    "|battery=120|charging=72|head_position=203|ac_freq=2";
    
    status.setStatus(res);
}

Rovio::~Rovio()
{
    httpUtils.stop();
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
    getReport();
}

void Rovio::stop()
{
    httpUtils.stop();
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

void Rovio::setCameraQuality(CameraQuality quality)
{
    map<string, DynamicAny> params;
    params.insert(map<string, DynamicAny>::value_type("Ratio", DynamicAny((int)quality)));
    sendRequest("/ChangeCompressRatio.cgi", params);
}

void Rovio::setCameraBrightness(CameraBrightness brightness)
{
    map<string, DynamicAny> params;
    params.insert( map<string, DynamicAny>::value_type("Brightness", DynamicAny((int) brightness)) );
    sendRequest("/ChangeBrightness.cgi", params);
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
    ofNotifyEvent(Rovio::responseReceived, response);
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

Status Rovio::getStatus()
{
    return status;
}