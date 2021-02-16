#pragma once
#include "ofMain.h"
#include "ofxGui.h"
#include "result.hpp"

class ofApp : public ofBaseApp {
  Result* P;

  // params
  int w;
  int h;
  int scale;
  float agent_rad;
  float goal_rad;
  int font_size;

  // gui
  ofxIntSlider step_slider;
  ofxFloatSlider timestep_slider;
  ofxFloatSlider speed_slider;
  ofxPanel gui_time_ind;
  ofxPanel gui_mapf;

  // flg
  bool flg_autoplay;
  bool flg_loop;
  bool flg_goal;
  bool flg_font;

  enum TAB { TAB_TIME_IND, TAB_MAPF };
  TAB focus_tab;

  enum LINE_MODE { STRAIGHT, PATH, NONE, NUM };
  LINE_MODE line_mode;

  // visual
  ofTrueTypeFont font;
  ofTrueTypeFont font_info;

  // execution
  std::vector<States> exec_full;

  enum struct Mode { CONTRACTED, EXTENDED, REQUESTING };

public:
  ofApp(Result* _P);
  ~ofApp();
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
  void printKeys();
};
