# Every marker found in one camera frame header.stamp(img stamp), so nodes can match detection to robot pose at time photo taken
std_msgs/Header header
MarkerDetection[] detections

# How long detection took
float32 processing_time_ms