ofxRovio
===

openFrameworks addon for communicating and controlling WowWee's Rovio.  
**The implementation is not correct, WIP.**

## Dependencies


This addon depends on the followings:

- ofxHttpUtils

## Quick Example

```
using namespace ofxRovio;

void testApp::setup()
{
	Rovio rovio;
	// set username and password, if needed
	rovio.setup("localhost", "username", "password");
	ofAddListener(rovio.responseReceived, this, &testApp::onRovioResponseReceived);
	rovio.start();
	
	// change camera settings
	rovio.setCameraQuality(CAM_QUALITY_HIGH);
	rovio.setCameraBrightness(CAM_BRIGHTNESS_BRIGHTEST);
}

…

void testApp::keyPressed(int key)
{
	if (OF_KEY_UP == key) {
		rovio.moveForward(SPEED_FASTEST);
	} else if (OF_KEY_DOWN == key) {
		rovio.moveBackward(SPEED_FASTEST);
	} else if (OF_KEY_LEFT == key) {
		rovio.moveLeft(SPEED_FASTEST);
	} else if (OF_KEY_RIGHT == key) {
		rovio.moveRight(SPEED_FASTEST);
	} else {
		rovio.stopMoving();
	}
}

void testApp::onRovioResponseReceived(ofxRovio::RovioHttpResponse &response)
{
	if (200 == response.status) {
		Action actionCommand = response.actionCommand;
		
		switch (actionCommand) {
			case GET_REPORT:
				ofLog() << rovio.getStatus().getPosition();
				break;
				
			default:
				break;
		}
	}
}
```

## License
The MIT License (MIT)

Copyright (c) 2013 hideyuki saito

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.