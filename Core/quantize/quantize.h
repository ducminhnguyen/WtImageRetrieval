#pragma
#ifndef QUANTIZE_H_INCLUDED
#define QUANTIZE_H_INCLUDED

#include "../configurations.h"
#include <iostream>
#include <cmath>
using namespace std;

 const double deltaSqr = 6250;
 const int nKdTree = 8;
 const int nChecks = 100;
 const int dataKnn = 1;
 const int queryKnn = 3;
 class Quantize
 {
 public:
     static cvflann::Index<cvflann::L2<double>> *treeIndex;
     static cvflann::Matrix<double> dataset;
 };




    inline void buildIndex(bool force = false) {
        cvflann::load_from_file(Quantize::dataset, codebookFile, "clusters");
        cout << "Done load" << endl;
        cvflann::IndexParams *indexParams;

        if (!force && boost::filesystem::exists(indexFile))
            indexParams = new cvflann::SavedIndexParams(indexFile);
        else
            indexParams = new cvflann::KDTreeIndexParams(nKdTree);

        Quantize::treeIndex = new cvflann::Index<cvflann::L2<double>> (Quantize::dataset, *indexParams);
        Quantize::treeIndex->buildIndex();
        Quantize::treeIndex->save(indexFile);
    }

    inline void buildBoW(const mat &imageDesc, vec &_weights, uvec &_termID, const string &weightPath, const string &termIDPath, bool force = false, bool save = true) {
        if (!force && boost::filesystem::exists(weightPath)) {
            _weights.load(weightPath);
            _termID.load(termIDPath);
            return;
        }

        double *tmpData = new double[imageDesc.n_elem];
        memcpy(tmpData, imageDesc.memptr(), sizeof(double) * imageDesc.n_elem);
        cvflann::Matrix<double> query(tmpData, imageDesc.n_cols, imageDesc.n_rows);



        cvflann::Matrix<int> indices(new int[query.rows*queryKnn], query.rows, queryKnn);
        cvflann::Matrix<double> dists(new double[query.rows*queryKnn], query.rows, queryKnn);



        Quantize::treeIndex->knnSearch(query, indices, dists, queryKnn, cvflann::SearchParams(nChecks));

        cvflann::Matrix<uword> indicesLong(new uword[query.rows*queryKnn], query.rows, queryKnn);
        for (int i=0;i<indices.rows;++i){
            for (int j=0;j<indices.cols;++j){
                        indicesLong[i][j] = (uword)indices[i][j];
            }
        }


        arma::umat bins(queryKnn, query.rows);
        memcpy(bins.memptr(), indicesLong.data, query.rows * queryKnn * sizeof(uword));
        arma::mat sqrDists(queryKnn, query.rows);
        memcpy(sqrDists.memptr(), dists.data, query.rows * queryKnn * sizeof(double));

        _termID = arma::vectorise(bins, 0);

        //mat weights = exp(-sqrDists / (2 * deltaSqr));
        mat weights(queryKnn, 1);
        for (int i=0;i<queryKnn;++i)
        {
            weights(i,0) = std::pow(0.5, i);
        }
        weights = repmat(weights, 1, query.rows);
        //weights = weights / repmat(sum(weights, 0), weights.n_rows, 1);
        _weights = vectorise(weights, 0);

        if (save)
        {
            _weights.save(weightPath);
            _termID.save(termIDPath);
        }

    }








#endif
