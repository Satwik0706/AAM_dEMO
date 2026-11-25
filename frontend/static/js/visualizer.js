/**
 * String Matching Workbench — Visualizer, Flow & Complexity Derivation Engine
 * Algorithms: Naive, Rabin-Karp, Finite Automata, Knuth-Morris-Pratt (KMP)
 */

const PRESETS = {
  clrs: { text: "AABAACAADAABAABA", pattern: "AABA" },
  dna: { text: "GATATATGCATATACTT", pattern: "ATATA" },
  repetitive: { text: "AAAAAAAAAAAAAAAAAB", pattern: "AAAAAB" },
  text: { text: "The quick brown fox jumps over the lazy fox", pattern: "fox" },
  binary: { text: "01001010110101001010", pattern: "0101" }
};

const ALGO_NAMES = {
  naive: "Naive String Matching",
  rabin_karp: "Rabin-Karp Algorithm",
  finite_automata: "Finite Automata Matcher",
  kmp: "Knuth-Morris-Pratt (KMP)"
};

const state = {
  text: "AABAACAADAABAABA",
  pattern: "AABA",
  currentAlgo: "kmp",
  apiData: null,
  activeSteps: [],
  currentStepIndex: 0,
  filterMatchesOnly: false,
  isPlaying: false,
  timerId: null,
  speedMs: 500
};

// DOM References (verified against index.html)
const elText = document.getElementById("inputTargetText");
const elPattern = document.getElementById("inputPattern");
const elTextLen = document.getElementById("textLengthBadge");
const elPatternLen = document.getElementById("patternLengthBadge");
const elRunBtn = document.getElementById("btnRunSimulation");
const elResetBtn = document.getElementById("btnResetAll");
const elCaseSensitive = document.getElementById("checkCaseSensitive");
const elEngineStatus = document.getElementById("engineStatusBadge");

const elBtnPrev = document.getElementById("btnStepPrev");
const elBtnNext = document.getElementById("btnStepNext");
const elBtnPlay = document.getElementById("btnPlayPause");
const elBtnRestart = document.getElementById("btnRestartStep");
const elPlayText = document.getElementById("playText");
const elSpeedSlider = document.getElementById("sliderSpeed");
const elSpeedLabel = document.getElementById("speedLabel");
const elStepCounter = document.getElementById("stepCounterBadge");
const elProgressBar = document.getElementById("stepProgressBar");

const elTrackText = document.getElementById("trackTextContainer");
const elTrackPattern = document.getElementById("trackPatternContainer");
const elPatternShiftLabel = document.getElementById("patternShiftLabel");
const elExplanation = document.getElementById("liveExplanationText");
const elAlgoLabel = document.getElementById("animatorCurrentAlgoLabel");
const elStepsLogList = document.getElementById("stepsLogList");
const elTotalStepsLabel = document.getElementById("totalStepsLabel");
const elBtnFilterMatches = document.getElementById("btnFilterMatches");
const elHighlightedBase = document.getElementById("highlightedBaseStringContainer");
const elMatchChips = document.getElementById("matchPositionChips");
const elFinalMatchBadge = document.getElementById("finalMatchSummaryBadge");

const elFlowAlgoName = document.getElementById("flowAlgoName");
const elFlowPipeline = document.getElementById("flowPipelineContainer");
const elDerivationBadge = document.getElementById("derivationAlgoBadge");
const elDerivationContainer = document.getElementById("complexityDerivationContainer");

const elStatMatches = document.getElementById("statMatchesCount");
const elStatIndices = document.getElementById("statMatchIndices");
const elStatCompLabel = document.getElementById("statCompLabel");
const elStatComparisons = document.getElementById("statComparisons");
const elStatCompSub = document.getElementById("statCompSub");
const elStatDuration = document.getElementById("statDuration");
const elStatComplexity = document.getElementById("statComplexity");
const elStatSpace = document.getElementById("statSpaceComplexity");

const elPanelKmp = document.getElementById("panelKmpPi");
const elPanelFa = document.getElementById("panelFiniteAutomata");
const elPanelRk = document.getElementById("panelRabinKarp");
const elPanelNaive = document.getElementById("panelNaive");
const elComparisonBody = document.getElementById("comparisonTableBody");

document.addEventListener("DOMContentLoaded", () => {
  setupEventListeners();
  updateInputBadges();
  runFullExecution();
});

function setupEventListeners() {
  document.querySelectorAll(".preset-chip").forEach((btn) => {
    btn.addEventListener("click", () => {
      const p = PRESETS[btn.dataset.preset];
      if (p) {
        elText.value = p.text;
        elPattern.value = p.pattern;
        updateInputBadges();
        runFullExecution();
      }
    });
  });

  let inputDebounce = null;
  const handleInputChange = () => {
    updateInputBadges();
    state.apiData = null;
    clearTimeout(inputDebounce);
    inputDebounce = setTimeout(() => {
      if (elText.value.length > 0 && elPattern.value.length > 0) {
        runFullExecution();
      }
    }, 250);
  };
  elText.addEventListener("input", handleInputChange);
  elPattern.addEventListener("input", handleInputChange);
  elCaseSensitive.addEventListener("change", () => {
    state.apiData = null;
    runFullExecution();
  });

  document.querySelectorAll(".algo-tab").forEach((tab) => {
    tab.addEventListener("click", () => {
      selectAlgorithm(tab.dataset.algo);
    });
  });

  elRunBtn.addEventListener("click", runFullExecution);
  elResetBtn.addEventListener("click", () => {
    elText.value = PRESETS.clrs.text;
    elPattern.value = PRESETS.clrs.pattern;
    elCaseSensitive.checked = true;
    updateInputBadges();
    runFullExecution();
  });

  elBtnPlay.addEventListener("click", togglePlayPause);
  elBtnNext.addEventListener("click", () => { pause(); stepForward(); });
  elBtnPrev.addEventListener("click", () => { pause(); stepBackward(); });
  elBtnRestart.addEventListener("click", () => { pause(); goToStep(0); });
  elBtnFilterMatches.addEventListener("click", () => {
    state.filterMatchesOnly = !state.filterMatchesOnly;
    elBtnFilterMatches.classList.toggle("bg-emerald-50", state.filterMatchesOnly);
    elBtnFilterMatches.classList.toggle("text-emerald-700", state.filterMatchesOnly);
    elBtnFilterMatches.classList.toggle("border-emerald-300", state.filterMatchesOnly);
    renderStepsLog();
  });

  elSpeedSlider.addEventListener("input", (e) => {
    state.speedMs = parseInt(e.target.value, 10);
    elSpeedLabel.textContent = `${(state.speedMs / 1000).toFixed(1)}s`;
    if (state.isPlaying) {
      pause();
      play();
    }
  });

  document.addEventListener("keydown", (e) => {
    if (document.activeElement === elText || document.activeElement === elPattern) return;
    if (e.code === "Space") {
      e.preventDefault();
      togglePlayPause();
    } else if (e.code === "ArrowRight") {
      e.preventDefault();
      pause();
      stepForward();
    } else if (e.code === "ArrowLeft") {
      e.preventDefault();
      pause();
      stepBackward();
    }
  });
}

function selectAlgorithm(algoKey) {
  pause();
  state.currentAlgo = algoKey;
  document.querySelectorAll(".algo-tab").forEach((t) => {
    t.classList.toggle("active", t.dataset.algo === algoKey);
  });
  const text = elCaseSensitive.checked ? elText.value : elText.value.toLowerCase();
  const pattern = elCaseSensitive.checked ? elPattern.value : elPattern.value.toLowerCase();
  const isStale = !state.apiData || !state.apiData.meta ||
    state.apiData.meta.text !== text || state.apiData.meta.pattern !== pattern;
  if (isStale) {
    runFullExecution();
  } else {
    renderCurrentAlgorithmView();
  }
}

function updateInputBadges() {
  state.text = elText.value;
  state.pattern = elPattern.value;
  elTextLen.textContent = `n = ${state.text.length}`;
  elPatternLen.textContent = `m = ${state.pattern.length}`;
}

// ============================================================================
// CLIENT-SIDE FULL COMPUTATION FALLBACK (accurate for all 4 algorithms)
// ============================================================================
function computeAllClientSide(text, pattern) {
  const n = text.length;
  const m = pattern.length;

  // 1. Naive
  const t0 = performance.now();
  const naiveMatches = [];
  const naiveSteps = [];
  let naiveComp = 0;
  for (let s = 0; s <= n - m; s++) {
    let ok = true;
    for (let j = 0; j < m; j++) {
      naiveComp++;
      const isM = text[s + j] === pattern[j];
      naiveSteps.push({
        step_number: naiveSteps.length + 1,
        shift: s,
        pattern_index: j,
        text_index: s + j,
        char_text: text[s + j],
        char_pattern: pattern[j],
        is_char_match: isM,
        state: "checking",
        explanation: `Compare T[${s + j}]='${text[s + j]}' with P[${j}]='${pattern[j]}' (shift=${s}).`
      });
      if (!isM) {
        ok = false;
        naiveSteps.push({
          step_number: naiveSteps.length + 1,
          shift: s,
          pattern_index: j,
          text_index: s + j,
          char_text: text[s + j],
          char_pattern: pattern[j],
          is_char_match: false,
          state: "mismatch",
          explanation: `Mismatch: '${text[s + j]}' != '${pattern[j]}'. Advance shift to ${s + 1}.`
        });
        break;
      }
    }
    if (ok && m > 0) {
      naiveMatches.push(s);
      naiveSteps.push({
        step_number: naiveSteps.length + 1,
        shift: s,
        pattern_index: m - 1,
        text_index: s + m - 1,
        char_text: text[s + m - 1],
        char_pattern: pattern[m - 1],
        is_char_match: true,
        state: "found",
        explanation: `Match found at shift ${s} (1-based: ${s + 1}).`
      });
    }
  }
  const naiveUs = Math.max(1, Math.round((performance.now() - t0) * 1000 * 10) / 10);

  // 2. Rabin-Karp
  const t1 = performance.now();
  const d = 256, q = 101;
  let h = 1;
  for (let i = 0; i < m - 1; i++) h = (h * d) % q;
  let pHash = 0, wHash = 0, rkHashes = 0, rkComp = 0, spurious = 0;
  const rkMatches = [], rkSteps = [];
  if (m > 0 && n >= m) {
    for (let i = 0; i < m; i++) {
      pHash = (d * pHash + pattern.charCodeAt(i)) % q;
      wHash = (d * wHash + text.charCodeAt(i)) % q;
      rkHashes += 2;
    }
    rkSteps.push({
      step_number: 1, shift: 0, pattern_hash: pHash, window_hash: wHash,
      hash_match: pHash === wHash, state: "init",
      explanation: `Initial hashes: H(P)=${pHash}, H(T[0..${m - 1}])=${wHash} (d=${d}, q=${q}).`
    });
    for (let s = 0; s <= n - m; s++) {
      const hashMatch = pHash === wHash;
      rkSteps.push({
        step_number: rkSteps.length + 1, shift: s, pattern_hash: pHash, window_hash: wHash,
        hash_match: hashMatch, state: "hash_check",
        explanation: `Shift ${s}: Window hash ${wHash} vs Pattern hash ${pHash} -> ${hashMatch ? "Hash match! Verify chars." : "Hashes differ, skip."}`
      });
      if (hashMatch) {
        let ok = true;
        for (let j = 0; j < m; j++) {
          rkComp++;
          if (text[s + j] !== pattern[j]) {
            ok = false;
            spurious++;
            rkSteps.push({
              step_number: rkSteps.length + 1, shift: s, text_index: s + j, pattern_index: j,
              pattern_hash: pHash, window_hash: wHash, state: "spurious_hit",
              explanation: `Spurious hit at shift ${s}: hash matched (${wHash}) but T[${s + j}] != P[${j}].`
            });
            break;
          }
        }
        if (ok) {
          rkMatches.push(s);
          rkSteps.push({
            step_number: rkSteps.length + 1, shift: s, text_index: s + m - 1, pattern_index: m - 1,
            pattern_hash: pHash, window_hash: wHash, state: "found",
            explanation: `Match verified at shift ${s} (hash=${wHash}).`
          });
        }
      }
      if (s < n - m) {
        const prev = wHash;
        wHash = (d * (wHash - text.charCodeAt(s) * h) + text.charCodeAt(s + m)) % q;
        if (wHash < 0) wHash += q;
        rkHashes++;
        rkSteps.push({
          step_number: rkSteps.length + 1, shift: s + 1, pattern_hash: pHash, window_hash: wHash,
          state: "rolling_hash",
          explanation: `Rolling hash O(1): ${prev} -> ${wHash} (out '${text[s]}', in '${text[s + m]}').`
        });
      }
    }
  }
  const rkUs = Math.max(1, Math.round((performance.now() - t1) * 1000 * 10) / 10);

  // 3. Finite Automata
  const t2 = performance.now();
  const alphabet = Array.from(new Set((text + pattern).split(""))).sort();
  const transitionTable = [];
  for (let stateQ = 0; stateQ <= m; stateQ++) {
    const row = {};
    for (const a of alphabet) {
      let k = Math.min(m, stateQ + 1);
      const pqa = pattern.slice(0, stateQ) + a;
      while (k > 0 && !pqa.endsWith(pattern.slice(0, k))) k--;
      row[a] = k;
    }
    transitionTable.push(row);
  }
  const faMatches = [], faSteps = [];
  let faTransitions = 0;
  faSteps.push({
    step_number: 1, text_index: -1, prev_state: 0, curr_state: 0, state: "init",
    explanation: `DFA built with ${m + 1} states and |Sigma|=${alphabet.length}. Accepting state = ${m}.`
  });
  let qState = 0;
  for (let i = 0; i < n; i++) {
    const ch = text[i];
    const prevQ = qState;
    qState = transitionTable[prevQ][ch] ?? 0;
    faTransitions++;
    if (qState === m) {
      const shift = i - m + 1;
      faMatches.push(shift);
      faSteps.push({
        step_number: faSteps.length + 1, text_index: i, pattern_index: m - 1, shift: shift,
        char_text: ch, prev_state: prevQ, curr_state: qState, state: "found",
        explanation: `T[${i}]='${ch}': delta(${prevQ}, '${ch}') -> ${qState} [ACCEPT]. Match at shift ${shift}.`
      });
    } else {
      faSteps.push({
        step_number: faSteps.length + 1, text_index: i, pattern_index: Math.max(0, qState - 1),
        shift: Math.max(0, i - qState + 1), char_text: ch, prev_state: prevQ, curr_state: qState, state: "transition",
        explanation: `T[${i}]='${ch}': delta(${prevQ}, '${ch}') -> State ${qState}.`
      });
    }
  }
  const faUs = Math.max(1, Math.round((performance.now() - t2) * 1000 * 10) / 10);

  // 4. KMP
  const t3 = performance.now();
  const pi = new Array(m).fill(0);
  let k = 0;
  for (let i = 1; i < m; i++) {
    while (k > 0 && pattern[k] !== pattern[i]) k = pi[k - 1];
    if (pattern[k] === pattern[i]) k++;
    pi[i] = k;
  }
  const kmpMatches = [], kmpSteps = [];
  let kmpComp = 0, kmpShifts = 0;
  kmpSteps.push({
    step_number: 1, text_index: -1, pattern_index: -1, shift: 0, state: "init", pi_table: pi,
    explanation: `Precomputed prefix table pi = [${pi.join(", ")}].`
  });
  let qK = 0;
  for (let i = 0; i < n; i++) {
    while (qK > 0 && pattern[qK] !== text[i]) {
      kmpComp++;
      const oldQ = qK;
      qK = pi[qK - 1];
      kmpShifts++;
      kmpSteps.push({
        step_number: kmpSteps.length + 1, text_index: i, pattern_index: oldQ, shift: i - qK,
        char_text: text[i], char_pattern: pattern[oldQ], state: "pi_fallback",
        explanation: `Mismatch T[${i}]='${text[i]}' != P[${oldQ}]='${pattern[oldQ]}'. Fallback via pi[${oldQ - 1}]=${qK}.`
      });
    }
    kmpComp++;
    if (pattern[qK] === text[i]) {
      kmpSteps.push({
        step_number: kmpSteps.length + 1, text_index: i, pattern_index: qK, shift: i - qK,
        char_text: text[i], char_pattern: pattern[qK], state: "matched_char",
        explanation: `Matched T[${i}]='${text[i]}' == P[${qK}] (${qK + 1}/${m}).`
      });
      qK++;
    } else {
      kmpSteps.push({
        step_number: kmpSteps.length + 1, text_index: i, pattern_index: 0, shift: i,
        char_text: text[i], char_pattern: pattern[0], state: "mismatch",
        explanation: `Mismatch T[${i}]='${text[i]}' != P[0]='${pattern[0]}'. Advance text pointer.`
      });
    }
    if (qK === m && m > 0) {
      const shift = i - m + 1;
      kmpMatches.push(shift);
      kmpSteps.push({
        step_number: kmpSteps.length + 1, text_index: i, pattern_index: m - 1, shift: shift,
        char_text: text[i], char_pattern: pattern[m - 1], state: "found",
        explanation: `Match found at shift ${shift}. Resume with q = pi[${m - 1}]=${pi[m - 1]}.`
      });
      qK = pi[qK - 1];
      kmpShifts++;
    }
  }
  const kmpUs = Math.max(1, Math.round((performance.now() - t3) * 1000 * 10) / 10);

  return {
    meta: { text, pattern, text_length: n, pattern_length: m },
    naive: { algorithm: ALGO_NAMES.naive, matches: naiveMatches, comparisons: naiveComp, execution_time_us: naiveUs, steps: naiveSteps, time_complexity: "O((n - m + 1) * m)", space_complexity: "O(1)" },
    rabin_karp: { algorithm: ALGO_NAMES.rabin_karp, matches: rkMatches, comparisons: rkComp, hash_computations: rkHashes, spurious_hits: spurious, pattern_hash: pHash, execution_time_us: rkUs, steps: rkSteps, time_complexity: "Avg O(n + m), Worst O(n * m)", space_complexity: "O(1)" },
    finite_automata: { algorithm: ALGO_NAMES.finite_automata, matches: faMatches, state_transitions: faTransitions, alphabet: alphabet, transition_table: transitionTable, execution_time_us: faUs, steps: faSteps, time_complexity: "Match O(n), Prep O(m * |Sigma|)", space_complexity: "O(m * |Sigma|)" },
    kmp: { algorithm: ALGO_NAMES.kmp, matches: kmpMatches, comparisons: kmpComp, shifts_using_pi: kmpShifts, pi_table: pi, execution_time_us: kmpUs, steps: kmpSteps, time_complexity: "Prep O(m), Match O(n)", space_complexity: "O(m)" },
    comparison: [
      { name: ALGO_NAMES.naive, key: "naive", matches: naiveMatches.length, comparisons: naiveComp, time_us: naiveUs, time_complexity: "O((n-m+1)m)", space_complexity: "O(1)" },
      { name: ALGO_NAMES.rabin_karp, key: "rabin_karp", matches: rkMatches.length, comparisons: rkComp + rkHashes, time_us: rkUs, time_complexity: "O(n+m) avg", space_complexity: "O(1)" },
      { name: ALGO_NAMES.finite_automata, key: "finite_automata", matches: faMatches.length, state_transitions: faTransitions, time_us: faUs, time_complexity: "O(n) match", space_complexity: "O(m|Sigma|)" },
      { name: ALGO_NAMES.kmp, key: "kmp", matches: kmpMatches.length, comparisons: kmpComp, time_us: kmpUs, time_complexity: "O(n) match", space_complexity: "O(m)" }
    ]
  };
}

// ============================================================================
// EXECUTION & DATA FETCH
// ============================================================================
async function runFullExecution() {
  pause();
  updateInputBadges();

  const rawText = state.text;
  const rawPattern = state.pattern;
  if (!rawText || !rawPattern) {
    elExplanation.textContent = "Please enter both a Target Text and a Search Pattern.";
    return;
  }

  const text = elCaseSensitive.checked ? rawText : rawText.toLowerCase();
  const pattern = elCaseSensitive.checked ? rawPattern : rawPattern.toLowerCase();

  if (pattern.length > text.length) {
    elExplanation.textContent = `Pattern length (${pattern.length}) cannot exceed Text length (${text.length}).`;
  }

  try {
    const response = await fetch("/api/match", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ text, pattern, algorithm: "all" })
    });
    if (response.ok) {
      state.apiData = await response.json();
      elEngineStatus.innerHTML = `<span class="w-1.5 h-1.5 rounded-full bg-emerald-600"></span> Flask Backend Connected`;
    } else {
      state.apiData = computeAllClientSide(text, pattern);
    }
  } catch (err) {
    state.apiData = computeAllClientSide(text, pattern);
    elEngineStatus.innerHTML = `<span class="w-1.5 h-1.5 rounded-full bg-amber-500"></span> Local JS Engine`;
  }

  renderCurrentAlgorithmView();
}

function renderCurrentAlgorithmView() {
  updateMetricCards();
  renderComparisonTable();
  setupActiveVisualizerSteps();
  renderKmpPiTable();
  renderFaTable();
  renderRkInfo();
  renderStepsLog();
  renderHighlightedBaseString();
  renderComplexityDerivation();
  goToStep(0);
}

// ============================================================================
// METRICS & COMPARISON TABLE
// ============================================================================
function updateMetricCards() {
  const data = state.apiData[state.currentAlgo];
  if (!data) return;

  const matches = data.matches || [];
  elStatMatches.textContent = matches.length;
  elStatIndices.textContent = matches.length > 0
    ? `Indices: [${matches.join(", ")}]`
    : "No matches found";

  if (state.currentAlgo === "finite_automata") {
    elStatCompLabel.textContent = "DFA State Transitions";
    elStatComparisons.textContent = data.state_transitions ?? state.text.length;
    elStatCompSub.textContent = "1 transition per text character";
  } else if (state.currentAlgo === "rabin_karp") {
    elStatCompLabel.textContent = "Hash + Char Checks";
    elStatComparisons.textContent = `${data.hash_computations ?? 0}h + ${data.comparisons ?? 0}c`;
    elStatCompSub.textContent = `Spurious hits: ${data.spurious_hits ?? 0}`;
  } else {
    elStatCompLabel.textContent = "Character Comparisons";
    elStatComparisons.textContent = data.comparisons ?? 0;
    elStatCompSub.textContent = state.currentAlgo === "kmp"
      ? `Smart pi-shifts: ${data.shifts_using_pi ?? 0}`
      : "Brute-force character checks";
  }

  elStatDuration.innerHTML = `${data.execution_time_us} &micro;s`;
  elStatComplexity.textContent = data.time_complexity || "O(n)";
  elStatSpace.textContent = `Space: ${data.space_complexity || "O(1)"}`;
  elAlgoLabel.textContent = ALGO_NAMES[state.currentAlgo];
}

function renderComparisonTable() {
  const comp = state.apiData.comparison;
  if (!comp || !elComparisonBody) return;

  const maxOps = Math.max(1, ...comp.map((c) => c.comparisons ?? c.state_transitions ?? 1));
  elComparisonBody.innerHTML = "";

  comp.forEach((item) => {
    const isSelected = item.key === state.currentAlgo;
    const ops = item.comparisons ?? item.state_transitions ?? 0;
    const pct = Math.min(100, Math.round((ops / maxOps) * 100));

    const tr = document.createElement("tr");
    tr.className = `cursor-pointer transition ${isSelected ? "bg-blue-50/70" : "hover:bg-neutral-50"}`;
    tr.innerHTML = `
      <td class="py-2.5 px-3 font-sans font-semibold ${isSelected ? "text-blue-700" : "text-neutral-900"}">
        ${isSelected ? "&#9654; " : ""}${item.name}
      </td>
      <td class="py-2.5 px-3 text-emerald-700 font-semibold">${item.matches}</td>
      <td class="py-2.5 px-3">${ops}</td>
      <td class="py-2.5 px-3 w-36">
        <div class="w-full bg-neutral-100 rounded-full h-2 overflow-hidden">
          <div class="h-2 rounded-full ${isSelected ? "bg-blue-600" : "bg-neutral-400"}" style="width: ${pct}%"></div>
        </div>
      </td>
      <td class="py-2.5 px-3">${item.time_us} &micro;s</td>
      <td class="py-2.5 px-3 text-neutral-600">${item.time_complexity}</td>
      <td class="py-2.5 px-3 text-neutral-500">${item.space_complexity}</td>
    `;
    tr.addEventListener("click", () => selectAlgorithm(item.key));
    elComparisonBody.appendChild(tr);
  });
}

// ============================================================================
// FLOW PIPELINE & COMPLEXITY DERIVATION
// ============================================================================
function renderFlowPipeline(step) {
  const algo = state.currentAlgo;
  elFlowAlgoName.textContent = ALGO_NAMES[algo];

  const flows = {
    naive: [
      { id: "shift", label: "1. Align Shift s" },
      { id: "check", label: "2. Compare T[s+j] == P[j]" },
      { id: "mismatch", label: "3. Mismatch -> s + 1" },
      { id: "found", label: "4. Full Match (j == m)" }
    ],
    rabin_karp: [
      { id: "init", label: "1. Compute H(P) & H(T_0)" },
      { id: "hash_check", label: "2. Compare Window Hash" },
      { id: "verify", label: "3. Verify Chars / Spurious Hit" },
      { id: "rolling", label: "4. O(1) Rolling Hash Update" }
    ],
    finite_automata: [
      { id: "init", label: "1. Build Alphabet & delta(q, a)" },
      { id: "read", label: "2. Read Next Char T[i]" },
      { id: "transition", label: "3. State q = delta(q, T[i])" },
      { id: "found", label: "4. Accepting State (q == m)" }
    ],
    kmp: [
      { id: "init", label: "1. Build Prefix Table pi[0..m-1]" },
      { id: "check", label: "2. Compare T[i] == P[q]" },
      { id: "fallback", label: "3. Mismatch -> q = pi[q-1]" },
      { id: "found", label: "4. Match Found (q == m)" }
    ]
  };

  const st = step ? step.state : "init";
  let activeIdx = 0;
  let errorMode = false;

  if (algo === "naive") {
    if (st === "checking") activeIdx = 1;
    else if (st === "mismatch") { activeIdx = 2; errorMode = true; }
    else if (st === "found") activeIdx = 3;
  } else if (algo === "rabin_karp") {
    if (st === "init") activeIdx = 0;
    else if (st === "hash_check") activeIdx = 1;
    else if (st === "spurious_hit") { activeIdx = 2; errorMode = true; }
    else if (st === "found") activeIdx = 2;
    else if (st === "rolling_hash") activeIdx = 3;
  } else if (algo === "finite_automata") {
    if (st === "init") activeIdx = 0;
    else if (st === "transition") activeIdx = 2;
    else if (st === "found") activeIdx = 3;
  } else if (algo === "kmp") {
    if (st === "init") activeIdx = 0;
    else if (st === "matched_char") activeIdx = 1;
    else if (st === "pi_fallback" || st === "mismatch") { activeIdx = 2; errorMode = true; }
    else if (st === "found") activeIdx = 3;
  }

  const nodes = flows[algo] || [];
  elFlowPipeline.innerHTML = nodes.map((n, idx) => {
    let cls = "flow-node";
    if (idx === activeIdx) cls += errorMode ? " flow-error" : " flow-active";
    else if (idx < activeIdx) cls += " flow-done";
    const arrow = idx < nodes.length - 1 ? `<span class="text-neutral-400 font-mono">&rarr;</span>` : "";
    return `<div class="${cls}">${n.label}</div>${arrow}`;
  }).join("");
}

function renderComplexityDerivation() {
  const algo = state.currentAlgo;
  const data = state.apiData[algo];
  if (!data) return;

  const n = state.text.length;
  const m = state.pattern.length;
  const shifts = Math.max(0, n - m + 1);
  elDerivationBadge.textContent = ALGO_NAMES[algo];

  let leftHtml = "", rightHtml = "";

  if (algo === "naive") {
    const worstOps = shifts * m;
    leftHtml = `
      <div class="p-3.5 rounded-md bg-neutral-50 border border-neutral-200 space-y-1.5">
        <div class="font-semibold text-neutral-900">1. Outer &amp; Inner Loop Derivation</div>
        <p class="text-neutral-600 leading-relaxed">
          Outer loop tests <code class="bg-white px-1 border border-neutral-200 rounded">n - m + 1</code> shifts.<br>
          For each shift, the inner loop compares at most <code class="bg-white px-1 border border-neutral-200 rounded">m</code> characters.<br>
          Total Worst-Case Comparisons = <span class="font-mono font-semibold text-neutral-900">(n - m + 1) &times; m</span>
        </p>
      </div>`;
    rightHtml = `
      <div class="p-3.5 rounded-md bg-blue-50/60 border border-blue-200 space-y-1.5">
        <div class="font-semibold text-blue-900">2. Applied to Your Input (n = ${n}, m = ${m})</div>
        <p class="text-blue-950 font-mono leading-relaxed">
          Shifts = ${n} - ${m} + 1 = ${shifts}<br>
          Theoretical Worst Case = ${shifts} &times; ${m} = ${worstOps} comparisons<br>
          Actual Comparisons Executed = <b>${data.comparisons}</b><br>
          Space Used = O(1) auxiliary variables
        </p>
      </div>`;
  } else if (algo === "rabin_karp") {
    const sigma = new Set((state.text + state.pattern).split("")).size;
    leftHtml = `
      <div class="p-3.5 rounded-md bg-neutral-50 border border-neutral-200 space-y-1.5">
        <div class="font-semibold text-neutral-900">1. Preprocessing &amp; Rolling Hash Derivation</div>
        <p class="text-neutral-600 leading-relaxed">
          Initial pattern &amp; window hash takes <span class="font-mono font-semibold">O(m)</span>.<br>
          Rolling hash updates across <code class="bg-white px-1 border border-neutral-200 rounded">n - m</code> shifts take <span class="font-mono font-semibold">O(1)</span> each.<br>
          Total Average Time = <span class="font-mono font-semibold text-neutral-900">O(m) + O(n - m + 1) = O(n + m)</span><br>
          Worst case (all hashes collide) = <span class="font-mono">O(n &middot; m)</span>
        </p>
      </div>`;
    rightHtml = `
      <div class="p-3.5 rounded-md bg-blue-50/60 border border-blue-200 space-y-1.5">
        <div class="font-semibold text-blue-900">2. Applied to Your Input (n = ${n}, m = ${m}, |&Sigma;| = ${sigma})</div>
        <p class="text-blue-950 font-mono leading-relaxed">
          Hash Computations = <b>${data.hash_computations}</b><br>
          Char Verifications = <b>${data.comparisons}</b> (Spurious hits: ${data.spurious_hits})<br>
          Expected Total Work &approx; ${n} + ${m} = ${n + m} operations<br>
          Space Used = O(1)
        </p>
      </div>`;
  } else if (algo === "finite_automata") {
    const sigma = (data.alphabet || []).length;
    const prepOps = (m + 1) * sigma;
    leftHtml = `
      <div class="p-3.5 rounded-md bg-neutral-50 border border-neutral-200 space-y-1.5">
        <div class="font-semibold text-neutral-900">1. Automaton Construction &amp; Matching</div>
        <p class="text-neutral-600 leading-relaxed">
          DFA has <code class="bg-white px-1 border border-neutral-200 rounded">m + 1</code> states and <code class="bg-white px-1 border border-neutral-200 rounded">|&Sigma;|</code> symbols.<br>
          Building &delta;(q, a) takes <span class="font-mono font-semibold">O(m<sup>3</sup>|&Sigma;|)</span> (or <span class="font-mono font-semibold">O(m|&Sigma;|)</span> optimized).<br>
          Matching scans each text char once: <span class="font-mono font-semibold text-neutral-900">&Theta;(n)</span> strictly!
        </p>
      </div>`;
    rightHtml = `
      <div class="p-3.5 rounded-md bg-blue-50/60 border border-blue-200 space-y-1.5">
        <div class="font-semibold text-blue-900">2. Applied to Your Input (n = ${n}, m = ${m}, |&Sigma;| = ${sigma})</div>
        <p class="text-blue-950 font-mono leading-relaxed">
          DFA Table Size = (${m} + 1) &times; ${sigma} = <b>${prepOps} cells</b><br>
          Matching Transitions = <b>${data.state_transitions}</b> (exactly n = ${n})<br>
          Backtracking = 0 (strictly 1 pass)<br>
          Space Used = O(m &middot; |&Sigma;|)
        </p>
      </div>`;
  } else if (algo === "kmp") {
    const bound = 2 * n + 2 * m;
    leftHtml = `
      <div class="p-3.5 rounded-md bg-neutral-50 border border-neutral-200 space-y-1.5">
        <div class="font-semibold text-neutral-900">1. Amortized KMP Derivation</div>
        <p class="text-neutral-600 leading-relaxed">
          Prefix function &pi; runs in <span class="font-mono font-semibold">&le; 2m = O(m)</span>.<br>
          In matching, text index <code>i</code> increments <code>n</code> times; <code>q</code> increases at most <code>n</code> times, so fallback <code>while</code> executes &le; <code>n</code> times total.<br>
          Total Time = <span class="font-mono font-semibold text-neutral-900">O(m) + O(n) = O(n + m)</span>
        </p>
      </div>`;
    rightHtml = `
      <div class="p-3.5 rounded-md bg-blue-50/60 border border-blue-200 space-y-1.5">
        <div class="font-semibold text-blue-900">2. Applied to Your Input (n = ${n}, m = ${m})</div>
        <p class="text-blue-950 font-mono leading-relaxed">
          Worst-Case Comparison Bound = 2(n + m) = ${bound}<br>
          Actual Matching Comparisons = <b>${data.comparisons}</b><br>
          Smart &pi;-Fallbacks Used = <b>${data.shifts_using_pi}</b><br>
          Space Used = O(m) = ${m} integers for &pi;[]
        </p>
      </div>`;
  }

  elDerivationContainer.innerHTML = leftHtml + rightHtml;
}

// ============================================================================
// INTERACTIVE TRACKS, LOGS & TABLES
// ============================================================================
function setupActiveVisualizerSteps() {
  const data = state.apiData[state.currentAlgo];
  state.activeSteps = (data && data.steps) ? data.steps : [];
  state.currentStepIndex = 0;

  elPanelKmp.classList.toggle("hidden", state.currentAlgo !== "kmp");
  elPanelFa.classList.toggle("hidden", state.currentAlgo !== "finite_automata");
  elPanelRk.classList.toggle("hidden", state.currentAlgo !== "rabin_karp");
  elPanelNaive.classList.toggle("hidden", state.currentAlgo !== "naive");

  buildTextTrack();
}

function buildTextTrack() {
  const text = state.text;
  elTrackText.innerHTML = "";
  for (let i = 0; i < text.length; i++) {
    const cell = document.createElement("div");
    cell.id = `text-cell-${i}`;
    cell.className = "char-cell char-default";
    cell.innerHTML = `<span class="index-tag">${i}</span><span class="char-val">${escapeHtml(text[i])}</span>`;
    elTrackText.appendChild(cell);
  }
}

function renderPatternShift(shift, activePatternIndex = -1, status = "default") {
  const pattern = state.pattern;
  elPatternShiftLabel.textContent = `Shift s = ${shift}`;
  elTrackPattern.innerHTML = "";

  for (let s = 0; s < shift; s++) {
    const spacer = document.createElement("div");
    spacer.className = "char-spacer";
    elTrackPattern.appendChild(spacer);
  }

  for (let j = 0; j < pattern.length; j++) {
    const cell = document.createElement("div");
    let cls = "char-cell char-default";
    if (status === "found") {
      cls = "char-cell char-occurrence";
    } else if (j === activePatternIndex) {
      if (status === "matched") cls = "char-cell char-matched";
      else if (status === "mismatch") cls = "char-cell char-mismatch";
      else cls = "char-cell char-checking";
    }
    cell.className = cls;
    cell.innerHTML = `<span class="index-tag">${j}</span><span class="char-val">${escapeHtml(pattern[j])}</span>`;
    elTrackPattern.appendChild(cell);
  }
}

function renderHighlightedBaseString() {
  const data = state.apiData[state.currentAlgo];
  if (!data || !elHighlightedBase) return;

  const text = state.text;
  const m = state.pattern.length;
  const matches = data.matches || [];

  elFinalMatchBadge.textContent = matches.length === 1 ? "1 match found" : `${matches.length} matches found`;
  elFinalMatchBadge.className = matches.length > 0
    ? "px-2.5 py-1 rounded-md text-xs font-mono font-semibold bg-emerald-50 text-emerald-700 border border-emerald-200"
    : "px-2.5 py-1 rounded-md text-xs font-mono font-semibold bg-neutral-100 text-neutral-600 border border-neutral-200";

  if (matches.length === 0) {
    elHighlightedBase.innerHTML = `<span class="text-neutral-500">${escapeHtml(text)}</span>`;
    elMatchChips.innerHTML = `<span class="text-neutral-500">Pattern "${escapeHtml(state.pattern)}" does not appear in the base string.</span>`;
    return;
  }

  // Mark every character index that belongs to at least one match (supports overlapping matches)
  const isMatchedChar = new Array(text.length).fill(false);
  const matchStartAt = new Map();
  matches.forEach((pos, idx) => {
    matchStartAt.set(pos, idx);
    for (let k = 0; k < m && pos + k < text.length; k++) {
      isMatchedChar[pos + k] = true;
    }
  });

  // Build inline highlighted base string
  let html = "";
  let i = 0;
  while (i < text.length) {
    if (isMatchedChar[i]) {
      let start = i;
      while (i < text.length && isMatchedChar[i]) i++;
      const segment = text.slice(start, i);
      const jumpShift = matches.find((pos) => pos >= start && pos < i) ?? start;
      html += `<span class="match-highlight" data-shift="${jumpShift}" title="Matched at index ${start}..${i - 1}">${escapeHtml(segment)}</span>`;
    } else {
      let start = i;
      while (i < text.length && !isMatchedChar[i]) i++;
      html += `<span class="text-neutral-600">${escapeHtml(text.slice(start, i))}</span>`;
    }
  }
  elHighlightedBase.innerHTML = html;

  elHighlightedBase.querySelectorAll(".match-highlight").forEach((span) => {
    span.addEventListener("click", () => {
      const shiftVal = parseInt(span.dataset.shift, 10);
      jumpToMatchShift(shiftVal);
    });
  });

  // Build clickable chips for each exact match position
  elMatchChips.innerHTML = `<span class="text-neutral-500 font-medium">Jump to match:</span>` + matches.map((pos, idx) => {
    const endPos = pos + m - 1;
    return `<button data-shift="${pos}" class="match-chip px-2.5 py-1 rounded-md bg-emerald-50 hover:bg-emerald-100 text-emerald-800 border border-emerald-200 font-mono transition cursor-pointer">
      #${idx + 1}: [${pos}..${endPos}] (1-based: ${pos + 1})
    </button>`;
  }).join("");

  elMatchChips.querySelectorAll(".match-chip").forEach((btn) => {
    btn.addEventListener("click", () => {
      jumpToMatchShift(parseInt(btn.dataset.shift, 10));
    });
  });
}

function jumpToMatchShift(shiftVal) {
  pause();
  const idx = state.activeSteps.findIndex((s) => s.state === "found" && s.shift === shiftVal);
  if (idx !== -1) {
    goToStep(idx);
  }
}

function renderStepsLog() {
  const steps = state.activeSteps;
  const visibleSteps = state.filterMatchesOnly
    ? steps.map((s, idx) => ({ s, idx })).filter((item) => item.s.state === "found")
    : steps.map((s, idx) => ({ s, idx }));

  elTotalStepsLabel.textContent = `${visibleSteps.length} / ${steps.length} steps`;
  elStepsLogList.innerHTML = "";

  if (visibleSteps.length === 0) {
    elStepsLogList.innerHTML = `<div class="text-xs text-neutral-500 p-3">No ${state.filterMatchesOnly ? "matched " : ""}steps to display.</div>`;
    return;
  }

  visibleSteps.forEach(({ s: st, idx }) => {
    const item = document.createElement("div");
    item.id = `step-log-item-${idx}`;
    item.className = "step-log-item text-xs";

    const badgeMap = {
      found: "bg-emerald-100 text-emerald-800",
      matched_char: "bg-emerald-50 text-emerald-700",
      mismatch: "bg-red-100 text-red-800",
      pi_fallback: "bg-amber-100 text-amber-800",
      spurious_hit: "bg-amber-100 text-amber-800",
      hash_check: "bg-blue-50 text-blue-700",
      rolling_hash: "bg-neutral-100 text-neutral-700",
      transition: "bg-blue-50 text-blue-700",
      checking: "bg-blue-50 text-blue-700",
      init: "bg-neutral-100 text-neutral-700"
    };
    const badgeCls = badgeMap[st.state] || "bg-neutral-100 text-neutral-700";

    item.innerHTML = `
      <div class="flex items-center justify-between gap-2 mb-0.5">
        <span class="font-mono font-semibold text-neutral-700">#${idx + 1}</span>
        <span class="px-1.5 py-0.5 rounded text-[10px] font-mono font-semibold uppercase ${badgeCls}">${st.state}</span>
      </div>
      <div class="text-[11px] font-mono text-neutral-600 leading-snug">${escapeHtml(st.explanation || "")}</div>
    `;
    item.addEventListener("click", () => {
      pause();
      goToStep(idx);
    });
    elStepsLogList.appendChild(item);
  });

  const activeLog = document.getElementById(`step-log-item-${state.currentStepIndex}`);
  if (activeLog) activeLog.classList.add("step-active");
}

function renderKmpPiTable() {
  const kmp = state.apiData.kmp;
  const wrapper = document.getElementById("kmpPiTableWrapper");
  if (!kmp || !kmp.pi_table || !wrapper) return;

  const pi = kmp.pi_table;
  const pattern = state.pattern;

  let html = `<table class="interactive-table w-full"><thead><tr><th class="w-16">i</th>`;
  for (let i = 0; i < pi.length; i++) html += `<th>${i}</th>`;
  html += `</tr><tr><th>P[i]</th>`;
  for (let i = 0; i < pi.length; i++) html += `<th>'${escapeHtml(pattern[i] || "")}'</th>`;
  html += `</tr></thead><tbody><tr><td class="font-semibold">&pi;[i]</td>`;
  for (let i = 0; i < pi.length; i++) html += `<td id="pi-cell-${i}">${pi[i]}</td>`;
  html += `</tr></tbody></table>`;
  wrapper.innerHTML = html;
}

function renderFaTable() {
  const fa = state.apiData.finite_automata;
  const wrapper = document.getElementById("faTransitionTableWrapper");
  if (!fa || !fa.transition_table || !wrapper) return;

  const table = fa.transition_table;
  const alphabet = fa.alphabet || [];
  const m = state.pattern.length;

  document.getElementById("faStatesCount").textContent = table.length;
  document.getElementById("faAcceptState").textContent = m;

  let html = `<table class="interactive-table w-full"><thead><tr><th>q</th>`;
  alphabet.forEach((a) => { html += `<th>'${escapeHtml(a)}'</th>`; });
  html += `</tr></thead><tbody>`;

  table.forEach((row, q) => {
    const isAcc = q === m;
    html += `<tr><td class="font-semibold ${isAcc ? "bg-emerald-50 text-emerald-800" : ""}">${q}${isAcc ? " *" : ""}</td>`;
    alphabet.forEach((a) => {
      const nextQ = row[a] ?? 0;
      html += `<td id="fa-cell-${q}-${a.charCodeAt(0)}">${nextQ}</td>`;
    });
    html += `</tr>`;
  });
  html += `</tbody></table>`;
  wrapper.innerHTML = html;
}

function renderRkInfo() {
  const rk = state.apiData.rabin_karp;
  if (!rk) return;
  document.getElementById("rkPatternHashVal").textContent = rk.pattern_hash ?? "--";
  document.getElementById("rkSpuriousHitsVal").textContent = rk.spurious_hits ?? 0;
}

// ============================================================================
// STEP NAVIGATION & ANIMATION
// ============================================================================
function applyStep(stepIndex) {
  const steps = state.activeSteps;
  if (!steps || steps.length === 0) {
    elStepCounter.textContent = "Step 0 / 0";
    elProgressBar.style.width = "0%";
    renderFlowPipeline(null);
    return;
  }

  const step = steps[stepIndex];
  const total = steps.length;

  elStepCounter.textContent = `Step ${stepIndex + 1} / ${total}`;
  elProgressBar.style.width = `${Math.round(((stepIndex + 1) / total) * 100)}%`;
  elExplanation.textContent = step.explanation || "";

  renderFlowPipeline(step);

  // Highlight active item in Steps Log
  document.querySelectorAll(".step-log-item.step-active").forEach((el) => el.classList.remove("step-active"));
  const activeLog = document.getElementById(`step-log-item-${stepIndex}`);
  if (activeLog) {
    activeLog.classList.add("step-active");
    activeLog.scrollIntoView({ block: "nearest", behavior: "smooth" });
  }

  // Reset Text Cells; if on the final step, also keep all matched occurrences highlighted
  const isFinalStep = stepIndex === total - 1;
  const matchedSet = new Set();
  if (isFinalStep && state.apiData && state.apiData[state.currentAlgo]) {
    const matches = state.apiData[state.currentAlgo].matches || [];
    matches.forEach((pos) => {
      for (let k = 0; k < state.pattern.length; k++) matchedSet.add(pos + k);
    });
  }
  for (let i = 0; i < state.text.length; i++) {
    const cell = document.getElementById(`text-cell-${i}`);
    if (cell) cell.className = matchedSet.has(i) ? "char-cell char-occurrence" : "char-cell char-default";
  }

  let shift = step.shift;
  if (shift === undefined || shift === null) {
    if (step.text_index !== undefined && step.pattern_index !== undefined && step.text_index >= 0) {
      shift = Math.max(0, step.text_index - step.pattern_index);
    } else {
      shift = 0;
    }
  }

  const textIdx = step.text_index;
  const patIdx = step.pattern_index !== undefined ? step.pattern_index : -1;
  const st = step.state;

  if (textIdx !== undefined && textIdx >= 0 && textIdx < state.text.length) {
    const cell = document.getElementById(`text-cell-${textIdx}`);
    if (cell) {
      if (st === "found") cell.className = "char-cell char-occurrence";
      else if (st === "matched_char" || step.is_char_match === true) cell.className = "char-cell char-matched";
      else if (st === "mismatch" || st === "pi_fallback" || st === "spurious_hit" || step.is_char_match === false) cell.className = "char-cell char-mismatch";
      else cell.className = "char-cell char-checking";
    }
  }

  if (st === "found") {
    for (let k = 0; k < state.pattern.length; k++) {
      const c = document.getElementById(`text-cell-${shift + k}`);
      if (c) c.className = "char-cell char-occurrence";
    }
    renderPatternShift(shift, -1, "found");
  } else {
    let charStatus = "checking";
    if (step.is_char_match === true || st === "matched_char") charStatus = "matched";
    else if (step.is_char_match === false || st === "mismatch" || st === "pi_fallback" || st === "spurious_hit") charStatus = "mismatch";
    renderPatternShift(shift, patIdx, charStatus);
  }

  // Algorithm-specific cell highlights
  document.querySelectorAll(".interactive-table td.active-cell").forEach((c) => c.classList.remove("active-cell"));
  if (state.currentAlgo === "kmp" && patIdx >= 0) {
    const piCell = document.getElementById(`pi-cell-${patIdx}`);
    if (piCell) piCell.classList.add("active-cell");
  } else if (state.currentAlgo === "finite_automata" && step.prev_state !== undefined && step.char_text) {
    const faCell = document.getElementById(`fa-cell-${step.prev_state}-${step.char_text.charCodeAt(0)}`);
    if (faCell) faCell.classList.add("active-cell");
  } else if (state.currentAlgo === "rabin_karp" && step.window_hash !== undefined) {
    document.getElementById("rkWindowHashVal").textContent = step.window_hash;
  }
}

function stepForward() {
  if (state.currentStepIndex < state.activeSteps.length - 1) {
    state.currentStepIndex++;
    applyStep(state.currentStepIndex);
  } else {
    pause();
  }
}

function stepBackward() {
  if (state.currentStepIndex > 0) {
    state.currentStepIndex--;
    applyStep(state.currentStepIndex);
  }
}

function goToStep(index) {
  if (state.activeSteps.length === 0) {
    applyStep(0);
    return;
  }
  if (index >= 0 && index < state.activeSteps.length) {
    state.currentStepIndex = index;
    applyStep(index);
  }
}

function play() {
  if (state.activeSteps.length === 0) return;
  if (state.currentStepIndex >= state.activeSteps.length - 1) {
    state.currentStepIndex = 0;
  }
  state.isPlaying = true;
  elPlayText.textContent = "Pause";
  state.timerId = setInterval(() => {
    if (state.currentStepIndex < state.activeSteps.length - 1) {
      stepForward();
    } else {
      pause();
    }
  }, state.speedMs);
}

function pause() {
  state.isPlaying = false;
  if (state.timerId) {
    clearInterval(state.timerId);
    state.timerId = null;
  }
  elPlayText.textContent = "Play";
}

function togglePlayPause() {
  if (state.isPlaying) pause();
  else play();
}

function escapeHtml(str) {
  return String(str)
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;");
}
