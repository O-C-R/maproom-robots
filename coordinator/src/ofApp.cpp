#include "ofApp.h"

//using namespace ofxCv;
//using namespace cv;

static const int kFrameWait = 3;
static const int kNumCornersCalibration = 15;
static const int kImgWidth = 1980;
static const int kImgHeight = 1080;

static const int kRobot01Idx = 23;
static const float kMarkerSize = 0.2032f;

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(true);
	ofSetBackgroundColor(0);

	cam.setDistance(3);
	cam.enableMouseInput();
	cam.setTarget(ofVec3f(0.0));
	cam.setNearClip(0.01);

	// on the pillar
//	cameraPos = ofVec3f(0., 0., 2.03);
//	cameraLook = ofVec3f(0.0, 0.82, 0.0);
//	cameraToWorld.makeLookAtMatrix(cameraPos, cameraLook, ofVec3f(0.0, 0.0, 1.0));

	// on the map hanging thing
	cameraPos = ofVec3f(0., 0., 2.845);
	cameraLook = ofVec3f(0.0, 0.0, 0.0);
//	ofVec3f cameraUp(0.0, 1.0, 0.0);

	cameraToWorld.makeIdentityMatrix();
	ofQuaternion cameraRotation;
	cameraRotation.makeRotate(180, ofVec3f(1.0, 0.0, 0.0));
	cameraToWorld.setRotate(cameraRotation);
	cameraToWorld.setTranslation(cameraPos);

	mapSvg.load("map.svg");
	cout << "mapSvg " << mapSvg.getWidth() << "x" << mapSvg.getHeight() << " w/ " << mapSvg.getNumPath() << endl;

//	cameraToWorld.makeLookAtMatrix(cameraPos, cameraLook, cameraUp);
//	cameraToWorld.scale(1.0, -1.0, 0.0);

//	cout << cameraToWorld << endl;
//
//	{
//		cout << "input" << ofVec3f(0.0, 0.0, 0.0) << endl;
//		ofVec3f pos = ofVec3f(0.0) * cameraToWorld;
//		cout << "output" << pos << endl;
//	}
//
//	{
//		cout << "input" << ofVec3f(1.0, 0.0, 0.0) << endl;
//		ofVec3f pos = ofVec3f(1.0, 0.0, 0.0) * cameraToWorld;
//		cout << "output" << pos << endl;
//	}
//
//	{
//		ofMatrix4x4 m2;
//		m2.getTransposedOf(cameraToWorld);
//		cout << "input" << ofVec3f(0.0, 0.0, 0.0) << endl;
//		ofVec3f pos = m2 * ofVec3f(0.0);
//		cout << "output" << pos << endl;
//	}
//
//	{
//		ofMatrix4x4 m2;
//		m2.getTransposedOf(cameraToWorld);
//		cout << "input" << ofVec3f(1.0, 0.0, 0.0) << endl;
//		ofVec3f pos = m2 * ofVec3f(1.0, 0.0, 0.0);
//		cout << "output" << pos << endl;
//	}

//	cam.setup(kImgWidth, kImgHeight);

//	cout << "---------------------------------" << endl;
//	cout << "Video devices:" << endl;
//	camDeviceList = cam.listDevices();
//	for (auto &device : camDeviceList) {
//		cout << device.id << ": " << device.deviceName << endl;
//	}
//	cout << "---------------------------------" << endl;

	cout << "---------------------------------" << endl;
	cout << "Serial devices:" << endl;
	serialDeviceList = serial.getDeviceList();
	for (auto &device : serialDeviceList) {
		cout << device.getDeviceID() << ": " << device.getDeviceName() << endl;
	}
	cout << "---------------------------------" << endl;

//	isCalibrating = false;
//	doneCalibrating = false;

//	dictionary = cv::aruco::getPredefinedDictionary(aruco::DICT_4X4_50);
//	board = aruco::CharucoBoard::create(2, 4, .05, .025, dictionary);
//	detectorParams = aruco::DetectorParameters::create();

//	parameters.setName("settings");
////	parameters.add(cameraDeviceIdx.set("cameraDeviceIndex", 0, 0, camDeviceList.size()-1));
////	parameters.add(serialDeviceIdx.set("serialDeviceIndex", 2, 0, serialDeviceList.size()-1));
//	parameters.add(robotTargetRot.set("robotTargetRot", 0, -180, 180));
//	parameters.add(robotRotAccuracy.set("robotRotAccuracy", 5, 0, 30));
//	parameters.add(robotTarget.set("robotTarget", ofVec2f(0), ofVec2f(-500.0f), ofVec2f(-500.0f)));
//	parameters.add(robotPos.set("robotPos", ofVec2f(0), ofVec2f(-500.0f), ofVec2f(-500.0f)));
//	parameters.add(robotPosAccuracy.set("robotPosAccuracy", 40, 0, 100));
//	parameters.add(robotMovStrength.set("robotMovStrength", 32, 0, 100));
//	gui.setup(parameters);
//	gui.setPosition(500, 20);

//	gui.loadFromFile("settings.xml");

	gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
	robot01GoRot = gui->addToggle("robot01GoRot", false);
	robot01GoPos = gui->addToggle("robot01GoPos", false);
	robotCurRot = gui->addSlider("robotCurRot", -180, 180);
	robotCurRot->setValue(0);
	robotTargetRot = gui->addSlider("robotTargetRot", -180, 180);
	robotTargetRot->setValue(0);
	robotTargetPad = gui->add2dPad("robotTargetPad", ofRectangle(ofVec2f(-2), ofVec2f(2)));
	robotPosPad = gui->add2dPad("robotPosPad", ofRectangle(ofVec2f(-2), ofVec2f(2)));
	robotRotAccuracy = gui->addSlider("robotRotAccuracy", 0, 30);
	robotRotAccuracy->setValue(5);
	robotPosAccuracy = gui->addSlider("robotPosAccuracy", 0, 1);
	robotPosAccuracy->setValue(0.1);
	robotMovStrength = gui->addSlider("robotMovStrength", 0, 1000);
	robotMovStrength->setValue(200);
	robotRotStrength = gui->addSlider("robotRotStrength", 0, 1000);
	robotRotStrength->setValue(70);

//	cameraDeviceSelectedIdx = cameraDeviceIdx;
//	cam.setDeviceID(cameraDeviceIdx);

//	serialDeviceSelectedIdx = serialDeviceIdx;
//	baud = 57600;
//	serial.setup(serialDeviceIdx, baud);

	serialMsg = "";
	mouseDown = false;

	robot01.Create();
	robot01.Connect("192.168.1.70",5111);
	robot01.SetNonBlocking(true);
	oscReceiver.setup( PORT );
}

//--------------------------------------------------------------
void ofApp::update(){
//	cam.update();
//
//	if (cam.isFrameNew()) {
//		frame = toCv(cam);
//		frame.copyTo(frameCopy);
//
//		vector<int> markerIds;
//		vector<vector<Point2f>> markerCorners, markersRejected;
//		aruco::detectMarkers(frame, dictionary, markerCorners, markerIds, detectorParams, markersRejected);
//
//		if (markerCorners.size() > 0) {
//			aruco::drawDetectedMarkers(frameCopy, markerCorners, markerIds);
//
//			Mat charucoCorners;
//			Mat charucoIds;
//			aruco::interpolateCornersCharuco(markerCorners, markerIds, frame, board, charucoCorners, charucoIds);
//
//			if (charucoCorners.total() > 0) {
//				aruco::drawDetectedCornersCharuco(frameCopy, charucoCorners, charucoIds);
//
//				if (isCalibrating && ofGetFrameNum() % kFrameWait == 0) {
//					if (allCharucoCorners.size() < kNumCornersCalibration) {
//						allCharucoCorners.push_back(markerCorners);
//						allCharucoIds.push_back(markerIds);
//					} else {
//						float err = calibrate();
//						cout << "Calibrated with err: " << err << endl;
//
//						isCalibrating = false;
//						doneCalibrating = true;
//					}
//				}
//
//				if (doneCalibrating) {
//					Mat rvec, tvec;
//					aruco::estimatePoseCharucoBoard(charucoCorners, charucoIds, board, cameraMatrix, distCoeffs, rvec, tvec);
//					cout << "rvec: " << rvec << endl;
//					cout << "tvec: " << tvec << endl;
//				}
//			}
//		}
//	}

//	while (serial.available()) {
//		int b = serial.readByte();
//		if (b == '\n') {
//			messages.push_back(serialMsg);
//			serialMsg = "";
//		} else {
//			serialMsg += (char)b;
//		}
//	}
//
//	for (string &message : messages) {
//		cout << "GOT MESSAGE: " << message << endl;
//	}
//	messages.clear();

//	if (cameraDeviceSelectedIdx != cameraDeviceIdx) {
//		cam.close();
//		cam.setDeviceID(cameraDeviceIdx);
//		cam.setup(kImgWidth, kImgHeight);
//
//		cameraDeviceSelectedIdx = cameraDeviceIdx;
//	}
//
//	if (serialDeviceSelectedIdx != serialDeviceIdx) {
//		serial.close();
//		serial.setup(serialDeviceIdx, baud);
//
//		serialDeviceSelectedIdx = serialDeviceIdx;
//	}

	if (mouseDown && ofGetFrameNum() % 2 == 0) {
		ofPoint diff = mouseCurPos - mouseDownPos;
//		cout << "cur " << mouseCurPos << endl << "down " << mouseDownPos << endl << "diff " << diff << endl;

		char *buf = (char *)malloc(50 * sizeof(char));
		int len = sprintf((char *)buf, "MR01MOV%04d%04d\n", (int)ofClamp(diff.x + 500, 0, 1000), (int)ofClamp(diff.y + 500, 0, 1000));



//		serial.flush(false, true);
//		serial.writeBytes(buf, strlen((char *)buf));

		string message = "You pressed a key";

		cout << "SENDING " << buf << endl;
		robot01.Send(buf, strlen((char *)buf));

		free(buf);
	}

	while( oscReceiver.hasWaitingMessages() )
	{
		// get the next message
		ofxOscMessage m;
		oscReceiver.getNextMessage( &m );

//		markers.clear();

		// check for mouse moved message
		if ( m.getAddress() == "/cv" )
		{
			string msg = m.getArgAsString(0);
			jsonMsg.parse(msg);
			int nIds = jsonMsg["ids"].size();
			for (int i = 0; i < nIds; ++i) {
				int idx = jsonMsg["ids"][i].asInt();
				ofVec3f rvec(jsonMsg["rvecs"][i][0].asFloat(), jsonMsg["rvecs"][i][1].asFloat(), jsonMsg["rvecs"][i][2].asFloat());
				ofVec3f tvec(jsonMsg["tvecs"][i][0].asFloat(), jsonMsg["tvecs"][i][1].asFloat(), jsonMsg["tvecs"][i][2].asFloat());

				marker mk;
				mk.idx = idx;
				mk.rvec = rvec;
				mk.tvec = tvec;

				// http://answers.opencv.org/question/110441/use-rotation-vector-from-aruco-in-unity3d/
				float angle = sqrt(rvec.x*rvec.x + rvec.y*rvec.y + rvec.z*rvec.z);
				ofVec3f axis(rvec.x, rvec.y, rvec.z);

				ofQuaternion rot;
				rot.makeRotate(angle / 3.14159 * 180.0, axis);

				mk.mat.makeIdentityMatrix();
				mk.mat.setRotate(rot);
				mk.mat.setTranslation(ofVec3f(tvec.x, tvec.y, tvec.z));

				stringstream sb;

//				mk.mat.translate(-cameraToWorld.getTranslation());
//				if (int(floor(ofGetFrameNum() / 10)) % 2 == 0) {
////					lastMessage = "neg";
//					sb << cameraToWorld.getRotate().getEuler();
//					mk.mat.rotate(-cameraToWorld.getRotate());
//				} else {
//					sb << cameraToWorld.getRotate().inverse().getEuler();
////					lastMessage = "pos";
//					mk.mat.postMultRotate(-cameraToWorld.getRotate());
//				}
//				lastMessage = sb.str();

				mk.mat = mk.mat * cameraToWorld.getInverse();

				mk.worldPos = mk.mat.getTranslation();
				mk.rot = mk.mat.getRotate().getEuler().z;

				markers[idx] = mk;
			}
		}
	}

	for (auto &p : markers) {
		int idx = p.first;
		marker &mk = p.second;

		if (idx == kRobot01Idx) {
			bool didWrite = false;
			char *buf = (char *)malloc(50 * sizeof(char));
			ofVec2f pos2d = ofVec2f(mk.worldPos.x, mk.worldPos.y);
			ofVec2f target2d = robotTargetPad->getPoint();

			float targetRot = robotTargetRot->getValue();
			float rotAccuracy = robotRotAccuracy->getValue();
			float posAccuracy = robotPosAccuracy->getValue();
			float rotStrength = robotRotStrength->getValue();
			float movStrength = robotMovStrength->getValue();

			if (robot01GoRot->getChecked() && (mk.rot > targetRot + rotAccuracy || mk.rot < targetRot - rotAccuracy)) {
				int angleVal = mk.rot > targetRot ? 500 - rotStrength : 500 + rotStrength;

				sprintf((char *)buf, "MR01ROT%04d\n", angleVal);
				didWrite = true;
			} else if (robot01GoPos->getChecked() && pos2d.distance(target2d) > posAccuracy) {
				ofVec2f go = target2d - pos2d;
				go = go.normalize() * movStrength;

				int goX = 500 - go.x;
				int goY = 500 - go.y;

				sprintf((char *)buf, "MR01MOV%04d%04d\n", goX, goY);
				didWrite = true;
			} else {
				sprintf((char *)buf, "MR01SET050005000500\n");
				didWrite = true;
			}

			if (didWrite) {
//				lastMessage = buf;
				cout << "SENDING " << buf << endl;
				robot01.Send(buf, strlen((char *)buf));
			}
			free(buf);
		}
	}

}

//bool ofApp::recordCharuco() {
//	return false;
//}
//
//float ofApp::calibrate() {
//	vector<vector< Point2f>> allCornersConcatenated;
//	vector<int> allIdsConcatenated;
//	vector< int > markerCounterPerFrame;
//	markerCounterPerFrame.reserve(allCharucoCorners.size());
//
//	for (int i = 0; i < allCharucoCorners.size(); i++) {
//		markerCounterPerFrame.push_back((int)allCharucoCorners[i].size());
//
//		for(unsigned int j = 0; j < allCharucoCorners[i].size(); j++) {
//			allCornersConcatenated.push_back(allCharucoCorners[i][j]);
//			allIdsConcatenated.push_back(allCharucoIds[i][j]);
//		}
//	}
//
//	int calibrationFlags = 0;
//	cv::Size imgSize(kImgWidth, kImgHeight);
//
//	double arucoRepErr;
//	arucoRepErr = aruco::calibrateCameraAruco(allCornersConcatenated, allIdsConcatenated,
//											  markerCounterPerFrame, board, imgSize, cameraMatrix,
//											  distCoeffs, noArray(), noArray(), calibrationFlags);
//
//	return arucoRepErr;
//}

//bool ofApp::saveCalibration(string filename, bool absolute) {
//	FileStorage fs(ofToDataPath(filename, absolute), FileStorage::WRITE);
//	fs << "cameraMatrix" << cameraMatrix;
//	fs << "distCoeffs" << distCoeffs;
//	fs << "rvecs" << "[";
//	for(int i = 0; i < (int)rvecs.size(); i++) {
//		fs << "[:" << rvecs[i] << "]";
//	}
//	fs << "]";
//	fs << "tvecs" << "[";
//	for(int i = 0; i < (int)tvecs.size(); i++) {
//		fs << "[:" << tvecs[i] << "]";
//	}
//	fs << "]";
//
//	return true;
//}
//
//bool ofApp::loadCalibration(string filename, bool absolute) {
//	FileStorage fs(ofToDataPath(filename, absolute), FileStorage::READ);
//	fs["cameraMatrix"] >> cameraMatrix;
//	fs["distCoeffs"] >> distCoeffs;
//
//	FileNode frvecs = fs["rvecs"];
//	rvecs.clear();
//	for(FileNodeIterator it = frvecs.begin(); it != frvecs.end(); it++) {
//		Mat cur;
//		(*it) >> cur;
//		rvecs.push_back(cur);
//	}
//
//	FileNode ftvecs = fs["tvecs"];
//	tvecs.clear();
//	for(FileNodeIterator it = ftvecs.begin(); it != ftvecs.end(); it++) {
//		Mat cur;
//		(*it) >> cur;
//		tvecs.push_back(cur);
//	}
//
//	return true;
//}


//--------------------------------------------------------------
void ofApp::draw(){
	stringstream posstr;

//	ofImage frameImg;
//	toOf(frameCopy, frameImg);
//	frameImg.draw(0, 0, ofGetWidth(), ofGetHeight());
//
//	if (allCharucoCorners.size() > 0) {
//		vector<vector<Point2f>> &f = allCharucoCorners[allCharucoCorners.size()-1];
//		for (vector<Point2f> &pts : f) {
//			for (Point2f &pt : pts) {
//				ofDrawCircle(pt.x, pt.y, 2);
//			}
//		}
//	}
//
//	ofDrawBitmapString(camDeviceList[cameraDeviceSelectedIdx].hardwareName, 10, 10);
//	ofDrawBitmapString(serialDeviceList[serialDeviceSelectedIdx].getDeviceName(), 10, 20);

	cam.begin();

	ofSetColor(255);
	ofDrawAxis(10.0);

	{
		ofPushMatrix();
		ofPushStyle();
		ofVec3f pos = ofVec3f(0.0) * cameraToWorld;
		ofSetColor(255, 255, 255);
		ofDrawIcoSphere(pos, 0.02);
		ofPopStyle();
		ofPopMatrix();
	}

	{
		ofPushMatrix();
		ofPushStyle();
		ofVec2f target = robotTargetPad->getPoint();
		ofVec3f pos(target.x, target.y, 0.0);
		ofSetColor(255, 0, 0);
		ofDrawIcoSphere(pos, 0.02);
		ofPopStyle();
		ofPopMatrix();
	}

	posstr << "lastMessage: " << lastMessage << endl;

	ofVec3f corner1(0.0),
		corner2(kMarkerSize, kMarkerSize, 0.0),
		corner3(kMarkerSize, 0.0, 0.0),
		corner4(0.0, kMarkerSize, 0.0),
		up(0.0, 0.0, kMarkerSize);

	for (map<int, marker>::iterator it = markers.begin(); it != markers.end(); ++it) {
		int idx = it->first;
		marker &m = it->second;

		ofVec3f c1 = corner1 * m.mat,
			c2 = corner2 * m.mat,
			c3 = corner3 * m.mat,
			c4 = corner4 * m.mat,
			u = up * m.mat;

		posstr << idx << ": " << c1 << " @ " << m.rot << endl;
//		robotPos.set(ofVec2f(c1.x, c1.y));
		robotPosPad->setPoint(ofVec2f(c1.x, c1.y));
		robotCurRot->setValue(m.rot);

		ofPushMatrix();
		ofPushStyle();
		ofMultMatrix(m.mat);
		ofDrawAxis(1.0);

//		ofSetColor(255, 255, 255);
//		ofDrawLine(c1, c3);
//		ofDrawLine(c1, c4);
//		ofDrawLine(c2, c3);
//		ofDrawLine(c2, c4);
//		ofSetColor(255, 255, 0);
//		ofDrawLine(c1, c2);
//		ofSetColor(0, 255, 255);
//		ofDrawLine(c1, u);
		ofPopStyle();
		ofPopMatrix();
	}
	cam.end();

	ofSetColor(255, 255, 255);
	ofDrawBitmapString(posstr.str(), 10, 15);

//	if (mouseDown) {
//		ofSetColor(255, 0, 0);
//		ofDrawLine(mouseDownPos, mouseCurPos);
//		ofSetColor(255, 255, 255);
//	}
//
//	gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
//	if (key == 'k') {
//		cout << "Calibrating!" << endl;
//
//		allCharucoCorners.clear();
//		allCharucoIds.clear();
//
//		isCalibrating = true;
//		doneCalibrating = false;
//	} else if (key == 's') {
//		cout << "Saving..." << endl;
//		if (doneCalibrating) {
//			saveCalibration("calibration");
//			cout << "Saved!" << endl;
//		} else {
//			cout << "Not done calibrating, can't save!" << endl;
//		}
//	} else if (key == 'l') {
//		cout << "Loading..." << endl;
//		doneCalibrating = loadCalibration("calibration");
//		cout << "Loaded!" << endl;
//	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
//	mouseCurPos = ofPoint(x, y);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
//	mouseDownPos = ofPoint(x, y);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
//	if (!mouseDown) {
//		cout << "mouse pressed" << endl;
//		mouseDownPos = ofPoint(x, y);
//	}
//	mouseCurPos = ofPoint(x,y);
//	mouseDown = true;
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
//	mouseDown = false;
//	cout << "mouse released" << endl;
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
