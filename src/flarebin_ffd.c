/**
  @file flarebin_ffd.c

  @brief Deterministic FLAREBIN FFD helpers (power-law first implementation).
*/

#include "vplanet.h"

static void fvFlareBinNotImplemented(const char *cFeature) {
  fprintf(stderr, "FLAREBIN not implemented: %s\n", cFeature);
  abort();
}

static int fbFlareBinUseCumulativeAnchor(const BODY *body, int iStar) {
  return fabs(body[iStar].dFlareBinSlope) > EPS ||
         fabs(body[iStar].dFlareBinYInt) > EPS;
}

static double fdFlareBinPowerLawAlpha(const BODY *body, int iStar) {
  if (body[iStar].iFlareBinNormMode == FLAREBIN_NORM_FROM_RATE_AT_E0 &&
      fbFlareBinUseCumulativeAnchor(body, iStar)) {
    return 1.0 - body[iStar].dFlareBinSlope;
  }
  return body[iStar].dFlareBinAlpha;
}

static double fdFlareBinPowerLawK(const BODY *body, int iStar) {
  if (body[iStar].iFlareBinNormMode == FLAREBIN_NORM_FROM_FFD) {
    return body[iStar].dFlareBinK;
  }

  if (body[iStar].iFlareBinNormMode == FLAREBIN_NORM_FROM_RATE_AT_E0) {
    if (fbFlareBinUseCumulativeAnchor(body, iStar)) {
      double dA = body[iStar].dFlareBinSlope;
      double dB = body[iStar].dFlareBinYInt;
      return pow(10.0, dB) * (-dA);
    }

    /* Differential anchor fallback: treat dFlareBinK as r(E0). */
    if (body[iStar].dFlareBinDurE0 <= 0) {
      fvFlareBinNotImplemented(
          "NORM_FROM_RATE_AT_E0 differential anchor requires dFlareBinDurE0 > 0.");
    }
    return body[iStar].dFlareBinK *
           pow(body[iStar].dFlareBinDurE0, fdFlareBinPowerLawAlpha(body, iStar));
  }

  if (body[iStar].iFlareBinNormMode ==
      FLAREBIN_NORM_FROM_FLAREPOWER_FRACTION) {
    fvFlareBinNotImplemented(
        "NORM_FROM_FLAREPOWER_FRACTION in fvFlareBinNormalizeFfd.");
  }

  fvFlareBinNotImplemented("Unknown FLAREBIN normalization mode.");
  return 0;
}

static double fdFlareBinRateDensityPowerLawCore(const BODY *body, int iStar,
                                                double dEin) {
  double dEmin = body[iStar].dFlareBinEmin;
  double dEmax = body[iStar].dFlareBinEmax;
  double dK    = fdFlareBinPowerLawK(body, iStar);
  double dAlpha;

  if (dEin <= 0 || dEin < dEmin || dEin > dEmax) {
    return 0;
  }

  dAlpha = fdFlareBinPowerLawAlpha(body, iStar);
  return dK * pow(dEin, -dAlpha);
}

double fdFlareBinEnergyToXUV(const BODY *body, int iStar, double dEin) {
  if (dEin <= 0) {
    return 0;
  }
  return body[iStar].dFlareBinBandC * pow(dEin, body[iStar].dFlareBinBandP);
}

double fdFlareBinDuration(const BODY *body, int iStar, double dEin) {
  double dTau = 0;

  if (dEin > 0 && body[iStar].dFlareBinTau0 > 0 &&
      body[iStar].dFlareBinDurE0 > 0) {
    dTau = body[iStar].dFlareBinTau0 *
           pow(dEin / body[iStar].dFlareBinDurE0, body[iStar].dFlareBinDurExp);
  }

#ifdef DEBUG
  if (dEin > 0 && body[iStar].dFlareBinTau0 > 0 &&
      body[iStar].dFlareBinDurE0 > 0) {
    assert(dTau > 0);
  }
#endif

  return dTau;
}

double fdFlareBinRateDensity(const BODY *body, int iStar, double dEin,
                             double dTimeEval) {
  double dRate;

  (void)dTimeEval;

  if (body[iStar].iFlareBinDist != FLAREBIN_DIST_POWERLAW) {
    fvFlareBinNotImplemented("Lognormal FFD rate density.");
  }

  dRate = fdFlareBinRateDensityPowerLawCore(body, iStar, dEin);

#ifdef DEBUG
  if (dEin < body[iStar].dFlareBinEmin || dEin > body[iStar].dFlareBinEmax ||
      dEin <= 0) {
    assert(fabs(dRate) <= EPS);
  }
#endif

  return dRate;
}

void fvFlareBinNormalizeFfd(BODY *body, int iStar) {
  if (body[iStar].iFlareBinDist != FLAREBIN_DIST_POWERLAW) {
    fvFlareBinNotImplemented("Lognormal normalization.");
  }

  if (body[iStar].iFlareBinNormMode ==
      FLAREBIN_NORM_FROM_FLAREPOWER_FRACTION) {
    fvFlareBinNotImplemented(
        "NORM_FROM_FLAREPOWER_FRACTION normalization.");
  }

  body[iStar].dFlareBinAlpha = fdFlareBinPowerLawAlpha(body, iStar);
  body[iStar].dFlareBinK     = fdFlareBinPowerLawK(body, iStar);

#ifdef DEBUG
  assert(body[iStar].dFlareBinK >= 0);
#endif
}

double fdFlareBinPowerIntegrand(const BODY *body, int iStar, double dEin,
                                double dTimeEval) {
  return fdFlareBinRateDensity(body, iStar, dEin, dTimeEval) *
         fdFlareBinEnergyToXUV(body, iStar, dEin);
}

double fdFlareBinOverlapIntegrand(const BODY *body, int iStar, double dEin,
                                  double dTimeEval) {
  return fdFlareBinRateDensity(body, iStar, dEin, dTimeEval) *
         fdFlareBinDuration(body, iStar, dEin);
}

double fdFlareBinOverlapSupportIntegrand(const BODY *body, int iStar,
                                         double dEin, double dTimeEval) {
  double dDeltaX = body[iStar].dFlareBinXEnd - body[iStar].dFlareBinXMin;
  return fdFlareBinOverlapIntegrand(body, iStar, dEin, dTimeEval) * dDeltaX;
}
