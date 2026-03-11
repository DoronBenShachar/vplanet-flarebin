import pathlib
import subprocess

import numpy as np


TEST_DIR = pathlib.Path(__file__).resolve().parent
REPO_ROOT = TEST_DIR.parents[3]
BIN = REPO_ROOT / "bin" / "vplanet"
INFILE = "vpl.in"
LOG_PATH = TEST_DIR / "flarebin_davenport.log"
STAR_PATH = TEST_DIR / "flarebin_davenport.star.forward"


def test_flarebin_davenport2019_coefficients():
    try:
        if LOG_PATH.exists():
            LOG_PATH.unlink()
        if STAR_PATH.exists():
            STAR_PATH.unlink()

        subprocess.run([str(BIN), INFILE], cwd=TEST_DIR, check=True)

        star = np.atleast_2d(np.loadtxt(STAR_PATH))
        final = star[-1]

        davenport_a = final[1]
        davenport_b = final[2]
        estoch_min_erg = final[3]
        pstoch = final[4]

        age_myr = 100.0
        mass_solar = 0.5
        expected_a = (-0.07 * np.log10(age_myr)) + (0.79 * mass_solar) - 1.06
        expected_b = (2.01 * np.log10(age_myr)) + (-25.15 * mass_solar) + 33.99

        assert np.isclose(davenport_a, expected_a, rtol=0, atol=1e-12)
        assert np.isclose(davenport_b, expected_b, rtol=0, atol=1e-12)
        assert np.isclose(estoch_min_erg, 1.0e34, rtol=1e-12, atol=0)
        assert np.isfinite(pstoch)
        assert pstoch >= 0.0
    finally:
        if LOG_PATH.exists():
            LOG_PATH.unlink()
        if STAR_PATH.exists():
            STAR_PATH.unlink()
