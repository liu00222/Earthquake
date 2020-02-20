/** CSci-4611 Assignment 3:  Earthquake
 */

#include "quake_app.h"
#include "config.h"

#include <deque>
#include <iostream>
#include <sstream>

// Number of seconds in 1 year (approx.)
const int PLAYBACK_WINDOW = 12 * 28 * 24 * 60 * 60;


// global variable storing the recent earthquakes information

std::vector<int> earthquakes;


// global variables to make the earth rotate and convert smoothly

// when target_mode = 1.0, there are three conditions:
// 1. convert from globe to map; 2. convert from map to globe; 3. stay at the globe mode
//
// when target_mode = 0.0, it means we stay at the map mode

bool sphere_mode = false;
float target_mode = 0.0;
float scalar_parameter = 0.0;
float rotation = 0.0;


using namespace std;

QuakeApp::QuakeApp() : GraphicsApp(1280,720, "Earthquake"),
    playback_scale_(15000000.0), debug_mode_(false)
{
    // Define a search path for finding data files (images and earthquake db)
    search_path_.push_back(".");
    search_path_.push_back("./data");
    search_path_.push_back(DATA_DIR_INSTALL);
    search_path_.push_back(DATA_DIR_BUILD);
    
    quake_db_ = EarthquakeDatabase(Platform::FindFile("earthquakes.txt", search_path_));
    current_time_ = quake_db_.earthquake(quake_db_.min_index()).date().ToSeconds();

 }


QuakeApp::~QuakeApp() {
}


void QuakeApp::InitNanoGUI() {
    // Setup the GUI window
    nanogui::Window *window = new nanogui::Window(screen(), "Earthquake Controls");
    window->setPosition(Eigen::Vector2i(10, 10));
    window->setSize(Eigen::Vector2i(400,200));
    window->setLayout(new nanogui::GroupLayout());
    
    date_label_ = new nanogui::Label(window, "Current Date: MM/DD/YYYY", "sans-bold");
    
    globe_btn_ = new nanogui::Button(window, "Globe");
    globe_btn_->setCallback(std::bind(&QuakeApp::OnGlobeBtnPressed, this));
    globe_btn_->setTooltip("Toggle between map and globe.");
    
    new nanogui::Label(window, "Playback Speed", "sans-bold");
    
    nanogui::Widget *panel = new nanogui::Widget(window);
    panel->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal,
                                            nanogui::Alignment::Middle, 0, 20));
    
    nanogui::Slider *slider = new nanogui::Slider(panel);
    slider->setValue(0.5f);
    slider->setFixedWidth(120);
    
    speed_box_ = new nanogui::TextBox(panel);
    speed_box_->setFixedSize(Eigen::Vector2i(60, 25));
    speed_box_->setValue("50");
    speed_box_->setUnits("%");
    slider->setCallback(std::bind(&QuakeApp::OnSliderUpdate, this, std::placeholders::_1));
    speed_box_->setFixedSize(Eigen::Vector2i(60,25));
    speed_box_->setFontSize(20);
    speed_box_->setAlignment(nanogui::TextBox::Alignment::Right);
    
    nanogui::Button* debug_btn = new nanogui::Button(window, "Toggle Debug Mode");
    debug_btn->setCallback(std::bind(&QuakeApp::OnDebugBtnPressed, this));
    debug_btn->setTooltip("Toggle displaying mesh triangles and normals (can be slow)");
    
    screen()->performLayout();
}

void QuakeApp::OnLeftMouseDrag(const Point2 &pos, const Vector2 &delta) {
    // Optional: In our demo, we adjust the tilt of the globe here when the
    // mouse is dragged up/down on the screen.
}


void QuakeApp::OnGlobeBtnPressed() {
    
    // when the earth transfers from plane to sphere, directly change the values of sphere_mode and target_mode
    
    if (!sphere_mode && target_mode == 0.0)
        target_mode = 1.0;
    
    sphere_mode = !sphere_mode;
}

void QuakeApp::OnDebugBtnPressed() {
    debug_mode_ = !debug_mode_;
}

void QuakeApp::OnSliderUpdate(float value) {
    speed_box_->setValue(std::to_string((int) (value * 100)));
    playback_scale_ = 30000000.0*value;
}


void QuakeApp::UpdateSimulation(double dt)  {
    // Advance the current time and loop back to the start if time is past the last earthquake
    current_time_ += playback_scale_ * dt;
    if (current_time_ > quake_db_.earthquake(quake_db_.max_index()).date().ToSeconds()) {
        current_time_ = quake_db_.earthquake(quake_db_.min_index()).date().ToSeconds();
    }
    if (current_time_ < quake_db_.earthquake(quake_db_.min_index()).date().ToSeconds()) {
        current_time_ = quake_db_.earthquake(quake_db_.max_index()).date().ToSeconds();
    }
    
    Date d(current_time_);
    stringstream s;
    s << "Current date: " << d.month()
        << "/" << d.day()
        << "/" << d.year();
    date_label_->setCaption(s.str());
    
    // TODO: Any animation, morphing, rotation of the earth, or other things that should
    // be updated once each frame would go here.
    
    if (target_mode > scalar_parameter)
        scalar_parameter += 1.5 * dt;
    
    if (target_mode < scalar_parameter)
        scalar_parameter -= 1.5 * dt;
    
    
    // keep scalar_parameter within the range [0, 1] and then update the earth data
    
    scalar_parameter = GfxMath::Clamp(scalar_parameter, 0.0, 1.0);
    earth_.UpdateEarth(scalar_parameter);
    
    
    // case 1: general rotation in sphere mode
    
    if (sphere_mode)
        rotation = rotation + 0.001 * scalar_parameter;
    
    
    // case 2: move from sphere mode to plane mode
    
    if (!sphere_mode && rotation > 0.0) {
        
        rotation -= 0.05;
        
        if(rotation < 0.0)
            rotation = 0.0;
        
        
        // only when the conversion completes, we set target_mode to 0.0
        
        if(rotation == 0)
            target_mode = 0.0;
    }
    
    
    // case 3: when some evil guys try to make use of the time difference in conversion of
    // the "sphere_mode" and "target_mode", we directly destroy the time difference.
    
    if (sphere_mode && target_mode == 0.0)
        target_mode = 1.0;
}


void QuakeApp::InitOpenGL() {
    // Set up the camera in a good position to see the entire earth in either mode
    proj_matrix_ = Matrix4::Perspective(60, aspect_ratio(), 0.1, 50);
    view_matrix_ = Matrix4::LookAt(Point3(0,0,3.5), Point3(0,0,0), Vector3(0,1,0));
    glClearColor(0.0, 0.0, 0.0, 1);
    
    // Initialize the earth object
    earth_.Init(search_path_);

    // Initialize the texture used for the background image
    stars_tex_.InitFromFile(Platform::FindFile("iss006e40544.png", search_path_));
}


void QuakeApp::DrawUsingOpenGL() {
    quick_shapes_.DrawFullscreenTexture(Color(1,1,1), stars_tex_);
    
    // You can leave this as the identity matrix and we will have a fine view of
    // the earth.  If you want to add any rotation or other animation of the
    // earth, the model_matrix is where you would apply that.
    Matrix4 model_matrix;
    
    // Draw the earth
    earth_.Draw(model_matrix * Matrix4::RotationY(rotation), view_matrix_, proj_matrix_);
    if (debug_mode_) {
        earth_.DrawDebugInfo(model_matrix * Matrix4::RotationY(rotation), view_matrix_, proj_matrix_);
    }

    Date d(current_time_);
    
    
    // get the earthquakes data from the data base
    
    earthquakes.clear();
    int e = quake_db_.FindMostRecentQuake(current_time_);
    
    while (e >= quake_db_.min_index() && d.SecondsUntil(quake_db_.earthquake(e).date()) <= PLAYBACK_WINDOW) {
        earthquakes.push_back(e);
        e--;
    }
    
    
    // draw each earthquake that happens in the past one year
    
    for (int i = 0; i < earthquakes.size(); i++) {
        
        Earthquake earthquake = quake_db_.earthquake(earthquakes.at(i));
        
        
        // determine the position of the earthquake
        float lat = (float) earthquake.latitude();
        float lon = (float) earthquake.longitude();
        Point3 position = earth_.LatLongToPlane(lat, lon).Lerp(earth_.LatLongToSphere(lat, lon), scalar_parameter);

        
        // determine the state of the earthquake, e.g. color, size, and fading factor
        
        float mag_factor = (float) (earthquake.magnitude() - quake_db_.min_magnitude());
        mag_factor = mag_factor * mag_factor / quake_db_.max_magnitude();
        
        Color earthquake_color = Color(1, 1.0 - mag_factor, 0);
        float earthquake_size = - mag_factor * 0.05 * fabs((float) d.SecondsUntil(earthquake.date()) / PLAYBACK_WINDOW);
        
        if (mag_factor >= 0.4)
            earthquake_size += 0.04;
        
        else
            earthquake_size += 0.022;
        
        Matrix4 mEarthquake = model_matrix * Matrix4::RotationY(rotation) *
                Matrix4::Translation(Vector3(position.x(), position.y(), position.z())) *
                Matrix4::Scale(Vector3(earthquake_size, earthquake_size, earthquake_size));
        
        quick_shapes_.DrawSphere(mEarthquake, view_matrix_, proj_matrix_, earthquake_color);
    }
}
