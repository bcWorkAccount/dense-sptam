#include "ProjectionThread.hpp"

#include <cv_bridge/cv_bridge.h>

ProjectionThread::ProjectionThread(
    DispImageQueue *disp_images, PointCloudQueue *point_clouds, Camera *camera
) : disp_images_(disp_images)
  , point_clouds_(point_clouds)
  , camera_(camera)
  , projectionThread_(&ProjectionThread::compute, this)
{}

void ProjectionThread::compute()
{
    while(1) {
        /* Calls to pop() are blocking */
        DispRawImagePtr disp_raw_img = disp_images_->pop();
        filterDisp(disp_raw_img, 30);
        PointCloudPtr cloud = processPoints(disp_raw_img);
        PointCloudEntry::Ptr entry = point_clouds_->getEntry(disp_raw_img->first->header.seq);

        entry->lock();
        entry->set_cloud(cloud);
        point_clouds_->schedule(entry);
        entry->unlock();
    }
}

bool ProjectionThread::isValidPoint(const cv::Vec3f& pt)
{
    // Check both for disparities explicitly marked as invalid (where OpenCV maps pt.z to MISSING_Z)
    // and zero disparities (point mapped to infinity).
    return pt[2] != image_geometry::StereoCameraModel::MISSING_Z && !isinf(pt[2]);
}

void ProjectionThread::filterDisp(const DispRawImagePtr disp_raw_img, float min_disparity)
{
    ImagePtr raw_left_image = disp_raw_img->first;
    DispImagePtr disp_img = disp_raw_img->second;

    for (unsigned int i = 0; i < raw_left_image->height; i++)
        for (unsigned int j = 0; j < raw_left_image->width; j++)
            if (disp_img->at<float>(i, j) < min_disparity)
                disp_img->at<float>(i, j) = 0;
}


PointCloudPtr ProjectionThread::processPoints(
    const DispRawImagePtr disp_raw_img
) {
    PointCloudPtr cloud(new PointCloud);
    ImagePtr raw_left_image = disp_raw_img->first;
    DispImagePtr disp_img = disp_raw_img->second;

    cv::Mat image_left(cv_bridge::toCvCopy(raw_left_image, sensor_msgs::image_encodings::TYPE_8UC1)->image);

    cv::Mat_<cv::Vec3f> dense_points_;
    pcl::PointXYZRGB new_pt3d;

    camera_->getStereoModel().projectDisparityImageTo3d(*disp_img, dense_points_, true);

    for (int32_t u = 0; u < dense_points_.rows; ++u)
        for (int32_t v = 0; v < dense_points_.cols; ++v)
            if (isValidPoint(dense_points_(u,v))) {
                memcpy(&new_pt3d.x, &dense_points_(u,v)[0], sizeof (float));
                memcpy(&new_pt3d.y, &dense_points_(u,v)[1], sizeof (float));
                memcpy(&new_pt3d.z, &dense_points_(u,v)[2], sizeof (float));
                uint8_t g = image_left.at<uint8_t>(u,v);
                int32_t rgb = (g << 16) | (g << 8) | g;
                memcpy(&new_pt3d.rgb, &rgb, sizeof (int32_t));

                cloud->push_back(new_pt3d);
            }

    return cloud;
}