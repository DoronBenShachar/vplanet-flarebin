/**
  @file flarebin_ffd.c

  @brief Deterministic FLAREBIN FFD helpers (power-law first implementation).
*/

#include "vplanet.h"

#define FLAREBIN_DAVENPORT_A1 (-0.07)
#define FLAREBIN_DAVENPORT_A2 (0.79)
#define FLAREBIN_DAVENPORT_A3 (-1.06)
#define FLAREBIN_DAVENPORT_B1 (2.01)
#define FLAREBIN_DAVENPORT_B2 (-25.15)
#define FLAREBIN_DAVENPORT_B3 (33.99)

static void fvFlareBinNotImplemented(const char *cFeature) {
  fprintf(stderr, "FLAREBIN not implemented: %s\n", cFeature);
  abort();
}

static void fvFlareBinFatal(const char *cMessage, int iStar, double dValue) {
  fprintf(stderr, "ERROR: FLAREBIN body %d: %s (value=%.16e).\n", iStar,
          cMessage, dValue);
  exit(EXIT_FAILURE);
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

  if (body[iStar].iFlareBinNormMode == FLAREBIN_NORM_DAVENPORT2019) {
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

static void fvFlareBinSetDavenportFfd(BODY *body, int iStar, double dTimeEval) {
  double dTimeMyr;
  double dMassSolar;
  double dA;
  double dB;
  double dLog10K;
  double dK;

  if (!isfinite(dTimeEval) || dTimeEval <= 0) {
    fprintf(stderr,
            "ERROR: FLAREBIN Davenport2019 on body %d requires positive "
            "stellar age at evaluation (dAge > 0). Set a positive initial age.\n",
            iStar);
    exit(EXIT_FAILURE);
  }

  dTimeMyr = dTimeEval / (1.0e6 * YEARSEC);
  if (!isfinite(dTimeMyr) || dTimeMyr <= 0) {
    fvFlareBinFatal(
        "invalid stellar age for Davenport2019 after conversion to Myr", iStar,
        dTimeMyr);
  }

  dMassSolar = body[iStar].dMass / MSUN;
  if (!isfinite(dMassSolar) || dMassSolar <= 0) {
    fvFlareBinFatal("invalid stellar mass for Davenport2019 in solar masses",
                    iStar, dMassSolar);
  }

  dA = FLAREBIN_DAVENPORT_A1 * log10(dTimeMyr) +
       FLAREBIN_DAVENPORT_A2 * dMassSolar + FLAREBIN_DAVENPORT_A3;
  dB = FLAREBIN_DAVENPORT_B1 * log10(dTimeMyr) +
       FLAREBIN_DAVENPORT_B2 * dMassSolar + FLAREBIN_DAVENPORT_B3;

  if (!isfinite(dA) || !isfinite(dB)) {
    fprintf(stderr,
            "ERROR: FLAREBIN Davenport2019 produced non-finite FFD coefficients "
            "on body %d (a=%.16e, b=%.16e).\n",
            iStar, dA, dB);
    exit(EXIT_FAILURE);
  }

  if (dA >= 0) {
    fprintf(stderr,
            "ERROR: FLAREBIN Davenport2019 on body %d produced non-physical "
            "cumulative slope a=%.16e (must be < 0).\n",
            iStar, dA);
    exit(EXIT_FAILURE);
  }

  /* Davenport et al. (2019):
   *   log10 nu = a log10 eps + b, with eps in erg and nu in flares/day.
   * Differential density in SI (events/s/J) written as r(E) = k E^{-alpha}:
   *   alpha = 1 - a
   *   log10(k) = log10(-a) + b + 7*a - log10(DAYSEC)
   * where log10(eps[erg]) = log10(E[J]) + 7.
   */
  dLog10K = log10(-dA) + dB + 7.0 * dA - log10(DAYSEC);
  if (!isfinite(dLog10K)) {
    fvFlareBinFatal("invalid Davenport2019 log10(k) conversion", iStar, dLog10K);
  }

  dK = pow(10.0, dLog10K);
  if (!isfinite(dK) || dK <= 0) {
    fvFlareBinFatal("invalid Davenport2019 SI normalization k", iStar, dK);
  }

  body[iStar].dFlareBinDavenportA = dA;
  body[iStar].dFlareBinDavenportB = dB;
  body[iStar].dFlareBinAlpha      = 1.0 - dA;
  body[iStar].dFlareBinK          = dK;
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

void fvFlareBinNormalizeFfd(BODY *body, int iStar, double dTimeEval) {
  if (body[iStar].iFlareBinDist != FLAREBIN_DIST_POWERLAW) {
    fvFlareBinNotImplemented("Lognormal normalization.");
  }

  body[iStar].dFlareBinDavenportA = FLAREBIN_OUTPUT_SENTINEL_DISABLED;
  body[iStar].dFlareBinDavenportB = FLAREBIN_OUTPUT_SENTINEL_DISABLED;

  if (body[iStar].iFlareBinNormMode == FLAREBIN_NORM_DAVENPORT2019) {
    fvFlareBinSetDavenportFfd(body, iStar, dTimeEval);
    return;
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
