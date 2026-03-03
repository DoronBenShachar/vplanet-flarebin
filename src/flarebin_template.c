/**
  @file flarebin_template.c

  @brief Davenport flare-template evaluation and deterministic analytic
  integrals for FLAREBIN.
*/

#include "vplanet.h"

/* Single coefficient table:
 * [0] rise polynomial c0..c4 for x in [-1, 0]
 * [1] decay exponential term 1: amp, rate for x >= 0
 * [2] decay exponential term 2: amp, rate for x >= 0
 */
static const double daFlareTplCoeff[3][5] = {
    {1.0, 1.941, -0.175, -2.246, -1.125},
    {0.6890, 1.600, 0.0, 0.0, 0.0},
    {0.3030, 0.2783, 0.0, 0.0, 0.0},
};

static double fdFlareTplRiseIntegral(double xMin, double xMax) {
  int i;
  double dPowMin = xMin;
  double dPowMax = xMax;
  double dSum    = 0;

  if (xMax <= xMin) {
    return 0;
  }

  for (i = 0; i < 5; i++) {
    dSum += daFlareTplCoeff[0][i] * (dPowMax - dPowMin) / (double)(i + 1);
    dPowMin *= xMin;
    dPowMax *= xMax;
  }

  return dSum;
}

static double fdExpTail(double dRate, double x) {
  if (isinf(x) && x > 0) {
    return 0;
  }
  return exp(-dRate * x);
}

static double fdFlareTplDecayIntegral(double xMin, double xMax) {
  double dA1 = daFlareTplCoeff[1][0];
  double dK1 = daFlareTplCoeff[1][1];
  double dA2 = daFlareTplCoeff[2][0];
  double dK2 = daFlareTplCoeff[2][1];

  if (xMax <= xMin) {
    return 0;
  }

  return dA1 * (fdExpTail(dK1, xMin) - fdExpTail(dK1, xMax)) / dK1 +
         dA2 * (fdExpTail(dK2, xMin) - fdExpTail(dK2, xMax)) / dK2;
}

static double fdFlareTplRiseIntegralSq(double xMin, double xMax) {
  int i, j;
  double daSq[9];
  double dSum = 0;
  double dPowMin;
  double dPowMax;

  if (xMax <= xMin) {
    return 0;
  }

  for (i = 0; i < 9; i++) {
    daSq[i] = 0;
  }

  for (i = 0; i < 5; i++) {
    for (j = 0; j < 5; j++) {
      daSq[i + j] += daFlareTplCoeff[0][i] * daFlareTplCoeff[0][j];
    }
  }

  dPowMin = xMin;
  dPowMax = xMax;
  for (i = 0; i < 9; i++) {
    dSum += daSq[i] * (dPowMax - dPowMin) / (double)(i + 1);
    dPowMin *= xMin;
    dPowMax *= xMax;
  }

  return dSum;
}

static double fdFlareTplDecayIntegralSq(double xMin, double xMax) {
  double dA1 = daFlareTplCoeff[1][0];
  double dK1 = daFlareTplCoeff[1][1];
  double dA2 = daFlareTplCoeff[2][0];
  double dK2 = daFlareTplCoeff[2][1];
  double dK12 = dK1 + dK2;

  if (xMax <= xMin) {
    return 0;
  }

  return dA1 * dA1 * (fdExpTail(2 * dK1, xMin) - fdExpTail(2 * dK1, xMax)) /
             (2 * dK1) +
         2 * dA1 * dA2 * (fdExpTail(dK12, xMin) - fdExpTail(dK12, xMax)) /
             dK12 +
         dA2 * dA2 * (fdExpTail(2 * dK2, xMin) - fdExpTail(2 * dK2, xMax)) /
             (2 * dK2);
}

static double fdFlareTplCore(double x) {
  if (x < -1.0) {
    return 0;
  }

  if (x <= 0.0) {
    return daFlareTplCoeff[0][0] + daFlareTplCoeff[0][1] * x +
           daFlareTplCoeff[0][2] * x * x + daFlareTplCoeff[0][3] * x * x * x +
           daFlareTplCoeff[0][4] * x * x * x * x;
  }

  return daFlareTplCoeff[1][0] * exp(-daFlareTplCoeff[1][1] * x) +
         daFlareTplCoeff[2][0] * exp(-daFlareTplCoeff[2][1] * x);
}

static double fdFlareTplIntegralCore(double xMin, double xMax) {
  double dA, dB;
  double dInt  = 0;
  double dSign = 1;

  if (xMax == xMin) {
    return 0;
  }

  dA = xMin;
  dB = xMax;
  if (dA > dB) {
    dA    = xMax;
    dB    = xMin;
    dSign = -1;
  }

  if (dB > -1.0 && dA < 0.0) {
    dInt += fdFlareTplRiseIntegral(fmax(dA, -1.0), fmin(dB, 0.0));
  }

  if (dB > 0.0) {
    dInt += fdFlareTplDecayIntegral(fmax(dA, 0.0), dB);
  }

  return dSign * dInt;
}

static double fdFlareTplIntegralSqCore(double xMin, double xMax) {
  double dA, dB;
  double dInt  = 0;
  double dSign = 1;

  if (xMax == xMin) {
    return 0;
  }

  dA = xMin;
  dB = xMax;
  if (dA > dB) {
    dA    = xMax;
    dB    = xMin;
    dSign = -1;
  }

  if (dB > -1.0 && dA < 0.0) {
    dInt += fdFlareTplRiseIntegralSq(fmax(dA, -1.0), fmin(dB, 0.0));
  }

  if (dB > 0.0) {
    dInt += fdFlareTplDecayIntegralSq(fmax(dA, 0.0), dB);
  }

  return dSign * dInt;
}

#ifdef DEBUG
static void fvFlareTplSelfCheck(void) {
  static int bCheckState = 0;
  double dTol            = 1e-14;
  double dXEnd           = 20.0;
  double dI1, dI2;

  if (bCheckState == 2) {
    return;
  }
  if (bCheckState == 1) {
    return;
  }
  bCheckState = 1;

  if (fabs(fdFlareTplCore(0.0) - 1.0) > dTol) {
    fprintf(stderr,
            "FLAREBIN template self-check failed: fdFlareTpl(0) != 1.\n");
    abort();
  }

  dI1 = fdFlareTplIntegralCore(-1.0, dXEnd);
  dI2 = fdFlareTplIntegralCore(-1.0, dXEnd);
  if (!(dI1 > 0) || fabs(dI1 - dI2) > dTol * (1.0 + fabs(dI1))) {
    fprintf(stderr,
            "FLAREBIN template self-check failed: I_tpl instability.\n");
    abort();
  }

  bCheckState = 2;
}
#endif

double fdFlareTpl(double x) {
#ifdef DEBUG
  fvFlareTplSelfCheck();
#endif

  return fdFlareTplCore(x);
}

double fdFlareTplIntegral(double xMin, double xMax) {
#ifdef DEBUG
  fvFlareTplSelfCheck();
#endif

  return fdFlareTplIntegralCore(xMin, xMax);
}

double fdFlareTplIntegralSq(double xMin, double xMax) {
#ifdef DEBUG
  fvFlareTplSelfCheck();
#endif

  return fdFlareTplIntegralSqCore(xMin, xMax);
}
