#ifndef DENSE_H
#define DENSE_H

#include "../dense/Camera.hpp"
#include "../dense/DisparityCalcThread.hpp"
#include "../dense/ProjectionThread.hpp"
#include "../dense/RefinementThread.hpp"
#include "../dense/ImageQueue.hpp"

#define PIXEL_DEPTH_INVALID     -1

struct dense_parameters {
	std::string base_frame;
	std::string camera_frame;
	std::string map_frame;
	double frustum_near_plane_dist;
	double frustum_far_plane_dist;
	double voxel_leaf_size;
	double max_distance;
	double stereoscan_threshold;
	double sigma;
	std::string disp_calc_method;
	std::string fusion_heuristic;
	int local_area_size;
	int libelas_ipol_gap;
	bool add_corners;
	std::string single_cloud_path;
	std::string single_depth_map_clouds;
	std::string single_depth_map_poses;
	std::string single_depth_map_timestamps;
	std::string single_depth_map_mode;
	std::string output_dir;
	int single_depth_map_region_size;
	double refinement_linear_threshold;
	double refinement_angular_threshold;
	double pub_area_filter_min;
};

class Dense
{
public:

    Dense(const sensor_msgs::CameraInfoConstPtr& left_info, const sensor_msgs::CameraInfoConstPtr& right_info,
		  struct dense_parameters *parameters);
    ~Dense();

    void WriteToLog(const char* fmt, ...);

    const sensor_msgs::CameraInfoConstPtr left_info_, right_info_;
    double frustumNearPlaneDist_, frustumFarPlaneDist_, voxelLeafSize_;
    std::string output_dir_, disp_calc_method_;
    double max_distance_, stereoscan_threshold_;
    std::string fusion_heuristic_;
    int local_area_size_;
    int libelas_ipol_gap_;
    bool add_corners_;
    double sigma_;
    double refinement_linear_threshold_, refinement_angular_threshold_;

    Camera *camera_;
    ImageQueue *raw_image_pairs_;
    DispImageQueue  *disp_images_;
    PointCloudQueue *point_clouds_;

    ProjectionThread *projectionThread_;

private:

    DisparityCalcThread *disparityCalcThread;

    RefinementThread *refinementThread_;

    FILE *log_file_;
};

typedef boost::shared_ptr<Dense> DensePtr;

#endif /* DENSE_H */
