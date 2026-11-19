// ROS 2 wrapper around MissionLogic

// Serves: inspect_shelves, get_report, set_focus_threshold
// Calls:  navigate_to_pose
// Subscribes: detections, focus_score
// Publishes: inspection_results, run_events, state

// all runs on single-threaded executor, so callbacks never run at the same time

#include <algorithm>
#include <chrono>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "inspector_interfaces/action/inspect_shelves.hpp"
#include "inspector_interfaces/msg/focus_score.hpp"
#include "inspector_interfaces/msg/inspection_result.hpp"
#include "inspector_interfaces/msg/marker_detection_array.hpp"
#include "inspector_interfaces/msg/run_event.hpp"
#include "inspector_interfaces/srv/get_inspection_report.hpp"
#include "inspector_interfaces/srv/set_focus_threshold.hpp"
#include "inspector_mission/mission_logic.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "std_msgs/msg/string.hpp"
#include "tf2/exceptions.h"
#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"

namespace inspector_mission
{

using InspectShelves = inspector_interfaces::action::InspectShelves;
using InspectGoalHandle = rclcpp_action::ServerGoalHandle<InspectShelves>;
using NavigateToPose = nav2_msgs::action::NavigateToPose;
using NavGoalHandle = rclcpp_action::ClientGoalHandle<NavigateToPose>;
using inspector_interfaces::msg::InspectionResult;
using inspector_interfaces::msg::RunEvent;

class MissionNode : public rclcpp::Node
{
public:
  MissionNode()
  : Node("mission_node")
  {
    MissionConfig config;
    config.inspect_timeout_s = declare_parameter<double>("inspect_timeout_s", 5.0);
    config.min_detections = static_cast<int>(declare_parameter<int>("min_detections", 3));
    config.focus_threshold = declare_parameter<double>("focus_threshold", 100.0);
    frame_id_ = declare_parameter<std::string>("frame_id", "map");
    const auto waypoints_file = declare_parameter<std::string>("waypoints_file", "");
    logic_ = MissionLogic(config);

    if (waypoints_file.empty()) {
      RCLCPP_WARN(get_logger(), "No waypoints_file parameter: missions will be empty");
    } else {
      try {
        waypoints_ = loadWaypoints(waypoints_file);
        RCLCPP_INFO(get_logger(), "Loaded %zu waypoints", waypoints_.size());
      } catch (const std::exception & e) {
        RCLCPP_ERROR(get_logger(), "%s", e.what());
      }
    }

    tf_buffer_ = std::make_unique<tf2_ros::Buffer>(get_clock());
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

    state_pub_ = create_publisher<std_msgs::msg::String>(
      "~/state", rclcpp::QoS(1).transient_local());
    result_pub_ = create_publisher<InspectionResult>("/inspection_results", 10);
    event_pub_ = create_publisher<RunEvent>("/run_events", 10);

    detections_sub_ = create_subscription<inspector_interfaces::msg::MarkerDetectionArray>(
      "/detections", 10,
      [this](inspector_interfaces::msg::MarkerDetectionArray::ConstSharedPtr msg) {
        std::vector<int> ids;
        for (const auto & d : msg->detections) {
          ids.push_back(d.marker_id);
        }
        apply(logic_.onMarkersSeen(ids, nowSeconds()));
      });
    focus_sub_ = create_subscription<inspector_interfaces::msg::FocusScore>(
      "/focus_score", 10,
      [this](inspector_interfaces::msg::FocusScore::ConstSharedPtr msg) {
        apply(logic_.onFocusScore(msg->tenengrad, nowSeconds()));
      });

    nav_client_ = rclcpp_action::create_client<NavigateToPose>(this, "navigate_to_pose");

    action_server_ = rclcpp_action::create_server<InspectShelves>(
      this, "/inspect_shelves",
      [this](const rclcpp_action::GoalUUID &, std::shared_ptr<const InspectShelves::Goal> goal) {
        return onGoalRequest(goal);
      },
      [this](std::shared_ptr<InspectGoalHandle>) {
        cancel_requested_ = true;
        return rclcpp_action::CancelResponse::ACCEPT;
      },
      [this](std::shared_ptr<InspectGoalHandle> handle) {onGoalAccepted(handle);});

    report_srv_ = create_service<inspector_interfaces::srv::GetInspectionReport>(
      "~/get_report",
      [this](
        const std::shared_ptr<inspector_interfaces::srv::GetInspectionReport::Request>,
        std::shared_ptr<inspector_interfaces::srv::GetInspectionReport::Response> response) {
        response->run_id = run_id_;
        fillResults(response->results, response->markers_ok, response->markers_failed);
      });

    threshold_srv_ = create_service<inspector_interfaces::srv::SetFocusThreshold>(
      "~/set_focus_threshold",
      [this](
        const std::shared_ptr<inspector_interfaces::srv::SetFocusThreshold::Request> request,
        std::shared_ptr<inspector_interfaces::srv::SetFocusThreshold::Response> response) {
        response->previous_threshold = logic_.config().focus_threshold;
        logic_.setFocusThreshold(request->threshold);
        response->success = true;
        response->message = "focus threshold updated";
      });

    timer_ = create_wall_timer(std::chrono::milliseconds(100), [this]() {onTimer();});

    publishState();
  }

private:
  double nowSeconds() {return now().seconds();}

  rclcpp_action::GoalResponse onGoalRequest(std::shared_ptr<const InspectShelves::Goal> goal)
  {
    if (active_goal_) {
      RCLCPP_WARN(get_logger(), "Rejecting goal: a mission is already running");
      return rclcpp_action::GoalResponse::REJECT;
    }
    if (goal->laps == 0) {
      RCLCPP_WARN(get_logger(), "Rejecting goal: laps must be >= 1");
      return rclcpp_action::GoalResponse::REJECT;
    }
    if (goal->laps > 100) {
      RCLCPP_WARN(get_logger(), "Rejecting goal: laps must be between 1 - 100");
      return rclcpp_action::GoalResponse::REJECT;
    }
    for (const auto & name : goal->shelves) {
      bool found = std::any_of(
        waypoints_.begin(), waypoints_.end(),
        [&](const Waypoint & wp) {return wp.shelf == name;});
      if (!found) {
        RCLCPP_WARN(get_logger(), "Rejecting goal: unknown shelf '%s'", name.c_str());
        return rclcpp_action::GoalResponse::REJECT;
      }
    }
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }

  void onGoalAccepted(std::shared_ptr<InspectGoalHandle> handle)
  {
    active_goal_ = handle;
    cancel_requested_ = false;
    published_.clear();
    run_id_ = "run-" + std::to_string(rclcpp::Clock(RCL_SYSTEM_TIME).now().nanoseconds());
    started_at_s_ = nowSeconds();

    const auto goal = handle->get_goal();
    const auto route = buildRoute(waypoints_, goal->shelves, goal->laps);

    event(RunEvent::INFO, "MISSION_STARTED", "Mission " + run_id_ + " started");
    apply(logic_.start(route, nowSeconds()));
  }

  std::vector<Waypoint> buildRoute(
    const std::vector<Waypoint> & all,
    const std::vector<std::string> & shelves,
    uint32_t laps)
  {
    std::vector<Waypoint> one_lap;
    if (shelves.empty()) {
      one_lap = all;
    } else {
      for (const auto & name : shelves) {
        for (const auto & wp : all) {
          if (wp.shelf == name) {one_lap.push_back(wp);}
        }
      }
    }
    std::vector<Waypoint> route;
    for (uint32_t i = 0; i < laps; i++) {route.insert(route.end(), one_lap.begin(), one_lap.end());}
    return route;
  }

  void onTimer()
  {
    if (!active_goal_) {
      return;
    }
    if (cancel_requested_ || active_goal_->is_canceling()) {
      if (nav_goal_) {
        nav_client_->async_cancel_goal(nav_goal_);
      }
      apply(logic_.cancel());
      return;
    }
    apply(logic_.onTick(nowSeconds()));
    publishFeedback();
  }

  // logic decisions -> ROS actions
  void apply(const Command & command)
  {
    if (command.result) {
      publishResult(*command.result);
    }
    if (command.goal) {
      sendNavGoal(*command.goal);
    }
    if (command.finished) {
      finishMission();
    }
    publishState();
  }

  void sendNavGoal(const Waypoint & waypoint)
  {
    if (!nav_client_->wait_for_action_server(std::chrono::seconds(2))) {
      RCLCPP_ERROR(get_logger(), "navigate_to_pose not available. Check if Nav2 is running");
      apply(logic_.onNavFailed(nowSeconds()));
      return;
    }

    NavigateToPose::Goal goal;
    goal.pose.header.frame_id = frame_id_;
    goal.pose.header.stamp = now();
    goal.pose.pose.position.x = waypoint.x;
    goal.pose.pose.position.y = waypoint.y;

    goal.pose.pose.orientation.z = std::sin(waypoint.yaw / 2.0);
    goal.pose.pose.orientation.w = std::cos(waypoint.yaw / 2.0);

    rclcpp_action::Client<NavigateToPose>::SendGoalOptions options;
    options.goal_response_callback = [this](NavGoalHandle::SharedPtr handle) {
        nav_goal_ = handle;
        if (!handle) {
          apply(logic_.onNavFailed(nowSeconds()));
        }
      };
    options.feedback_callback =
      [this](NavGoalHandle::SharedPtr, const std::shared_ptr<const NavigateToPose::Feedback> fb) {
        distance_remaining_m_ = fb->distance_remaining;
      };
    options.result_callback = [this](const NavGoalHandle::WrappedResult & result) {
        nav_goal_.reset();
        if (result.code == rclcpp_action::ResultCode::SUCCEEDED) {
          event(RunEvent::INFO, "WAYPOINT_REACHED", "Reached waypoint");
          apply(logic_.onNavSucceeded(nowSeconds()));
        } else if (result.code != rclcpp_action::ResultCode::CANCELED) {
          event(RunEvent::WARN, "NAV_FAILED", "Nav2 did not reach the waypoint");
          apply(logic_.onNavFailed(nowSeconds()));
        }
      };

    RCLCPP_INFO(
      get_logger(), "Navigating to %s (%.2f, %.2f)", waypoint.shelf.c_str(), waypoint.x,
      waypoint.y);
    nav_client_->async_send_goal(goal, options);
  }

  InspectionResult toMsg(const Result & r)
  {
    InspectionResult msg;
    msg.header.stamp = now();
    msg.header.frame_id = frame_id_;
    msg.shelf = r.waypoint.shelf;
    msg.expected_marker_id = r.waypoint.marker_id;
    msg.observed_marker_id = r.observed_marker_id;
    msg.status = static_cast<std::uint8_t>(r.status);
    msg.focus_score = r.best_focus;
    msg.dwell_time_s = r.dwell_time_s;

    try {
      const auto tf = tf_buffer_->lookupTransform("map", "base_footprint", tf2::TimePointZero);
      msg.robot_pose.position.x = tf.transform.translation.x;
      msg.robot_pose.position.y = tf.transform.translation.y;
      msg.robot_pose.position.z = tf.transform.translation.z;
      msg.robot_pose.orientation = tf.transform.rotation;
    } catch(const tf2::TransformException & e) {
      RCLCPP_WARN(get_logger(), "no map -> base_footprint: %s", e.what());
    }

    // fix marker_position_error_m once the detector gives pose
    msg.marker_position_error_m = -1.0;
    return msg;
  }

  void publishResult(const Result & r)
  {
    auto msg = toMsg(r);
    result_pub_->publish(msg);
    published_.push_back(msg);
    const bool ok = r.status == ResultStatus::OK;
    event(
      ok ? RunEvent::INFO : RunEvent::WARN, ok ? "MARKER_OK" : "MARKER_FAILED",
      r.waypoint.shelf + " marker " + std::to_string(r.waypoint.marker_id));
  }

  void fillResults(
    std::vector<InspectionResult> & out, std::uint32_t & ok, std::uint32_t & failed)
  {
    ok = 0;
    failed = 0;
    for (const auto & msg : published_) {
      out.push_back(msg);
      (msg.status == InspectionResult::OK ? ok : failed)++;
    }
  }

  void finishMission()
  {
    if (!active_goal_) {
      return;
    }
    auto result = std::make_shared<InspectShelves::Result>();
    result->run_id = run_id_;
    result->duration_s = nowSeconds() - started_at_s_;
    fillResults(result->results, result->markers_ok, result->markers_failed);

    if (logic_.state() == State::ABORTED) {
      active_goal_->canceled(result);
      event(RunEvent::WARN, "MISSION_CANCELED", "Mission " + run_id_ + " canceled");
    } else {
      active_goal_->succeed(result);
      event(RunEvent::INFO, "MISSION_DONE", "Mission " + run_id_ + " finished");
    }
    active_goal_.reset();
  }

  void publishFeedback()
  {
    auto feedback = std::make_shared<InspectShelves::Feedback>();
    feedback->state = toString(logic_.state());
    feedback->waypoint_index = static_cast<std::uint32_t>(logic_.index());
    feedback->waypoint_count = static_cast<std::uint32_t>(logic_.size());
    feedback->distance_remaining_m = distance_remaining_m_;
    feedback->current_shelf = logic_.getCurrentShelf();
    active_goal_->publish_feedback(feedback);
  }

  void publishState()
  {
    std_msgs::msg::String msg;
    msg.data = toString(logic_.state());
    if (msg.data != last_state_) {
      last_state_ = msg.data;
      state_pub_->publish(msg);
    }
  }

  void event(std::uint8_t severity, const std::string & code, const std::string & text)
  {
    RunEvent msg;
    msg.header.stamp = now();
    msg.severity = severity;
    msg.source = get_name();
    msg.code = code;
    msg.message = text;
    event_pub_->publish(msg);
  }

  MissionLogic logic_;
  std::vector<Waypoint> waypoints_;
  std::string frame_id_;
  std::string run_id_;
  std::string last_state_;
  double started_at_s_{0.0};
  double distance_remaining_m_{0.0};
  bool cancel_requested_{false};

  std::shared_ptr<InspectGoalHandle> active_goal_;
  std::vector<InspectionResult> published_;
  NavGoalHandle::SharedPtr nav_goal_;

  std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;

  rclcpp_action::Server<InspectShelves>::SharedPtr action_server_;
  rclcpp_action::Client<NavigateToPose>::SharedPtr nav_client_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr state_pub_;
  rclcpp::Publisher<InspectionResult>::SharedPtr result_pub_;
  rclcpp::Publisher<RunEvent>::SharedPtr event_pub_;
  rclcpp::Subscription<inspector_interfaces::msg::MarkerDetectionArray>::SharedPtr
    detections_sub_;
  rclcpp::Subscription<inspector_interfaces::msg::FocusScore>::SharedPtr focus_sub_;
  rclcpp::Service<inspector_interfaces::srv::GetInspectionReport>::SharedPtr report_srv_;
  rclcpp::Service<inspector_interfaces::srv::SetFocusThreshold>::SharedPtr threshold_srv_;
  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace inspector_mission

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<inspector_mission::MissionNode>());
  rclcpp::shutdown();
  return 0;
}
