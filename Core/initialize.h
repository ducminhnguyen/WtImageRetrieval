#ifndef INITIALIZE_H_INCLUDED
#define INITIALIZE_H_INCLUDED


#include "configurations.h"
#include "app/appdata.h"
#include "extract/extract.h"
#include "quantize/quantize.h"
#include <iostream>
using namespace std;

void extractAll() {

    AppData *app = AppData::getInstance();

//    Get list of image files
    DIR *dir = opendir(dataFolder.c_str());
    while (dirent *pdir = readdir(dir)) {
        string fName = pdir->d_name;
        if (fName[0] == '.') continue;

        app->path.push_back(fName);
    }

    debugInfo("Extracting features");

//    Extract features
    app->path.shrink_to_fit();

    app->kp.reserve(app->path.size());
    app->sift.reserve(app->path.size());

    boost::filesystem::create_directories(kpFolder);
    boost::filesystem::create_directories(siftFolder);
    boost::filesystem::create_directories(tempFolder);
    for (string imgPath : app->path) {

//        debugVar(imgPath);

        string tmp = imgPath;
        tmp.replace(tmp.size() - 3, 3, "mat");

        string kpPath = kpFolder + "/" + tmp;
        string siftPath = siftFolder + "/" + tmp;
        string tempPath = tempFolder + "/" + tmp;
        imgPath = dataFolder + "/" + imgPath;

//        debugVar(kpPath);
//        debugVar(siftPath);
        mat _kp, _sift;
        extractFeatures(imgPath, _kp, _sift, kpPath, siftPath, tempPath);

        app->kp.push_back(_kp);
        app->sift.push_back(_sift);
    }
}

void quantizeAllData() {

    AppData *app = AppData::getInstance();

    debugInfo("Quantizing images");

    buildIndex(true);
    debugInfo("Done build index");
    app->weights.reserve(app->path.size());
    app->termID.reserve(app->path.size());
    boost::filesystem::create_directories(weightFolder);
    boost::filesystem::create_directories(termIDFolder);
    int nDocs = app->path.size();
    cout << "For loop quantize" <<endl;
    for (int i = 0; i < nDocs; i++) {
        cout << "Head of loop" << endl;
        string tmp = app->path[i];
        tmp.replace(tmp.size() - 3, 3, "mat");

        string weightPath = weightFolder + "/" + tmp;
        string termIDPath = termIDFolder + "/" + tmp;

//        debugVar(i);
//        debugVar(weightPath);
//        debugVar(termIDPath);

        vec _weights;
        uvec _termID;

        buildBoW(app->sift[i], _weights, _termID, weightPath, termIDPath, false);
        cout << "Done building BOW" << endl;
//        Insert to inverted index
        app->ivt.add(_weights, _termID, i);

        app->weights.push_back(_weights);
        app->termID.push_back(_termID);
    }

//    Build TFIDF
    app->ivt.buildTfidf();
}

inline void extractAndQuantizeAll() {

    AppData *app = AppData::getInstance();

//    Get list of image files
    DIR *dir = opendir(dataFolder.c_str());
    while (dirent *pdir = readdir(dir)) {
        string fName = pdir->d_name;
        if (fName[0] == '.') continue;

        app->path.push_back(fName);
    }

    debugInfo("Extracting features");

//    Extract features
    app->path.shrink_to_fit();
    // app->kp.reserve(app->path.size());
    // app->sift.reserve(app->path.size());

    buildIndex(false);

    for (int i = 0; i < app->path.size(); i++) {
        string imgPath = app->path[i];

        debugVar(imgPath);

        string tmp = imgPath;
        tmp.replace(tmp.size() - 3, 3, "mat");

        string kpPath = kpFolder + "/" + tmp;
        string siftPath = siftFolder + "/" + tmp;
        string tempPath = tempFolder + "/" + tmp;
        string weightPath = weightFolder + "/" + tmp;
        string termIDPath = termIDFolder + "/" + tmp;

        imgPath = dataFolder + "/" + imgPath;

        debugVar(imgPath);
        debugVar(kpPath);
        debugVar(siftPath);
        debugVar(weightPath);
        debugVar(termIDPath);

        mat _kp, _sift;
        extractFeatures(imgPath, _kp, _sift, kpPath, siftPath, tempPath, false);

        app->kp.push_back(_kp);

        vec _weights;
        uvec _termID;

        buildBoW(_sift, _weights, _termID, weightPath, termIDPath, false);

//        Insert to inverted index
        app->ivt.add(_weights, _termID, i);
    }

    //    Build TFIDF
    app->ivt.buildTfidf();
}

//inline void extractAndQuantize()
//{
//    AppData *app = AppData::getInstance();

////    Get list of image files
//    DIR *dir = opendir(dataFolder.c_str());
//    while (dirent *pdir = readdir(dir)) {
//        string fName = pdir->d_name;
//        if (fName[0] == '.') continue;

//        app->path.push_back(fName);
//    }

//    debugInfo("Extracting features");

////    Extract features
//    app->path.shrink_to_fit();

//    app->kp.reserve(app->path.size());
//    app->sift.reserve(app->path.size());
//    app->weights.reserve(app->path.size());
//    app->termID.reserve(app->path.size());

//    int nDocs = app->path.size();

//    boost::filesystem::create_directories(kpFolder);
//    boost::filesystem::create_directories(siftFolder);
//    boost::filesystem::create_directories(tempFolder);
//    for (string imgPath : app->path) {
//        string tmp = imgPath;
//        tmp.replace(tmp.size() - 3, 3, "mat");

//        string kpPath = kpFolder + "/" + tmp;
//        string siftPath = siftFolder + "/" + tmp;
//        string tempPath = tempFolder + "/" + tmp;
//        string weightPath = weightFolder + "/" + tmp;
//        string termIDPath = termIDFolder + "/" + tmp;

//        imgPath = dataFolder + "/" + imgPath;

//        mat _kp, _sift;
//        extractFeatures(imgPath, _kp, _sift, kpPath, siftPath, tempPath);
//        vec _weights;
//        uvec _termID;
//        buildBoW(app->sift[i], _weights, _termID, weightPath, termIDPath, false);


//        app->kp.push_back(_kp);
//        app->ivt.add(_weights, _termID, i);
//        app->termID.push_back(_termID);
//        app->weights.push_back(_weights);
//    }
//}
//inline void loadSimpleData()
//{
//    // simple data contains: keypoint, bow representation, database aug

//}

#endif
