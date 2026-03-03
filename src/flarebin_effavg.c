/**
  @file flarebin_effavg.c

  @brief Deterministic internal numerical helpers for FLAREBIN effective
  averaging.
*/

#include "vplanet.h"

#define FLAREBIN_GL_MAX_ITERS 128
#define FLAREBIN_GL_ROOT_TOL 1e-14
#define FLAREBIN_GL_VERIFY_TOL 1e-12
#define FLAREBIN_EFFAVG_DIAG_TOL 1e-12

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

static double fdFlareBinTruncMassNgt1(double dMu) {
  double dProb;

  if (dMu <= 0 || !isfinite(dMu)) {
    return 0;
  }

  dProb = 1.0 - exp(-dMu) * (1.0 + dMu);

  if (dProb < 0 && fabs(dProb) <= FLAREBIN_EFFAVG_DIAG_TOL) {
    dProb = 0;
  }
  if (dProb > 1 && fabs(dProb - 1.0) <= FLAREBIN_EFFAVG_DIAG_TOL) {
    dProb = 1;
  }

  if (dProb < 0) {
    dProb = 0;
  }
  if (dProb > 1) {
    dProb = 1;
  }

  return dProb;
}

static double fdFlareBinIdentityCallback(double dFXUV, void *pContext) {
  (void)pContext;
  return dFXUV;
}

#ifdef DEBUG
static void fvFlareBinEffAvgDebugChecks(const BODY *body, int iStar, double dMu,
                                        double dExpMinusMu, int iNMax) {
  static int bWarnedTruncN1 = 0;
  static int bWarnedNgt1    = 0;
  double dNormN1;
  double dLostMass;

  dNormN1  = dExpMinusMu * (1.0 + dMu);
  dLostMass = fdFlareBinTruncMassNgt1(dMu);

  if (dNormN1 < -FLAREBIN_EFFAVG_DIAG_TOL ||
      dNormN1 > 1.0 + FLAREBIN_EFFAVG_DIAG_TOL) {
    fprintf(stderr,
            "ERROR: FLAREBIN normalization check failed on body %d "
            "(N_max=1 normalization %.16e).\n",
            iStar, dNormN1);
    abort();
  }

  if (fabs((1.0 - dLostMass) - dNormN1) >
      FLAREBIN_EFFAVG_DIAG_TOL * (1.0 + fabs(dNormN1))) {
    fprintf(stderr,
            "ERROR: FLAREBIN truncation-mass check failed on body %d "
            "(mu=%.16e, P(N>1)=%.16e, normN1=%.16e).\n",
            iStar, dMu, dLostMass, dNormN1);
    abort();
  }

  if (iNMax == FLAREBIN_OVERLAP_N1 &&
      dLostMass > body[iStar].dFlareBinOverlapTol && !bWarnedTruncN1) {
    fprintf(stderr,
            "WARNING: FLAREBIN truncation diagnostic on body %d: "
            "N_max=1 neglects P(N>1)=%.6e (mu=%.6e, tol=%.6e).\n",
            iStar, dLostMass, dMu, body[iStar].dFlareBinOverlapTol);
    bWarnedTruncN1 = 1;
  }

  if (iNMax > FLAREBIN_OVERLAP_N1 && !bWarnedNgt1) {
    fprintf(stderr,
            "WARNING: FLAREBIN currently evaluates effective averaging "
            "through N_max=1 only; higher N_max requests are truncated.\n");
    bWarnedNgt1 = 1;
  }
}
#endif

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

double fdFlareBinMeanFXUV(BODY *body, SYSTEM *system, int iStar, int iPlanet) {
  (void)system;
  (void)iStar;

  if (iPlanet < 0 || !isfinite(body[iPlanet].dFXUV) || body[iPlanet].dFXUV < 0) {
    return 0;
  }

  return body[iPlanet].dFXUV;
}

double fdFlareBinExpectFunction(BODY *body, SYSTEM *system, int iStar, int iPlanet,
                                double (*fnG)(double, void *), void *pContext) {
  int iE, iX;
  int iNE, iNX;
  double dMu, dExpMinusMu;
  double dLbar, dLq, dFXUVMean, dInvLbar;
  double dFq, dC0, dC1, dC1Sum;
  double dDeltaX, dInvDeltaX, dITpl;
  double dTimeEval;
  int iNMax;

  if (fnG == NULL) {
    fprintf(stderr, "ERROR: FLAREBIN expectation callback is NULL.\n");
    exit(EXIT_FAILURE);
  }

  if (!body[iStar].bFlareBin) {
    return fnG(fdFlareBinMeanFXUV(body, system, iStar, iPlanet), pContext);
  }

  dTimeEval = body[iStar].dAge;
  fvFlareBinPrecompute(body, system, iStar, dTimeEval);

  iNE = body[iStar].iFlareBinNEnergy;
  iNX = body[iStar].iFlareBinNPhase;
  if (iNE <= 0 || iNX <= 0 || body[iStar].daFlareBinQuadE == NULL ||
      body[iStar].daFlareBinQuadWE == NULL || body[iStar].daFlareBinQuadWX == NULL ||
      body[iStar].daFlareBinTplAtX == NULL) {
    fprintf(stderr,
            "ERROR: FLAREBIN expectation cache missing on body %d.\n",
            iStar);
    exit(EXIT_FAILURE);
  }

  dMu = body[iStar].dFlareBinMu;
  if (dMu < 0 && fabs(dMu) <= FLAREBIN_EFFAVG_DIAG_TOL) {
    dMu = 0;
  }
  if (dMu < 0) {
    fprintf(stderr,
            "ERROR: FLAREBIN invalid mu on body %d: %.16e.\n",
            iStar, dMu);
    exit(EXIT_FAILURE);
  }

  dExpMinusMu = exp(-dMu);
  dLbar       = body[iStar].dLXUV;
  dLq         = body[iStar].dFlareBinLQ;
  dFXUVMean   = fdFlareBinMeanFXUV(body, system, iStar, iPlanet);

  if (dLbar > 0) {
    dInvLbar = 1.0 / dLbar;
  } else {
    dInvLbar = 0;
  }

  dFq = dFXUVMean * dLq * dInvLbar;
  dC0 = dExpMinusMu * fnG(dFq, pContext);

  iNMax = body[iStar].iFlareBinMaxOverlapN;
  if (iNMax <= FLAREBIN_OVERLAP_N0) {
    return dC0;
  }

  dDeltaX = body[iStar].dFlareBinDeltaX;
  dITpl   = body[iStar].dFlareBinITpl;
  if (dDeltaX <= 0 || dITpl <= 0) {
    fprintf(stderr,
            "ERROR: FLAREBIN invalid template normalization on body %d.\n",
            iStar);
    exit(EXIT_FAILURE);
  }

  dInvDeltaX = 1.0 / dDeltaX;
  dC1Sum     = 0;

  for (iE = 0; iE < iNE; iE++) {
    double dE       = body[iStar].daFlareBinQuadE[iE];
    double dWE      = body[iStar].daFlareBinQuadWE[iE];
    double dRate    = fdFlareBinRateDensity(body, iStar, dE, dTimeEval);
    double dTau     = fdFlareBinDuration(body, iStar, dE);
    double dAmpLum;
    double dAmpFluxScale;
    double dInner = 0;

    if (dRate <= 0 || dTau <= 0 || dWE == 0) {
      continue;
    }

    dAmpLum = fdFlareBinEnergyToXUV(body, iStar, dE) / (dTau * dITpl);
    dAmpFluxScale = dFXUVMean * dAmpLum * dInvLbar;

    for (iX = 0; iX < iNX; iX++) {
      double dWX   = body[iStar].daFlareBinQuadWX[iX];
      double dFXUV = dFq + dAmpFluxScale * body[iStar].daFlareBinTplAtX[iX];
      dInner += dWX * fnG(dFXUV, pContext);
    }

    dInner *= dInvDeltaX;
    dC1Sum += dWE * dRate * dTau * dInner;
  }

  dC1 = dExpMinusMu * dC1Sum;

#ifdef DEBUG
  fvFlareBinEffAvgDebugChecks(body, iStar, dMu, dExpMinusMu, iNMax);
#endif

  return dC0 + dC1;
}

double fdFlareBinExpectAtmEscRhs(BODY *body, SYSTEM *system, int iStar,
                                 int iPlanet) {
  return fdFlareBinExpectFunction(body, system, iStar, iPlanet,
                                  fdFlareBinIdentityCallback, NULL);
}
