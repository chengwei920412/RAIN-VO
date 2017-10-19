//
// Created by rain on 17-9-19.
//

#include "Tracking.h"

#include "TicToc.h"


namespace RAIN_VIO
{

Tracking::Tracking()
{
}
Tracking::Tracking(const string &strSettingsFile, int nWindowSize)
{
    mstrSettingsFile = strSettingsFile;

    EQUALIZE = true;
    mbFirstImage = true;
    mFirstImageTime = 0;
    mIDcnt = 0;
    mdFrameCount = 0;
    etrackingState = NO_INITIALIZED;
    mnWindowSize = nWindowSize;

    mlpFrames.clear();

    mpcamera = new Camera(strSettingsFile);

    cv::FileStorage fsSettings(strSettingsFile.c_str(), cv::FileStorage::READ);

    if (!fsSettings.isOpened())
    {
        cerr << "Failed to open settings file at " << strSettingsFile << endl;
        exit(-1);
    }

    CmaeraK = cv::Mat::eye(3, 3, CV_32F);
    CmaeraK.at<float>(0, 0) = fsSettings["Camera.fx"];
    CmaeraK.at<float>(1, 1) = fsSettings["Camera.fy"];
    CmaeraK.at<float>(0, 2) = fsSettings["Camera.cx"];
    CmaeraK.at<float>(1, 2) = fsSettings["Camera.cy"];

    DistCoef = cv::Mat::zeros(4, 1, CV_32F);
    DistCoef.at<float>(0) = fsSettings["Camera.k1"];
    DistCoef.at<float>(1) = fsSettings["Camera.k2"];
    DistCoef.at<float>(2) = fsSettings["Camera.p1"];
    DistCoef.at<float>(3) = fsSettings["Camera.p2"];

    ImageHeight = fsSettings["Camera.height"];
    ImageWidth = fsSettings["Camera.width"];

    ImageGridHeight = fsSettings["ImageGridHeight"];
    ImageGridWidth = fsSettings["ImageGridWidth"];

    mnumFeatures = fsSettings["ORBextractor.numFeatures"];
    minDist = fsSettings["ORBextractor.minDist"];

    mCurrentFrame = new Frame(mstrSettingsFile, nWindowSize);
    mpMap = new Map(nWindowSize);
    mpinitializer = new Initializer(CmaeraK, mpMap, nWindowSize);
}

Tracking::~Tracking()
{
}

void Tracking::Track(const cv::Mat &image, const double &TimeStamps)
{
    Eigen::Matrix3d RelativeR;
    Eigen::Vector3d RelativeT;
    int idx = 0;

    vector<pair<int, Eigen::Vector3d>> Features;

    mCurrentFrame->DetectKeyPoint(image, TimeStamps);

//    if (mlpFrames.size() > 1)
//    {
//        cv::drawMatches(mlpFrames.back()->mImageShow, mlpFrames.back()->mvFraPointsPts, mCurrentFrame->mImageShow, mCurrentFrame->mvFraPointsPts, );
//    }

    // the list of the map point
    mlpFrames.emplace_back(mCurrentFrame);

    // whether keyframe or not
    if (mpMap->AddFeatureCheckParallax(mdFrameCount, mCurrentFrame->mvFraFeatures))
    {
        // this is a keyframe
        eMarginflag = MARGINOLD;
    }
    else
        eMarginflag = MARGINSECONDNEW;


        if (etrackingState == NO_INITIALIZED)
        {
            if (mdFrameCount == mnWindowSize)
            {
                if (mpinitializer->RelativePose(RelativeR, RelativeT, idx))
                {
                    etrackingState = OK;
                    cout << "the initialize the rotation and translation" << endl;
                    cout << RelativeR << endl;
                    cout << RelativeT.transpose() << endl;
                    cout << idx << endl;
                }
                else
                {

                    SlideWindow();
                }

            }
            else
                mdFrameCount++;

        }
        else
        {
            SlideWindow();

//            for (size_t i = 0; i < mnWindowSize; i++)
//            {
//                vector<pair<Eigen::Vector3d, Eigen::Vector3d>> corres;
//                corres = mpMap->GetCorresponding(i, mnWindowSize);
//
//                cout << static_cast<int>(corres.size()) << endl;
//
//                corres.clear();
//            }
        }


//    for (size_t i = 0; i < mnWindowSize; i++)
//    {
//        vector<pair<Eigen::Vector3d, Eigen::Vector3d>> corres;
//        corres = mpMap->GetCorresponding(i, mnWindowSize);
//
//        cout << static_cast<int>(corres.size()) << endl;
//    }

    // checke if there are enough correnspondences
}

void Tracking::SlideWindow()
{
    if (eMarginflag == MARGINOLD)
    {
        if (mdFrameCount == mnWindowSize)
        {
            mpMap->RemoveBack();
        }

    }
    else
    {
        if (mdFrameCount == mnWindowSize)
        {
            mpMap->RemoveFront(mdFrameCount);
        }
    }
}


} // namespace RAIN_VIO
