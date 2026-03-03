/**
  @file flarebin_effavg.c

  @brief Deterministic internal numerical helpers for FLAREBIN effective
  averaging.
*/

#include "vplanet.h"

#define FLAREBIN_GL_MAX_ITERS 128
#define FLAREBIN_GL_ROOT_TOL 1e-14
#define FLAREBIN_GL_VERIFY_TOL 1e-12

static void fvFlareBinLegendreWithDeriv(int iN, double dX, double *dPN,
                                        double *dDPN) {
  int i;
  double dP0, dP1, dP2;

  if (iN == 0) {
    *dPN  = 1.0;
    *dDPN = 0.0;
    return;
  }

  if (iN == 1) {
    *dPN  = dX;
    *dDPN = 1.0;
    return;
  }

  dP0 = 1.0;
  dP1 = dX;
  for (i = 2; i <= iN; i++) {
    dP2 = (((2.0 * i) - 1.0) * dX * dP1 - (i - 1.0) * dP0) / i;
    dP0 = dP1;
    dP1 = dP2;
  }

  *dPN = dP1;
  /* d/dx P_n(x) */
  *dDPN = iN * (dX * dP1 - dP0) / (dX * dX - 1.0);
}

/*
 * Deterministic Gauss-Legendre nodes/weights on [dA, dB].
 * Caller allocates daNode/daWeight with iN entries.
 * This symbol is shared within FLAREBIN implementation units.
 */
int fiFlareBinGaussLegendreRule(int iN, double dA, double dB, double *daNode,
                                double *daWeight) {
  int i, iIter;
  int iHalf;
  double dMid, dHalfWidth;

  if (iN < 1 || daNode == NULL || daWeight == NULL) {
    return 0;
  }

  dMid       = 0.5 * (dB + dA);
  dHalfWidth = 0.5 * (dB - dA);
  iHalf      = (iN + 1) / 2;

  for (i = 0; i < iHalf; i++) {
    double dZ, dZPrev;
    double dPN, dDPN;
    double dW;

    dZ = cos(PI * ((double)i + 0.75) / ((double)iN + 0.5));

    for (iIter = 0; iIter < FLAREBIN_GL_MAX_ITERS; iIter++) {
      fvFlareBinLegendreWithDeriv(iN, dZ, &dPN, &dDPN);
      dZPrev = dZ;
      dZ     = dZPrev - dPN / dDPN;
      if (fabs(dZ - dZPrev) <= FLAREBIN_GL_ROOT_TOL) {
        break;
      }
    }

    if (iIter == FLAREBIN_GL_MAX_ITERS) {
      return 0;
    }

    fvFlareBinLegendreWithDeriv(iN, dZ, &dPN, &dDPN);
    dW = 2.0 / ((1.0 - dZ * dZ) * dDPN * dDPN);

    daNode[i]         = dMid - dHalfWidth * dZ;
    daNode[iN - 1 - i] = dMid + dHalfWidth * dZ;
    daWeight[i]       = dHalfWidth * dW;
    daWeight[iN - 1 - i] = dHalfWidth * dW;
  }

  return 1;
}

static int fiFlareBinGaussLegendreVerify(void) {
  int iNCase, iCase, i;
  const int aiN[] = {2, 4, 8};
  const double daA[] = {-1.0, 0.0, -2.0};
  const double daB[] = {1.0, 1.0, 3.0};
  double daNode[8];
  double daWeight[8];

  for (iCase = 0; iCase < 3; iCase++) {
    double dA = daA[iCase];
    double dB = daB[iCase];
    double dI0Exact = dB - dA;
    double dI1Exact = 0.5 * (dB * dB - dA * dA);
    double dI2Exact = (dB * dB * dB - dA * dA * dA) / 3.0;

    for (iNCase = 0; iNCase < 3; iNCase++) {
      int iN      = aiN[iNCase];
      double dI0  = 0;
      double dI1  = 0;
      double dI2  = 0;
      double dTol = FLAREBIN_GL_VERIFY_TOL;

      if (!fiFlareBinGaussLegendreRule(iN, dA, dB, daNode, daWeight)) {
        return 0;
      }

      for (i = 0; i < iN; i++) {
        dI0 += daWeight[i];
        dI1 += daWeight[i] * daNode[i];
        dI2 += daWeight[i] * daNode[i] * daNode[i];
      }

      if (fabs(dI0 - dI0Exact) > dTol * (1.0 + fabs(dI0Exact))) {
        return 0;
      }
      if (fabs(dI1 - dI1Exact) > dTol * (1.0 + fabs(dI1Exact))) {
        return 0;
      }
      if (fabs(dI2 - dI2Exact) > dTol * (1.0 + fabs(dI2Exact))) {
        return 0;
      }
    }
  }

  return 1;
}

#ifdef DEBUG
/*
 * Debug-only probe used by future tests to validate deterministic quadrature.
 * Returns 1 on success, 0 on failure.
 */
int fiFlareBinDebugVerifyGaussLegendre(void) {
  return fiFlareBinGaussLegendreVerify();
}
#endif

void fvFlareBinPrecompute(BODY *body, SYSTEM *system, int iStar,
                          double dTimeEval) {
  int i, iNE;
  double dLbar, dLQ, dPStoch, dMu;
  double dNegTol;
  double dConsTol = 1e-10;

  (void)system;

  if (!body[iStar].bFlareBin) {
    return;
  }

  /* Deterministic cache short-circuit for repeated calls at identical t_eval. */
  if (body[iStar].dFlareBinLastPrecomputeAge == dTimeEval) {
    return;
  }

  iNE = body[iStar].iFlareBinNEnergy;
  if (iNE <= 0 || body[iStar].daFlareBinQuadE == NULL ||
      body[iStar].daFlareBinQuadWE == NULL) {
    fprintf(stderr,
            "ERROR: FLAREBIN precompute cache missing for body %d.\n",
            iStar);
    exit(EXIT_FAILURE);
  }

  fvFlareBinNormalizeFfd(body, iStar);

  dPStoch = 0;
  dMu     = 0;
  for (i = 0; i < iNE; i++) {
    double dE  = body[iStar].daFlareBinQuadE[i];
    double dWE = body[iStar].daFlareBinQuadWE[i];

    dPStoch += dWE * fdFlareBinPowerIntegrand(body, iStar, dE, dTimeEval);
    dMu += dWE *
           fdFlareBinOverlapSupportIntegrand(body, iStar, dE, dTimeEval);
  }

  dLbar = body[iStar].dLXUV;
  dLQ   = dLbar - dPStoch;

  dNegTol = dConsTol * (1.0 + fabs(dLbar));
  if (dLQ < -dNegTol) {
    fprintf(stderr,
            "ERROR: FLAREBIN invalid configuration on body %d: "
            "L_q = %.16e < 0 (Lbar=%.16e, P_stoch=%.16e).\n",
            iStar, dLQ, dLbar, dPStoch);
    exit(EXIT_FAILURE);
  }
  if (dLQ < 0) {
    dLQ = 0;
  }

  body[iStar].dFlareBinPStoch            = dPStoch;
  body[iStar].dFlareBinLQ                = dLQ;
  body[iStar].dFlareBinMu                = dMu;
  body[iStar].dFlareBinLastPrecomputeAge = dTimeEval;

#ifdef DEBUG
  if (fabs((body[iStar].dFlareBinLQ + body[iStar].dFlareBinPStoch) - dLbar) >
      dConsTol * (1.0 + fabs(dLbar))) {
    fprintf(stderr,
            "ERROR: FLAREBIN energy consistency check failed on body %d.\n",
            iStar);
    abort();
  }
#endif
}
