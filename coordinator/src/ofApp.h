#pragma once

#include "ofMain.h"
//#include "ofxCv.h"
#include "ofxSVG.h"
#include "ofxDatGui.h"
#include "ofxOsc.h"
#include "ofxUDPManager.h"
#include "ofxJSON.h"

#define PORT 5100

//#include <opencv2/aruco/charuco.hpp>
typedef struct {
	int idx;
	ofVec3f tvec, rvec;

	ofMatrix4x4 mat;
	ofVec3f worldPos;
	float rot;
} marker;

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

//	bool recordCharuco();
//	float calibrate();

//	bool saveCalibration(string filename, bool absolute = false);
//	bool loadCalibration(string filename, bool absolute = false);

private:
//	bool isCalibrating;
//	bool doneCalibrating;

//	ofVideoGrabber cam;
//	vector<ofVideoDevice> camDeviceList;

//	ofxCv::Calibration calibration;
//	ofMatrix4x4 modelMatrix;
//	bool found;
//	cv::Size patternSize;
//	ofLight light;

	bool mouseDown;
	ofPoint mouseDownPos, mouseCurPos;

//	cv::Mat cameraMatrix, distCoeffs;
//	vector<cv::Mat> rvecs, tvecs;
//
//	vector<vector<vector<cv::Point2f>>> allCharucoCorners;
//	vector<vector<int>> allCharucoIds;
//
//	cv::Ptr<cv::aruco::DetectorParameters> detectorParams;
//	cv::Ptr<cv::aruco::CharucoBoard> board;
//	cv::Ptr<cv::aruco::Dictionary> dictionary;
//
//	cv::Mat frame, frameCopy;

	ofVec3f cameraPos;
	ofVec3f cameraLook;
	ofMatrix4x4 cameraToWorld;

	ofxUDPManager robot01;
	ofxOscReceiver	oscReceiver;
	ofxJSONElement jsonMsg;
	std::string lastMessage;

	map<int, marker> markers;

	int baud;
	ofSerial serial;
	vector <ofSerialDeviceInfo> serialDeviceList;
	string serialMsg;
	vector<string> messages;

//	ofxPanel gui;
//	ofParameter<int> cameraDeviceIdx; int cameraDeviceSelectedIdx;
//	ofParameter<int> serialDeviceIdx; int serialDeviceSelectedIdx;
//	ofParameter<float> robotTargetRot, robotRotAccuracy, robotPosAccuracy;
//	ofParameter<ofVec2f> robotTarget, robotPos;
//	ofParameter<int> robotMovStrength;
//	ofParameterGroup parameters;

	ofxDatGui *gui;
	ofxDatGuiToggle *robot01GoRot, *robot01GoPos;
	ofxDatGui2dPad *robotTargetPad, *robotPosPad;
	ofxDatGuiSlider *robotTargetRot, *robotCurRot;
	ofxDatGuiSlider *robotRotAccuracy, *robotPosAccuracy;
	ofxDatGuiSlider *robotMovStrength, *robotRotStrength;

	ofxSVG mapSvg;

	ofEasyCam cam;
};
