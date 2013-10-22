#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup()
{
    ofSetFrameRate(30);
    ofBackground(0);
    
    // set your hostname / username / password
    rovio.setup("10.4.0.96", "admin", "admin");
    ofAddListener(rovio.responseReceived, this, &testApp::onRovioResponseReceived);
    ofAddListener(rovio.statusUpdated, this, &testApp::onRovioStatusUpdated);
    rovio.start();
    rovio.startStreaming();
}

//--------------------------------------------------------------
void testApp::update()
{
    rovio.update();
}

//--------------------------------------------------------------
void testApp::draw()
{
    ofPushMatrix();
    {
        float x = position.x * 300;
        float y = position.y * 300;
        
        ofTranslate(ofGetWidth() * 0.5, ofGetHeight() * 0.5);
        
        ofPushMatrix();
        {
            ofRotateX(90);
            ofDrawGrid(300, 10, false, false, true, false);
        }
        ofPopMatrix();
        
        ofEnableAlphaBlending();
        ofPushMatrix();
        {
            ofTranslate(x, y);
            ofRotateZ(degrees);
            
            ofDrawAxis(100);
            
            ofSetColor(14, 102, 237, 200 - 200 * ((float)(ofGetFrameNum() % 30) / 30.0));
            ofFill();
            ofCircle(0, 0, 15 + (1.3 * (float)(ofGetFrameNum() % 30)));
            ofDisableAlphaBlending();
            ofCircle(0, 0, 15);
        }
        ofPopMatrix();
    }
    ofPopMatrix();
    
    rovio.draw(0, 0, 320, 240);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key)
{
    switch (key)
	{
		case OF_KEY_UP:
			rovio.moveForward(SPEED_FASTEST);
			break;
            
		case OF_KEY_DOWN:
			rovio.moveBackward(SPEED_FASTEST);
			break;
			
		case OF_KEY_LEFT:
			rovio.moveLeft(SPEED_FASTEST);
			break;
			
		case OF_KEY_RIGHT:
			rovio.moveRight(SPEED_FASTEST);
			break;
			
		default:
			break;
	}
}

void testApp::onRovioStatusUpdated(ofxRovio::Status &status)
{
    ofLog() << "status updated";
    position = status.getNormalizedPosition();
    degrees = ofRadToDeg(status.getTheta());
}

void testApp::onRovioResponseReceived(RovioHttpResponse &response)
{
    if (200 == response.status)
    {
        Action actionCommand = response.actionCommand;
        
        switch (actionCommand)
        {
            case GET_REPORT:
                ofLog() << rovio.getStatus().getPosition();
                break;
                
            default:
                break;
        }
    }
}

void testApp::exit()
{
    rovio.stop();
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
