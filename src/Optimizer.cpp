//
// Created by rain on 17-11-17.
//

#include "Optimizer.h"

namespace RAIN_VIO
{

void Optimizer::PoseOptimization(int IdxWin, Frame *pFrame, Map *pMap)
{
    ceres::Problem problem;
    ceres::LocalParameterization* localParameterization = new ceres::QuaternionParameterization();

    double Rdcw[4];
    double tdcw[4];
    double Point3dw[3];

    Rdcw[0] = Eigen::Quaterniond(pFrame->GetRotation()).w();
    Rdcw[1] = Eigen::Quaterniond(pFrame->GetRotation()).x();
    Rdcw[2] = Eigen::Quaterniond(pFrame->GetRotation()).y();
    Rdcw[3] = Eigen::Quaterniond(pFrame->GetRotation()).z();

    tdcw[0] = pFrame->GetTranslation()[0];
    tdcw[1] = pFrame->GetTranslation()[1];
    tdcw[2] = pFrame->GetTranslation()[2];

    problem.AddParameterBlock(Rdcw, 4, localParameterization);
    problem.AddParameterBlock(tdcw, 3);

    for (auto &MapPoint : pMap->mlMapPoints)
    {
        if (!(MapPoint.mdEstimatedDepth > 0))
            continue;

        if (!(MapPoint.mnUsedNum >= 2 && MapPoint.mnStartFrame < gWindowSize-1 && MapPoint.mnStartFrame < IdxWin))
        {
            FeaturePerFrame featurePerFrame = MapPoint.mvFeaturePerFrame.at((size_t)(IdxWin - MapPoint.mnStartFrame));

            ceres::CostFunction* costFunction = ReprojectionError2::Create(featurePerFrame.Point[0], featurePerFrame.Point[1]);

            Point3dw[0] = MapPoint.mvFeaturePerFrame.at((size_t)MapPoint.mnStartFrame).Point[0];
            Point3dw[1] = MapPoint.mvFeaturePerFrame.at((size_t)MapPoint.mnStartFrame).Point[1];
            Point3dw[2] = MapPoint.mvFeaturePerFrame.at((size_t)MapPoint.mnStartFrame).Point[2];

            problem.AddResidualBlock(costFunction, nullptr, Rdcw, tdcw, Point3dw);
        }
    }

    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_SCHUR;
    options.minimizer_progress_to_stdout = true;
    options.max_solver_time_in_seconds = 0.3;
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);

    LOG(INFO) << summary.BriefReport() << endl;
}

} // namespace RAIN_VIO
