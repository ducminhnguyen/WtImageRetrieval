#include "imageresource.h"
#include <Wt/WApplication>
#include <Wt/Http/Request>
#include <Wt/Http/Response>
#include <string>
#include <Wt/Json/Parser>
#include <iostream>
#include <ostream>
#include <istream>
#include <streambuf>
#include <string>
#include "Core/configurations.h"
#include "Core/app/appdata.h"
#include "Core/extract/extract.h"
#include "Core/quantize/quantize.h"
#include "Core/query/query.h"
#include "Core/utils/score.h"
#include "Core/utils/utils.h"


using namespace Wt;
using namespace Wt::Http;
using namespace std;


ImageResource::ImageResource()
{

}

typedef long long long64; typedef unsigned long long ulong64;

/* Returns the amount of milliseconds elapsed since the UNIX epoch. Works on both
 * windows and linux. */

inline ulong64 GetTimeMs64()
{
#ifdef _WIN32
 /* Windows */
 FILETIME ft;
 LARGE_INTEGER li;

 /* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
  * to a LARGE_INTEGER structure. */
 GetSystemTimeAsFileTime(&ft);
 li.LowPart = ft.dwLowDateTime;
 li.HighPart = ft.dwHighDateTime;

 uint64 ret = li.QuadPart;
 ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
 ret /= 10000; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */

 return ret;
#else
    /* Linux */
     struct timeval tv;

     gettimeofday(&tv, NULL);

     ulong64 ret = tv.tv_usec;
     /* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
     ret /= 1000;

     /* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
     ret += (tv.tv_sec * 1000);

     return ret;
    #endif
 }


void ImageResource::handleRequest(const Http::Request &request, Http::Response &response)
{
    ulong64 startTime = GetTimeMs64();
    const string* imgPath = request.getParameter("imgPath");
    const string* imgName = request.getParameter("imgName");

    if (!imgPath || !imgName)
    {
        response.out() << "No image is selected sorry\n";
        return;
    }

    string tmp = *imgName;
    boost::filesystem::create_directory(tempImageFolder);
    // download image and store
    std::string wgetCommand = "";
    wgetCommand = wgetCommand + wgetCmd + " -O " + tempImageFolder + "/" + tmp + " " + *imgPath;
    system(wgetCommand.c_str());
    string imgLocalPath = tempImageFolder + "/" + *imgName;

    // /////////////////////////////////

    AppData *app = AppData::getInstance();
    std::cout << "Done download and get data" << std::endl;

    tmp.replace(tmp.size() - 3, 3, "mat");

    string kpPath = kpFolder + "/" + tmp;
    string siftPath = siftFolder + "/" + tmp;
    string tempPath = tempFolder + "/" + tmp;
    //imgPath = queryFolder + "/" + *imgPath;
    std::cout << "Done string op" << endl;
    mat _kp, _sift;
    extractFeatures(imgLocalPath, _kp, _sift, kpPath, siftPath, tempPath);

    int nDocs = app->path.size();
    arma::vec _weights;
    arma::uvec _termID;
    string weightPath = weightFolder + "/" + tmp;
    string termIDPath = termIDFolder + "/" + tmp;

    ulong64 timeDiff = GetTimeMs64() - startTime;
    cout << timeDiff << endl;

    buildBoW(_sift, _weights, _termID, weightPath, termIDPath, true, false);

    debugInfo("Done build BoW");
    cout << sizeof(int) << endl;
    cout << sizeof(long long) << endl;
//    for (unsigned int i=0;i<_termID.n_elem;++i)
//    {
//        cout << sizeof(_termID[i]) << endl;
//    }

    vector<double> qTfidf = app->ivt.makeQueryTfidf(_weights, _termID);

    Score score(computeAllScores(qTfidf));

    vector<int> rankedList(nDocs);
    for (int i = 0; i < nDocs; i++)
        rankedList[i] = i;
    sort(rankedList.begin(), rankedList.end(), score);
    timeDiff = GetTimeMs64() - startTime;
    cout << timeDiff << endl;

    int resLength = std::min(50,nDocs);

    response.out() << "[";
    for (int i=0; i < resLength; ++i) {
        response.out() << "\"" << app->path[rankedList[i]] << "\"";
        if (i != resLength-1) {
            response.out() << ",";
        }
    }
    response.out() << "]";

//    Read post data if you want to get the structure parse manually
//    const istream *reqBody = &request.in();
//    while (temp = request.in().peek())
//    {
//         request.in() >> inputName;
//         if (inputName.compare(prevString) == 0)
//         {
//            break;
//         }
//         prevString = inputName;
    //    }
}

void ImageResource::queryExpansion()
{

}

ImageResource::~ImageResource()
{

}

