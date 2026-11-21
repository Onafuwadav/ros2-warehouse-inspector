# ArUco marker seen in camera frame

# ArUco id (0-49); warehouse uses 0-15
int32 marker_id

# Marker pose in the camera frame 
geometry_msgs/Pose pose

# corners in image pixels, clockwise from top-left: x0,y0,x1,y1x2,y2,x3,y3
float32[8] corners_px

# Distance(m): camera -> marker centre 
float32 distance_m

# Mean reprojection error of the pose fit(lower = better)
float32 reprojection_error_px
