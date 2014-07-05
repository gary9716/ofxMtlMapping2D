#include "ofxMtlMapping2DControls.h"

#include "ofxMtlMapping2DSettings.h"
#include "ofxMtlMapping2DShape.h"
#include "ofxMtlMapping2DShapes.h"
#include "ofxMtlMapping2DGrid.h"

#include "ofMain.h"

//--------------------------------------------------------------
//--------------------------------------------------------------

const int kSliderHeight = 16;
const int kSpacerHeight = 20;
const int kToggleSize = 24;
const int kBottomSpacerHeight = 100; // padding to be able to scroll until the end/bottom of the UI canvas
const string uiDataPath = "ui/";
bool _bInitialized = false;

//--------------------------------------------------------------
//--------------------------------------------------------------
static ofxMtlMapping2D * _mtlMapping2D = NULL;

//--------------------------------------------------------------
ofxMtlMapping2DControls* ofxMtlMapping2DControls::sharedInstance()
{
    static ofxMtlMapping2DControls* instance = NULL;
    if (instance == NULL) {
        instance = new ofxMtlMapping2DControls();
    }
    return instance;
}

//--------------------------------------------------------------
ofxMtlMapping2DControls& ofxMtlMapping2DControlsSharedInstance(ofxMtlMapping2D * mtlMapping2D)
{
    //if (mtlMapping2D == NULL && _mtlMapping2D == NULL) {
    //    ofLogError() << "You need to pass a reference to the ofxMtlMapping2D the Controls before going further.";
    //    return;
   // } else 
	if (mtlMapping2D != NULL && _mtlMapping2D == NULL) {
        _mtlMapping2D = mtlMapping2D;
    }
    
    return *ofxMtlMapping2DControls::sharedInstance();
}

//--------------------------------------------------------------
ofxMtlMapping2DControls::ofxMtlMapping2DControls() //ofxMtlMapping2D * mtlMapping2D)
{

#if defined(TARGET_OSX)
	_rootPath = "../../../data/settings/";
#elif defined(TARGET_WIN32)
	_rootPath = "settings/";
#endif
    
    ofColor uiColor;
    uiColor.set(0, 210, 255, 130);
    ofColor uiColorB;
    uiColorB.set(0, 210, 255, 90);
    
    // --- Tool box
    shapeTypesAsString[MAPPING_2D_SHAPE_QUAD] = "quad";
    shapeTypesAsString[MAPPING_2D_SHAPE_GRID] = "grid";
    shapeTypesAsString[MAPPING_2D_SHAPE_TRIANGLE] = "triangle";
    shapeTypesAsString[MAPPING_2D_SHAPE_MASK] = "mask";
    
    // set default values

    static const int kWidgetWidth = kControlsMappingToolsPanelWidth - 16;
    _toolsCanvas = new ofxUICanvas(0, 0, kControlsMappingToolsPanelWidth, ofGetHeight());
    _toolsCanvas->setColorBack(uiColor);
    
    // --- Fullscreen
    _fullscreenExpandIcon.loadImage(uiDataPath + "expand.png");
    _fullscreenContractIcon.loadImage(uiDataPath + "contract.png");
    _lockedIcon.loadImage(uiDataPath + "locked.png");
    _unlockedIcon.loadImage(uiDataPath + "unlocked.png");
    
    _toolsCanvas->addImageToggle(kSettingMappingFullscreen, uiDataPath + "expand.png", false, kToggleSize, kToggleSize);
    
    // Edit
    _toolsCanvas->addImageToggle(kSettingMappingSettings, uiDataPath + "settings-3.png", false, kToggleSize, kToggleSize);
    _toolsCanvas->addImageToggle(kSettingMappingEditShapes, uiDataPath + "edit.png", false, kToggleSize, kToggleSize);
    _toolsCanvas->addImageButton(kSettingMappingSave, uiDataPath + "file-down.png", &_mtlMapping2D->bSaveShapes, kToggleSize, kToggleSize);
    _toolsCanvas->addImageButton(kSettingMappingLoad, uiDataPath + "file-up.png", &_mtlMapping2D->bLoadShapes, kToggleSize, kToggleSize);
    
    // Mapping controls Output / Input
    ofxUISpacer *spacer = new ofxUISpacer(kWidgetWidth, kSpacerHeight);
    spacer->setDrawFill(false);
    
    _toolsCanvas->addWidgetDown(spacer);
    _toolsCanvas->addImageToggle(kSettingMappingModeOutput, uiDataPath + "projo.png", true, kToggleSize, kToggleSize);
    _toolsCanvas->addImageToggle(kSettingMappingModeInput, uiDataPath + "texture.png", false, kToggleSize, kToggleSize);
    
    // Mapping shape controls
    if (ofxMtlMapping2DSettings::kIsManuallyCreatingShapeEnabled) {
        _toolsCanvas->addWidgetDown(spacer);
        _toolsCanvas->addImageToggle(kSettingMappingCreateNewQuad, uiDataPath + "quad.png", &_mtlMapping2D->bCreateQuad, kToggleSize, kToggleSize);
        _toolsCanvas->addImageToggle(kSettingMappingCreateNewGrid, uiDataPath + "grid.png", &_mtlMapping2D->bCreateGrid, kToggleSize, kToggleSize);
        _toolsCanvas->addImageToggle(kSettingMappingCreateNewTriangle, uiDataPath + "triangle.png", &_mtlMapping2D->bCreateTriangle, kToggleSize, kToggleSize);
        _toolsCanvas->addImageToggle(kSettingMappingCreateNewMask, uiDataPath + "mask.png", &_mtlMapping2D->bCreateMask, kToggleSize, kToggleSize);
    }
    
    ofAddListener(_toolsCanvas->newGUIEvent, this, &ofxMtlMapping2DControls::toolsUiEvent);
    _uiSuperCanvases.push_back(_toolsCanvas);
    
    // ---
    // Output Settings UI
    _settingsUI = new ofxUISuperCanvas("SETTINGS");
    _settingsUI->setColorBack(uiColor);
    
    _settingsUI->addSpacer(_settingsUI->getRect()->width - 10, 2);
    _settingsUI->addButton("SAVE", false);
    _settingsUI->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    _settingsUI->addButton("LOAD", false);
    _settingsUI->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    
    _settingsUI->addSpacer((_settingsUI->getRect()->width - 10) / 2, 1);
    _settingsUI->addToggle("LOAD FILE ON START", false);
    _settingsUI->addButton("SELECT FILE", false);
    _settingsUI->addTextInput("FILE PATH", "NONE", OFX_UI_FONT_SMALL);
    
    _settingsUI->addSpacer((_settingsUI->getRect()->width - 10) / 2, 1);
    _settingsUI->addLabel("PROJECTION SIZE");
    _settingsUI->addTextInput("PROJ. WIDTH", ofToString(ofGetWidth()));
    _settingsUI->addTextInput("PROJ. HEIGHT", ofToString(ofGetHeight()));
    
    _settingsUI->addSpacer((_settingsUI->getRect()->width - 10) / 2, 1);
    _settingsUI->addToggle("DRAG", &ofxMtlMapping2DGlobal::bIsDraggingZone);
    _settingsUI->addSlider("ZOOM", .0f, 10.0f, 1.0f)->setIncrement(.1f);

    _settingsUI->autoSizeToFitWidgets();
    ofAddListener(_settingsUI->newGUIEvent, this, &ofxMtlMapping2DControls::settingsUiEvent);
    _settingsUI->disable();
    _uiSuperCanvases.push_back(_settingsUI);
    
    // ---
    // Shapes List UI    
    _shapesListCanvas = new ofxUIScrollableCanvas(kControlsMappingToolsPanelWidth, 0, kControlsMappingShapesListPanelWidth, ofGetHeight());
    _shapesListCanvas->setScrollArea(kControlsMappingToolsPanelWidth, 0, kControlsMappingShapesListPanelWidth, ofGetHeight());
    _shapesListCanvas->setScrollableDirections(false, false);
    _shapesListCanvas->setColorBack(uiColorB);
    _shapesListCanvas->autoSizeToFitWidgets();
    
    ofAddListener(_shapesListCanvas->newGUIEvent, this, &ofxMtlMapping2DControls::shapesListUiEvent);
    _uiSuperCanvases.push_back(_shapesListCanvas);
    
    
    // ---
    // Grid Settings UI
    int gridSettingCanvasWidth = 200.0f;
    _gridSettingsCanvas = new ofxUICanvas();
    _gridSettingsCanvas->setWidth(gridSettingCanvasWidth);
    _gridSettingsCanvas->setColorBack(uiColorB);
    
    _gridSettingsCanvas->addLabel("GRID SETTINGS");
    _gridSettingsCanvas->addIntSlider("NB COLS", 1, 20, &ofxMtlMapping2DSettings::gridDefaultNbCols);
    _gridSettingsCanvas->addIntSlider("NB ROWS", 1, 20, &ofxMtlMapping2DSettings::gridDefaultNbRows);
    
    _gridSettingsCanvas->autoSizeToFitWidgets();
    ofAddListener(_gridSettingsCanvas->newGUIEvent, this, &ofxMtlMapping2DControls::gridSettingsListUiEvent);
    _gridSettingsCanvas->disable();
    _uiSuperCanvases.push_back(_gridSettingsCanvas);
    
    
#if defined(USE_OFX_DETECT_DISPLAYS)
    // ---
    // Output Settings UI
    _outputUI = new ofxUISuperCanvas("OUTPUT SETTINGS");
    _outputUI->setColorBack(uiColor);
    
    _outputUI->addSpacer(_outputUI->getRect()->width - 10, 2);
    
    _outputUI->addButton("DETECT DISPLAYS", false);
    _outputUI->addSpacer((_outputUI->getRect()->width - 10) / 2, 1);
    
    _outputUI->addToggle("OUTPUT WINDOW", &_bISOutScreenOn);
    
    ofAddListener(_outputUI->newGUIEvent, this, &ofxMtlMapping2DControls::outputUiEvent);
    _outputUI->autoSizeToFitWidgets();
    _outputUI->disable();
    _uiSuperCanvases.push_back(_outputUI);
    
    // ---
    // Displays Settings UI
    
    _displaysListUI = new ofxUISuperCanvas("DISPLAYS LIST");
    _displaysListUI->setColorBack(uiColor);
    
    ofAddListener(_displaysListUI->newGUIEvent, this, &ofxMtlMapping2DControls::displaysUiEvent);
    _displaysListUI->autoSizeToFitWidgets();
    _displaysListUI->disable();
    _uiSuperCanvases.push_back(_displaysListUI);
    
#endif
    
#if defined(USE_VIDEO_PLAYER_OPTION)
    // ---
    // Video player UI
    _videoPlayIcon.loadImage(uiDataPath + "play.png");
    _videoPauseIcon.loadImage(uiDataPath + "pause.png");
    
    _videoPlayerUI = new ofxUISuperCanvas("VIDEO PLAYER");
    _videoPlayerUI->setColorBack(uiColor);
    
    _videoPlayerUI->addButton("NONE", false);
    _videoPlayerUI->addToggle("LOAD VIDEO ON START", false);
    _videoPlayerUI->addButton("SELECT FILE", false);
    _videoPlayerUI->addTextInput("FILE PATH", "NONE", OFX_UI_FONT_SMALL);
    
    _videoPlayerUI->addSpacer(_videoPlayerUI->getRect()->width - 10, 2);
    _videoPlayerUI->addImageButton("STOP", uiDataPath + "stop.png", false, kToggleSize, kToggleSize);
    _videoPlayerUI->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    _videoPlayerUI->addImageToggle("PLAY", uiDataPath + "play.png", false, kToggleSize, kToggleSize);
    _videoPlayerUI->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);

    _videoPlayerUI->addMinimalSlider("TRACK", .0f, 100.0f, &_mtlMapping2D->videoPositionInSeconds);
    
    ofAddListener(_videoPlayerUI->newGUIEvent, this, &ofxMtlMapping2DControls::videoPlayerUiEvent);
    _videoPlayerUI->autoSizeToFitWidgets();
    _videoPlayerUI->disable();
    _uiSuperCanvases.push_back(_videoPlayerUI);
#endif
    
#if defined(USE_OFX_SYPHON) && defined(TARGET_OSX)
    // ---
    // Syphon UI
    _syphonUI = new ofxUISuperCanvas("SYPHON SETTINGS");
    
    _syphonUI->setColorBack(uiColor);
    
    ofAddListener(_syphonUI->newGUIEvent, this, &ofxMtlMapping2DControls::syphonUiEvent);
    _syphonUI->autoSizeToFitWidgets();
    _syphonUI->disable();
    _uiSuperCanvases.push_back(_syphonUI);
#endif

    // ---
#if defined(USE_OFX_DETECT_DISPLAYS)
    #if defined(TARGET_OSX)
        // You should only use the shared instance of ofxDetectDisplays,
        // otherwise event registration will be messed up.
        // Events only works on Mac for now.
        ofAddListener(ofxDetectDisplaysSharedInstance().displayConfigurationChanged, this, &ofxMtlMapping2DControls::displayConfigurationChanged);
    #endif
    displayConfigurationChanged();
#endif
    
    // ---
    updateUIsPosition();
    
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::init()
{
    // ---
    // Load UI XML files and initialize
    loadSettings();
    
    if (getToggleValue(_toolsCanvas, kSettingMappingModeOutput)) {
        ofxMtlMapping2DGlobal::setEditView(MAPPING_OUTPUT_VIEW);
    
    } else if (getToggleValue(_toolsCanvas, kSettingMappingModeInput)) {
        ofxMtlMapping2DGlobal::setEditView(MAPPING_INPUT_VIEW);
    }
    
    // ---
    ((ofxUIToggle *)_toolsCanvas->getWidget(kSettingMappingEditShapes))->setValue(false);
    setUIShapeEditingState(false);
    
    disable();
    
    _bInitialized = true;
}

//--------------------------------------------------------------
ofxMtlMapping2DControls::~ofxMtlMapping2DControls()
{
    while(!_uiSuperCanvases.empty()) delete _uiSuperCanvases.back(), _uiSuperCanvases.pop_back();
	_uiSuperCanvases.clear();
}


#pragma mark -
#pragma mark Tool Box

//--------------------------------------------------------------
void ofxMtlMapping2DControls::toolsUiEvent(ofxUIEventArgs &event)
{
    string name = event.widget->getName();
    
    if (name == kSettingMappingFullscreen) {
        bool bGoFullscreen = getToggleValue(_toolsCanvas, name);
        ofSetFullscreen(bGoFullscreen);
        
#if defined(USE_OFX_DETECT_DISPLAYS)
        if (!bGoFullscreen) {
            ofxUIRadio* uiRadio = (ofxUIRadio*) _displaysListUI->getWidget("DISPLAYS");
            
            for (int i=0; i<uiRadio->getToggles().size(); i++) {
                uiRadio->getToggles()[i]->setValue(false);
            }
        }
#endif
        
    }
    
    // --- Settings
    else if (name == kSettingMappingSettings) {
        if(getToggleValue(_toolsCanvas, name)) {
            _settingsUI->enable();
#if defined(USE_OFX_DETECT_DISPLAYS)
            _outputUI->enable();
            _displaysListUI->enable();
#endif
   
#if defined(USE_VIDEO_PLAYER_OPTION)
            _videoPlayerUI->enable();
#endif
            
#if defined(USE_OFX_SYPHON) && defined(TARGET_OSX)
            _syphonUI->enable();
#endif
        
        } else {
            _settingsUI->disable();
#if defined(USE_OFX_DETECT_DISPLAYS)
            _outputUI->disable();
            _displaysListUI->disable();
#endif

#if defined(USE_VIDEO_PLAYER_OPTION)
            _videoPlayerUI->disable();
#endif
            
#if defined(USE_OFX_SYPHON) && defined(TARGET_OSX)
            _syphonUI->disable();
#endif
        }
    }
    
    // --- Editing
    else if (name == kSettingMappingEditShapes) {
        setUIShapeEditingState(getToggleValue(_toolsCanvas, name));
        
    }
    
    // ---
    if ((name == kSettingMappingModeOutput || name == kSettingMappingModeInput) && getToggleValue(_toolsCanvas, name)) {
        unselectShapesToggles();
        ofxMtlMapping2DShape::resetActiveShapeVars();
        ofxMtlMapping2DPolygon::resetActivePolygonVars();
        
        if (name == kSettingMappingModeOutput) {
            ofxMtlMapping2DGlobal::setEditView(MAPPING_CHANGE_TO_OUTPUT_VIEW);
            refreshShapesListForMappingView(MAPPING_CHANGE_TO_OUTPUT_VIEW);

            ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingModeInput))->setValue(false);
            
            // ---
            if (ofxMtlMapping2DSettings::kIsManuallyCreatingShapeEnabled) {
                ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingCreateNewQuad))->setVisible(true);
                ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingCreateNewGrid))->setVisible(true);
                ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingCreateNewTriangle))->setVisible(true);
                ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingCreateNewMask))->setVisible(true);
            }
            
            // ---
            showGridSettingsCanvas();
            
            setSliderValue(_settingsUI, "ZOOM", ofxMtlMapping2DGlobal::outputViewZoomFactor);
        }
        else if (name == kSettingMappingModeInput) {
            ofxMtlMapping2DGlobal::setEditView(MAPPING_CHANGE_TO_INPUT_VIEW);
            refreshShapesListForMappingView(MAPPING_CHANGE_TO_INPUT_VIEW);

            ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingModeOutput))->setValue(false);
            
            // ---
            if (ofxMtlMapping2DSettings::kIsManuallyCreatingShapeEnabled) {
                ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingCreateNewQuad))->setVisible(false);
                ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingCreateNewGrid))->setVisible(false);
                ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingCreateNewTriangle))->setVisible(false);
                ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingCreateNewMask))->setVisible(false);
            }
            
            // ---
            hideGridSettingsCanvas();
            
            setSliderValue(_settingsUI, "ZOOM", ofxMtlMapping2DGlobal::inputViewZoomFactor);
        }        
    }
}


//--------------------------------------------------------------
void ofxMtlMapping2DControls::settingsUiEvent(ofxUIEventArgs &event)
{
    string name = event.widget->getName();
        
    if (name == "SAVE" && getButtonValue(_settingsUI, name)) {
        saveSettings();
    }
    else if (name == "LOAD"  && getButtonValue(_settingsUI, name)) {
        loadSettings();
#if defined(USE_OFX_SYPHON) && defined(TARGET_OSX)
        loadSyphonSettings();
#endif
    }
    else if (name == "LOAD FILE ON START"  && getToggleValue(_settingsUI, name)) {
        if (!_bInitialized && extraOutputSettings["path"] != "") {
            _mtlMapping2D->loadXml(extraOutputSettings["path"]);
        }
    }
    
    
    else if ((name == "SELECT FILE" && getButtonValue(_settingsUI, name)) || (_bInitialized && name == "FILE PATH")) {
        
        ofxUITextInput* textInput = (ofxUITextInput*) _settingsUI->getWidget("FILE PATH");
        
        // If the event comes from the TextInput widget, and the TextInput widget does not have the focus
        // it means that the user just presented 'cancel' in the system dialog window.
        if (name == "FILE PATH" && !textInput->isFocused()) {
            return;
        }
        
        ofFileDialogResult fileDialogResult = ofSystemLoadDialog();
        
        textInput->setFocus(false);
        
        if (!fileDialogResult.bSuccess) {
            return;
        }
        
        string path = fileDialogResult.getPath();
        extraOutputSettings["path"] = path;
        textInput->setTextString(fileDialogResult.getName());
        _mtlMapping2D->loadXml(path);
    }
    
    else if (name == "ZOOM") {
        if (ofxMtlMapping2DGlobal::getEditView() == MAPPING_INPUT_VIEW) {
            ofxMtlMapping2DGlobal::inputViewZoomFactor = getSliderValue(_settingsUI, name);
        }
        
        else if (ofxMtlMapping2DGlobal::getEditView() == MAPPING_OUTPUT_VIEW) {
            ofxMtlMapping2DGlobal::outputViewZoomFactor = getSliderValue(_settingsUI, name);
        }
        _mtlMapping2D->updateZoomAndOutput();
    }

    else if (name == "PROJ. WIDTH" || name == "PROJ. HEIGHT") {
        ofxUITextInput* textInput = (ofxUITextInput*) _settingsUI->getWidget(name);
        
        if (!textInput->isFocused()) {
            // Make sure we have an Int value
            if (name == "PROJ. WIDTH") {
                ofxMtlMapping2DGlobal::outputWidth = ofToInt(textInput->getTextString());
                textInput->setTextString(ofToString(ofxMtlMapping2DGlobal::outputWidth));
            } else {
                ofxMtlMapping2DGlobal::outputHeight = ofToInt(textInput->getTextString());
                textInput->setTextString(ofToString(ofxMtlMapping2DGlobal::outputHeight));
            }
            
            // Update Zoomed area and Resize the FBO
            _mtlMapping2D->updateZoomAndOutput(true);
        }
    }
}

/*
 bool is_number(const std::string& s)
 {
 std::string::const_iterator it = s.begin();
 while (it != s.end() && std::isdigit(*it)) ++it;
 return !s.empty() && it == s.end();
 }
 */

#if defined(USE_OFX_DETECT_DISPLAYS)
//--------------------------------------------------------------
void ofxMtlMapping2DControls::outputUiEvent(ofxUIEventArgs &event)
{
    string name = event.widget->getName();
    
    if(name == "DETECT DISPLAYS") {
        if (getButtonValue(_outputUI, name)) {
            ofxDetectDisplaysSharedInstance().detectDisplays();

        } else {
            displayConfigurationChanged();
        }
    }
    else if (name == "OUTPUT WINDOW") {
        if (!getToggleValue(_outputUI, name)) {
            _mtlMapping2D->closeOutputWindow();
        }
    }
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::displaysUiEvent(ofxUIEventArgs &event)
{
    string name = event.widget->getName();
    
    for (int i=0; i<_displayNames.size(); i++) {
        if (name == _displayNames[i] && getToggleValue(_displaysListUI, name)) {
            
            if (_bISOutScreenOn) {
                _mtlMapping2D->openOuputWindow(ofxDetectDisplaysSharedInstance().getDisplayBounds(i));
            } else {
                ofxDetectDisplaysSharedInstance().fullscreenWindowOnDisplay(i);
            }
            return;
        }
    }
}
#endif

#if defined(USE_OFX_SYPHON) && defined(TARGET_OSX)
//--------------------------------------------------------------
void ofxMtlMapping2DControls::syphonUiEvent(ofxUIEventArgs &event)
{
    string name = event.widget->getName();

    if(name == "NONE") {
        if (getButtonValue(_syphonUI, name)) {
            _mtlMapping2D->selectSyphonServer(-1);
            
            for (int i=0; i<_syphonServersNames.size(); i++) {
                setToggleValue(_syphonUI, _syphonServersNames[i], false);
            }
        }
    }
    else {
        for (int i=0; i<_syphonServersNames.size(); i++) {
            if (name == _syphonServersNames[i] && getToggleValue(_syphonUI, name)) {
                _mtlMapping2D->selectSyphonServer(i);
                return;
            }
        }
    }
}
#endif


#if defined(USE_VIDEO_PLAYER_OPTION)
//--------------------------------------------------------------
void ofxMtlMapping2DControls::videoPlayerUiEvent(ofxUIEventArgs &event)
{
    string name = event.widget->getName();
    
    if (name == "NONE") {
        // TO DO:: Set this to clear the video settings!
    }
    else if ((name == "SELECT FILE" && getButtonValue(_videoPlayerUI, name)) || (_bInitialized && name == "FILE PATH")) {
        
        ofxUITextInput* textInput = (ofxUITextInput*) _videoPlayerUI->getWidget("FILE PATH");
        
        // If the event comes from the TextInput widget, and the TextInput widget does not have the focus
        // it means that the user just presented 'cancel' in the system dialog window.
        if (name == "FILE PATH" && !textInput->isFocused()) {
            return;
        }
        
        ofFileDialogResult fileDialogResult = ofSystemLoadDialog();
        
        textInput->setFocus(false);
        
        if (!fileDialogResult.bSuccess) {
            return;
        }
        
        string path = fileDialogResult.getPath();
        extraOutputSettings["video_file_path"] = path;
        textInput->setTextString(fileDialogResult.getName());
        _mtlMapping2D->loadVideo(path);
        
        // ---
        ((ofxUIImageToggle *)_videoPlayerUI->getWidget("PLAY"))->setImage(&_videoPlayIcon);
        setToggleValue(_videoPlayerUI, "PLAY", false);
    }
    else if (name == "LOAD VIDEO ON START"  && getToggleValue(_videoPlayerUI, name)) {
        if (!_bInitialized && extraOutputSettings["video_file_path"] != "") {
            _mtlMapping2D->loadVideo(extraOutputSettings["video_file_path"]);
        }
    }
    else if (name == "STOP" && getButtonValue(_videoPlayerUI, name)) {
        _mtlMapping2D->stopVideo();
        
        // ---
        ((ofxUIImageToggle *)_videoPlayerUI->getWidget("PLAY"))->setImage(&_videoPlayIcon);
        setToggleValue(_videoPlayerUI, "PLAY", false);
    }
    else if (name == "PLAY") {
        bool toggleValue = getToggleValue(_videoPlayerUI, name);

        if (toggleValue) {
            _mtlMapping2D->playVideo();
            ((ofxUIImageToggle *)_videoPlayerUI->getWidget("PLAY"))->setImage(&_videoPauseIcon);
        } else {
            _mtlMapping2D->pauseVideo();
            ((ofxUIImageToggle *)_videoPlayerUI->getWidget("PLAY"))->setImage(&_videoPlayIcon);
        }
        
    }
    else if (name == "TRACK") {
        _mtlMapping2D->setVideoPostion(getSliderValue(_videoPlayerUI, name));
    }
}
#endif

//--------------------------------------------------------------
void ofxMtlMapping2DControls::setUIShapeEditingState(bool isOn)
{
    
    if (!isOn) {
        _mtlMapping2D->setModeState(MAPPING_LOCKED);
        
        // Set all Shapes and their InteractiveObj
        list<ofxMtlMapping2DShape*>::iterator it;
        for (it=ofxMtlMapping2DShapes::pmShapes.begin(); it!=ofxMtlMapping2DShapes::pmShapes.end(); it++) {
            ofxMtlMapping2DShape* shape = *it;
            shape->setAsIdle();
        }
        
        // ---
        ofxMtlMapping2DShape::resetActiveShapeVars();
        ofxMtlMapping2DShape::resetActivePolygonVars();
        
        // ---
        _shapesListCanvas->disable();
        hideGridSettingsCanvas();
        
    } else {
        _mtlMapping2D->setModeState(MAPPING_EDIT);
        
        _shapesListCanvas->enable();
        showGridSettingsCanvas();
    }
    
    // ---
    ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingSave))->setVisible(isOn);
    ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingLoad))->setVisible(isOn);

    ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingModeInput))->setVisible(isOn);
    ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingModeOutput))->setVisible(isOn);
    
    if (ofxMtlMapping2DSettings::kIsManuallyCreatingShapeEnabled) {
        ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingCreateNewQuad))->setVisible(isOn);
        ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingCreateNewGrid))->setVisible(isOn);
        ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingCreateNewTriangle))->setVisible(isOn);
        ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingCreateNewMask))->setVisible(isOn);
    }
}


#pragma mark -
#pragma mark Shapes List Related

//--------------------------------------------------------------
void ofxMtlMapping2DControls::shapesListUiEvent(ofxUIEventArgs &event)
{
    string name = event.widget->getName();
    
    vector<string> result = ofSplitString(name, " ");
    string action = result[0];
    int shapeId = ofToInt(result[1]);
    string shapeTypeAsString = result[2];
    int shapeType = -1;
    
    if (action == "Select") {
        map<ofxMtlMapping2DShapeType,string>::iterator it;
        for ( it = shapeTypesAsString.begin() ; it != shapeTypesAsString.end(); it++ ) {
            if (it->second == shapeTypeAsString)
            {
                shapeType = it->first;
                break;
            }
        }
        
        //setAsActiveShapeWithId(shapeId, shapeType);
        
        for (int i=0; i < _shapesListCanvas->getWidgetsOfType(OFX_UI_WIDGET_TOGGLE).size(); i++) {
            ofxUIToggle * shapeToggle = (ofxUIToggle *)_shapesListCanvas->getWidgetsOfType(OFX_UI_WIDGET_TOGGLE)[i];
            
            string nameB = shapeToggle->getName();
            
            vector<string> resultB = ofSplitString(nameB, " ");
            string actionB = resultB[0];
            int sId = ofToInt(resultB[1]);
            
            if (actionB == "Select") {
                if ( sId != shapeId ) {
                    shapeToggle->setValue(false);
                }
            }
        }
        
        
        _mtlMapping2D->selectShapeId(shapeId);
    }
    else if (action == "Shape") {
        ofxUIImageToggle* uiImageToggle = ((ofxUIImageToggle *)_shapesListCanvas->getWidget(name));
        
        // Lock / Unlock this shape
        if (getToggleValue(_shapesListCanvas, name)) {
            _mtlMapping2D->setLockForShapeId(shapeId, true);
            uiImageToggle->setImage(&_lockedIcon);
        } else {
            _mtlMapping2D->setLockForShapeId(shapeId, false);
            uiImageToggle->setImage(&_unlockedIcon);
        }
    }

}


//--------------------------------------------------------------
void ofxMtlMapping2DControls::addShapeToList(int shapeID, int shapeType, bool bLocked)
{
    //_shapesListCanvas->addWidgetDown(new ofxUIToggle(("Shape " + ofToString(shapeID) + " " + shapeTypesAsString.find((ofxMtlMapping2DShapeType)shapeType)->second), false, kToggleSize, kToggleSize));
    _shapesListCanvas->addToggle("Select " + ofToString(shapeID) + " " + shapeTypesAsString.find((ofxMtlMapping2DShapeType)shapeType)->second, false, 16, 16)->setLabelVisible(false);
    _shapesListCanvas->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    ofxUIImageToggle* uiImageToggle = _shapesListCanvas->addImageToggle("Shape " + ofToString(shapeID) + " " + shapeTypesAsString.find((ofxMtlMapping2DShapeType)shapeType)->second, "", bLocked, 16, 16);
    uiImageToggle->setLabelVisible(true);
    
    if (bLocked) {
        uiImageToggle->setImage(&_lockedIcon);
    } else {
        uiImageToggle->setImage(&_unlockedIcon);
    }
    
    _shapesListCanvas->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    
    resizeShapeList();
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::clearShapesList()
{
    _shapesListCanvas->removeWidgets();
    _shapesListCanvas->resetPlacer();
    resizeShapeList();
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::refreshShapesListForMappingView(MappingEditView currView)
{
    clearShapesList();
    
    // Re populate the UI List
    ofxMtlMapping2DShapes::pmShapes.sort(Comparator());

    list<ofxMtlMapping2DShape*>::reverse_iterator it;
    for (it=ofxMtlMapping2DShapes::pmShapes.rbegin(); it!=ofxMtlMapping2DShapes::pmShapes.rend(); it++) {
        ofxMtlMapping2DShape* shape = *it;
        
        if (currView == MAPPING_CHANGE_TO_OUTPUT_VIEW || (currView == MAPPING_CHANGE_TO_INPUT_VIEW && shape->shapeType != MAPPING_2D_SHAPE_MASK)) {
            addShapeToList(shape->shapeId, shape->shapeType, ofToBool(shape->shapeSettings["isLocked"]));
        }
    }
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::resizeShapeList()
{
    _shapesListCanvas->autoSizeToFitWidgets();
    _shapesListCanvas->getRect()->setY(.0f);
    _shapesListCanvas->getRect()->setWidth(kControlsMappingShapesListPanelWidth);
    
    float listHeight = _shapesListCanvas->getRect()->height;
    if(listHeight < ofGetHeight()) {
        //_shapesListCanvas->getRect()->setHeight(ofGetHeight());
        _shapesListCanvas->setScrollableDirections(false, false);
    } else {
        //_shapesListCanvas->getRect()->setHeight(listHeight + kBottomSpacerHeight);
        _shapesListCanvas->setScrollableDirections(false, true);

    }
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::setAsActiveShapeWithId(int shapeID, int shapeType)
{    
    // ---
    for (int i=0; i < _shapesListCanvas->getWidgetsOfType(OFX_UI_WIDGET_TOGGLE).size(); i++) {
        ofxUIToggle * shapeToggle = (ofxUIToggle *)_shapesListCanvas->getWidgetsOfType(OFX_UI_WIDGET_TOGGLE)[i];
        
        string name = shapeToggle->getName();
        
        vector<string> result = ofSplitString(name, " ");
        string action = result[0];
        int sId = ofToInt(result[1]);
        
        if (action == "Select") {
            if ( sId == shapeID ) {
                shapeToggle->setValue(true);
            } else {
                shapeToggle->setValue(false);
            }
        }
    }
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::unselectShapesToggles()
{
    for (int i=0; i < _shapesListCanvas->getWidgetsOfType(OFX_UI_WIDGET_TOGGLE).size(); i++) {
        ofxUIToggle * shapeToggle = (ofxUIToggle *)_shapesListCanvas->getWidgetsOfType(OFX_UI_WIDGET_TOGGLE)[i];
        shapeToggle->setValue(false);
    }
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::updateUIsPosition()
{
    if (ofGetWindowMode() == OF_FULLSCREEN) {
        ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingFullscreen))->setImage(&_fullscreenContractIcon);
        ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingFullscreen))->setValue(true);
    } else {
        ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingFullscreen))->setImage(&_fullscreenExpandIcon);
        ((ofxUIImageToggle *)_toolsCanvas->getWidget(kSettingMappingFullscreen))->setValue(false);
    }
    
    _toolsCanvas->setHeight(ofGetHeight());

    _gridSettingsCanvas->setPosition(_toolsCanvas->getRect()->width, ofGetHeight() - 90);
    _settingsUI->setPosition(ofGetWidth() - _settingsUI->getRect()->width, 0);
#if defined(USE_OFX_DETECT_DISPLAYS)
    _outputUI->setPosition(ofGetWidth() - _outputUI->getRect()->width, _settingsUI->getRect()->height + 5);
    _displaysListUI->setPosition(ofGetWidth() - _displaysListUI->getRect()->width, _outputUI->getRect()->y + _outputUI->getRect()->height);
#endif
    
#if defined(USE_VIDEO_PLAYER_OPTION)
    #if defined(USE_OFX_DETECT_DISPLAYS)
        _videoPlayerUI->setPosition(ofGetWidth() - _videoPlayerUI->getRect()->width, _displaysListUI->getRect()->y + _displaysListUI->getRect()->height + 5);
    #elif
        _videoPlayerUI->setPosition(ofGetWidth() - _videoPlayerUI->getRect()->width, _settingsUI->getRect()->height + 5);
    #endif
#endif
    
#if defined(USE_OFX_SYPHON) && defined(TARGET_OSX)
    #if defined(USE_VIDEO_PLAYER_OPTION)
        _syphonUI->setPosition(ofGetWidth() - _syphonUI->getRect()->width, _videoPlayerUI->getRect()->y + _videoPlayerUI->getRect()->height + 5);
    #else
        #if defined(USE_OFX_DETECT_DISPLAYS)
            _syphonUI->setPosition(ofGetWidth() - _syphonUI->getRect()->width, _displaysListUI->getRect()->y + _displaysListUI->getRect()->height + 5);
        #elif
            _syphonUI->setPosition(ofGetWidth() - _syphonUI->getRect()->width, _settingsUI->getRect()->height + 5);
        #endif
    #endif
#endif
}

#pragma mark -
#pragma mark Settings UI

#if defined(USE_OFX_DETECT_DISPLAYS)

//--------------------------------------------------------------
void ofxMtlMapping2DControls::displayConfigurationChanged()
{
    _displaysListUI->removeWidgets();
    _displayNames.clear();
    
    for (int i=0; i<ofxDetectDisplaysSharedInstance().getDisplays().size(); i++) {
        _displayNames.push_back(ofToString(ofxDetectDisplaysSharedInstance().getDisplays()[i]->width) + "x" + ofToString(ofxDetectDisplaysSharedInstance().getDisplays()[i]->height) + " - UID:" + ofxDetectDisplaysSharedInstance().getDisplays()[i]->UID);
    }
    
    _displaysListUI->addRadio("DISPLAYS", _displayNames);
    _displaysListUI->autoSizeToFitWidgets();
    
    updateUIsPosition();
}

#endif

#pragma mark -
#pragma mark Grid settings

//--------------------------------------------------------------
void ofxMtlMapping2DControls::gridSettingsListUiEvent(ofxUIEventArgs &event)
{
    if(ofxMtlMapping2DShape::activeShape) {
        if (ofxMtlMapping2DShape::activeShape->shapeType == MAPPING_2D_SHAPE_GRID) {
            ((ofxMtlMapping2DGrid*)ofxMtlMapping2DShape::activeShape)->updateGrid();
        }
    }
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::showGridSettingsCanvas()
{
    if(isEnabled() && ofxMtlMapping2DShape::activeShape) {
        if (ofxMtlMapping2DShape::activeShape->shapeType == MAPPING_2D_SHAPE_GRID) {
            _gridSettingsCanvas->removeWidgets();
            _gridSettingsCanvas->resetPlacer();
            
            ofxUISlider *nSlider;
            _gridSettingsCanvas->addLabel("GRID SETTINGS");
            nSlider = _gridSettingsCanvas->addSlider("NB COLS", 1.0, 20.0, &(((ofxMtlMapping2DGrid*)ofxMtlMapping2DShape::activeShape)->gridNbCols));
            nSlider->setIncrement(1.0f);
            nSlider = _gridSettingsCanvas->addSlider("NB ROWS", 1.0, 20.0, &(((ofxMtlMapping2DGrid*)ofxMtlMapping2DShape::activeShape)->gridNbRows));
            nSlider->setIncrement(1.0f);
             
            _shapesListCanvas->autoSizeToFitWidgets();

            _gridSettingsCanvas->enable();
        }
    }
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::hideGridSettingsCanvas()
{
    _gridSettingsCanvas->disable();
}

#pragma mark -
#pragma mark Set avalaible options
//--------------------------------------------------------------
void ofxMtlMapping2DControls::showInputModeToggle()
{
    ((ofxUIToggle *)_toolsCanvas->getWidget(kSettingMappingModeInput))->setVisible(true);
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::hideInputModeToggle()
{
    ((ofxUIToggle *)_toolsCanvas->getWidget(kSettingMappingModeInput))->setVisible(false);
}

#pragma mark -
#pragma mark Interface Methods

//--------------------------------------------------------------
bool ofxMtlMapping2DControls::getButtonValue(ofxUICanvas* ui, const string& name)
{
    return ((ofxUIButton *)ui->getWidget(name))->getValue();
}


//--------------------------------------------------------------
bool ofxMtlMapping2DControls::getToggleValue(ofxUICanvas* ui, const string& name)
{
    return ((ofxUIToggle *)ui->getWidget(name))->getValue();
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::setToggleValue(ofxUICanvas* ui, const string& name, float value)
{
    ((ofxUIToggle *)ui->getWidget(name))->setValue(value);
}

//--------------------------------------------------------------
float ofxMtlMapping2DControls::getSliderValue(ofxUICanvas* ui, const string& name)
{
    return ((ofxUISlider *)ui->getWidget(name))->getScaledValue();
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::setSliderValue(ofxUICanvas* ui, const string& name, float value)
{
    ((ofxUISlider *)ui->getWidget(name))->setValue(value);
}

#pragma mark -
//--------------------------------------------------------------
void ofxMtlMapping2DControls::saveSettings()
{
    _settingsUI->saveSettings(_rootPath + _settingsUI->getCanvasTitle()->getLabel() + ".xml");
#if defined(USE_OFX_DETECT_DISPLAYS)
    _outputUI->saveSettings(_rootPath + _outputUI->getCanvasTitle()->getLabel() + ".xml");
    _displaysListUI->saveSettings(_rootPath + _displaysListUI->getCanvasTitle()->getLabel() + ".xml");
#endif
    
#if defined(USE_OFX_SYPHON) && defined(TARGET_OSX)
    _syphonUI->saveSettings(_rootPath + _syphonUI->getCanvasTitle()->getLabel() + ".xml");
#endif
    
#if defined(USE_VIDEO_PLAYER_OPTION)
    _videoPlayerUI->saveSettings(_rootPath + _videoPlayerUI->getCanvasTitle()->getLabel() + ".xml");
#endif
    
    saveExtraSettings();
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::loadSettings()
{
    loadExtraSettings();
    
    _settingsUI->loadSettings(_rootPath + _settingsUI->getCanvasTitle()->getLabel() + ".xml");
#if defined(USE_OFX_DETECT_DISPLAYS)
    _outputUI->loadSettings(_rootPath + _outputUI->getCanvasTitle()->getLabel() + ".xml");
    _displaysListUI->loadSettings(_rootPath + _displaysListUI->getCanvasTitle()->getLabel() + ".xml");
#endif
    
#if defined(USE_VIDEO_PLAYER_OPTION)
    loadVideoPlayerSettings();
#endif

    updateUIsPosition();
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::enable()
{
    for (int i=0; i<_uiSuperCanvases.size(); i++) {
        _uiSuperCanvases[i]->enable();
    }
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::disable()
{
    for (int i=0; i<_uiSuperCanvases.size(); i++) {
        _uiSuperCanvases[i]->disable();
    }
}

//--------------------------------------------------------------
bool ofxMtlMapping2DControls::isEnabled()
{
    return _toolsCanvas->isEnabled();
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::toggleVisible()
{
    _toolsCanvas->toggleVisible();
    
    if (_toolsCanvas->isVisible() && _mtlMapping2D->getModeState() == MAPPING_EDIT) {
        _shapesListCanvas->enable();
        showGridSettingsCanvas();
    } else {
        _shapesListCanvas->disable();
        hideGridSettingsCanvas();
    }
}

//--------------------------------------------------------------
bool ofxMtlMapping2DControls::isVisible()
{
    return _toolsCanvas->isVisible();
}

//--------------------------------------------------------------
bool ofxMtlMapping2DControls::isHit(int x, int y) {
    
    for (auto& it : _uiSuperCanvases) {
        if (it->isHit(x, y)) {
            return true;
        }
    }
    
    return false;
}


//--------------------------------------------------------------
void ofxMtlMapping2DControls::saveExtraSettings()
{
    ofxXmlSettings xmlSettings;
    
	xmlSettings.addTag("root");
	xmlSettings.pushTag("root", 0);
    
    map<string,string>::iterator itSettings;
    for ( itSettings=extraOutputSettings.begin() ; itSettings != extraOutputSettings.end(); itSettings++ ) {
        int tagNum = xmlSettings.addTag("setting");
        xmlSettings.addAttribute("setting", "key", (*itSettings).first, tagNum);
        xmlSettings.setValue("setting", (*itSettings).second, tagNum);
    }
		
	//Save to file
	xmlSettings.saveFile(_rootPath + "extraSettings.xml");
    ofLogNotice() << "Status > settings saved to xml!";
    
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::loadExtraSettings()
{
    
    ofxXmlSettings xmlSettings;
    string xmlFile = _rootPath + "extraSettings.xml";
    string feedBackMessage = "";
    
	//we load our settings file
	if( xmlSettings.loadFile(xmlFile) ){
		feedBackMessage = xmlFile + " loaded!";
	}else{
		feedBackMessage = "unable to load " + xmlFile + " check data/ folder";
	}
    ofLogNotice() << "Status > " << feedBackMessage;
    
    // ---
	int numRootTags = xmlSettings.getNumTags("root");
	
	if(numRootTags > 0){
		// ---
        xmlSettings.pushTag("root", 0);
		
        //Settings
        int numSettingTags = xmlSettings.getNumTags("setting");
        
        if(numSettingTags > 0) {
            for(int j = 0; j < numSettingTags; j++){
                string key = xmlSettings.getAttribute("setting", "key", "nc", j);
                string value = xmlSettings.getValue("setting", "", j);
                extraOutputSettings[key] = value;
            }
        }
				
		// ---
		xmlSettings.popTag();
	}
}


#if defined(USE_OFX_SYPHON) && defined(TARGET_OSX)
#pragma mark -
#pragma mark Syphon
//--------------------------------------------------------------
void ofxMtlMapping2DControls::addSyphonServer(vector<ofxSyphonServerDescription> servers)
{
    for( auto& syphonServerDir : servers ){
        string serverName = syphonServerDir.serverName;
        string appName = syphonServerDir.appName;
        
        
        if (serverName.empty()) {
            serverName = "N.C.";
        }
        
        if (appName.empty()) {
            appName = "N.C.";
        }
        
        string serverID = serverName; // + " - " + appName;

        list<string>::iterator it;
        for (it=_syphonServersList.begin(); it!=_syphonServersList.end(); it++) {
            if (*it == serverID) {
                return;
            }
        }
        
        _syphonServersList.push_back(serverID);

    }
    
    updateSyphonServersList();
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::removeSyphonServer(vector<ofxSyphonServerDescription> servers)
{
    for( auto& syphonServerDir : servers ){
        string serverName = syphonServerDir.serverName;
        string appName = syphonServerDir.appName;
        
        if (serverName.empty()) {
            serverName = "N.C.";
        }
        
        if (appName.empty()) {
            appName = "N.C.";
        }
        
        string serverID = serverName;// + " - " + appName;

        list<string>::iterator it;
        for (it=_syphonServersList.begin(); it!=_syphonServersList.end(); it++) {
            if (*it == serverID) {
                _syphonServersList.remove(serverID);
            } else {
                // do nothing for now
            }
        }
    }
    
    updateSyphonServersList();
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::updateSyphonServersList()
{
    _syphonUI->removeWidgets();
    _syphonServersNames.clear();
    
    //_syphonUI->setWidgetPosition(OFX_UI_WIDGET_POSITION_UP);
    //_syphonUI->addLabel("SYPHON SETTINGS");
    //_syphonUI->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);

    _syphonUI->addSpacer(_syphonUI->getRect()->width - 10, 2);
    
    _syphonUI->addButton("NONE", false);
    _syphonUI->addSpacer((_syphonUI->getRect()->width - 10) / 2, 1);
    
    cout << "updateSyphonServersList" << endl;

    list<string>::iterator it;
    for (it=_syphonServersList.begin(); it!=_syphonServersList.end(); it++) {
        cout << "updateSyphonServersList: " << *it << endl;

        _syphonServersNames.push_back(*it);
    }
    
    _syphonUI->addRadio("SERVERS", _syphonServersNames);
    _syphonUI->autoSizeToFitWidgets();
}

//--------------------------------------------------------------
void ofxMtlMapping2DControls::loadSyphonSettings()
{
    _syphonUI->loadSettings(_rootPath + _syphonUI->getCanvasTitle()->getLabel() + ".xml");
    
    updateUIsPosition();
}

#endif

#if defined(USE_VIDEO_PLAYER_OPTION)
//--------------------------------------------------------------
void ofxMtlMapping2DControls::loadVideoPlayerSettings()
{
    _videoPlayerUI->loadSettings(_rootPath + _videoPlayerUI->getCanvasTitle()->getLabel() + ".xml");
    
    updateUIsPosition();
}
#endif


