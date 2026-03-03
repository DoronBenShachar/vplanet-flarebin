import pathlib
import subprocess

import numpy as np


TEST_DIR = pathlib.Path(__file__).resolve().parent
REPO_ROOT = TEST_DIR.parents[2]
BIN = REPO_ROOT / "bin" / "vplanet"


def _run_case():
    for filename in (
        "Minimal.log",
        "Minimal.star.forward",
        "Minimal.planet.forward",
    ):
        path = TEST_DIR / filename
        if path.exists():
            path.unlink()

    subprocess.run([str(BIN), "vpl.in"], cwd=TEST_DIR, check=True)

    star = np.atleast_2d(np.loadtxt(TEST_DIR / "Minimal.star.forward"))
    planet = np.atleast_2d(np.loadtxt(TEST_DIR / "Minimal.planet.forward"))
    log_bytes = (TEST_DIR / "Minimal.log").read_bytes()

    return star, planet, log_bytes


def test_minimal_flarebin_atmesc_regression():
    star_1, planet_1, log_1 = _run_case()
    star_2, planet_2, log_2 = _run_case()

    # Deterministic regression: repeated runs must produce identical outputs.
    assert np.array_equal(star_1, star_2)
    assert np.array_equal(planet_1, planet_2)
    assert log_1 == log_2

    star_final = star_1[-1]
    planet_final = planet_1[-1]

    lxuv_mean = star_final[1]
    lxuv_q = star_final[2]
    flarebin_pstoch = star_final[3]

    # Energy consistency anchor for flarebin on the host star.
    assert np.isclose(
        lxuv_mean,
        lxuv_q + flarebin_pstoch,
        rtol=1e-10,
        atol=1e-3,
    )

    # Flarebin convenience mean flux should match the planet mean XUV flux.
    assert np.isclose(planet_final[5], planet_final[6], rtol=0, atol=1e-12)

    p_above_1 = planet_final[8]
    p_above_2 = planet_final[9]

    # Indicator-function expectations are probabilities.
    assert 0.0 <= p_above_1 <= 1.0
    assert 0.0 <= p_above_2 <= 1.0

    mdot_mean = planet_final[10]
    mdot_eff = planet_final[11]

    # Effective and mean-forcing rates must be defined and finite.
    assert np.isfinite(mdot_mean)
    assert np.isfinite(mdot_eff)

    # If threshold crossings are plausible, effective averaging should alter the rate.
    if (0.0 < p_above_1 < 1.0) or (0.0 < p_above_2 < 1.0):
        assert not np.isclose(mdot_mean, mdot_eff, rtol=0, atol=0)
