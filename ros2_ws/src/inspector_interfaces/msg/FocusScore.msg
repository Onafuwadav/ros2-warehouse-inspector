# Image sharpness for camera frame, from focus_node
std_msgs/Header header

# Tenengrad score: mean of squared Sobel gradient magnitude over region of interest(Bigger = sharper)
float64 tenengrad

float64 threshold

# tenengrad >= threshold
bool in_focus

# Region the score was computed on(width == 0 means whole image)
uint32 roi_x
uint32 roi_y
uint32 roi_width
uint32 roi_height
