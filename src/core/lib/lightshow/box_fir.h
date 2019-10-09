//
// Created by Johannes on 17.05.2019.
//

#ifndef RASPITOLIGHT_SRC_UI_LIGHTSHOW_BOX_FIR_H_
#define RASPITOLIGHT_SRC_UI_LIGHTSHOW_BOX_FIR_H_

#include <iostream>
#include <vector>
#include "analysis.h"

// todo split in header and cpp file
class BoxFIR {
  /*
   * MUST be > 0
   */
  int numCoeffs; //
  /*
   * Filter coefficients
   */
  std::vector<double> b;
  /*
   * Filter memories
   */
  std::vector<double> m;

 public:
  BoxFIR(int _numCoeffs) :
      numCoeffs(_numCoeffs) {
    if (numCoeffs < 1)
      numCoeffs = 1; //Must be > 0 or bad stuff happens

    double val = 1. / numCoeffs;
    for (int ii = 0; ii < numCoeffs; ++ii) {
      b.push_back(val);
      m.push_back(0.);
    }
  }

  void filter(std::vector<int> &a) {
    int output;
    // todo use for each
    for (int nn = 0; nn < a.size(); ++nn) {
      //Apply smoothing filter to signal
      output = 0;
      m[0] = a[nn];
      for (int ii = 0; ii < numCoeffs; ++ii) {
        output += b[ii] * m[ii];
      }

      //Reshuffle memories
      for (int ii = numCoeffs - 1; ii != 0; --ii) {
        m[ii] = m[ii - 1];
      }
      a[nn] = output;
    }
  }

};

#endif //RASPITOLIGHT_SRC_UI_LIGHTSHOW_BOX_FIR_H_
