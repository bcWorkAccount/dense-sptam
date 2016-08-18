#ifndef __PROJECTIONTHREAD_H
#define __PROJECTIONTHREAD_H

#include <thread>
#include <pcl_ros/point_cloud.h>

#include "Camera.hpp"
#include "DispImageQueue.hpp"
#include "PointCloudQueue.hpp"

class ProjectionThread
{
public:

    ProjectionThread(DispImageQueue *disp_images, PointCloudQueue *point_clouds, Camera::Ptr camera);

    inline void WaitUntilFinished()
    { projectionThread_.join(); }

private:

    DispImageQueue *disp_images_;
    PointCloudQueue *point_clouds_;
    Camera::Ptr camera_;

    std::thread projectionThread_;
    void compute();

    void filterDisp(const DispRawImagePtr disp_raw_img, float min_disparity);
    MatVec3fPtr processPoints(const DispRawImagePtr disp_raw_img);
};

#endif /* __PROJECTIONTHREAD_H */
