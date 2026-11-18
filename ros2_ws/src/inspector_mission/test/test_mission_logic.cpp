// Unit tests for the mission state machine. No ROS, no Nav2: we play the
// node's part by calling the event methods and checking the Commands.

#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "inspector_mission/mission_logic.hpp"

using inspector_mission::loadWaypoints;
using inspector_mission::MissionLogic;
using inspector_mission::ResultStatus;
using inspector_mission::State;
using inspector_mission::Waypoint;

namespace
{

std::vector<Waypoint> twoWaypoints()
{
  return {
    Waypoint{"shelf_A1", "aisle", 0, 2.4, 0.1, 1.57},
    Waypoint{"shelf_A2", "aisle", 1, 4.8, 0.1, 1.57},
  };
}

}  // namespace

TEST(Waypoints, LoadsYamlList)
{
  const std::string path = "/tmp/test_waypoints.yaml";
  {
    std::ofstream f(path);
    f << "waypoints:\n"
      << "  - {shelf: shelf_A1, side: aisle, marker_id: 0, x: 1.0, y: 2.0, yaw: 0.5}\n";
  }
  const auto wps = loadWaypoints(path);
  ASSERT_EQ(wps.size(), 1u);
  EXPECT_EQ(wps[0].shelf, "shelf_A1");
  EXPECT_EQ(wps[0].marker_id, 0);
  EXPECT_DOUBLE_EQ(wps[0].y, 2.0);
  std::remove(path.c_str());
}

TEST(Waypoints, MissingFileThrows)
{
  EXPECT_THROW(loadWaypoints("/nonexistent/waypoints.yaml"), std::runtime_error);
}

TEST(Mission, StartsIdle)
{
  MissionLogic logic;
  EXPECT_EQ(logic.state(), State::IDLE);
}

TEST(Mission, CancelAborts)
{
  MissionLogic logic;
  const auto cmd = logic.cancel();
  EXPECT_TRUE(cmd.finished);
  EXPECT_EQ(logic.state(), State::ABORTED);
}

TEST(Mission, StartSendsFirstGoal)
{
  MissionLogic logic;
  const auto cmd = logic.start(twoWaypoints(), 0.0);
  ASSERT_TRUE(cmd.goal.has_value());
  EXPECT_EQ(cmd.goal->shelf, "shelf_A1");
  EXPECT_EQ(logic.state(), State::NAVIGATING);
}

TEST(Mission, SeenAndSharpGivesOk)
{
  MissionLogic logic;
  logic.start(twoWaypoints(), 0.0);
  logic.onNavSucceeded(1.0);
  logic.onFocusScore(1000.0, 1.1);
  inspector_mission::Command cmd;
  for (int i = 0; i < logic.config().min_detections; ++i) {
    cmd = logic.onMarkersSeen({0}, 1.2 + 0.1 * i);
  }
  ASSERT_TRUE(cmd.result.has_value());
  EXPECT_EQ(cmd.result->status, ResultStatus::OK);
  ASSERT_TRUE(cmd.goal.has_value());
  EXPECT_EQ(cmd.goal->shelf, "shelf_A2");
}

TEST(Mission, TimeoutGivesMissing)
{
  MissionLogic logic;
  logic.start(twoWaypoints(), 0.0);
  logic.onNavSucceeded(1.0);
  const auto cmd = logic.onTick(1.0 + logic.config().inspect_timeout_s + 0.1);
  ASSERT_TRUE(cmd.result.has_value());
  EXPECT_EQ(cmd.result->status, ResultStatus::MISSING);
}

TEST(Mission, NavFailureIsRecordedAndSkipped)
{
  MissionLogic logic;
  logic.start(twoWaypoints(), 0.0);
  const auto cmd = logic.onNavFailed(3.0);
  ASSERT_TRUE(cmd.result.has_value());
  EXPECT_EQ(cmd.result->status, ResultStatus::NAV_FAILED);
  ASSERT_TRUE(cmd.goal.has_value());
}

TEST(Mission, LastWaypointFinishes)
{
  MissionLogic logic;
  logic.start(twoWaypoints(), 0.0);

  auto cmd = logic.onNavFailed(1.0);
  ASSERT_TRUE(cmd.goal.has_value());
  EXPECT_FALSE(cmd.finished);

  cmd = logic.onNavFailed(2.0);
  ASSERT_TRUE(cmd.result.has_value());
  EXPECT_FALSE(cmd.goal.has_value());
  EXPECT_TRUE(cmd.finished);
  EXPECT_EQ(logic.state(), State::DONE);
  EXPECT_EQ(logic.results().size(), 2u);
}

TEST(Mission, EmptyRouteFinishes)
{
  MissionLogic logic;
  auto cmd = logic.start({}, 0.0);

  EXPECT_TRUE(cmd.finished);
  EXPECT_FALSE(cmd.goal.has_value());
  EXPECT_EQ(logic.state(), State::DONE);
  EXPECT_TRUE(logic.results().empty());
}
