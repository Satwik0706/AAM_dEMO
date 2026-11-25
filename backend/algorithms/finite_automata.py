import time

def compute_transition_table(pattern: str, alphabet: list):
    """
    Computes the transition function delta(state, char) for the Finite Automata matcher.
    delta(q, a) = max { k | P_k is a suffix of P_q + a }
    """
    m = len(pattern)
    # delta[state][char] = next_state
    transition_table = []
    
    for q in range(m + 1):
        row = {}
        for a in alphabet:
            # k = min(m + 1, q + 2)
            k = min(m, q + 1)
            pq_a = pattern[:q] + a
            while k > 0 and not pq_a.endswith(pattern[:k]):
                k -= 1
            row[a] = k
        transition_table.append(row)
        
    return transition_table

def run_finite_automata(text: str, pattern: str):
    """
    Finite Automata String Matching Algorithm
    Preprocessing Time: O(m * |Sigma|)
    Matching Time: O(n)
    Space Complexity: O(m * |Sigma|)
    """
    start_time = time.perf_counter()
    n = len(text)
    m = len(pattern)
    
    matches = []
    steps = []
    state_transitions = 0
    
    if m == 0 or n == 0 or m > n:
        return {
            "algorithm": "Finite Automata String Matcher",
            "matches": matches,
            "total_matches": len(matches),
            "state_transitions": state_transitions,
            "alphabet": [],
            "transition_table": [],
            "execution_time_us": round((time.perf_counter() - start_time) * 1e6, 2),
            "steps": steps,
            "time_complexity": "Preprocessing O(m^3 * |Sigma|) / O(m * |Sigma|), Matching O(n)",
            "space_complexity": "O(m * |Sigma|)"
        }

    # Build unique alphabet
    alphabet = sorted(list(set(text + pattern)))
    transition_table = compute_transition_table(pattern, alphabet)
    
    steps.append({
        "step_number": len(steps) + 1,
        "text_index": -1,
        "char_text": "",
        "prev_state": 0,
        "curr_state": 0,
        "is_accepting": False,
        "state": "init",
        "explanation": f"Automaton initialized with {m + 1} states (0 to {m}) and alphabet size |Sigma|={len(alphabet)}. Accepting State is State {m}."
    })

    q = 0 # Initial state
    for i in range(n):
        char = text[i]
        prev_q = q
        q = transition_table[prev_q].get(char, 0)
        state_transitions += 1
        
        is_accepting = (q == m)
        if is_accepting:
            shift = i - m + 1
            matches.append(shift)
            steps.append({
                "step_number": len(steps) + 1,
                "text_index": i,
                "char_text": char,
                "prev_state": prev_q,
                "curr_state": q,
                "is_accepting": True,
                "shift": shift,
                "state": "found",
                "explanation": f"Index {i}: Read '{char}'. delta({prev_q}, '{char}') -> State {q} [ACCEPTING STATE]! Match found at shift {shift} (1-based: {shift + 1})."
            })
        else:
            steps.append({
                "step_number": len(steps) + 1,
                "text_index": i,
                "char_text": char,
                "prev_state": prev_q,
                "curr_state": q,
                "is_accepting": False,
                "shift": None,
                "state": "transition",
                "explanation": f"Index {i}: Read '{char}'. delta({prev_q}, '{char}') -> Transitioned to State {q} (longest prefix matched is '{pattern[:q]}')."
            })

    execution_time_us = round((time.perf_counter() - start_time) * 1e6, 2)
    return {
        "algorithm": "Finite Automata String Matcher",
        "matches": matches,
        "total_matches": len(matches),
        "state_transitions": state_transitions,
        "alphabet": alphabet,
        "transition_table": transition_table,
        "execution_time_us": execution_time_us,
        "steps": steps,
        "time_complexity": "Preprocessing O(m * |Sigma|), Matching O(n)",
        "space_complexity": "O(m * |Sigma|)"
    }
