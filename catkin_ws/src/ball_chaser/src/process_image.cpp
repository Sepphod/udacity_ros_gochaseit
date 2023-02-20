#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>
#include <iterator>
#include <algorithm>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    ROS_INFO_STREAM("robot moves");

    // TODO: Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;

    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the command_robot service and pass the requested motor commands
    if (!client.call(srv)) {
        ROS_ERROR("Failed to call command_robot service");
    }
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    constexpr unsigned char whitePixel{255};

    auto is_white = [](unsigned char colour){return colour == whitePixel;};

    auto result = std::find_if(std::begin(img.data),std::end(img.data),is_white);

    if (std::end(img.data) != result) {
        size_t index = std::distance(std::begin(img.data), result);
        auto lineNo = static_cast<int>(index) / img.width;
        auto lineIndex = static_cast<int>(index) - lineNo * img.width;

        constexpr int numOfRanges{3};

        if (lineIndex < img.width/numOfRanges) {
            ROS_INFO_STREAM("robot needs to move left");
            drive_robot(0.1f, 0.5f);
        } else if (lineIndex >= (img.width - img.width/numOfRanges) ) {
            ROS_INFO_STREAM("robot needs to move right");
            drive_robot(0.1f, -0.5f);
        } else {
            ROS_INFO_STREAM("robot needs to move mid");
            drive_robot(0.1f, 0.f);
        }
    } else {
        ROS_INFO_STREAM("robot needs to stop");
        drive_robot(0.f, 0.f);

    }

}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}