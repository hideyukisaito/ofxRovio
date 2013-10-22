ofxRovio
===

openFrameworks addon for communicating and controlling WowWee's Rovio.  
**The implementation is not correct. Please use at your own risk.**

## Dependencies


This addon depends on the followings:

- ofxHttpUtils

## Quick Example

```
using namespace ofxRovio;

Rovio rovio;

void testApp::setup()
{
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

void testApp::onRovioResponseReceived(ofxRovio::RovioHttpResponse &response)
{
	if (200 == response.status)
	{
		Action actionCommand = response.actionCommand;
		
		switch (actionCommand)
		{
			case GET_REPORT:
				ofLog() << rovio.getStatus()->getPosition();
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
