#include "cinder/app/AppBasic.h"
#include "cinder/audio/Io.h"
#include "cinder/audio/Output.h"
#include "OscListener.h"
#include "cinderSyphon.h"
#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define MH_THRESH 3.5

enum Track
{
    REST,
    ACTIVE
};

class ExpressionsApp : public AppBasic {
    public:
        ExpressionsApp(){}
        ~ExpressionsApp();
        void setup();
        void update();
        void draw();
    
        syphonClient    mClientSyphon;
        osc::Listener   mListener;

        audio::TrackRef mTrackRest, mTrackActive;
        Track mTrack;
    
        float mMh;
};

void ExpressionsApp::setup()
{
    std::string appPath = ci::app::getAppPath().string();
    std::string faceOSCCmd = "open " + appPath + "/Contents/Resources/FaceOSCSyphon.app --background";
    
    std::system(faceOSCCmd.c_str());
    
    mClientSyphon.setup();
    mClientSyphon.setApplicationName("FaceOSC");
    mClientSyphon.setServerName("");
    mClientSyphon.bind();
    
    mListener.setup(8338);
    
    mTrackActive = audio::Output::addTrack( audio::load( loadResource( RES_AUD_ACTIVE ) ) );
    mTrackActive->setLooping(true);
    mTrackActive->stop();
    
    mTrackRest = audio::Output::addTrack( audio::load( loadResource( RES_AUD_REST ) ) );
    mTrackRest->setLooping(true);
    mTrackRest->setVolume(0.05);
    mTrack = REST;

    mMh = 0;
}


void ExpressionsApp::update()
{

    while (mListener.hasWaitingMessages()) {
		
        osc::Message message;
        
		mListener.getNextMessage(&message);
        
        if ("/gesture/mouth/height" == message.getAddress()) {
            mMh = message.getArgAsFloat(0);
        }
        
    }
    
    switch (mTrack) {
        case REST:
            if (mMh >= MH_THRESH) {
                mTrack = ACTIVE;
                mTrackRest->stop();
                mTrackActive->play();
                mTrackActive->setTime(0.0);
            }
            break;
        case ACTIVE:
            if (mMh < MH_THRESH) {
                mTrack = REST;
                mTrackActive->stop();
                mTrackRest->play();
                mTrackRest->setVolume(0.05);
            }
            break;
    }

}

void ExpressionsApp::draw()
{
    mClientSyphon.draw(0,0, 640, 480);
}

ExpressionsApp::~ExpressionsApp()
{
    std::system("killall FaceOSC");
}

CINDER_APP_BASIC( ExpressionsApp, RendererGl )
