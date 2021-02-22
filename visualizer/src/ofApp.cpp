#include "../include/ofApp.hpp"
#include "../include/params.hpp"


ofApp::ofApp(Result* _P): P(_P)
{
  flg_autoplay = true;
  flg_loop = true;
  flg_goal = true;
  flg_font = false;

  line_mode = LINE_MODE::STRAIGHT;
  focus_tab = TAB::TAB_TIME_IND;

  States s;
  for (int i = 0; i < P->num_agents; ++i) {
    s.push_back(std::make_tuple(i, 0, 0, -1, P->config_s[i]->id));
  }
  exec_full.push_back(s);

  for (int t = 0; t < P->activated_cnt; ++t) {
    auto new_s = s;
    auto c = P->exec[t];
    new_s[std::get<0>(c)] = c;
    exec_full.push_back(new_s);
    s = new_s;
  }
}

ofApp::~ofApp()
{
  delete P;
}

void ofApp::setup()
{
  // setup size
  int map_w = P->G->getWidth();
  int map_h = P->G->getHeight();
  int window_max_w = BufferSize::default_screen_width
    - BufferSize::screen_x_buffer * 2 - BufferSize::window_x_buffer * 2;
  int window_max_h = BufferSize::default_screen_height
    - BufferSize::window_y_top_buffer - BufferSize::window_y_bottom_buffer;
  scale = std::min(window_max_w / map_w, window_max_h / map_h) + 1;
  w = map_w * scale + 2 * BufferSize::window_x_buffer;
  h = map_h * scale
    + BufferSize::window_y_top_buffer + BufferSize::window_y_bottom_buffer;

  agent_rad = std::max((float)scale/std::sqrt(2)/2, 3.0);
  goal_rad  = std::max((float)scale/4.0, 2.0);
  font_size = std::max(scale/8, 6);

  ofSetWindowShape(w * 2, h);
  ofBackground(Color::bg);
  ofSetCircleResolution(32);
  ofSetFrameRate(30);

  const std::string font_name = "MuseoModerno-VariableFont_wght.ttf";
  font.load(font_name, font_size);
  font_info.load(font_name, 10);

  // setup gui
  gui_time_ind.setup("time-independent", "", 10, 10);
  gui_time_ind.add(step_slider.setup("step", 0, 0, P->activated_cnt));
  gui_mapf.setup("mapf execution", "", 10 + w, 10);
  gui_mapf.add(timestep_slider.setup("time step", 0, 0, P->makespan));
  gui_mapf.add(speed_slider.setup("speed", 0.1, 0, 1));
  printKeys();
}

void ofApp::update()
{
  if (!flg_autoplay) return;

  if (focus_tab == TAB::TAB_TIME_IND) {
    int step = step_slider;
    step = step + 1;
    if (step <= P->activated_cnt) {
      step_slider = step;
    } else {
      step_slider = 0;
      if (flg_loop) {
        step_slider = 0;
      } else {
        step_slider = P->activated_cnt;
      }
    }
  } else {
    // t <- t + speed
    float t;
    t = timestep_slider + speed_slider;
    if (t <= P->makespan) {
      timestep_slider = t;
    } else {
      timestep_slider = 0;
      if (flg_loop) {
        timestep_slider = 0;
      } else {
        timestep_slider = P->makespan;
      }
    }
  }
}

void ofApp::draw()
{
  // focus
  constexpr int focus_width = 8;
  ofNoFill();
  ofSetColor(Color::focus);
  ofSetLineWidth(focus_width);
  if (focus_tab == TAB::TAB_TIME_IND) {
    ofDrawRectangle(focus_width/2, focus_width/2,
                    w-focus_width, h-focus_width);
  } else {
    ofDrawRectangle(focus_width/2 + w, focus_width/2,
                    w-focus_width, h-focus_width);
  }

  // draw nodes
  ofSetLineWidth(1);
  ofFill();
  for (int x = 0; x < P->G->getWidth(); ++x) {
    for (int y = 0; y < P->G->getHeight(); ++y) {
      if (!P->G->existNode(x, y)) continue;
      ofSetColor(Color::node);
      int x_draw = x*scale-scale/2+0.5
        + BufferSize::window_x_buffer + scale/2;
      int y_draw = y*scale-scale/2+0.5
        + BufferSize::window_y_top_buffer + scale/2;
      ofDrawRectangle(x_draw, y_draw, scale-0.5, scale-0.5);
      ofDrawRectangle(x_draw + w, y_draw, scale-0.5, scale-0.5);
      if (flg_font) {
        ofSetColor(Color::font);
        font.drawString(std::to_string(y*P->G->getWidth()+x),
                        x_draw + 1, y_draw + font_size + 1);
        font.drawString(std::to_string(y*P->G->getWidth()+x),
                        x_draw + 1 + w, y_draw + font_size + 1);
      }
    }
  }

  // draw goals
  if (flg_goal) {
    for (int i = 0; i < P->num_agents; ++i) {
      ofSetColor(Color::agents[i % Color::agents.size()]);
      Node* g = P->config_g[i];
      Pos pos1 = g->pos * scale;
      int x = pos1.x + BufferSize::window_x_buffer + scale/2;
      int y = pos1.y + BufferSize::window_y_top_buffer + scale/2;
      ofDrawRectangle(x - goal_rad/2, y - goal_rad/2, goal_rad, goal_rad);
      ofDrawRectangle(x - goal_rad/2 + w, y - goal_rad/2, goal_rad, goal_rad);
    }
  }

  // draw agents
  for (int i = 0; i < P->num_agents; ++i) {

    // === time-independent model ===
    {
      ofSetColor(Color::agents[i % Color::agents.size()]);

      int step = (int)step_slider;
      auto state = exec_full[step][i];

      // tail
      Pos pos_t = P->G->getNode(std::get<4>(state))->pos;
      float x_t = pos_t.x * scale + BufferSize::window_x_buffer + scale/2;
      float y_t = pos_t.y * scale + BufferSize::window_y_top_buffer + scale/2;

      // activated?
      if (step > 0 && std::get<0>(P->exec[step-1]) == i) {
        ofSetColor(Color::activated);
        ofDrawRectangle(x_t-agent_rad-2, y_t-agent_rad-2,
                        agent_rad*2+4, agent_rad*2+4);
        ofSetColor(Color::agents[i % Color::agents.size()]);
      }

      ofDrawCircle(x_t, y_t, agent_rad);

      // head
      auto head = std::get<3>(state);
      if (head != -1) {
        Pos pos_h = P->G->getNode(head)->pos;
        float x_h = pos_h.x * scale + BufferSize::window_x_buffer + scale/2;
        float y_h = pos_h.y * scale + BufferSize::window_y_top_buffer + scale/2;
        auto mode = std::get<2>(state);
        if (mode == (int)Mode::EXTENDED) {
          ofDrawCircle(x_h, y_h, agent_rad/2);
          ofSetLineWidth(2);
          ofDrawLine(x_t, y_t, x_h, y_h);
          ofSetLineWidth(1);
        } else {  // requesting
          ofNoFill();
          ofDrawCircle(x_h, y_h, agent_rad/2);
          ofDrawLine(x_t, y_t, x_h, y_h);
          ofFill();
        }
      }

      // agent at goal
      if (P->G->getNode(std::get<4>(state)) == P->config_g[i] && head == -1) {
        ofSetColor(255,255,255);
        ofDrawCircle(x_t, y_t, agent_rad-2);
        ofSetColor(Color::agents[i % Color::agents.size()]);
      }

      // id
      if (flg_font) {
        ofSetColor(Color::font);
        font.drawString(std::to_string(i), x_t-font_size/2, y_t+font_size/2);
      }
    }

    // === mapf execution ===
    {
      ofSetColor(Color::agents[i % Color::agents.size()]);
      int t1 = (int)timestep_slider;
      int t2 = t1 + 1;

      // agent position
      Node* v = P->transitions[t1][i];
      Pos pos1 = v->pos;
      float x = pos1.x;
      float y = pos1.y;

      if (t2 <= P->makespan) {
        Pos pos2 = P->transitions[t2][i]->pos;
        x += (pos2.x - x) * (timestep_slider - t1);
        y += (pos2.y - y) * (timestep_slider - t1);
      }
      x *= scale;
      y *= scale;
      x += BufferSize::window_x_buffer + scale/2;
      y += BufferSize::window_y_top_buffer + scale/2;

      ofDrawCircle(x+w, y, agent_rad);

      // goal
      if (line_mode == LINE_MODE::STRAIGHT) {
        Pos pos3 = P->config_g[i]->pos * scale;
        ofDrawLine(pos3.x + BufferSize::window_x_buffer + scale/2 + w,
                   pos3.y + BufferSize::window_y_top_buffer + scale/2, x + w, y);
      } else if (line_mode == LINE_MODE::PATH) {
        // next loc
        ofSetLineWidth(2);
        Pos pos2(x-BufferSize::window_x_buffer-scale/2, y-BufferSize::window_y_top_buffer-scale/2);
        for (int t = t1; t < P->makespan; ++t) {
          Pos pos3 = P->transitions[t+1][i]->pos * scale;
          if (pos3 == pos2) continue;
          ofDrawLine(pos2.x + BufferSize::window_x_buffer + scale/2 + w,
                     pos2.y + BufferSize::window_y_top_buffer + scale/2,
                     pos3.x + BufferSize::window_x_buffer + scale/2 + w,
                     pos3.y + BufferSize::window_y_top_buffer + scale/2);
          pos2 = pos3;
        }
        ofSetLineWidth(1);
      }

      // agent at goal
      if (v == P->config_g[i]) {
        ofSetColor(255,255,255);
        ofDrawCircle(x+w, y, agent_rad-2);
      }

      // id
      if (flg_font) {
        ofSetColor(Color::font);
        font.drawString(std::to_string(i), x-font_size/2+w, y-font_size/2);
        // delay probabilities
        font.drawString(std::to_string(P->delay_probs[i]).substr(1,4), x-font_size+w, y+font_size);
      }
    }
  }

  // info
  ofSetColor(Color::font_info);
  int x = 220;
  int y = 5;
  font_info.drawString("solved by " + P->solver,
                       x, y+=15);
  std::string solved_status = P->solved ? "success" : "failed";
  font_info.drawString(solved_status
                       + ", comp-time (ms): "
                       + std::to_string(P->comp_time),
                       x, y+=15);
  font_info.drawString("agents: "
                       + std::to_string(P->num_agents)
                       + ", map: " + P->G->getMapFileName(),
                       x, y+=15);
  font_info.drawString("activated_cnt: "
                       + std::to_string(P->activated_cnt),
                       x, y+=15);
  x = 220 + w;
  y = 5;
  font_info.drawString("makespan: "
                       + std::to_string(P->makespan)
                       + ", soc: "
                       + std::to_string(P->soc),
                       x, y+=15);
  font_info.drawString("ub_delay_prob: " + std::to_string(P->ub_delay_prob),
                       x, y+=15);

  gui_time_ind.draw();
  gui_mapf.draw();
}

void ofApp::keyPressed(int key) {
  if (key == 9) {  // tab
    focus_tab = (focus_tab == TAB::TAB_TIME_IND) ? TAB::TAB_MAPF : TAB::TAB_TIME_IND;
  }
  if (key == 'r') {
    if (focus_tab == TAB_TIME_IND) {
      step_slider = 0;
    } else {
      timestep_slider = 0;
    }
  }
  if (key == 'p') flg_autoplay = !flg_autoplay;
  if (key == 'l') flg_loop = !flg_loop;
  if (key == 'v') {
    line_mode = static_cast<LINE_MODE>(((int)line_mode + 1)%(int)LINE_MODE::NUM);
  }
  if (key == 'g') flg_goal = !flg_goal;
  if (key == 'f') {
    flg_font = !flg_font;
    flg_font &= (scale - font_size > 6);
  }

  float t;
  int step;
  if (key == OF_KEY_RIGHT) {
    if (focus_tab == TAB_TIME_IND) {
      step = step_slider + 1;
      step_slider = std::min(P->activated_cnt, step);
    } else {
      t = timestep_slider + speed_slider;
      timestep_slider = std::min((float)P->makespan, t);
    }
  }
  if (key == OF_KEY_LEFT) {
    if (focus_tab == TAB_TIME_IND) {
      step = step_slider - 1;
      step_slider = std::max(0, step);
    } else {
      t = timestep_slider - speed_slider;
      timestep_slider = std::max((float)0, t);
    }
  }
  if (key == OF_KEY_UP) {
    t = speed_slider + 0.001;
    speed_slider = std::min(t, (float)speed_slider.getMax());
  }
  if (key == OF_KEY_DOWN) {
    t = speed_slider - 0.001;
    speed_slider = std::max(t, (float)speed_slider.getMin());
  }
}

void ofApp::dragEvent(ofDragInfo dragInfo) {}
void ofApp::keyReleased(int key) {}
void ofApp::mouseMoved(int x, int y) {}
void ofApp::mouseDragged(int x, int y, int button) {}
void ofApp::mousePressed(int x, int y, int button) {}
void ofApp::mouseReleased(int x, int y, int button) {}
void ofApp::mouseEntered(int x, int y) {}
void ofApp::mouseExited(int x, int y) {}
void ofApp::windowResized(int w, int h) {}
void ofApp::gotMessage(ofMessage msg) {}

void ofApp::printKeys()
{
  std::cout << "keys for visualizer" << std::endl;
  std::cout << "- p : play or pause" << std::endl;
  std::cout << "- l : loop or not" << std::endl;
  std::cout << "- r : reset" << std::endl;
  std::cout << "- v : show virtual line to goals" << std::endl;
  std::cout << "- f : show agent & node id" << std::endl;
  std::cout << "- g : show goals" << std::endl;
  std::cout << "- right : progress" << std::endl;
  std::cout << "- left  : back" << std::endl;
  std::cout << "- up    : speed up" << std::endl;
  std::cout << "- down  : speed down" << std::endl;
  std::cout << "- esc : terminate" << std::endl;
}
