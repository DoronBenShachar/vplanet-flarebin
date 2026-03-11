# flarebin: Effective-average (subgrid) stellar XUV flare population module for VPLanet

## Architecture Overview

flarebin is a new VPLanet star module that models stellar XUV flares as a stochastic population **without time-resolving individual flare events** during long integrations. Instead, it provides a deterministic, numerically stable way to compute **effective (time-averaged) impacts** of unresolved, fast flare variability on slow-evolving planetary state variables (e.g., atmospheric escape).

Core idea: replace fast stochastic forcing `F_XUV(t)` (minutes–hours) by an **effective right-hand-side (RHS)** for the ODEs evolved by VPLanet (kyr–Myr timesteps), using a timescale-separation / stochastic-averaging approximation:

`dy/dt = g(y, F_XUV(t), ...)  ==>  dy/dt = < g(y, F_XUV, ...) >`

where `<...>` is an expectation over the **stationary distribution** of the instantaneous XUV flux implied by the flare population model at the current stellar age and baseline XUV luminosity.

flarebin integrates into VPLanet as follows:

- STELLAR provides the baseline long-term mean XUV luminosity evolution `L̄_XUV(t)` (what VPLanet would have used as the forcing without flarebin).
- flarebin defines a flare population (FFD + duration scaling + Davenport template + band conversion) and enforces **energy consistency** with `L̄_XUV(t)`.
- flarebin exposes a C API that allows other modules (primarily ATMESC) to replace any direct dependence on instantaneous `F_XUV` by an **expected value of their own physics**, computed deterministically by quadrature over the flare-induced flux distribution.
- No timestep subdivision for minute-scale dynamics is required; therefore, this design is suitable for hundreds of Myr–Gyr integrations.

This document intentionally removes time-resolved event lists, timestep constraints, and “next-flare scheduling” logic. Those are not compatible with 10^8–10^9-year runs when flare rates are high.

## Physics Model

### 1) Flare population as a Poisson shot-noise process

Model flare occurrences as a Poisson point process in time with an energy-dependent differential rate `r(E)` (units: events / time / energy) over energies `[E_min, E_max]` in a specified “input band” (e.g., Kepler, TESS, bolometric, or XUV). Each flare has an energy `E_in` (input band) and an XUV-band energy `E_XUV` obtained by a configurable conversion.

Define the instantaneous flare luminosity contribution of a single flare with energy `E_XUV` and characteristic duration `τ(E)` as:

`ℓ(t; E) = (E_XUV / (τ(E) * I_tpl)) * f(t / τ(E))`

where:
- `f(x)` is a dimensionless flare template (Davenport), normalized with `f(0)=1`.
- `I_tpl = ∫_{-∞}^{∞} f(x) dx` (finite). In practice we truncate at `x ∈ [x_min, x_max]` with `x_min=-1` and `x_max=x_end` large enough that tail energy outside is negligible. Then use `I_tpl = ∫_{x_min}^{x_max} f(x) dx` consistently.

The total flare luminosity is a Poisson shot-noise sum:

`L_flare(t) = Σ_i ℓ(t - t_i; E_i)`

The total stellar XUV luminosity is:

`L_XUV(t) = L_q(t) + L_flare(t)`

where `L_q(t)` is the quiescent/background component.

### 2) Davenport flare template (time profile)

Use the analytic “classical flare” template (Davenport 2014) with rise polynomial on `x ∈ [-1,0]` and decay as a sum of exponentials on `x ≥ 0`.

Implementation requirements:
- Store template coefficients in a single table in `flarebin_template.c`.
- Provide functions:
  - `double fdFlareTpl(double x);`
  - `double fdFlareTplIntegral(double x_min, double x_max);`
  - `double fdFlareTplIntegralSq(double x_min, double x_max);` (optional; for variance/microflare approximations).

### 3) Duration–energy scaling

Define a power-law duration scaling in the **input energy band** (or in XUV; pick one and be consistent):

`τ(E_in) = τ0 * (E_in / E0)^γ`

with parameters `(τ0, E0, γ)` user-configurable. This is required to translate an energy distribution into a distribution of **instantaneous luminosities** and flare overlap statistics.

### 4) Bandpass conversion to XUV energy

Observed FFDs are often in optical bands. Map input energies to XUV energies as:

`E_XUV = C_band * (E_in)^p_band`

Defaults: `p_band=1` and `C_band` user-specified. If the FFD is already in XUV, set `C_band=1`, `p_band=1`.

### 5) Energy consistency with STELLAR baseline (no double-counting)

Interpret `L̄_XUV(t)` from STELLAR as the **long-term mean total XUV luminosity** (including flare energy on average). flarebin must ensure:

`<L_XUV(t)> = L̄_XUV(t)`

Given a modeled flare population, the mean flare power is:

`P_flare(t) = <L_flare(t)> = ∫_{E_min}^{E_max} r(E_in,t) * E_XUV(E_in) dE_in`

Then enforce:

`L_q(t) = L̄_XUV(t) - P_flare(t)`

This guarantees `<L_q + L_flare> = L̄_XUV` and avoids double-counting flare energy if STELLAR already represents a mean.

If `L_q(t) < 0`, the configuration is invalid. flarebin must either:
- abort with a diagnostic (default), or
- renormalize the flare population downward to satisfy `P_flare <= L̄_XUV` (explicitly enabled behavior).

### 6) Effective-averaged forcing for slow ODEs

Let `y` be any slow state variable (e.g., atmospheric mass, composition proxies) evolved by VPLanet with an ODE:

`dy/dt = g(y, F_XUV(t), other forcing)`

Assume flare timescales are short compared to the ODE timestep and to the timescale on which `y` changes. Then approximate:

`dy/dt = < g(y, F_XUV, ...) >`

where the expectation is taken over the **stationary distribution of the instantaneous flux** at a randomly sampled time within the timestep, holding `y` fixed during the averaging. This is a standard stochastic-averaging / homogenization assumption; it is valid when:
- `τ_flare << Δt_ODE`, and
- `|Δy|` over `Δt_ODE` is small enough that `g(y,·)` is well-approximated by freezing `y` during the expectation.

flarebin does not compute `<g>` itself (to avoid hard-wiring ATMESC physics). Instead it provides a generic expected-value operator that other modules call with their own callback `g(F)`.

## Parameters

Naming follows VPLanet style. The set below is intentionally focused on effective averages.

### A) System options

- `iFlareBinSeed` (int, default 0): seed used only for optional Monte Carlo validation outputs. The production effective-averaging path must be deterministic and not depend on RNG.
- `iFlareBinQuadNE` (int, default 24): number of quadrature nodes in log-energy.
- `iFlareBinQuadNX` (int, default 16): number of quadrature nodes in template phase `x`.
- `iFlareBinMaxOverlapN` (int, default 1): maximum overlap order used in Poisson-mixture expansion (0, 1, or 2). Default 1 is the “single-active-flare” approximation; set 2 for overlap correction when needed.
- `dFlareBinOverlapTol` (double, default 1e-6): maximum allowed neglected probability mass `P(N > N_max)`; flarebin can auto-increase `N_max` up to 2 or warn.

### B) Body options (star body only)

Population model:
- `iFlareBinDist` (enum): `POWERLAW_FFD` or `LOGNORMAL`.
- `iFlareBinNormMode` (enum):
  - `NORM_FROM_FFD` (use user-provided normalization),
  - `NORM_FROM_FLAREPOWER_FRACTION` (solve normalization so `P_flare = dFlareBinFrac * L̄_XUV`),
  - `NORM_FROM_RATE_AT_E0` (use `ν(>E0)`),
  - `NORM_DAVENPORT2019` (age-evolving `a(t),b(t)` from Davenport et al. 2019 Eq. 2-3).
- `dFlareBinFrac` (double): target mean flare power fraction of `L̄_XUV` (only for `NORM_FROM_FLAREPOWER_FRACTION`).

Energy band and bounds:
- `iFlareBinBandPass` (enum): `XUV`, `Kepler`, `TESS`, `Bolometric`.
- `dFlareBinEmin` (energy): lower bound of the modeled flare population in the input band.
- `dFlareBinEmax` (energy): upper bound (required when power-law slope implies divergent moments without truncation).
- `dFlareBinEStochMin` (energy): lower bound of the *stochastically represented* population used to build the instantaneous-flux distribution. Flares below `EStochMin` are included **only through their mean power** in `L_q` (no variability contribution). This parameter is the main performance/accuracy knob.

FFD parameters:
- Power-law form: `dFlareBinAlpha` (differential index in `dN/dE ∝ E^{-α}`) and either `dFlareBinK` or `(dFlareBinSlope, dFlareBinYint)` for cumulative form `log10 ν(>E)=a log10 E + b`.
- Lognormal form: `dFlareBinLogMu`, `dFlareBinLogSigma`, plus a total rate `dFlareBinRateTot` (events/time integrated over `[Emin,Emax]`).

Duration scaling:
- `dFlareBinTau0` (time): `τ0`.
- `dFlareBinDurE0` (energy): `E0`.
- `dFlareBinDurExp` (double): `γ`.

Template truncation:
- `dFlareBinXMin` (double, default -1): template start in units of `τ`.
- `dFlareBinXEnd` (double, default 20): template end in units of `τ` (must be large enough that omitted tail energy is negligible).

Band conversion:
- `dFlareBinBandC` (double): `C_band`.
- `dFlareBinBandP` (double): `p_band`.

Diagnostics thresholds (optional):
- `dFlareBinFXUVThresh1`, `dFlareBinFXUVThresh2` (flux units): optional thresholds used only for output diagnostics (probability of exceeding thresholds), not required for physics integration.

Age-evolving Davenport mode (`NORM_DAVENPORT2019`)
:
- Cumulative FFD form (reverse cumulative):
  `log10 ν = a log10 ε + b` with `ε` in `erg` and `ν` in `flares/day`.
- Evolution law:
  `a = a1 log10 t + a2 m + a3`, `b = b1 log10 t + b2 m + b3`,
  with `t` in `Myr`, `m = M/Msun`.
- Coefficients (Table 1 medians): `a1=-0.07`, `a2=0.79`, `a3=-1.06`,
  `b1=2.01`, `b2=-25.15`, `b3=33.99`.
- Runtime guards:
  `t > 0` required (`log10 t`), and computed `a < 0` required.

Negative-unit input support
:
- Energies (`dFlareBinEmin`, `dFlareBinEmax`, `dFlareBinEStochMin`,
  `dFlareBinDurE0`): negative values are parsed as `ergs`.
- Timescale (`dFlareBinTau0`): negative values are parsed as `days`.
- Rate (`dFlareBinRateTot`): negative values are parsed as `/day`.
- New diagnostics outputs: `FlareBinDavenportA`, `FlareBinDavenportB`.

## Algorithm Design

### 1) Rate density `r(E)`

All calculations are performed in the input energy band `E_in`, with conversion `E_XUV(E_in)` applied when needed.

#### Power-law FFD (truncated)

Use:

`r(E) = k * E^{-α}` for `E ∈ [Emin, Emax]`

where `k` has units events/time/energy.

If the user provides cumulative form `ν(>E)=10^b E^a` (with `E` in SI), then:
- `α = 1 - a`
- `k = 10^b * (-a)`

For Davenport mode (defined in `erg` and `day` units), flarebin converts exactly
to SI `r(E[J]) = k E^{-α}` using:
- `α = 1 - a`
- `log10(k) = log10(-a) + b + 7*a - log10(DAYSEC)`

Consistency requirements:
- If `α <= 2` (common for active M dwarfs), `Emax` must be finite to keep the mean power finite.

Mean flare power:
`P_flare = ∫_{Emin}^{Emax} r(E) * E_XUV(E) dE`

For `E_XUV = C E^p`:
`P_flare = k*C * ∫ E^{p-α} dE` (analytic if `p-α != -1`).

#### Lognormal (truncated)

Let `ln E ~ Normal(μ, σ)` with truncation to `[Emin,Emax]`. Define a normalized PDF `p(E)` and a total rate `λ_tot` (events/time):

`r(E) = λ_tot * p(E)`

Mean flare power:
`P_flare = λ_tot * ∫ p(E) E_XUV(E) dE`

Compute the truncated-lognormal normalization and moments in log-space for numerical stability.

### 2) Effective averaging operator using a Poisson-mixture expansion

flarebin must provide an operator that evaluates:

`< g(F_XUV) >`

for any callback `g` supplied by another module.

#### 2.1) Define quiescent luminosity and macroflare “stochastic” range

Compute mean flare power in the stochastic range `[EStochMin, Emax]`:

`P_stoch = ∫_{EStochMin}^{Emax} r(E) * E_XUV(E) dE`

Then define:

`L_q = L̄_XUV - P_stoch`

and represent instantaneous luminosity as:

`L_XUV(t) = L_q + L_stoch(t)`

where `L_stoch(t)` is the shot noise from flares with `E >= EStochMin` only.

This ensures:
`<L_XUV> = L̄_XUV` and avoids representing a prohibitively large number of microflares.

#### 2.2) Stationary overlap statistics and expansion

Define the template support length (in physical time) as:

`T(E) = τ(E) * (x_end - x_min)`

Define the expected number of overlapping stochastic flares at a random time:

`μ = ∫_{EStochMin}^{Emax} r(E) * T(E) dE`

Let `N ~ Poisson(μ)` be the number of stochastic flares “active” at that random time under the truncated-support approximation.

Then for a planet at distance `a`, the instantaneous XUV flux is:

`F = (L_q + Σ_{i=1}^N A(E_i) * f(x_i)) / (4π a^2)`

where:
- `x_i ~ Uniform(x_min, x_end)`
- `A(E) = E_XUV(E) / (τ(E) * I_tpl)`
- `E_i` distribution follows from the Poisson random measure (see below).

The expected value of `g(F)` can be written as a Poisson-mixture expansion:

`<g(F)> = Σ_{n=0}^{∞} P(N=n) * < g(F) | N=n >`

with `P(N=n)=exp(-μ) μ^n / n!`.

For implementation, truncate at `n <= N_max` where `P(N>N_max) <= dFlareBinOverlapTol`. Default `N_max=1`.

#### 2.3) Avoid division by μ for small-μ robustness

Compute the n=0 and n=1 contributions in *unnormalized* form to avoid numerical issues when μ is tiny.

For `N_max >= 1`, the n=1 contribution can be written as:

`C1 = exp(-μ) * ∫_{EStochMin}^{Emax} r(E) T(E) * [ (1/Δx) ∫_{x_min}^{x_end} g(F_q + F_A(E,x)) dx ] dE`

where:
- `Δx = x_end - x_min`
- `F_q = L_q / (4π a^2)`
- `F_A(E,x) = (A(E) f(x)) / (4π a^2)`

This expression is equivalent to `P(N=1) * E[g | N=1]` but does not require computing `p(E|active)=r(E)T(E)/μ`.

Similarly, the n=0 contribution is:
`C0 = exp(-μ) * g(F_q)`

For `N_max=1`:
`<g(F)> ≈ C0 + C1`

For `N_max=2`, add:

`C2 = exp(-μ)/2 * ∬ r(E1)T(E1) r(E2)T(E2) * [ (1/Δx^2) ∬ g(F_q + F_A(E1,x1)+F_A(E2,x2)) dx1 dx2 ] dE1 dE2`

### 3) Deterministic quadrature

All integrals must be computed deterministically (no per-step RNG). Use fixed quadrature nodes and weights.

Energy integration:
- Integrate in `u = ln E` to handle decades of energies.
- Precompute Gauss–Legendre nodes `{u_j,w_j}` on `[ln(E_lo), ln(E_hi)]`.
- Convert: `E_j = exp(u_j)` and `dE = E du`.

Template phase integration:
- Precompute Gauss–Legendre nodes `{x_k, wx_k}` on `[x_min, x_end]`.

Inner integral for n=1:
`I1(E_j) ≈ (1/Δx) * Σ_k wx_k * g(F_q + F_A(E_j, x_k))`

Then:
`C1 ≈ exp(-μ) * Σ_j wE_j * r(E_j)T(E_j) * I1(E_j)`

where `wE_j` incorporates the Jacobian `E_j` from `dE`.

Compute μ with the same quadrature for consistency:
`μ ≈ Σ_j wE_j * r(E_j)T(E_j)`

Compute `P_stoch` similarly:
`P_stoch ≈ Σ_j wE_j * r(E_j) * E_XUV(E_j)`

### 4) Optional: sampling formulas (validation only)

If you need to generate random flare energies (e.g., to validate the quadrature against Monte Carlo), use:

Power-law sampling (α ≠ 1) on `[Emin,Emax]`:
`E = ( Emin^{1-α} + u*(Emax^{1-α}-Emin^{1-α}) )^{1/(1-α)}`

Lognormal sampling:
`E = exp( μ + σ z )` with `z ~ Normal(0,1)` (use truncated-normal inversion for `[Emin,Emax]`).

These sampling paths must not be used to drive the production ODE RHS because it introduces noise into the integrator.

## Integration Points

This section lists required source-tree changes consistent with VPLanet’s architecture.

### 1) New files / directory structure

Add:
- `src/flarebin.h`
- `src/flarebin.c` (module glue: options, Verify, output registration)
- `src/flarebin_effavg.c` (effective-averaging core: μ, P_stoch, deterministic quadrature, expectation operator)
- `src/flarebin_template.c` (Davenport template evaluation + integrals)
- `src/flarebin_ffd.c` (FFD math: r(E), normalization, moments)

### 2) Module registration

In `src/vplanet.h`:
- Add a module bit `FLAREBIN`.
- Add `int bFlareBin;` and flarebin configuration fields to `BODY` for star bodies.
- Add derived fields to store `L_q`, `P_stoch`, `μ`, `I_tpl`, `Δx`, and quadrature settings.

In `src/module.c`:
- Allow `"flarebin"` in the module list.
- Implement `AddModuleFlareBin` and ensure it is activated for star bodies only.
- flarebin must run after STELLAR so it can access `L̄_XUV(t)`.

### 3) STELLAR

No changes required if STELLAR continues to write the baseline `dLXUV` as `L̄_XUV`. flarebin reads it and computes derived quantities, but does not need to overwrite `dLXUV` for effective-averaging mode.

### 4) evolve.c / time stepping

No timestep constraint logic is required.

However, to avoid recomputing quadrature-derived statistics multiple times per RK4 stage, add a **pre-step cache hook**:
- In the main integrator loop, once per accepted step at time `t_n` (and optionally per RK4 stage time if desired), call:

`fvFlareBinPrecompute(body, system, iStar, t_eval)`

This function updates:
- `L̄_XUV(t_eval)` (read from `body[iStar].dLXUV`),
- `P_stoch(t_eval)`, `L_q(t_eval)`,
- `μ(t_eval)`,
- cached quantities needed for fast expectation evaluation.

### 5) ATMESC (required modifications)

ATMESC must be modified to consume effective-averaged forcing.

Pattern:
- Factor any dependence of atmospheric-escape RHS on `F_XUV` into a pure function (no side effects):

```c
typedef struct {
  double dMdot;      // or a vector of RHS terms
  /* add other RHS components if needed */
} ATMESC_RHS;

ATMESC_RHS AtmEscRhsGivenFXUV(const BODY *body, const SYSTEM *system, int iBody, double dFXUV);
```

- Then, in the ATMESC derivative evaluation for each planet, replace direct use of instantaneous `dFXUV` by:

```c
ATMESC_RHS rhs_eff = FlareBinExpectAtmEscRhs(body, system, iStar, iBody);
```

Implementation options:
1) A generic expectation operator with callback(s):

```c
typedef ATMESC_RHS (*FN_ATMESC_RHS)(const BODY*, const SYSTEM*, int, double);

ATMESC_RHS FlareBinExpectRhs_AtmEsc(
    const BODY *body, const SYSTEM *system,
    int iStar, int iBody,
    FN_ATMESC_RHS fn);
```

2) A specialized function that computes the expectation internally for ATMESC only (less modular but simpler).

Either way, the expectation must use the deterministic quadrature described above so that the RHS is deterministic for a given state and time.

This automatically captures threshold-crossing physics (e.g., oxygen drag activation) because ATMESC’s own piecewise logic is evaluated inside the integral `<g>`.

### 6) system.c / flux computation

No changes are required for effective-averaging mode, because VPLanet’s global flux values can remain at the baseline mean. flarebin only affects modules that explicitly request effective averaging.

## Outputs

Outputs are designed for debugging and for analyzing flare impacts in a long-run effective-averaging regime.

### Star-level outputs (flarebin)

- `LXUVMean` : `L̄_XUV` (baseline from STELLAR)
- `LXUVQuiescent` : `L_q`
- `FlareBinPStoch` : `P_stoch` (mean power in the stochastically represented range)
- `FlareBinMuActive` : `μ` (expected overlapping stochastic flares at a random time)
- `FlareBinPAnyActive` : `1 - exp(-μ)`
- `FlareBinEStochMin` : `EStochMin`
- `FlareBinItemplate` : `I_tpl` (for traceability)

### Planet-level diagnostics (optional, flarebin)

Given planet-star separation `a`:
- `FXUVMean` : `L̄_XUV/(4π a^2)` (already available elsewhere; duplicated for convenience)
- `FXUVQuiescent` : `L_q/(4π a^2)`
- `FlareBinPFXUVAbove1` : `P(FXUV > dFlareBinFXUVThresh1)`
- `FlareBinPFXUVAbove2` : `P(FXUV > dFlareBinFXUVThresh2)`

Probability diagnostics are computed using the same Poisson-mixture/quadrature machinery with `g(F)=1[F>F_thr]`.

### ATMESC outputs (recommended)

ATMESC should output both its baseline and effective-averaged rates:
- `Mdot_XUV_MeanForcing` (computed with `F=FXUVMean`)
- `Mdot_XUV_Effective` (computed with `<g>`)
- Additional regime flags averaged or “fraction of time in regime” diagnostics can be produced with indicator functions inside the expectation operator.

## Performance Considerations

Primary scaling driver is the quadrature evaluation count per planet per RHS evaluation.

For `N_max=1`, the expected-value evaluation cost is approximately:
- `O(N_E * N_X)` calls to the module callback `g(F)` (typically ATMESC RHS function).

Typical values:
- `N_E=24`, `N_X=16`  ⇒ 384 evaluations per RHS call (plus one background evaluation).

This is feasible for systems with few planets and long timesteps, and is orders of magnitude cheaper than time-resolving flare minute-scale dynamics over hundreds of Myr.

Performance knobs:
- Increase `EStochMin` to reduce μ and reduce the importance of high-order overlaps.
- Reduce `N_E` and `N_X` when convergence tests show stability.
- Use `N_max=1` unless `μ` is large enough that `P(N>=2)` is non-negligible. flarebin should compute `P(N>1)=1-exp(-μ)(1+μ)` and warn if it exceeds tolerance.

Caching:
- Cache `{E_j, wE_j}` and `{x_k, wx_k}`, and template values `f(x_k)`.
- Cache per star per step: `L̄_XUV`, `P_stoch`, `L_q`, `μ`, and any quantities derived from them.
- Do not cache values that depend on planet state `y` (those must be recomputed via the callback).

## Pseudocode

### Initialization (star body)

```text
InitFlareBin(star):
  Read options
  Precompute template f(x) and I_tpl over [x_min, x_end]
  Build deterministic quadrature grids:
    {u_j,w_j} on [ln(EStochMin), ln(Emax)]   (energy)
    {x_k,wx_k} on [x_min, x_end]            (phase)
  Store grids and f(x_k) in BODY (star)
  Normalize FFD:
    - If NORM_FROM_FLAREPOWER_FRACTION:
        Solve for normalization so that P_flare = dFlareBinFrac * L̄_XUV(t0)
    - Else: use provided normalization
  Verify bounds and consistency (Emax finite if needed)
```

### Pre-step cache update (per accepted integrator step)

```text
FlareBinPrecompute(star, t_eval):
  Lbar = body[star].dLXUV            // STELLAR baseline mean at t_eval
  Compute P_stoch via quadrature:
    P_stoch = Σ_j wE_j * r(E_j,t_eval) * E_XUV(E_j)
  Lq = Lbar - P_stoch
  if Lq < 0: abort or renormalize (config-dependent)
  Compute μ via quadrature:
    μ = Σ_j wE_j * r(E_j,t_eval) * T(E_j)
  Cache {Lbar, P_stoch, Lq, μ}
```

### Generic expected-value operator for a planet

```text
FlareBinExpect(body, system, star, planet, g):
  a = orbital distance (or instantaneous distance if needed)
  Fq = Lq / (4π a^2)
  μ  = cached μ

  C0 = exp(-μ) * g(Fq)

  // n=1 contribution (default)
  C1 = exp(-μ) * Σ_j [ wE_j * r(E_j) * T(E_j) * (1/Δx) * Σ_k wx_k * g( Fq + (A(E_j)*f(x_k))/(4π a^2) ) ]

  if N_max == 1:
    return C0 + C1

  // optional n=2 correction
  C2 = exp(-μ)/2 * Σ_{j1,j2} [ wE_j1 r(E_j1)T(E_j1) wE_j2 r(E_j2)T(E_j2)
                              * (1/Δx^2) * Σ_{k1,k2} wx_k1 wx_k2
                              * g(Fq + FA(E_j1,x_k1) + FA(E_j2,x_k2)) ]
  return C0 + C1 + C2
```

where:
- `A(E) = E_XUV(E) / (τ(E)*I_tpl)`
- `FA(E,x) = (A(E)*f(x)) / (4π a^2)`
- `Δx = x_end - x_min`

### Convergence / validation (optional)

```text
Validate:
  Compare FlareBinExpect(g(F)=F) against mean FXUV = Lbar/(4πa^2)
  Compare indicator-function expectations against Monte Carlo with fixed seed
  Increase (N_E,N_X) until changes are below tolerance
```
